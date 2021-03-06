#pragma once

#include "codec.h"

#include "envoy/network/filter.h"
#include "envoy/upstream/cluster_manager.h"

#include "common/buffer/buffer_impl.h"
#include "common/common/logger.h"

namespace Envoy {
namespace Extensions {
namespace NetworkFilters {
namespace MongoDB {
class ProxyFilter : public Network::ReadFilter,
                    public DecoderCallbacks,
                    public Network::ConnectionCallbacks,
                    Logger::Loggable<Logger::Id::filter> {
public:
  ProxyFilter();

  // Network::ReadFilter
  Network::FilterStatus onData(Buffer::Instance& data, bool end_stream) override;
  Network::FilterStatus onNewConnection() override;
  void initializeReadFilterCallbacks(Network::ReadFilterCallbacks& callbacks) override;

  // Network::ConnectionCallbacks
  void onEvent(Network::ConnectionEvent event) override;
  void onAboveWriteBufferHighWatermark() override {}
  void onBelowWriteBufferLowWatermark() override {}

  // MongoDB::DecoderCallbacks
  void onHandshake(MessagePtr&& message) override;
  void onMessage(MessagePtr&& message) override;

private:
  // struct PendingRequest : public CommandSplitter::SplitCallbacks {
  //   PendingRequest(ProxyFilter& parent);
  //   ~PendingRequest() override;
  //
  //   ProxyFilter& parent_;
  // };

  Network::ReadFilterCallbacks* read_callbacks_{};
  DecoderPtr decoder_;
  EncoderPtr encoder_{};
  Buffer::OwnedImpl read_buffer_;
  Buffer::OwnedImpl encoder_buffer_;
  // std::list<PendingRequest> pending_requests_;
};

} // namespace MongoDB
} // namespace NetworkFilters
} // namespace Extensions
} // namespace Envoy
