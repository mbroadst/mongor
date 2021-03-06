#include "codec.h"

#include <cstdint>
#include <list>
#include <memory>
#include <sstream>
#include <string>

#include "envoy/buffer/buffer.h"
#include "envoy/common/exception.h"

#include "common/common/assert.h"
#include "common/common/fmt.h"

#include "extensions/filters/network/mongo_proxy/bson_impl.h"

namespace Envoy {
namespace Extensions {
namespace NetworkFilters {
namespace MongoDB {

Message* Message::fromBuffer(Buffer::Instance& data, uint32_t message_length,
                             int32_t request_id, uint32_t response_to) {
  ENVOY_LOG(trace, "reading OP_MSG message");
  const uint64_t original_data_length = data.length();
  ASSERT(message_length <= original_data_length);

  auto message = new Message(request_id, response_to);
  /* int32_t flags_ = */ Bson::BufferHelper::removeInt32(data);
  // TODO: payload type 1
  /* uint8_t payload_type = */ Bson::BufferHelper::removeByte(data);
  // TODO: this is fully copying and parsing BSON as soon as we receive it, we can do better with views
  message->body_ = Bson::DocumentImpl::create(data);

  ENVOY_LOG(trace, "{}", message->toString());
  return message;
}

Message* Message::fromQueryBuffer(Buffer::Instance& data, uint32_t message_length,
                                  int32_t request_id, uint32_t response_to) {
  ENVOY_LOG(trace, "reading OP_QUERY message");
  uint64_t original_data_length = data.length();
  ASSERT(message_length <= original_data_length);

  auto message = new Message(request_id, response_to);
  /* int32_t flags_ = */ Bson::BufferHelper::removeInt32(data);
  /* std::string full_collection_name_ = */ Bson::BufferHelper::removeCString(data);
  /* int32_t number_to_skip_ = */ Bson::BufferHelper::removeInt32(data);
  /* int32_t number_to_return_ = */ Bson::BufferHelper::removeInt32(data);
  message->body_ = Bson::DocumentImpl::create(data);

  // if (data.length() - (original_data_length - message_length) > 0) {
  //   Bson::DocumentSharedPtr return_fields_selector_ = Bson::DocumentImpl::create(data);
  // }

  ENVOY_LOG(trace, "{}", message->toString());
  return message;
}

Message* Message::fromError(Bson::DocumentSharedPtr&& error) {
  auto message = new Message(0, 0);
  message->body_ = error;
  return message;
}

Message* Message::reply(Bson::DocumentSharedPtr&& body) {
  // TODO: what should request_id be here? maybe some counter
  auto message = new Message(1001, request_id_);
  message->body_ = body;
  return message;
}

std::string Message::toString() const {
  return fmt::format(
      R"EOF({{"opcode": "OP_MSG", "id": {}, "response_to": {}, "body": {}}})EOF",
      request_id_, response_to_, body_->toString());
}

bool Message::operator==(const Message& rhs) const {
  if (!(requestId() == rhs.requestId() && responseTo() == rhs.responseTo() && !body() == !rhs.body())) {
    return false;
  }

  if (body()) {
    if (!(*body() == *rhs.body())) {
      return false;
    }
  }

  return true;
}

void Decoder::decode(Buffer::Instance& data) {
  while (data.length() > 0) {
    // See if we have enough data for the message length.
    ENVOY_LOG(trace, "decoding {} bytes", data.length());
    if (data.length() < sizeof(int32_t)) {
      return;
    }

    uint32_t message_length = Bson::BufferHelper::peekInt32(data);
    ENVOY_LOG(trace, "message is {} bytes", message_length);
    if (data.length() < message_length) {
      return;
    }

    data.drain(sizeof(int32_t));
    int32_t request_id = Bson::BufferHelper::removeInt32(data);
    int32_t response_to = Bson::BufferHelper::removeInt32(data);
    Message::OpCode op_code = static_cast<Message::OpCode>(Bson::BufferHelper::removeInt32(data));
    ENVOY_LOG(trace, "message op: {}", static_cast<int32_t>(op_code));

    // Some messages need to know how long they are to parse. Subtract the header that we have already
    // parsed off before passing the final value.
    message_length -= Message::MessageHeaderSize;

    switch (op_code) {
    case Message::OpCode::Query: {
      MessagePtr message(
        Message::fromQueryBuffer(data, message_length, request_id, response_to));
      callbacks_.onHandshake(std::move(message));
      break;
    }

    case Message::OpCode::Msg: {
      MessagePtr message(
        Message::fromBuffer(data, message_length, request_id, response_to));
      callbacks_.onMessage(std::move(message));
      break;
    }

    default:
      throw EnvoyException(fmt::format("invalid mongo op {}", static_cast<int32_t>(op_code)));
    }

    ENVOY_LOG(trace, "{} bytes remaining after decoding", data.length());
  }
}

void Encoder::encode(const Message& message, Buffer::Instance& out) {
  // https://docs.mongodb.com/manual/reference/mongodb-wire-protocol/#op-msg
  int32_t total_size =
    Message::MessageHeaderSize + Message::Int32Length + 1 + message.body()->byteSize();

  Bson::BufferHelper::writeInt32(out, total_size);
  Bson::BufferHelper::writeInt32(out, message.requestId());
  Bson::BufferHelper::writeInt32(out, message.responseTo());
  Bson::BufferHelper::writeInt32(out, 2013);
  Bson::BufferHelper::writeInt32(out, message.flags());

  // TODO: support payload 1
  out.add(&Message::Payload0Byte, sizeof(uint8_t));
  message.body()->encode(out);
}


} // namespace MongoDB
} // namespace NetworkFilters
} // namespace Extensions
} // namespace Envoy
