package(default_visibility = ["//visibility:public"])

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_binary",
    "envoy_cc_library",
    "envoy_cc_test",
)

envoy_cc_binary(
    name = "envoy",
    repository = "@envoy",
    deps = [
        ":mongor_config",
        "@envoy//source/exe:envoy_main_entry_lib",
    ],
)

envoy_cc_library(
    name = "codec_lib",
    srcs = ["src/codec.cc"],
    hdrs = ["src/codec.h"],
    repository = "@envoy",
    deps = [
        "@envoy//source/extensions/filters/network/mongo_proxy:bson_interface",
        "@envoy//source/extensions/filters/network/mongo_proxy:bson_lib",
        "@envoy//include/envoy/buffer:buffer_interface",
        "@envoy//source/common/common:assert_lib",
        "@envoy//source/common/common:minimal_logger_lib",
    ],
)

envoy_cc_library(
    name = "mongor_lib",
    srcs = ["src/proxy_filter.cc"],
    hdrs = ["src/proxy_filter.h"],
    repository = "@envoy",
    deps = [
        ":codec_lib",
        "@envoy//include/envoy/buffer:buffer_interface",
        "@envoy//include/envoy/network:connection_interface",
        "@envoy//include/envoy/network:filter_interface",
        "@envoy//include/envoy/upstream:cluster_manager_interface",
        "@envoy//source/common/common:assert_lib",
        "@envoy//source/common/common:logger_lib",
        "@envoy//source/common/config:utility_lib"
    ],
)

envoy_cc_library(
    name = "mongor_config",
    srcs = ["src/config.cc"],
    repository = "@envoy",
    deps = [
        ":mongor_lib",
        "@envoy//include/envoy/network:filter_interface",
        "@envoy//include/envoy/registry:registry",
        "@envoy//include/envoy/server:filter_config_interface",
    ],
)

envoy_cc_test(
    name = "mongor_integration_test",
    srcs = ["test/mongor_integration_test.cc"],
    data =  ["test/mongor_server.yml"],
    repository = "@envoy",
    deps = [
        ":mongor_config",
        "@envoy//test/integration:integration_lib"
    ],
)

envoy_cc_test(
    name = "codec_test",
    srcs = ["test/codec_test.cc"],
    repository = "@envoy",
    deps = [
        ":codec_lib",
        "@envoy//source/common/buffer:buffer_lib",
        "@envoy//source/common/json:json_loader_lib",
    ],
)
