#pragma once

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "extensions/filters/network/mongo_proxy/bson.h"

namespace Envoy {
namespace Extensions {
namespace NetworkFilters {

using namespace MongoProxy;
namespace MongoDB {

class Message : Logger::Loggable<Logger::Id::filter> {
public:
  enum class OpCode {
    Reply = 1,
    DeprecatedMsg = 1000,
    Update = 2001,
    Insert = 2002,
    Query = 2004,
    GetMore = 2005,
    Delete = 2006,
    KillCursors = 2007,
    Command = 2010,
    CommandReply = 2011,
    Msg = 2013
  };

  static Message* fromBuffer(Buffer::Instance& data, uint32_t message_length,
                             int32_t request_id, uint32_t response_to);
  static Message* fromQueryBuffer(Buffer::Instance& data, uint32_t message_length,
                                  int32_t request_id, uint32_t response_to);
  static Message* fromError(Bson::DocumentSharedPtr&& error);
  ~Message() = default;

  bool operator==(const Message& rhs) const;
  std::string toString() const;

  Message* reply(Bson::DocumentSharedPtr&& body);

  int32_t requestId() const { return request_id_; }
  int32_t responseTo() const { return response_to_; }
  int32_t flags() const { return flags_; }
  const Bson::Document* body() const { return body_.get(); }

  // Define some constants used in mongo messages encoding
  constexpr static uint32_t MessageHeaderSize = 16;
  constexpr static uint32_t Int32Length = 4;
  constexpr static uint32_t Int64Length = 8;
  constexpr static uint32_t StringPaddingLength = 1;
  constexpr static uint8_t Payload0Byte = 0;

private:
  Message(int32_t request_id, uint32_t response_to)
      : request_id_(request_id), response_to_(response_to) {}

  // struct DocumentSequence {
  //     std::string name;
  //     std::vector<Bson::DocumentSharedPtr> documents;
  // };

  const int32_t request_id_;
  const int32_t response_to_;
  int32_t flags_{};
  Bson::DocumentSharedPtr body_;
  // std::vector<DocumentSequence> sequences_;
};

using MessagePtr = std::unique_ptr<Message>;

class DecoderCallbacks {
public:
  virtual ~DecoderCallbacks() = default;
  virtual void onHandshake(MessagePtr&& message) PURE;
  virtual void onMessage(MessagePtr&& message) PURE;
};

class Decoder : Logger::Loggable<Logger::Id::filter> {
public:
  Decoder(DecoderCallbacks& callbacks) : callbacks_(callbacks) {}
  ~Decoder() = default;

  void decode(Buffer::Instance& data);

private:
  DecoderCallbacks& callbacks_;

};

using DecoderPtr = std::unique_ptr<Decoder>;

class Encoder : Logger::Loggable<Logger::Id::filter> {
public:
  void encode(const Message& message, Buffer::Instance& out);
};

using EncoderPtr = std::unique_ptr<Encoder>;

} // namespace MongoDB
} // namespace NetworkFilters
} // namespace Extensions
} // namespace Envoy
