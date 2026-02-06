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

It is recommended to use the `XGETOPT_PARSER` and `XGETOPT_OPTION` macros to define your option parser and options, as shown in the example above.

### `OptionParser` Class

`XGetOpt::OptionParser` is the main class used to define and parse command-line options. It is a `constexpr` class that takes a variable number of `Option` definitions as template parameters.

You can create an `OptionParser` instance using the `XGETOPT_PARSER` macro, which simplifies the syntax.

Methods available:

 - `parse(int argc, char* argv[])`: Parses the command-line arguments and returns an `OptionSequence` containing the parsed options and non-option arguments. Throws an exception if invalid options are provided.
 - `getHelpString()`: Returns the compile-time generated help string for the defined options.

#### Compile-Time Generated Help String

The help string is generated at compile-time based on the options provided to the `OptionParser`. This means that you don't have to manually maintain help text that matches your options; it is always in sync.

The descriptions for each option will automatically wrap at 80 characters for better readability in terminal output. For example:

```cpp
	constexpr auto parser = XGETOPT_PARSER(
		XGETOPT_OPTION('h', "help",
			"Display this help message and exit.",
			XGetOpt::ArgumentRequirement::NoArgument),
		XGETOPT_OPTION('v', "verbose",
			"This option has an extremely long description that is intended to test the help string generation functionality of the XGetOpt library. XGetOpt should correctly format this description across multiple lines, ensuring that it remains readable and well-structured within the constraints of an 80-character line limit.",
			XGetOpt::ArgumentRequirement::NoArgument)
	);
```

Produces the following help string:

```cpp
  -h, --help    Display this help message and exit.
  -v, --verbose This option has an extremely long description that is intended 
                to test the help string generation functionality of the XGetOpt 
                library. XGetOpt should correctly format this description across
                multiple lines, ensuring that it remains readable and 
                well-structured within the constraints of an 80-character line 
                limit.
```


### `Option` Class

`XGetOpt::Option` represents a single command-line option. It contains:

 - `shortopt`: The short option character (e.g., 'h' for `-h`). Use a unique value outside the ASCII printable range (conventionally, integers numbered from 1000 onward) for options without a short form.
 - `longopt`: The long option string (e.g., "help" for `--help`). Use an empty string for options without a long form.
 - `description`: A brief description of the option, used in the help string.
 - `argRequirement`: An enum value indicating whether the option requires an argument, has an optional argument, or has no argument.
 - `argumentPlaceholder`: A string representing the placeholder for the argument in the help string (e.g., "file" for an output file).

You can create an `Option` instance using the `XGETOPT_OPTION` macro, which simplifies the syntax.


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

The `hasOption(int shortopt)` member function allows you to check if a specific option was provided.

```cpp
if (options.hasOption('h')) {
	// '-h' was one of the options specified by the user
}
```

## License

XGetOpt is released under the GNU General Public License v2 or later. See the [LICENSE](LICENSE) file for details.
