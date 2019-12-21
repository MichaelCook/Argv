/*
  Easy command-line option parsing.

  Copyright (c) 2019, Michael Cook <michael@waxrat.com>
*/

#undef NDEBUG                   // enable `assert` macro
#include <cassert>

#include "Argv.hpp"
#include <stdexcept>
#include <iostream>
#include <limits>
#include <cstring>

#define HERE \
    std::clog << "ok at " << __FILE__ << " line " << __LINE__ << std::endl

#define ARGC(V) (sizeof(V) / sizeof(V[0]) - 1)

int main(int main_argc, char**)
try
{
    if (main_argc != 1)
        throw std::runtime_error("wrong usage");

    HERE;
    {
        char const* arguments[] = {
            "/usr/bin/hello_world",
            "foo.mp3",
            "--output=bar.mp3",
            "--samples=12345",
            "--seconds=123.45",
            "--debug",
            "--verbose",
            "--verbose",
            "--verbose",
            nullptr
        };
        char** argv = (char**) arguments;
        int argc = ARGC(arguments);

        Argv args(argc, argv, "Usage: {name} [options] input.mp3");

        assert(strcmp(args.name(), "hello_world") == 0);

        int samples = 100;
        double seconds = 1.0;
        bool debug = false;
        int verbosity = 0;
        char const* output = nullptr;

        while (args) {
            args.option('n', "--samples", samples, 1);
            args.option('t', "--seconds", seconds, 0.0);
            args.option('o', "--output", output);
            args.option('D', "--debug", debug);
            args.counter('v', "--verbose", verbosity);
        }

        if (argc != 2)
            args.try_help("wrong usage");
        char const* input = argv[1];

        assert(input);
        assert(strcmp(input, "foo.mp3") == 0);
        assert(output);
        assert(strcmp(output, "bar.mp3") == 0);
        assert(samples == 12345);
        assert(std::to_string(seconds) == "123.450000");
        assert(debug);
        assert(verbosity == 3);

        assert(argc == 2);
        assert(strcmp(argv[0], "/usr/bin/hello_world") == 0);
        assert(strcmp(argv[1], "foo.mp3") == 0);
        assert(argv[2] == nullptr);
    }

    // bundling
    HERE;
    {
        char const* arguments[] = {
            "bundler",
            "-abc42",
            "-vvv",
            nullptr
        };
        char** argv = (char**) arguments;
        int argc = ARGC(arguments);

        Argv args(argc, argv, "Usage: {name}");

        assert(strcmp(args.name(), "bundler") == 0);

        bool a = false;
        bool b = false;
        int c = 0;
        int v = 0;

        while (args) {
            args.option('a', nullptr, a);
            args.option('b', nullptr, b);
            args.option('c', nullptr, c);
            args.counter('v', nullptr, v);
        }

        assert(a);
        assert(b);
        assert(c == 42);
        assert(v == 3);

        assert(argc == 1);
        assert(strcmp(argv[0], "bundler") == 0);
        assert(argv[1] == nullptr);
    }

    // '=' is optional: '--output=bar.mp3' vs '--output' 'bar.mp3'
    HERE;
    {
        char const* arguments[] = {
            "equals",
            "--output=1234",
            "--input", "5678",
            nullptr
        };
        char** argv = (char**) arguments;
        int argc = ARGC(arguments);

        Argv args(argc, argv, "Usage: {name}");

        assert(strcmp(args.name(), "equals") == 0);

        int output = 0;
        int input = 0;

        while (args) {
            args.option('\0', "--output", output);
            args.option('\0', "--input", input);
        }

        assert(output == 1234);
        assert(input == 5678);

        assert(argc == 1);
        assert(strcmp(argv[0], "equals") == 0);
        assert(argv[1] == nullptr);
    }

    // A single hyphen "-" is considered a non-option.
    // A double hyphen "--" marks the end of options.
    HERE;
    {
        char const* arguments[] = {
            "hyphen",
            "one",
            "-a",
            "-",                // not an option
            "-b",
            "--",
            "two",
            "-abc",             // not an option
            "three",
            nullptr
        };
        char** argv = (char**) arguments;
        int argc = ARGC(arguments);

        Argv args(argc, argv, "Usage: {name}");

        assert(strcmp(args.name(), "hyphen") == 0);

        bool a = false;
        bool b = false;

        while (args) {
            args.option('a', nullptr, a);
            args.option('b', nullptr, b);
        }

        assert(a);
        assert(b);

        assert(argc == 6);
        assert(strcmp(argv[0], "hyphen") == 0);
        assert(strcmp(argv[1], "one") == 0);
        assert(strcmp(argv[2], "-") == 0);
        assert(strcmp(argv[3], "two") == 0);
        assert(strcmp(argv[4], "-abc") == 0);
        assert(strcmp(argv[5], "three") == 0);
        assert(argv[6] == nullptr);
    }

    HERE;
}
catch (std::exception& exc)
{
    std::clog << "caught exception: " << exc.what() << std::endl;
    return EXIT_FAILURE;
}
