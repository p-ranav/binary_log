# Building with CMake

Enabling developer mode will build the benchmarks, tests, and examples.

```sh
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release -D BINARY_LOG_DEVELOPER_MODE=ON
cmake --build build
```

Here is the command for installing the release mode artifacts:

```sh
cmake --install build
```

# Running the Tests

Once built, you can run the tests like so:

```console
foo@bar:~/dev/binary_log$ ./build/test/binary_log_test
[doctest] doctest version is "2.3.5"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:     10 |     10 passed |      0 failed |      0 skipped
[doctest] assertions:    522 |    522 passed |      0 failed |
[doctest] Status: SUCCESS!
```
