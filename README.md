# Argv

Easy command line parsing in C++

The goal is to allow easy parsing of command-line options with minimal
boilerplate and low runtime overhead (in time & space).
Easy things should be easy, and harder things should be possible.

Example:

```
#include "Argv.hpp"
#include <iostream>
#include <cassert>

int main(int argc, char** argv)
{
    Argv args(argc, argv, R"(
Usage: {name} [options] foo....

Options:

--output (-o) FILE
    Write the results to FILE

--period (-p) SECONDS
    Duration in each period.  Units are floating point seconds

--debug (-d)
    Enable debug logging

--verbose (-v)
    Be verbose.  Multiple occurrences increase verbosity

)");

    char const* output = nullptr;
    double period = 10.0;
    bool debug = false;
    int verbosity = 0;
    while (args) {
        args.option('o', "--output", output);
        args.option('p', "--period", period);
        args.option('d', "--debug", debug);
        args.counter('v', "--verbose", verbosity);
    }
```

That all that's needed for parsing.

After the `while` loop, `argv` contains only the non-option arguments.
`argv[0]` is the program name.  `argv[i]` is the i'th non-option argument.

The help text (the 3rd argument to the Argv constructor) is merely informative.
Unlike some other argument parsers, Argv does not try to interpret this text.
If Argv finds the `--help` or `-h` options, Argv writes this help text to
stdout and then invokes `exit(0)`.  Argv replaces `{name}` with the
application's name (the basename of `argv[0]` which is also available
as `args.name()`).

Next we can verify we were passed the right number of non-option arguments:
```
    if (argc < 2)
        args.try_help("wrong usage");
```

Then proceed with whatever the application would do.
For example:
```
    if (output)
        std::cout << "Output: " << output << '\n';
    else
        std::cout << "No output specified\n";

    std::cout << "Period: " << period << '\n';
    if (debug)
        std::cout << "Debug logging enabled\n";
    std::cout << "Verbosity: " << verbosity << '\n';

    for (int i = 0; i < argc; ++i)
        std::cout << "argv[" << i << "]=" << argv[i] << '\n';
    assert(argv[argc] == nullptr);
}
```

Running this example:
```
$ g++ -Wall -Werror -std=c++17 ex1.cpp Argv.cpp
$ ./a.out -vvv -o hello world
Output: hello
Period: 10
Verbosity: 3
argv[0]=./a.out
argv[1]=world
$
$
$ ./a.out --help

Usage: a.out [options] foo....

Options:

--output (-o) FILE
    Write the results to FILE

--period (-p) SECONDS
    Duration in each period.  Units are floating point seconds

--debug (-d)
    Enable debug logging

--verbose (-v)
    Be verbose.  Multiple occurrences increase verbosity

$
```

Short options can be bundled: `-dvvv` is the same `-d -v -v -v`.

For options that require an additional argument, the argument can be separated
or not.  `-ofoo` versus `-o foo`.  Or `--output=foo` versus `--output foo`.

For arithmetic options (integer or floating point), you may specify lower and
upper bounds:

```
    int count = 1;
...
    args.option('n', "--count", count, 1, 10);
```

With `--count=42`, for example, the program would throw `std::runtime_error`.

For more complex validation, the `option` functions return true to indicate
when an option was matched:

```
    if (args.option('x', "--extra", extra)) {
        do_more_validation(extra);
    }
```

Options may appear in any order, before or after non-option arguments.

A double-hyphen `--` ends argument processing.  Any arguments after the `--`
are not considered options even if they begin with a hyphen.

A single hyphen `-` is always considered to be a non-option argument.

## Memory Considerations

The `Argv` class assumes the `argv` argument is the traditional array of
pointers to legacy C strings.  `Argv` will modify the `argv` array to remove
option arguments (and update `argc` accordingly).  `Argv` always maintains the
condition that `argv[argc]==nullptr`.  `Argv` never modifies the pointed-to
strings of `argv[]`.

You may safely destroy the `Argv` object after parsing is complete.
The results of the parsing remain valid.
In particular:

* the `argv` array remains valid
* all of the `argv[]` pointers remain valid
* the value returned from `Argv::name` remains valid -- it points at or into the `argv[0]` string
* a `char*` gotten from `option` remains valid -- it points at or into one of the original `argv[]` strings

The `help_text` argument to the `Argv` constructor must remain valid at least
until the `Argv` object is destroyed.  For example, don't do this:

```
    Argv args(argc, argv, std::string{...}.c_str()); // don't do this
```

If you must construct the help text at runtime, use a `std::string` object
whose lifetime is greater than the `Argv` object's.

```
    std::string help_text{ ... };
    Argv args(argc, argc, help_text.c_str());
```
