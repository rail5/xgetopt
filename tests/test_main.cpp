#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "../xgetopt.h"

namespace test {

struct Failure : std::runtime_error {
	explicit Failure(const std::string& msg) : std::runtime_error(msg) {}
};

static int g_failed = 0;
static int g_passed = 0;

#define TOSTR2(x) #x
#define TOSTR(x) TOSTR2(x)

#define TEST_ASSERT(expr) \
	do { \
		if (!(expr)) throw ::test::Failure(std::string("ASSERT failed: ") + #expr + " @" + __FILE__ + ":" + TOSTR(__LINE__)); \
	} while (0)

#define TEST_EQ(a, b) \
	do { \
		auto _a = (a); \
		auto _b = (b); \
		if (!((_a) == (_b))) { \
			throw ::test::Failure(std::string("EQ failed: ") + #a + " == " + #b + " @" + __FILE__ + ":" + TOSTR(__LINE__)); \
		} \
	} while (0)

#define TEST_NE(a, b) \
	do { \
		auto _a = (a); \
		auto _b = (b); \
		if (((_a) == (_b))) { \
			throw ::test::Failure(std::string("NE failed: ") + #a + " != " + #b + " @" + __FILE__ + ":" + TOSTR(__LINE__)); \
		} \
	} while (0)

#define TEST_THROWS(stmt) \
	do { \
		bool threw = false; \
		try { (void)(stmt); } catch (...) { threw = true; } \
		if (!threw) throw ::test::Failure(std::string("THROWS failed: ") + #stmt + " @" + __FILE__ + ":" + TOSTR(__LINE__)); \
	} while (0)

#define TEST_NOTHROW(stmt) \
	do { \
		try { (void)(stmt); } catch (const std::exception& e) { \
			throw ::test::Failure(std::string("NOTHROW failed: ") + #stmt + " threw: " + e.what() + " @" + __FILE__ + ":" + TOSTR(__LINE__)); \
		} \
	} while (0)

struct ArgvBuilder {
	std::vector<std::string> storage;
	std::vector<char*> argv;

	explicit ArgvBuilder(std::initializer_list<std::string_view> args) {
		storage.reserve(args.size());
		argv.reserve(args.size() + 1);
		for (auto s : args) storage.emplace_back(s);
		for (auto& s : storage) argv.push_back(s.data());
		argv.push_back(nullptr);
	}

	int argc() const { return static_cast<int>(storage.size()); }
	char** data() { return argv.data(); }
};

static constexpr auto make_main_parser() {
	return XGetOpt::OptionParser<
		XGetOpt::Option<'h', "help", "help", XGetOpt::NoArgument>,
		XGetOpt::Option<'v', "verbose", "verbose", XGetOpt::NoArgument>,
		XGetOpt::Option<'o', "output", "output", XGetOpt::RequiredArgument, "file">,
		XGetOpt::Option<'p', "param", "param", XGetOpt::OptionalArgument>,
		XGetOpt::Option<1001, "long-only", "long-only", XGetOpt::NoArgument>,
		XGetOpt::Option<'s', "", "short-only", XGetOpt::NoArgument>,
		XGetOpt::Option<1002, "long-description", "This item has an extremely long description, which XGetOpt is expected to wrap at 80-character lines for easy display in a terminal. If it fails to do this, it is not functioning properly.", XGetOpt::RequiredArgument, "arg">
	>{};
}

static constexpr auto make_sub_parser() {
	return XGetOpt::OptionParser<
		XGetOpt::Option<'a', "alpha", "alpha", XGetOpt::NoArgument>,
		XGetOpt::Option<'b', "beta", "beta", XGetOpt::RequiredArgument, "value">
	>{};
}

static void test_help_string_smoke() {
	constexpr auto parser = make_main_parser();
	constexpr std::string_view help = parser.getHelpString();
	TEST_ASSERT(!help.empty());
	TEST_ASSERT(help.find("--help") != std::string_view::npos);
	TEST_ASSERT(help.find("--output") != std::string_view::npos);
}

static void test_help_string_lines_dont_exceed_80_chars() {
	constexpr auto parser = make_main_parser();
	constexpr std::string_view help = parser.getHelpString();

	size_t line_start = 0;
	while (line_start < help.size()) {
		size_t line_end = help.find('\n', line_start);
		if (line_end == std::string_view::npos) {
			line_end = help.size();
		}
		size_t line_length = line_end - line_start;
		TEST_ASSERT(line_length <= 80);
		line_start = line_end + 1;
	}
}

static void test_parse_short_and_long() {
	constexpr auto parser = make_main_parser();
	ArgvBuilder av{"prog", "-h", "--verbose", "--output", "out.txt"};
	auto seq = parser.parse(av.argc(), av.data());
	TEST_ASSERT(seq.hasOption('h'));
	TEST_ASSERT(seq.hasOption('v'));

	bool saw_output = false;
	for (const auto& opt : seq) {
		if (opt.getShortOpt() == 'o') {
			saw_output = true;
			TEST_ASSERT(opt.hasArgument());
			TEST_EQ(opt.getArgument(), std::string_view("out.txt"));
		}
	}
	TEST_ASSERT(saw_output);
}

static void test_parse_required_argument_forms() {
	constexpr auto parser = make_main_parser();

	{ // long form with '='
		ArgvBuilder av{"prog", "--output=out.txt"};
		auto seq = parser.parse(av.argc(), av.data());
		bool saw_output = false;
		for (const auto& opt : seq) {
			if (opt.getShortOpt() == 'o') {
				saw_output = true;
				TEST_ASSERT(opt.hasArgument());
				TEST_EQ(opt.getArgument(), std::string_view("out.txt"));
			}
		}
		TEST_ASSERT(saw_output);
	}

	{ // short form with attached arg
		ArgvBuilder av{"prog", "-oout.txt"};
		auto seq = parser.parse(av.argc(), av.data());
		bool saw_output = false;
		for (const auto& opt : seq) {
			if (opt.getShortOpt() == 'o') {
				saw_output = true;
				TEST_ASSERT(opt.hasArgument());
				TEST_EQ(opt.getArgument(), std::string_view("out.txt"));
			}
		}
		TEST_ASSERT(saw_output);
	}
}

static void test_parse_optional_argument() {
	constexpr auto parser = make_main_parser();

	{ // no argument
		ArgvBuilder av{"prog", "--param"};
		auto seq = parser.parse(av.argc(), av.data());
		bool saw = false;
		for (const auto& opt : seq) {
			if (opt.getShortOpt() == 'p') {
				saw = true;
				TEST_ASSERT(!opt.hasArgument());
			}
		}
		TEST_ASSERT(saw);
	}

	{ // with argument using =
		ArgvBuilder av{"prog", "--param=zzz"};
		auto seq = parser.parse(av.argc(), av.data());
		bool saw = false;
		for (const auto& opt : seq) {
			if (opt.getShortOpt() == 'p') {
				saw = true;
				TEST_ASSERT(opt.hasArgument());
				TEST_EQ(opt.getArgument(), std::string_view("zzz"));
			}
		}
		TEST_ASSERT(saw);
	}

	{ // long optional args typically only bind via '='; separate token becomes non-option
		ArgvBuilder av{"prog", "--param", "zzz"};
		auto seq = parser.parse(av.argc(), av.data());
		bool saw = false;
		for (const auto& opt : seq) {
			if (opt.getShortOpt() == 'p') {
				saw = true;
				TEST_ASSERT(!opt.hasArgument());
			}
		}
		TEST_ASSERT(saw);
		auto nonopts = seq.getNonOptionArguments();
		TEST_EQ(nonopts.size(), size_t{1});
		TEST_EQ(nonopts[0], std::string_view("zzz"));
	}
}

static void test_parse_long_only_and_short_only() {
	constexpr auto parser = make_main_parser();
	ArgvBuilder av{"prog", "--long-only", "-s"};
	auto seq = parser.parse(av.argc(), av.data());
	TEST_ASSERT(seq.hasOption(1001));
	TEST_ASSERT(seq.hasOption('s'));
}

static void test_option_clustering() {
	constexpr auto parser = make_main_parser();
	ArgvBuilder av{"prog", "-vh"};
	auto seq = parser.parse(av.argc(), av.data());
	TEST_ASSERT(seq.hasOption('v'));
	TEST_ASSERT(seq.hasOption('h'));
}

static void test_non_option_arguments_are_collected() {
	constexpr auto parser = make_main_parser();
	ArgvBuilder av{"prog", "file1", "-v", "file2"};
	auto seq = parser.parse(av.argc(), av.data());
	TEST_ASSERT(seq.hasOption('v'));
	auto nonopts = seq.getNonOptionArguments();
	TEST_EQ(nonopts.size(), size_t{2});
	TEST_EQ(nonopts[0], std::string_view("file1"));
	TEST_EQ(nonopts[1], std::string_view("file2"));
}

static void test_double_parse_resets_global_state() {
	constexpr auto parser = make_main_parser();
	ArgvBuilder av1{"prog", "-v"};
	ArgvBuilder av2{"prog", "-h"};

	auto s1 = parser.parse(av1.argc(), av1.data());
	auto s2 = parser.parse(av2.argc(), av2.data());
	TEST_ASSERT(s1.hasOption('v'));
	TEST_ASSERT(s2.hasOption('h'));
}

static void test_double_dash_collects_remaining_as_nonoptions() {
	constexpr auto parser = make_main_parser();
	ArgvBuilder av{"prog", "-v", "--", "file1", "-h"};
	auto seq = parser.parse(av.argc(), av.data());
	TEST_ASSERT(seq.hasOption('v'));

	// After `--`, everything is treated as positional arguments.
	auto nonopts = seq.getNonOptionArguments();
	TEST_EQ(nonopts.size(), size_t{2});
	TEST_EQ(nonopts[0], std::string_view("file1"));
	TEST_EQ(nonopts[1], std::string_view("-h"));
}

static void test_parse_until_before_first_nonoption_subcommand_pattern() {
	constexpr auto global = make_main_parser();
	constexpr auto sub = make_sub_parser();

	// global opts, then subcommand, then subcommand opts
	ArgvBuilder av{"prog", "-v", "subcmd", "-a", "--beta", "B"};
	auto [gopts, rem] = global.parse_until<XGetOpt::BeforeFirstNonOptionArgument>(av.argc(), av.data());
	TEST_ASSERT(gopts.hasOption('v'));

	// remainder should begin at the subcommand token
	TEST_ASSERT(rem.argc >= 1);
	TEST_EQ(std::string_view(rem.argv[0]), std::string_view("subcmd"));

	// Passing remainder directly should treat rem.argv[0] as argv[0] (program name)
	// and not skip the first option after subcmd.
	auto subopts = sub.parse(rem.argc, rem.argv);
	TEST_ASSERT(subopts.hasOption('a'));
	bool saw_beta = false;
	for (const auto& opt : subopts) {
		if (opt.getShortOpt() == 'b') {
			saw_beta = true;
			TEST_ASSERT(opt.hasArgument());
			TEST_EQ(opt.getArgument(), std::string_view("B"));
		}
	}
	TEST_ASSERT(saw_beta);
}

static void test_parse_until_after_first_nonoption_consumes_one_nonoption() {
	constexpr auto parser = make_main_parser();
	ArgvBuilder av{"prog", "-v", "cmd", "--output", "x"};

	auto [opts, rem] = parser.parse_until<XGetOpt::AfterFirstNonOptionArgument>(av.argc(), av.data());
	TEST_ASSERT(opts.hasOption('v'));

	// Exactly one non-option should be in parsed results.
	auto nonopts = opts.getNonOptionArguments();
	TEST_EQ(nonopts.size(), size_t{1});
	TEST_EQ(nonopts[0], std::string_view("cmd"));

	// remainder begins after cmd
	TEST_ASSERT(rem.argc >= 0);
	if (rem.argc >= 1) {
		TEST_EQ(std::string_view(rem.argv[0]), std::string_view("--output"));
	}
}

static void test_parse_throws_on_unknown_and_missing_arg() {
	constexpr auto parser = make_main_parser();
	{ // unknown
		ArgvBuilder av{"prog", "--does-not-exist"};
		TEST_THROWS(parser.parse(av.argc(), av.data()));
	}
	{ // missing required argument
		ArgvBuilder av{"prog", "--output"};
		TEST_THROWS(parser.parse(av.argc(), av.data()));
	}
}

static void test_parse_until_before_first_error_does_not_throw_and_returns_remainder() {
	constexpr auto parser = make_main_parser();

	{ // unknown option
		ArgvBuilder av{"prog", "-v", "--nope", "zzz"};
		auto [opts, rem] = parser.parse_until<XGetOpt::BeforeFirstError>(av.argc(), av.data());
		TEST_ASSERT(opts.hasOption('v'));
		TEST_ASSERT(rem.argc >= 1);
		TEST_EQ(std::string_view(rem.argv[0]), std::string_view("--nope"));
	}

	{ // missing required arg
		ArgvBuilder av{"prog", "--output"};
		auto [opts, rem] = parser.parse_until<XGetOpt::BeforeFirstError>(av.argc(), av.data());
		TEST_EQ(opts.size(), size_t{0});
		TEST_ASSERT(rem.argc >= 1);
		TEST_EQ(std::string_view(rem.argv[0]), std::string_view("--output"));
	}

	{ // clustered short options where the error occurs mid-token
		ArgvBuilder av{"prog", "-vz"}; // 'v' known, 'z' unknown
		auto [opts, rem] = parser.parse_until<XGetOpt::BeforeFirstError>(av.argc(), av.data());
		TEST_ASSERT(opts.hasOption('v'));
		TEST_ASSERT(rem.argc >= 1);
		TEST_EQ(std::string_view(rem.argv[0]), std::string_view("-vz"));
	}

	{ // clustered short options missing required argument (-o requires an arg)
		ArgvBuilder av{"prog", "-vo"};
		auto [opts, rem] = parser.parse_until<XGetOpt::BeforeFirstError>(av.argc(), av.data());
		TEST_ASSERT(opts.hasOption('v'));
		TEST_ASSERT(rem.argc >= 1);
		TEST_EQ(std::string_view(rem.argv[0]), std::string_view("-vo"));
	}
}

static void test_multiple_parse_and_combine() {
	// Effect: Stop after Nth non-option argument
	// Achieved by repeatedly parsing and combining the resulting OptionSequences.
	constexpr auto parser = make_main_parser();
	ArgvBuilder av{"prog", "-v", "file1", "--output", "out.txt", "file2", "-h"};
	auto total_opts = XGetOpt::OptionSequence{};
	int nonopt_count = 0;
	int argc = av.argc();
	char** argv = av.data();
	while (nonopt_count < 2) {
		auto [opts, rem] = parser.parse_until<XGetOpt::AfterFirstNonOptionArgument>(argc, argv);
		total_opts += opts;
		nonopt_count++;
		argc = rem.argc + 1;
		argv = rem.argv - 1; // Adjust to add an argv[0] that's ignored
			// This is unsafe generally and shouldn't be done in production code
			// In production, you would either want to:
			// 1. FIRST verify that rem.argv does NOT point to argv[0]
			// Or better, 2. Copy rem.argv into a new array with a proper, ignorable argv[0]
	}
	TEST_ASSERT(total_opts.hasOption('v'));
	TEST_ASSERT(!total_opts.hasOption('h'));
	bool saw_output = false;
	for (const auto& opt : total_opts) {
		if (opt.getShortOpt() == 'o') {
			saw_output = true;
			TEST_ASSERT(opt.hasArgument());
			TEST_EQ(opt.getArgument(), std::string_view("out.txt"));
		}
	}
	TEST_ASSERT(saw_output);

	// Two non-option arguments collected
	auto nonopts = total_opts.getNonOptionArguments();
	TEST_EQ(nonopts.size(), size_t{2});
	TEST_EQ(nonopts[0], std::string_view("file1"));
	TEST_EQ(nonopts[1], std::string_view("file2"));

	// Remaining args after two non-options should be "-h"
	TEST_ASSERT(argc >= 1);
	TEST_EQ(std::string_view(argv[1]), std::string_view("-h"));
}

static void run_test(const char* name, void (*fn)()) {
	try {
		fn();
		++g_passed;
		std::cout << "[PASS] " << name << "\n";
	} catch (const Failure& f) {
		++g_failed;
		std::cout << "[FAIL] " << name << ": " << f.what() << "\n";
	} catch (const std::exception& e) {
		++g_failed;
		std::cout << "[FAIL] " << name << ": unexpected exception: " << e.what() << "\n";
	} catch (...) {
		++g_failed;
		std::cout << "[FAIL] " << name << ": unknown exception\n";
	}
}

} // namespace test

int main() {
	using namespace test;

	run_test("help_string_smoke", test_help_string_smoke);
	run_test("help_string_lines_dont_exceed_80_chars", test_help_string_lines_dont_exceed_80_chars);
	run_test("parse_short_and_long", test_parse_short_and_long);
	run_test("parse_required_argument_forms", test_parse_required_argument_forms);
	run_test("parse_optional_argument", test_parse_optional_argument);
	run_test("parse_long_only_and_short_only", test_parse_long_only_and_short_only);
	run_test("option_clustering", test_option_clustering);
	run_test("non_option_arguments_are_collected", test_non_option_arguments_are_collected);
	run_test("double_parse_resets_global_state", test_double_parse_resets_global_state);
	run_test("double_dash_collects_remaining_as_nonoptions", test_double_dash_collects_remaining_as_nonoptions);
	run_test("parse_until_before_first_nonoption_subcommand_pattern", test_parse_until_before_first_nonoption_subcommand_pattern);
	run_test("parse_until_after_first_nonoption_consumes_one_nonoption", test_parse_until_after_first_nonoption_consumes_one_nonoption);
	run_test("multiple_parse_and_combine", test_multiple_parse_and_combine);
	run_test("parse_throws_on_unknown_and_missing_arg", test_parse_throws_on_unknown_and_missing_arg);
	run_test("parse_until_before_first_error", test_parse_until_before_first_error_does_not_throw_and_returns_remainder);

	std::cout << "\npassed: " << g_passed << ", failed: " << g_failed << "\n";
	return g_failed == 0 ? 0 : 1;
}
