# Getting Started

## Requirements

- C++20 compiler
- GNU-compatible `getopt_long` (XGetOpt includes `<getopt.h>` and expects GNU `getopt_long` behavior)

XGetOpt is header-only: add `xgetopt.h` to your include path and `#include "xgetopt.h"`.

## Defining options

Options are defined at compile time using the `XGETOPT_OPTION(...)` macro:

```cpp
XGETOPT_OPTION(shortopt, longopt, description, requirement)
XGETOPT_OPTION(shortopt, longopt, description, requirement, arg_placeholder)
```

Where:

- `shortopt`: a printable ASCII character like `'h'`, or a unique integer (commonly 1000+) for long-only options.
- `longopt`: a string literal like `"help"` or `"output"` (use `""` for short-only options).
- `description`: help text.
- `requirement`: `XGetOpt::NoArgument`, `XGetOpt::RequiredArgument`, or `XGetOpt::OptionalArgument`.
- `arg_placeholder`: optional placeholder used in help output (`"file"`, `"path"`, etc.).

## Creating a parser

Use the `XGETOPT_PARSER(...)` macro:

```cpp
constexpr auto parser = XGETOPT_PARSER(
	XGETOPT_OPTION('h', "help", "Display help", XGetOpt::NoArgument),
	XGETOPT_OPTION('o', "output", "Write output to file", XGetOpt::RequiredArgument, "file"),
	XGETOPT_OPTION(1001, "long-only", "Long option with no short form", XGetOpt::NoArgument)
);
```

## Parsing arguments

### Parse everything (most common)

```cpp
XGetOpt::OptionSequence options = parser.parse(argc, argv);
```

- Throws `std::runtime_error` on unknown options or missing required arguments.
- Non-option arguments are collected into `options.getNonOptionArguments()`.

### Parse until a condition

If you need to stop at the first non-option argument or stop before the first error, use:

```cpp
auto [options, remainder] = parser.parse_until<XGetOpt::BeforeFirstNonOptionArgument>(argc, argv);
```

See **Stop Conditions and Remainders** for precise behavior.

## Handling parsed options

Iterate over the options:

```cpp
for (const auto& opt : options) {
	switch (opt.getShortOpt()) {
		case 'h':
			std::cout << parser.getHelpString();
			break;
		case 'o':
			if (opt.hasArgument()) {
				std::cout << "output=" << opt.getArgument() << "\n";
			}
			break;
	}
}
```

## Help string

`parser.getHelpString()` returns a compile-time generated help string.

- Wrapped to 80 columns for descriptions
- Displays required args as `<arg>` and optional args as `[=arg]` (or `[arg]` for short-only)

## Potential pitfalls

- XGetOpt relies on GNU `getopt_long` and `RETURN_IN_ORDER` behavior. It is not a POSIX-only implementation.
- Parsing uses global `getopt` state. Do not call `parse` concurrently from multiple threads.

