/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 *
 * XGetOpt is a C++20 compile-time wrapper around getopt_long for parsing command-line options.
 * It allows defining options at compile-time and automatically generates help strings without runtime overhead.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef XGETOPT_H_
#define XGETOPT_H_

// Throw a compiler error if the C++ version is less than C++20
#include <cstddef>
#if __cplusplus < 202002L
#error "xgetopt.h requires C++20 or later"
#endif

#include <getopt.h>

#include <string_view>
#include <array>
#include <vector>
#include <optional>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <algorithm>

namespace XGetOpt {

enum ArgumentRequirement : uint8_t {
	NoArgument = 0,
	RequiredArgument = 1,
	OptionalArgument = 2
};

// Compile-time view of available options
struct Option {
	const int shortopt;
	const char* longopt;
	const char* description;
	const ArgumentRequirement argRequirement;

	constexpr Option(int s, const char* l, const char* d, ArgumentRequirement a)
		: shortopt(s), longopt(l), description(d), argRequirement(a) {}
	
	constexpr Option(const Option&) = default;
	constexpr Option(Option&&) noexcept = default;

	Option() = delete;
};

// Runtime view of a parsed option
class ParsedOption {
	private:
		int shortopt;
		std::optional<std::string_view> argument;
	public:
		ParsedOption(int s, std::optional<std::string_view> arg)
			: shortopt(s), argument(arg) {}
		
		/**
		 * @brief Get the shortopt of this parsed option (or the unique integer identifier for long-only options)
		 * 
		 * @return int The shortopt character or unique integer identifier
		 */
		int getShortOpt() const {
			return shortopt;
		}

		/**
		 * @brief Check if this option was given with an argument
		 * 
		 * @return true if an argument was provided
		 * @return false if no argument was provided
		 */
		bool hasArgument() const {
			return argument.has_value();
		}

		/**
		 * @brief Get the argument provided for this option
		 * 
		 * @return std::string_view The argument string
		 * @throws std::runtime_error if no argument was provided (you should check hasArgument() first if the argument is optional)
		 */
		std::string_view getArgument() const {
			if (!argument.has_value()) {
				throw std::runtime_error("No argument present for this option");
			}
			return argument.value();
		}
};

class OptionSequence {
	private:
		std::vector<ParsedOption> options;
		std::vector<std::string_view> nonOptionArguments;
	public:
		void addOption(const ParsedOption& opt) {
			options.push_back(opt);
		}

		void addNonOptionArgument(std::string_view arg) {
			nonOptionArguments.push_back(arg);
		}

		auto begin() const {
			return options.begin();
		}

		auto end() const {
			return options.end();
		}

		size_t size() const {
			return options.size();
		}

		bool empty() const {
			return options.empty();
		}

		const ParsedOption& operator[](size_t index) const {
			return options[index];
		}

		const ParsedOption& at(size_t index) const {
			return options.at(index);
		}

		/**
		 * @brief Check whether a specific option was provided
		 * 
		 * @param shortopt The shortopt character or unique integer identifier to check for
		 * @return true if the option was provided
		 * @return false if the option was not provided
		 */
		bool hasOption(int shortopt) const {
			return std::any_of(options.begin(), options.end(),
				[shortopt](const ParsedOption& opt) {
					return opt.getShortOpt() == shortopt;
				});
		}

		/**
		 * @brief Get a std::vector of all non-option arguments provided, in the order they were given
		 * 
		 * @return const std::vector<std::string_view>& The vector of non-option arguments
		 */
		const std::vector<std::string_view>& getNonOptionArguments() const {
			return nonOptionArguments;
		}
};

template<size_t N>
struct FixedString {
	std::array<char, N> data{};
	size_t size = 0;

	constexpr void append(std::string_view str) {
		if (size + str.size() > N) {
			// If this happens during constant evaluation, compilation fails (good).
			throw std::out_of_range("FixedString capacity exceeded");
		}
		for (char c : str) {
			data[size++] = c;
		}
	}

	constexpr void append(char c, size_t count = 1) {
		if (size + count > N) {
			throw std::out_of_range("FixedString capacity exceeded");
		}
		for (size_t i = 0; i < count; i++) {
			data[size++] = c;
		}
	}

	constexpr std::string_view view() const {
		return std::string_view(data.data(), size);
	}

	friend std::ostream& operator<<(std::ostream& os, const FixedString& fs) {
		return os.write(fs.data.data(), fs.size);
	}
};

// Helper function to figure a string's length at compile-time
constexpr size_t string_length(const char* str) {
	size_t length = 0;
	while (str && str[length] != '\0') {
		length++;
	}
	return length;
}

constexpr size_t option_label_length(const Option& option) {
	size_t length = 0;

	// Check if the shortopt is within printable ASCII range
	if (option.shortopt >= 33 && option.shortopt <= 126) {
		length += 2; // For "-x"
		if (option.longopt && option.longopt[0] != '\0') {
			length += 2; // For ", "
		}
	} else {
		// No shortopt, account for spaces
		length += 4; // For "    "
	}

	if (option.longopt && option.longopt[0] != '\0') {
		length += 2 + string_length(option.longopt); // For "--longopt"
	}

	switch (option.argRequirement) {
		case XGetOpt::ArgumentRequirement::RequiredArgument:
		case XGetOpt::ArgumentRequirement::OptionalArgument:
			length += 6; // For " <arg>" or " [arg]"
			break;
		case XGetOpt::ArgumentRequirement::NoArgument:
		default:
			break;
	}

	return length;
}

template<size_t N>
constexpr size_t max_option_label_length(const std::array<Option, N>& options) {
	size_t max_length = 0;
	for (size_t i = 0; i < N; i++) {
		size_t length = option_label_length(options[i]);
		if (length > max_length) {
			max_length = length;
		}
	}

	return max_length;
}

template<size_t N>
constexpr size_t calculate_help_string_length(const std::array<Option, N>& options) {
	size_t total_length = 0;
	const size_t max_label_length = max_option_label_length(options);

	for (size_t i = 0; i < N; i++) {
		total_length += 2; // For initial whitespace indentation
		const size_t label_length = option_label_length(options[i]);

		// How much padding is required?
		const size_t padding_amount = max_label_length - label_length;

		// label + padding + space-before-desc + desc + newline
		total_length += label_length + padding_amount + 1 + string_length(options[i].description) + 1;
	}

	// +1 for the trailing '\0'
	return total_length + 1;
}

template<size_t N>
using OptionArray = std::array<Option, N>;

template<size_t N>
struct OptionParser {
		const OptionArray<N> options;
		const std::array<char, 3*N + 1> short_options = build_short_options_(options);
		const std::array<struct option, N + 1> long_options = build_long_options_(options);

		constexpr std::array<struct option, N + 1> build_long_options_(const OptionArray<N>& opts) {
			std::array<struct option, N + 1> long_opts{};

			for (size_t i = 0; i < N; i++) {
				long_opts[i].name = (opts[i].longopt && opts[i].longopt[0] != '\0')
					? opts[i].longopt
					: nullptr;
				long_opts[i].has_arg = static_cast<int>(opts[i].argRequirement);
				long_opts[i].flag = nullptr;
				long_opts[i].val = opts[i].shortopt;
			}

			// Null-terminate the array
			long_opts[N] = {nullptr, 0, nullptr, 0};
			return long_opts;
		}

		constexpr std::array<char, 3*N + 1> build_short_options_(const OptionArray<N>& opts) {
			size_t short_opt_index = 0;
			std::array<char, 3*N + 1> short_opts{};

			// GNU getopt extension: RETURN_IN_ORDER
			// Non-option arguments are returned as character code 1, with optarg set to the argument string.
			short_opts[short_opt_index++] = '-';

			for (size_t i = 0; i < N; i++) {
				if (opts[i].shortopt >= 33 && opts[i].shortopt <= 126) {
					short_opts[short_opt_index++] = static_cast<char>(opts[i].shortopt);
					switch (opts[i].argRequirement) {
						case XGetOpt::ArgumentRequirement::RequiredArgument:
							short_opts[short_opt_index++] = ':';
							break;
						case XGetOpt::ArgumentRequirement::OptionalArgument:
							short_opts[short_opt_index++] = ':';
							short_opts[short_opt_index++] = ':';
							break;
						case XGetOpt::ArgumentRequirement::NoArgument:
						default:
							// No extra characters needed
							break;
					}
				}
			}

			// Null-terminate the array
			short_opts[short_opt_index] = '\0';
			return short_opts;
		}

		explicit constexpr OptionParser(OptionArray<N> opts) : options(opts) {}

		// Variadic constructor
		template<typename... Options>
		explicit constexpr OptionParser(Options... opts) : options{opts...} {}

		OptionParser() = delete;

		/**
		 * @brief Get a compile-time generated help string detailing all available options and their descriptions
		 * 
		 * @return constexpr XGetOpt::FixedString<4096> The generated help string, with a maximum size of 4096 bytes
		 */
		constexpr auto generateHelpString() const {
			return generateHelpString<4096>(options);
		}

		/**
		 * @brief Get a compile-time generated help string detailing all available options and their descriptions
		 * 
		 * @tparam MaxSize The maximum size of the help string to generate
		 * @param options The array of options to generate the help string for
		 * @return constexpr XGetOpt::FixedString<MaxSize> The generated help string
		 */
		template<size_t MaxSize>
		static constexpr auto generateHelpString(const OptionArray<N>& options) {
			FixedString<MaxSize> help_string;

			// NOTE: not constexpr (depends on function parameter)
			const size_t max_label_length = max_option_label_length(options);

			for (size_t i = 0; i < N; i++) {
				help_string.append("  ");

				if (options[i].shortopt >= 33 && options[i].shortopt <= 126) {
					help_string.append('-');
					help_string.append(static_cast<char>(options[i].shortopt));
					if (options[i].longopt && options[i].longopt[0] != '\0') {
						help_string.append(", ");
					}
				} else {
					// Longopt only, add spaces for alignment
					help_string.append(' ', 4); // The space that would've been used by "-x, "
				}

				if (options[i].longopt && options[i].longopt[0] != '\0') {
					help_string.append("--");
					help_string.append(options[i].longopt);
				}

				switch (options[i].argRequirement) {
					case XGetOpt::ArgumentRequirement::RequiredArgument:
						help_string.append(" <arg>");
						break;
					case XGetOpt::ArgumentRequirement::OptionalArgument:
						help_string.append(" [arg]");
						break;
					case XGetOpt::ArgumentRequirement::NoArgument:
					default:
						break;
				}

				const size_t label_length = option_label_length(options[i]);
				const size_t padding_amount = max_label_length - label_length;
				help_string.append(' ', padding_amount + 1);
				help_string.append(options[i].description);
				help_string.append('\n');
			}

			// No need to append '\0' (ostream<< and string_view don't require it)
			return help_string;
		}

		/**
		 * @brief Parse command-line arguments according to the defined options
		 * 
		 * @param argc The number of command-line arguments
		 * @param argv The array of command-line argument strings
		 * @return OptionSequence The parsed options and their values
		 */
		OptionSequence parse(int argc, char* argv[]) const {
			OptionSequence parsed_options;

			// Don't let getopt print messages.
			opterr = 0;

			// Reset in case parse() is called more than once in a process.
			optind = 1;

			auto find_by_short = [&](int s) -> const Option* {
				for (size_t i = 0; i < N; i++) {
					if (options[i].shortopt == s) return &options[i];
				}
				return nullptr;
			};

			auto find_by_long = [&](std::string_view name) -> const Option* {
				for (size_t i = 0; i < N; i++) {
					if (options[i].longopt && options[i].longopt[0] != '\0'
						&& name == std::string_view(options[i].longopt)) {
						return &options[i];
					}
				}
				return nullptr;
			};

			auto token_to_long_name = [&](const char* tok) -> std::string_view {
				if (!tok) return {};
				std::string_view sv(tok);
				if (sv.rfind("--", 0) != 0) return {};
				sv.remove_prefix(2);
				if (auto eq = sv.find('='); eq != std::string_view::npos) {
					sv = sv.substr(0, eq);
				}
				return sv;
			};

			int opt;
			int longindex = 0;
			while ((opt = getopt_long(argc, argv, short_options.data(), long_options.data(), &longindex)) != -1) {
				// Non-option argument (in-order)
				if (opt == 1) {
					if (optarg != nullptr) {
						parsed_options.addNonOptionArgument(std::string_view(optarg));
					}
					continue;
				}

				// Error from getopt_long: unknown option OR missing required argument
				if (opt == '?') {
					// Try to classify missing-argument vs unknown.
					// For short options missing arg: optopt is set to the option character.
					if (optopt != 0) {
						if (const Option* o = find_by_short(optopt)) {
							if (o->argRequirement == XGetOpt::ArgumentRequirement::RequiredArgument) {
								std::string option_name;
								if (optopt >= 33 && optopt <= 126) {
									option_name += '-';
									option_name += static_cast<char>(optopt);
								} else {
									option_name = std::string(o->longopt ? o->longopt : "???");
								}

								throw std::runtime_error(std::string("Missing required argument for option: ")
									+ option_name);
							}
						}
					}

					// For long options missing arg: argv[optind-1] is typically the option token.
					const char* culprit_tok =
						(optind > 0 && optind <= argc) ? argv[optind - 1] : nullptr;

					const std::string_view long_name = token_to_long_name(culprit_tok);
					if (!long_name.empty()) {
						if (const Option* o = find_by_long(long_name)) {
							if (o->argRequirement == XGetOpt::ArgumentRequirement::RequiredArgument) {
								throw std::runtime_error(std::string("Missing required argument for option: --")
									+ std::string(long_name));
							}
						}
					}

					// Otherwise: unknown option
					if (culprit_tok) {
						throw std::runtime_error(std::string("Unknown option: ") + culprit_tok);
					}
					throw std::runtime_error("Unknown option");
				}

				std::optional<std::string_view> argument;

				// Determine if an argument is present
				if (optarg != NULL) {
					argument = std::string_view(optarg);
				} else {
					// Check if the option accepts an optional argument
					// GNU Getopt's behavior is maybe a bit odd, or unexpected at least.
					// If an option is defined to take an optional argument, then either:
					// 1) The argument is given as part of the same argv token, e.g. "--opt=arg" or "-oarg"
					//    In this case, getopt sets optarg = "arg" as expected.
					// 2) No argument is given, in which case optarg = NULL, again, as expected.
					// 3) The argument is given as **the next** argv token, e.g. "--opt arg" or "-o arg".
					//    In this case, getopt sets optarg = NULL, but keeps the next argv token intact.
					//    This is a bit counterintuitive and needs to be handled specially here.
					for (size_t i = 0; i < N; i++) {
						// First, we check if this option is expected to handle an optional argument.
						if (options[i].shortopt == opt
							&& options[i].argRequirement == XGetOpt::ArgumentRequirement::OptionalArgument
						) {
							// If it's meant to handle an optional argument, let's process it
							bool has_optional_argument = [&]() {
								// CHECK: optarg not set, we're not at the end of options, and the next one doesn't start with '-'
								if (optarg == NULL && optind < argc && argv[optind][0] != '-') {
									// If so, set optarg = the next argument,
									// Increment the opt-index to tell getopt to skip the next one since we've already handled it
									// And evaluate to true (has_optional_argument = true)
									optarg = argv[optind];
									optind++;
									return true;
								} else {
									// Otherwise, evaluate to true if optarg is set, false if not
									return (optarg != NULL);
								}
							}();
							// If an optional argument is present, set it
							if (has_optional_argument) argument = std::string_view(optarg);
							break;
						}
					}
				}

				parsed_options.addOption(ParsedOption(opt, argument));
			}

			return parsed_options;
		}
};

// Deduction guide for OptionParser
template<typename... Options>
OptionParser(Options...) -> OptionParser<sizeof...(Options)>;

} // namespace XGetOpt

#endif // XGETOPT_H_
