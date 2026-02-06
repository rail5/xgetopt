# XGetOpt Wiki

XGetOpt is a **header-only, constexpr-first** C++20 library that wraps **GNU `getopt_long`** behind a more convenient, compile-time-friendly interface.

- Source header: `xgetopt.h`
- Intended platforms: GNU libc environments (and any libc providing GNU-compatible `getopt_long`).

## Highlights

- Compile-time option definitions (`XGETOPT_OPTION`)
- Compile-time help string generation (`OptionParser::getHelpString()`)
- Runtime parsing that produces a strongly-typed result (`OptionSequence`)
- Optional early-stop parsing via `parse_until<StopCondition>()`

## One-minute example

```cpp
#include "xgetopt.h"
#include <iostream>

int main(int argc, char* argv[]) {
	constexpr auto parser = XGETOPT_PARSER(
		XGETOPT_OPTION('h', "help", "Show help", XGetOpt::NoArgument),
		XGETOPT_OPTION('o', "output", "Output file", XGetOpt::RequiredArgument, "file")
	);

	try {
		auto options = parser.parse(argc, argv);

		if (options.hasOption('h')) {
			std::cout << parser.getHelpString();
			return 0;
		}

		for (const auto& arg : options.getNonOptionArguments()) {
			std::cout << "non-option: " << arg << "\n";
		}

	} catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
		return 1;
	}
}
```

## Scope and non-goals

- XGetOpt is intentionally tied to GNU `getopt_long` semantics.
- XGetOpt does **not** attempt to be fully portable or Windows-compatible.
- Parsing uses `getopt` global state (`optind`, `optarg`, `optopt`, `opterr`). See **GNU Getopt Notes**.
