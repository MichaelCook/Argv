/*
  Easy command-line option parsing.

  Copyright (c) 2019, Michael Cook <michael@waxrat.com>
*/

#include "Argv.hpp"
#include <iostream>
#include <cassert>
#include <stdexcept>

int main(int argc, char** argv)
try
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

    char* output = nullptr;
    double period = 10.0;
    bool debug = false;
    int verbosity = 0;
    while (args) {
        args.option('o', "--output", output);
        args.option('p', "--period", period);
        args.option('d', "--debug", debug);
        args.counter('v', "--verbose", verbosity);
    }

    /*
      Now argv contains only the non-option arguments.
      argv[0] is the program name.
      argv[i] is the i'th non-option argument.
      argv[argc] is nullptr.
     */

    if (argc < 2)
        args.try_help("wrong usage");

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
catch (std::exception& exc)
{
    std::clog << "caught: " << exc.what() << std::endl;
    return EXIT_FAILURE;
}
