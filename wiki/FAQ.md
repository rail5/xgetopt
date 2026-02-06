# FAQ

## Is XGetOpt portable?

Not fully. XGetOpt is a wrapper around **GNU `getopt_long()`** and expects GNU-compatible behavior.

If you need Windows compatibility etc, consider a different backend or a different parsing library.

## Is it thread-safe?

No. GNU `getopt` relies on global variables (`optind`, `optarg`, `optopt`, `opterr`). XGetOpt resets and uses those globals during parsing.

## Does it allocate?

Yes, `OptionSequence` stores results in `std::vector`.

The “constexpr-first” part is about:

- defining options at compile-time
- generating the help text at compile-time

Parsing is (and *must* be) still runtime work.

## Does it copy argv strings?

No. Parsed arguments are exposed as `std::string_view` pointing into the original `argv` strings.

That means:

- Don’t store the views longer than the lifetime of the argv strings.
- In practice, `argv` is stable for the duration of `main`.

## How are non-option arguments handled?

XGetOpt enables GNU `RETURN_IN_ORDER` so that non-option arguments appear as `opt == 1` during parsing and can be collected.

When collected, they are available via `OptionSequence::getNonOptionArguments()`. They are stored *in the order they are encountered*.

If you use `parse_until`, you can choose whether to stop before or after the first non-option.

## What exceptions can be thrown?

`parse(...)` throws `std::runtime_error` for:

- unknown options
- missing required option arguments

`parse_until<BeforeFirstError>(...)` does not throw for the first encountered error; it returns a remainder beginning at the error-causing token.

## Why do long-only options use an integer “shortopt”?

GNU `getopt_long()` reports parsed options using the `val` field from the `struct option` array.

XGetOpt uses your `shortopt` integer as that `val`, so it becomes the identifier returned via `ParsedOption::getShortOpt()`.
