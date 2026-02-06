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
