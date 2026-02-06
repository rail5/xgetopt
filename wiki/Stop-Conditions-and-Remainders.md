# Stop Conditions and Remainders

XGetOpt supports early-stop parsing via:

```cpp
auto [options, remainder] = parser.parse_until<XGetOpt::BeforeFirstNonOptionArgument>(argc, argv);
```

This returns:

- `options`: an `OptionSequence` of parsed options (and possibly some non-option arguments)
- `remainder`: an `OptionRemainder` pointing at the unparsed tail of argv

## Key rule: remainder includes the stopping token for “Before...” conditions

XGetOpt computes the remainder so that when parsing stops due to a “Before...” stop condition, the remainder begins at the **argv element that caused parsing to stop**.

Concretely:

- When stopping because the first non-option argument was encountered, the remainder starts at that non-option argument.
- When stopping because the first error was encountered, the remainder starts at the offending option token.

This is implemented deterministically by capturing the argv index being examined on entry to each `getopt_long()` call.

## StopCondition definitions

### `AllOptions`

- Parses until `getopt_long()` returns `-1`.
- All non-option arguments encountered (RETURN_IN_ORDER) are appended to `OptionSequence::getNonOptionArguments()`.
- Remainder is empty (`remainder.argc == 0`) or begins at `--` if present.

This is what `OptionParser::parse()` uses internally.

### `BeforeFirstNonOptionArgument`

- Parses options until the first non-option argument is encountered.
- The non-option argument is **not** appended to `options.getNonOptionArguments()`.
- The remainder begins at that first non-option argument.

Example use: "parse global options, then hand the rest to a subcommand."

### `AfterFirstNonOptionArgument`

- Parses options until the first non-option argument is encountered.
- That first non-option argument is appended to `options.getNonOptionArguments()`.
- The remainder begins *after* that first non-option argument.

Example use: "capture the subcommand name, then pass the rest to the subcommand parser."

### `BeforeFirstError`

- Parses options until the first error is encountered (unknown option or missing required argument).
- No exception is thrown for that first error.
- The remainder begins at the offending argv token.

This is useful when you want to try multiple parsers or implement custom error reporting.

## Examples

### Subcommand-style CLI

```cpp
auto [global, rem] = parser.parse_until<XGetOpt::BeforeFirstNonOptionArgument>(argc, argv);

if (rem.argc == 0) {
	// no subcommand
	return 1;
}

std::string_view subcommand = rem.argv[0];
```

### Capture subcommand name as a non-option

```cpp
auto [global, rem] = parser.parse_until<XGetOpt::AfterFirstNonOptionArgument>(argc, argv);

if (!global.getNonOptionArguments().empty()) {
	std::string_view subcommand = global.getNonOptionArguments().front();
}
```

### Try-parse pattern

```cpp
auto [opts, rem] = parser.parse_until<XGetOpt::BeforeFirstError>(argc, argv);
if (rem.argc != 0) {
	// First error token is rem.argv[0]
}
```

## Notes

- The `OptionRemainder` pointers are borrowed; no argv copying is performed.
- The `--` terminator is handled by GNU `getopt_long()`.
- See **GNU Getopt Notes** for details about `RETURN_IN_ORDER`, option clustering, and global state.
