# XGetOpt

XGetOpt is a simple, **header-only, constexpr-first** C++20 library for parsing command-line options.

Underneath, it is actually just a wrapper around GNU `getopt_long` with a more convenient C++ interface.

## Features

 - Fully constexpr, compile-time "help string" generation
 - Fully constexpr option definitions and metadata
 - Supports short options (e.g. `-a`), and long options (e.g. `--alpha`)
   - Options may have both short and long forms, or only one or the other
 - Supports options with no argument, required argument, and optional argument
 - Out-of-the-box support for non-option arguments
 - Classic option clustering (e.g. `-abc` is equivalent to `-a -b -c`)

## Example Usage

```cpp
#include "xgetopt.h"
#include <iostream>

int main(int argc, char* argv[]) {
	constexpr XGetOpt::OptionParser parser(
		XGetOpt::Option('h', "help", "Display this help message", XGetOpt::NoArgument),
		XGetOpt::Option('o', "output", "Specify output file", XGetOpt::RequiredArgument),
		XGetOpt::Option('p', "parameter", "Specify optional parameter", XGetOpt::OptionalArgument),

		XGetOpt::Option(1001, "long-option-only", "This has no shortopt", XGetOpt::NoArgument),
		XGetOpt::Option(1002, "long-option-with-arg", "This has no shortopt and requires an argument", XGetOpt::RequiredArgument),

		XGetOpt::Option('s', "", "This has no longopt", XGetOpt::NoArgument)
	);
	
	constexpr auto help = parser.generateHelpString();

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
				std::cout << help;
				return 0;
			case 'p': // Optional argument
				if (opt.hasArgument()) {
					std::cout << "Parameter option with argument: " << opt.getArgument() << std::endl;
				} else {
					std::cout << "Parameter option with no argument" << std::endl;
				}
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
  -h, --help                  Display this help message
  -o, --output <arg>          Specify output file
  -p, --parameter [arg]       Specify optional parameter
	  --long-option-only      This has no shortopt
	  --long-option-with-arg <arg> This has no shortopt and requires an argument
  -s                          This has no longopt
```

And is fully generated at compile-time.

### `OptionParser` Class

`XGetOpt::OptionParser` is the main entry point for defining and working with command-line options.

The options accepted by the program must be provided in the **constructor** of `OptionParser` as a variadic list of `XGetOpt::Option` objects.

```cpp
constexpr XGetOpt::OptionParser parser(
	XGetOpt::Option('h', "help", "Display this help message", XGetOpt::NoArgument),
	XGetOpt::Option('o', "output", "Specify output file", XGetOpt::RequiredArgument)
);
```

Once an `OptionParser` is constructed, the following member functions are available:

 - `generateHelpString()`: Provides a compile-time generated help string detailing all options and their descriptions.
 - `parse(int argc, char* argv[])`: Parses the command-line arguments and returns an `XGetOpt::OptionSequence` containing the parsed options and non-option arguments.

### `Option` Class

`XGetOpt::Option` represents a single command-line option. It is constructed with the following parameters:

 - `shortopt`: An int representing the short option (e.g. 'h' for `-h`). If there is no short option, this should be a unique integer value greater than 255, typically starting from 1000, so that you can identify the option when handling.
   - For example, using `1001` for a long-only option will allow you to identify it later by checking for `opt.getShortOpt() == 1001`.
 - `longopt`: A `const char*` representing the long option (e.g. "help" for `--help`). Use `nullptr` or an empty string if there is no long option.
 - `description`: A `const char*` providing a human-readable description of the option for help text.
 - `argRequirement`: An `XGetOpt::ArgumentRequirement` enum value indicating whether the option requires an argument, has an optional argument, or has no argument.

### `OptionSequence` Class

`XGetOpt::OptionSequence` holds the results of parsing command-line arguments. It contains:

 - A list of `ParsedOption` objects representing each option that was provided on the command line, along with any arguments if they were given.
 - A list of non-option arguments that were provided.

You can iterate over the `OptionSequence` to handle each parsed option using range-based for loops or standard algorithms.

```cpp
for (const auto& opt : options) {
	switch (opt.getShortOpt()) {
		case 'h':
			// Handle help option
			break;
		case 'o':
			// Handle output option
			break;
		// Etc
	}
}
```

You can also access the list of non-option arguments:

```cpp
for (const auto& arg : options.getNonOptionArguments()) {
	std::cout << "Non-option argument: " << arg << std::endl;
}
```

These non-option arguments are stored in the order they were provided on the command line.

## License

XGetOpt is released under the GNU General Public License v2 or later. See the [LICENSE](LICENSE) file for details.
