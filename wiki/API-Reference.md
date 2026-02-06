# API Reference

This page describes the public surface of XGetOpt.

If you’re new: start with **Getting Started**.

## Namespaces

- `XGetOpt`: public types and macros
- `XGetOpt::Helpers`: internal helpers (not intended as stable API)

## Macros

### `XGETOPT_OPTION(...)`

Creates a constexpr option definition.

Supported forms:

```cpp
XGETOPT_OPTION(shortopt, longopt, description, requirement)
XGETOPT_OPTION(shortopt, longopt, description, requirement, arg_placeholder)
```

See **Option**.

### `XGETOPT_PARSER(...)`

Creates an `OptionParser<...>` from a list of `XGETOPT_OPTION(...)` expressions.

```cpp
constexpr auto parser = XGETOPT_PARSER(
	XGETOPT_OPTION('h', "help", "Show help", XGetOpt::NoArgument)
);
```

## Enums

### `ArgumentRequirement`

```cpp
enum ArgumentRequirement : uint8_t {
	NoArgument = 0,
	RequiredArgument = 1,
	OptionalArgument = 2
};
```

Controls whether an option takes no argument, a required argument, or an optional argument.

### `StopCondition`

```cpp
enum StopCondition {
	AllOptions,
	BeforeFirstNonOptionArgument,
	AfterFirstNonOptionArgument,
	BeforeFirstError
};
```

Used with `OptionParser::parse_until<>()`. See **Stop Conditions and Remainders**.

## Types

### `Option<...>`

A compile-time option definition. Instances are typically created via `XGETOPT_OPTION`.

Key properties:

- `shortopt` (int): printable ASCII for short options, or an integer (commonly 1000+) for long-only options.
- `longopt` (FixedString): string literal used for `--longopt`.
- `description` (FixedString): help text.
- `argRequirement` (ArgumentRequirement)
- `argumentPlaceholder` (FixedString): used in help output.

### `ParsedOption`

Represents a single parsed option as returned by `getopt_long`.

Methods:

- `int getShortOpt() const`
- `bool hasArgument() const`
- `std::string_view getArgument() const`

Notes:

- For long-only options, `getShortOpt()` returns the integer value you used as the option’s `shortopt`/`val`.
- Arguments are returned as `std::string_view` pointing into the original argv memory.

### `OptionSequence`

Holds parsing results.

- Iterable over parsed options (range-for supported)
- Stores non-option arguments separately

Methods:

- `void addOption(const ParsedOption&)` (internal use)
- `void addNonOptionArgument(std::string_view)` (internal use)
- `bool hasOption(int shortopt) const`
- `const std::vector<std::string_view>& getNonOptionArguments() const`

### `OptionRemainder`

Returned from `parse_until`. Represents the unparsed tail of argv.

```cpp
struct OptionRemainder {
	int argc;
	char** argv;
};
```

Notes:

- `argv` points into the original argument vector; it is not copied.
- The remainder rules depend on the `StopCondition`. See **Stop Conditions and Remainders**.

### `OptionParser<...Options>`

The main parser type.

Methods:

- `constexpr std::string_view getHelpString() const`
- `constexpr const OptionArray& getOptions() const`
- `OptionSequence parse(int argc, char* argv[]) const`
- `template<StopCondition end> std::pair<OptionSequence, OptionRemainder> parse_until(int argc, char* argv[]) const`

Error behavior:

- `parse(...)` stops on the first error by throwing `std::runtime_error`.
- `parse_until<BeforeFirstError>(...)` stops before the first error and returns a remainder (no exception for the *first* error).

Thread-safety:

- Not thread-safe due to GNU `getopt` global state. See **GNU Getopt Notes**.
