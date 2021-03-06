#include "proxy_filter.h"

#include "envoy/buffer/buffer.h"
#include "envoy/network/connection.h"

#include "common/common/assert.h"

#include "extensions/filters/network/mongo_proxy/bson_impl.h"


namespace Envoy {
namespace Extensions {
namespace NetworkFilters {
namespace MongoDB {

ProxyFilter::ProxyFilter()
  : decoder_(new Decoder(*this)) {}

Network::FilterStatus ProxyFilter::onData(Buffer::Instance& data, bool) {
  // ENVOY_CONN_LOG(trace, "mongor: got {} bytes", read_callbacks_->connection(), data.length());
  read_buffer_.add(data);

  try {
    decoder_->decode(read_buffer_);
  } catch (EnvoyException& e) {
    ENVOY_LOG(info, "mongo decoding error: {}", e.what());

    Bson::DocumentSharedPtr error_doc = Bson::DocumentImpl::create()
      ->addInt32("ok", 0)
      ->addString("errmsg", e.what());

    MessagePtr response(Message::fromError(std::move(error_doc)));
    encoder_->encode(*response.get(), encoder_buffer_);
    read_callbacks_->connection().write(encoder_buffer_, false);
    read_callbacks_->connection().close(Network::ConnectionCloseType::NoFlush);
}

  // we fully own incoming requests, don't forward them to another filter
  return Network::FilterStatus::StopIteration;
}

Network::FilterStatus ProxyFilter::onNewConnection() {
  return Network::FilterStatus::Continue;

}
void ProxyFilter::initializeReadFilterCallbacks(Network::ReadFilterCallbacks& callbacks) {
  read_callbacks_ = &callbacks;

  // if we want to do connection tracing:
  // callbacks_->connection().addConnectionCallbacks(*this);
  // callbacks_->connection().setConnectionStats({config_->stats_.downstream_cx_rx_bytes_total_,
  //                                              config_->stats_.downstream_cx_rx_bytes_buffered_,
  //                                              config_->stats_.downstream_cx_tx_bytes_total_,
  //                                              config_->stats_.downstream_cx_tx_bytes_buffered_,
  //                                              nullptr, nullptr});
}

void ProxyFilter::onEvent(Network::ConnectionEvent event) {
  if (event == Network::ConnectionEvent::RemoteClose ||
      event == Network::ConnectionEvent::LocalClose) {
    // while (!pending_requests_.empty()) {
    //   if (pending_requests_.front().request_handle_ != nullptr) {
    //     pending_requests_.front().request_handle_->cancel();
    //   }
    //   pending_requests_.pop_front();
    // }
  }
}

void ProxyFilter::onHandshake(MessagePtr&& message) {
  Bson::DocumentSharedPtr hello_doc = Bson::DocumentImpl::create()
    ->addBoolean("ismaster", true)
    ->addString("msg", "isdbgrid")
    ->addInt32("maxBsonObjectSize", 16777216)
    ->addInt32("maxMessageSizeBytes", 48000000)
    ->addInt32("maxWriteBatchSize", 100000)
    // ->addInt32("logicalSessionTimeoutMinutes", 30) // TODO: sessions support
    ->addInt32("maxWireVersion", 8)
    ->addInt32("minWireVersion", 0)
    ->addInt32("ok", 1);

  MessagePtr response(message->reply(std::move(hello_doc)));
  encoder_->encode(*response.get(), encoder_buffer_);
  read_callbacks_->connection().write(encoder_buffer_, false);
}

void ProxyFilter::onMessage(MessagePtr&& message) {
  // pending_requests_.emplace_back(*this);
  // PendingRequest& request = pending_requests_.back();

  auto hello = message->body()->find("ismaster");
  if (hello) {
    ENVOY_LOG(trace, "returning hello in OP_MSG");
    Bson::DocumentSharedPtr hello_doc = Bson::DocumentImpl::create()
      ->addBoolean("ismaster", true)
      ->addString("msg", "isdbgrid")
      ->addInt32("maxBsonObjectSize", 16777216)
      ->addInt32("maxMessageSizeBytes", 48000000)
      ->addInt32("maxWriteBatchSize", 100000)
      // ->addInt32("logicalSessionTimeoutMinutes", 30) // TODO: sessions support
      ->addInt32("maxWireVersion", 8)
      ->addInt32("minWireVersion", 0)
      ->addInt32("ok", 1);

    MessagePtr response(message->reply(std::move(hello_doc)));
    encoder_->encode(*response.get(), encoder_buffer_);
    read_callbacks_->connection().write(encoder_buffer_, false);
    return;
  }

  Bson::DocumentSharedPtr error_doc = Bson::DocumentImpl::create()
    ->addInt32("ok", 0)
    ->addString("errmsg", "some fake error message");

  MessagePtr response(message->reply(std::move(error_doc)));
  encoder_->encode(*response.get(), encoder_buffer_);
  read_callbacks_->connection().write(encoder_buffer_, false);
}

} // namespace MongoDB
} // namespace NetworkFilters
} // namespace Extensions
} // namespace Envoy
