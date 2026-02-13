# XGetOpt Tests

This directory contains a small, dependency-free test runner for XGetOpt.

## Build + run

From this directory:

- Build: `make`
- Run: `make run` (or `./xgetopt_tests`)
- Clean: `make clean`

## Adding tests

Add new test functions in `test_main.cpp` and register them in `main()`.

If the suite grows, it can be split into multiple `*.cpp` files and listed in the Makefile.

## Results

The following is a table of test results on various platforms. XGetOpt only guarantees functionality on GNU platforms, but it should work with most `getopt_long` implementations.

| Platform  | Status                | Latest Test Date | Notes                          |
|-----------|-----------------------|------------------|--------------------------------|
| GNU glibc | ✅ All tests passing  | 2026-02-13       | Tested on Debian 13            |
| musl      | ✅ All tests passing  | 2026-02-13       | Tested on Alpine Linux 3.23.3  |
| OpenBSD   | ✅ All tests passing  | 2026-02-13       | Tested on OpenBSD 7.8          |
| FreeBSD   | ✅ All tests passing  | 2026-02-13       | Tested on FreeBSD 15.0         |
| macOS     | ✅ All tests passing  | 2026-02-13       | Tested on macOS "Ventura" 13.7 |
| Haiku     | ✅ All tests passing  | 2026-02-13       | Tested on Haiku R1/beta5       |

Any unmentioned platforms are untested.
