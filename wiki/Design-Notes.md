# Design Notes

This page explains a few design choices in XGetOpt.

## Why GNU `getopt_long()`?

- It is widely available on GNU/Linux distributions.
- It provides long options and optional args.
- Its behavior is well-defined, documented, and generally expected by users.

XGetOpt is explicit about being a wrapper; the goal is not to hide `getopt_long`, but to provide a safer, more ergonomic C++20 interface.

## Compile-time help string generation

XGetOpt uses compile-time metadata (option descriptions, names, argument requirements) to generate a formatted help string.

Properties:

- No runtime formatting work
- Help output is always consistent with the option definitions
- Descriptions wrap at 80 columns to keep output readable

## Why `std::string_view` for arguments?

Parsing exposes option arguments and non-option arguments as `std::string_view`.

Pros:

- No string copies
- Easy to pass around

Cons:

- The views are borrowed; they depend on the lifetime of the original `argv` strings.

## Early-stop parsing (`parse_until`)

XGetOpt supports early-stop parsing to make patterns like subcommands easier.

A key property is that for "Before..." stop conditions, the returned remainder begins at the argv token that caused parsing to stop.

This is implemented by capturing the argv index examined on entry to each `getopt_long()` call rather than attempting to reason about how GNU `getopt` advances `optind` under clustering.

See **Stop Conditions and Remainders** and **GNU Getopt Notes**.

## Error handling philosophy

- `parse(...)` is strict and throws on the first error.
- `parse_until<BeforeFirstError>(...)` is non-throwing for the first error, enabling custom error messages or fallback parsers.

## Known limitations

- Not thread-safe (global `getopt` state)
- Not Windows compatible
- Does not currently attempt to preserve/restore prior `optind`/`opterr` values
