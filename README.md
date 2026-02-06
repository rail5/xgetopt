# XGetOpt

XGetOpt is a simple, **header-only, constexpr-first** C++20 library for parsing command-line options.

Underneath, it is actually just a wrapper around GNU `getopt_long` with a more convenient C++ interface.

[See the wiki](../../../../../../rail5/xgetopt/wiki/) for complete documentation and examples.

## Features

 - Fully constexpr, compile-time "help string" generation
 - Fully constexpr option definitions and metadata
 - Supports short options (e.g. `-a`), and long options (e.g. `--alpha`)
   - Options may have both short and long forms, or only one or the other
 - Supports options with no argument, required argument, and optional argument
 - Out-of-the-box support for non-option arguments
 - Supports early-stop parsing via `parse_until<StopCondition>()`
   - Stop before/after first non-option argument, or before first error
   - Remainder of unparsed arguments provided for further processing
 - Classic option clustering (e.g. `-abc` is equivalent to `-a -b -c`)

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

## API Reference

[See the wiki](../../../../../../rail5/xgetopt/wiki/) for complete documentation and examples.

## License

XGetOpt is released under the GNU General Public License v2 or later. See the [LICENSE](LICENSE) file for details.
