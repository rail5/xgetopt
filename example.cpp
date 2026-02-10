/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 */

#include "xgetopt.h"
#include <iostream>

int main(int argc, char* argv[]) {
	constexpr XGetOpt::OptionParser<
		XGetOpt::Option<'h', "help", "Display this help message", XGetOpt::NoArgument>,
		XGetOpt::Option<'o', "output", "Specify output file", XGetOpt::RequiredArgument, "file">,
		XGetOpt::Option<'p', "parameter", "Specify optional parameter", XGetOpt::OptionalArgument>,
		XGetOpt::Option<1001, "long-option-only", "This has no shortopt", XGetOpt::NoArgument>,
		XGetOpt::Option<1002, "long-option-with-arg", "This has no shortopt and requires an argument", XGetOpt::RequiredArgument>,
		XGetOpt::Option<'s', "", "This has no longopt", XGetOpt::NoArgument>
	> parser;

	XGetOpt::OptionSequence options;
	try {
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
			case 'o':
				std::cout << "Output file: " << opt.getArgument() << std::endl;
				break;
			case 'p':
				std::cout << "-p given";
				if (opt.hasArgument()) {
					std::cout << " with argument: " << opt.getArgument();
				} else {
					std::cout << " with no argument";
				}
				std::cout << std::endl;
				break;
			case 1001:
				std::cout << "--long-option-only given" << std::endl;
				break;
			case 1002:
				std::cout << "--long-option-with-arg given with argument: " << opt.getArgument() << std::endl;
				break;
			case 's':
				std::cout << "-s given" << std::endl;
				break;
			default:
				std::cout << "Unknown option: " << opt.getShortOpt() << std::endl;
				break;
		}
	}

	for (const auto& arg : options.getNonOptionArguments()) {
		std::cout << "Non-option argument: " << arg << std::endl;
	}

	return 0;
}
