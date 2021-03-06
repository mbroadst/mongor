#include "test/integration/integration.h"
#include "test/integration/utility.h"

namespace Envoy {
class MongoDBIntegrationTest : public BaseIntegrationTest,
                               public testing::TestWithParam<Network::Address::IpVersion> {

  std::string mongodbConfig() {
    return TestEnvironment::readFileToStringForTest(
        TestEnvironment::runfilesPath("test/mongor_server.yml", "mongor"));
  }

public:
  MongoDBIntegrationTest() : BaseIntegrationTest(GetParam(), mongodbConfig()) {}

  /**
   * Initializer for an individual integration test.
   */
  void SetUp() override { BaseIntegrationTest::initialize(); }

  /**
   * Destructor for an individual integration test.
   */
  void TearDown() override {
    test_server_.reset();
    fake_upstreams_.clear();
  }
};

INSTANTIATE_TEST_SUITE_P(IpVersions, MongoDBIntegrationTest,
                        testing::ValuesIn(TestEnvironment::getIpVersionsForTest()));

TEST_P(MongoDBIntegrationTest, Mongor) {
  std::string response;
  auto connection = createConnectionDriver(
      lookupPort("listener_0"), "hello",
      [&](Network::ClientConnection& conn, const Buffer::Instance& data) -> void {
        response.append(data.toString());
        conn.close(Network::ConnectionCloseType::FlushWrite);
      });

  connection->run();
  EXPECT_EQ("hello", response);
}
} // namespace Envoy
