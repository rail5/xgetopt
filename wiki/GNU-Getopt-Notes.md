# GNU Getopt Notes

XGetOpt is intentionally built on **GNU `getopt_long()`**.

This page documents the assumptions and practical implications.

## Global state and thread-safety

GNU `getopt` uses global variables:

- `optind`: index into argv of the next element to process
- `optarg`: pointer to the current option argument (if any)
- `optopt`: the option character that caused an error (in some error cases)
- `opterr`: controls whether `getopt` prints its own diagnostics

XGetOpt sets:

- `opterr = 0` (suppresses `getopt` printing)
- `optind = 1` (resets parsing)

Implications:

- Not thread-safe: do not parse concurrently from multiple threads.
- If your program uses `getopt` elsewhere, treat XGetOpt parsing as owning that state.

## RETURN_IN_ORDER / non-option arguments

XGetOpt enables the GNU extension `RETURN_IN_ORDER` by placing `'-'` as the first character in the short option string.

Result:

- Non-option argv elements are returned as `opt == 1`.
- `optarg` is set to the non-option argument.

This is how XGetOpt collects non-option arguments into `OptionSequence` while still using `getopt_long()`.

## Option clustering

GNU `getopt` supports clustered short options:

- `-abc` is equivalent to `-a -b -c`

Important detail:

- A single argv token can yield multiple `getopt_long()` returns.
- `optind` is not required to advance for each short option returned from a cluster; it advances when GNU `getopt` is ready to move past the current argv token.

This is why XGetOpt’s early-stop remainder logic captures the token index *before* calling `getopt_long()`.

## The `--` terminator

When the special token `--` is present:

- GNU `getopt_long()` treats it as “end of options”.
- Parsing stops, and `optind` is positioned after the `--`.

## Error behavior

GNU `getopt_long()` signals errors by returning `'?'` (and sometimes `':'` depending on configuration; XGetOpt does not enable that mode).

Common situations:

- Unknown option
- Missing required argument

XGetOpt’s `parse(...)` throws `std::runtime_error` on these errors.

XGetOpt’s `parse_until<BeforeFirstError>(...)` stops and returns a remainder beginning at the offending token.

## Portability

- `getopt_long()` is a GNU extension; not guaranteed on non-GNU platforms.
- Some libcs provide a compatible `getopt_long()` (e.g., the vast majority of Linux distributions)

If you need portability beyond this, consider providing an alternate backend or using a different argument parsing library.
