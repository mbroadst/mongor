# build notes

- download the clang binary distro for ubuntu 18.04 [here](https://github.com/llvm/llvm-project/releases/download/llvmorg-10.0.0/clang+llvm-10.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz)
-

```
$ git submodule update --init
$ source envoy/bazel/setup_clang.sh /path/to/clang/binary/distro
$ bazel build //:envoy --config=clang --check_visibility=false
```
- need `check_visibility=false` so we can reuse the internal BSON implementation (consider bundling our own more performant impl in the future)

- testing with an actual client:
```
$ ./bazel-bin/envoy -c test/mongor_server.yaml -l trace
```

