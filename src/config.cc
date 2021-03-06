#include <string>

#include "proxy_filter.h"

#include "envoy/registry/registry.h"
#include "envoy/server/filter_config.h"

namespace Envoy {
namespace Extensions {
namespace NetworkFilters {
namespace MongoDB {

class ConfigFactory : public Server::Configuration::NamedNetworkFilterConfigFactory {
public:
  Network::FilterFactoryCb
  createFilterFactoryFromProto(const Protobuf::Message&,
                               Server::Configuration::FactoryContext&) override {
    return [](Network::FilterManager& filter_manager) -> void {
      filter_manager.addReadFilter(Network::ReadFilterSharedPtr{new ProxyFilter()});
    };
  }

  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return ProtobufTypes::MessagePtr{new Envoy::ProtobufWkt::Struct()};
  }

  std::string name() const override { return "mongor"; }

  bool isTerminalFilter() override { return true; }
};

static Registry::RegisterFactory<ConfigFactory,
                                 Server::Configuration::NamedNetworkFilterConfigFactory>
    registered_;

} // namespace MongoDB
} // namespace NetworkFilters
} // namespace Extensions
} // namespace Envoy

