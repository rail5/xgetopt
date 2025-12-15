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
  -o, --output <arg>               Specify output file
  -p, --parameter [arg]            Specify optional parameter
      --long-option-only           This has no shortopt
      --long-option-with-arg <arg> This has no shortopt and requires an argument
  -s                               This has no longopt
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

 - `getHelpString()`: Provides a compile-time generated help string detailing all options and their descriptions.
 - `parse(int argc, char* argv[])`: Parses the command-line arguments and returns an `XGetOpt::OptionSequence` containing the parsed options and non-option arguments.

#### Compile-Time Generated Help String

The help string is generated at compile-time based on the options provided to the `OptionParser` constructor. and is stored as a member of the instantiated `OptionParser`.

Because it's generated at compile-time, a fixed-sized buffer is given to the string. This is, by default, 4096 bytes, which should be more than enough for most applications. If the generated string exceeds the buffer limit, you'll get a compile-time error. If this happens, you can increase the size of the buffer by defining the `XGETOPT_HELPSTRING_BUFFER_SIZE` macro before including `xgetopt.h`.

```cpp
#define XGETOPT_HELPSTRING_BUFFER_SIZE 8192
#include "xgetopt.h"
```

Though it's unlikely any application will come up against this 4096-byte default limit. Nevertheless, the option is provided for edge cases.

> *Note to potential contributors*: it would be infinitely preferable if the buffer size could be determined dynamically at compile-time based on the options provided.
>
> If the array of options was declared as a static, global variable, there would be absolutely no difficulty here. We would declare a `consteval auto` function, which as its first step would calculate `constexpr size_t RequiredLength = calculate_help_string_length(OptionArray)`, and then would return a `FixedString<RequiredLength>`.
>
> However, it seems that even though the OptionParser class has a `constexpr` constructor, and the object and all of its properties are **fixed** and **known** at compile-time, the fact that the options array is (and must be) a non-static data member of the class precludes us from using data about it in a constant expression like that.
>
> If anyone sees any clever solutions to this, please contribute.

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

The `hasOption(int shortopt)` member function allows you to check if a specific option was provided.

```cpp
if (options.hasOption('h')) {
	// '-h' was one of the options specified by the user
}
```

## License

XGetOpt is released under the GNU General Public License v2 or later. See the [LICENSE](LICENSE) file for details.
