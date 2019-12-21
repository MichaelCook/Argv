# Argv

Easy command line parsing in C++

The goal is to allow easy parsing of command-line options with minimal
boilerplate.  Easy things should be easy, and harder things should be
possible.

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
`argv[argc]` is `nullptr`.

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
