# Examples

This page contains a few patterns that XGetOpt supports well.

## 1) Basic parse with help

```cpp
constexpr auto parser = XGETOPT_PARSER(
	XGETOPT_OPTION('h', "help", "Show help", XGetOpt::NoArgument),
	XGETOPT_OPTION('v', "verbose", "Verbose output", XGetOpt::NoArgument)
);

try {
	auto options = parser.parse(argc, argv);

	if (options.hasOption('h')) {
		std::cout << parser.getHelpString();
		return 0;
	}

} catch (const std::exception& e) {
	std::cerr << e.what() << "\n";
	return 1;
}
```

## 2) Required and optional arguments

```cpp
constexpr auto parser = XGETOPT_PARSER(
	XGETOPT_OPTION('o', "output", "Output file", XGetOpt::RequiredArgument, "file"),
	XGETOPT_OPTION('p', "parameter", "Optional parameter", XGetOpt::OptionalArgument)
);

auto options = parser.parse(argc, argv);

for (const auto& opt : options) {
	switch (opt.getShortOpt()) {
		case 'o':
			// Required, so hasArgument() should be true
			std::cout << "output=" << opt.getArgument() << "\n";
			break;
		case 'p':
			if (opt.hasArgument()) {
				std::cout << "parameter=" << opt.getArgument() << "\n";
			} else {
				std::cout << "parameter present with no argument\n";
			}
			break;
	}
}
```

## 3) Long-only options

Use an integer `shortopt` value for long-only options:

```cpp
constexpr auto parser = XGETOPT_PARSER(
	XGETOPT_OPTION(1001, "long-only", "No short form", XGetOpt::NoArgument)
);

auto options = parser.parse(argc, argv);

for (const auto& opt : options) {
	if (opt.getShortOpt() == 1001) {
		std::cout << "--long-only was provided\n";
	}
}
```

## 4) Subcommands: stop before first non-option

Parse global flags until the subcommand token:

```cpp
auto [global, rem] = parser.parse_until<XGetOpt::BeforeFirstNonOptionArgument>(argc, argv);

if (rem.argc == 0) {
	std::cerr << "missing subcommand\n";
	return 1;
}

std::string_view subcommand = rem.argv[0];
// rem.argv[1..] are the subcommand args
```

## 5) Subcommands: capture the command name

```cpp
auto [global, rem] = parser.parse_until<XGetOpt::AfterFirstNonOptionArgument>(argc, argv);

if (global.getNonOptionArguments().empty()) {
	std::cerr << "missing subcommand\n";
	return 1;
}

std::string_view subcommand = global.getNonOptionArguments().front();
```

## 6) “Try parse” without throwing on the first error

```cpp
auto [opts, rem] = parser.parse_until<XGetOpt::BeforeFirstError>(argc, argv);

if (rem.argc != 0) {
	std::cerr << "first error token: " << rem.argv[0] << "\n";
	// decide how to handle: show help, fallback parser, etc.
}
```
