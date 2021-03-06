#include <string>

#include "src/codec.h"

#include "common/buffer/buffer_impl.h"
#include "common/json/json_loader.h"

#include "extensions/filters/network/mongo_proxy/bson_impl.h"

#include "test/test_common/printers.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::Eq;
using testing::NiceMock;
using testing::Pointee;

namespace Envoy {
namespace Extensions {
namespace NetworkFilters {

using namespace MongoProxy;
namespace MongoDB {

class TestDecoderCallbacks : public DecoderCallbacks {
public:
  void onHandshake(MessagePtr&& message) override { handleHandshake_(message); }
  void onMessage(MessagePtr&& message) override { handleMessage_(message); }

  MOCK_METHOD(void, handleHandshake_, (MessagePtr & message));
  MOCK_METHOD(void, handleMessage_, (MessagePtr & message));
};

class CodecTest : public testing::Test {
public:
  Buffer::OwnedImpl output_;
  NiceMock<TestDecoderCallbacks> callbacks_;
  Decoder decoder_{callbacks_};
};

TEST_F(CodecTest, MessageEqual) {
  {
    MessagePtr q1(Message::fromBuffer(0, 0, 0, output_));
    MessagePtr q2(Message::fromBuffer(0, 1, 1, output_));
    EXPECT_FALSE(q1.get() == q2.get());
  }
}

TEST_F(CodecTest, Handshake) {
  uint8_t hello_msg_header[] = { // using OP_QUERY
    0x3a, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd4, 0x07, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0x64, 0x6d, 0x69, 0x6e, 0x2e, 0x24, 0x63, 0x6d, 0x64,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
  };

  uint8_t hello_msg_body[] = {
    0x13, 0x00, 0x00, 0x00,
    0x10, 0x69, 0x73, 0x6d, 0x61, 0x73, 0x74, 0x65, 0x72, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00
  };

  Buffer::OwnedImpl buffer;
  buffer.add(hello_msg_header, 39);
  buffer.add(hello_msg_body, 19);
  decoder_.decode(buffer);
}

} // namespace MongoDB
} // namespace NetworkFilters
} // namespace Extensions
} // namespace Envoy
