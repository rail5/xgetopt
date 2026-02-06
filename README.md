# XGetOpt

XGetOpt is a simple, **header-only, constexpr-first** C++20 library for parsing command-line options.

[See the wiki](../../../../../../rail5/xgetopt/wiki/) for complete documentation and examples.

## Features

 - Fully constexpr, **compile-time help-string generation**
 - Fully constexpr option definitions and metadata
 - **First-class** support for **non-option arguments**
 - Support for **early-stop parsing** via `parse_until<StopCondition>()`
   - Stop before/after first non-option argument, or before first error
   - Remainder of unparsed arguments provided for further processing
 - Supports short options (e.g. `-a`), and long options (e.g. `--alpha`)
   - Options may have both short and long forms, or only one or the other
 - Supports options with no argument, required argument, and optional argument
 - Classic option clustering (e.g. `-abc` is equivalent to `-a -b -c`)
 - Preservation of GNU `getopt` semantics
   - Matches user expectations
   - XGetOpt is in fact a wrapper around `getopt_long`

## Example Usage

```cpp
#include "xgetopt.h"
#include <iostream>

int main(int argc, char* argv[]) {
	constexpr auto parser = XGETOPT_PARSER(
		XGETOPT_OPTION('h', "help", "Display this help message", XGetOpt::NoArgument),
		XGETOPT_OPTION('o', "output", "Specify output file", XGetOpt::RequiredArgument, "file"),
		XGETOPT_OPTION('p', "parameter", "Specify optional parameter", XGetOpt::OptionalArgument),

		XGETOPT_OPTION(1001, "long-option-only", "This has no shortopt", XGetOpt::NoArgument),
		XGETOPT_OPTION(1002, "long-option-with-arg", "This has no shortopt and requires an argument", XGetOpt::RequiredArgument),

		XGETOPT_OPTION('s', "", "This has no longopt", XGetOpt::NoArgument)
	);
	
	XGetOpt::OptionSequence options;

	try { // .parse() will throw an exception if invalid options are provided
		options = parser.parse(argc, argv);
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	for (const auto& opt : options) {
		switch (opt.getShortOpt()) {
			case 'h':
				std::cout << parser.getHelpString();
				return 0;
			case 'p': // Optional argument
				if (opt.hasArgument()) {
					std::cout << "Parameter option with argument: " << opt.getArgument() << std::endl;
				} else {
					std::cout << "Parameter option with no argument" << std::endl;
				}
				break;
			case 1001: // --long-option-only
				std::cout << "--long-option-only given" << std::endl;
				break;
			// Etc
		}
	}

	for (const auto& arg : options.getNonOptionArguments()) {
		std::cout << "Non-option argument: " << arg << std::endl;
	}
}
```

The generated help string looks like this:

```
  -h, --help                       Display this help message
  -o, --output <file>              Specify output file
  -p, --parameter[=arg]            Specify optional parameter
      --long-option-only           This has no shortopt
      --long-option-with-arg <arg> This has no shortopt and requires an argument
  -s                               This has no longopt
```

And is fully generated at compile-time.

### Subcommand Pattern Example

```cpp
#include "xgetopt.h"
#include <iostream>

int main(int argc, char* argv[]) {
	constexpr auto parser = XGETOPT_PARSER(
		XGETOPT_OPTION('h', "help", "Display this help message", XGetOpt::NoArgument),
		XGETOPT_OPTION('o', "output", "Specify output file", XGetOpt::RequiredArgument, "file"),
		XGETOPT_OPTION('p', "parameter", "Specify optional parameter", XGetOpt::OptionalArgument)
	);

	constexpr auto subCommandParser = XGETOPT_PARSER(
		XGETOPT_OPTION('a', "alpha", "Alpha option for subcommand", XGetOpt::NoArgument),
		XGETOPT_OPTION('b', "beta", "Beta option for subcommand", XGetOpt::RequiredArgument, "value"),
		XGETOPT_OPTION('h', "help", "Display this help message for subcommand", XGetOpt::NoArgument)
	);

	try {
		// The first non-option argument is treated as the subcommand name
		auto [options, remainder] = parser.parse_until<XGetOpt::BeforeFirstNonOptionArgument>(argc, argv);

		for (const auto& opt : options) {
			// Base program options processing...
		}

		if (remainder.argc == 0) {
			return 0; // No subcommand, done
		}

		std::string_view subcommandName = remainder.argv[0];

		if (subcommandName == "subcmd") {
			auto subOptions = subCommandParser.parse(remainder.argc, remainder.argv);
			for (const auto& opt : subOptions) {
				// Subcommand options processing...
			}
		}
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
}
```

In the above example, the first non-option argument is treated as the subcommand name, and the remaining arguments are parsed using a separate parser for that subcommand.

## API Reference

[See the wiki](../../../../../../rail5/xgetopt/wiki/) for complete documentation and examples.

## License

XGetOpt is released under the GNU General Public License v2 or later. See the [LICENSE](LICENSE) file for details.
