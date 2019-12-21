/*
  Easy command-line option parsing.

  Copyright (c) 2019, Michael Cook <michael@waxrat.com>
*/

#include "Argv.hpp"
#include <stdexcept>
#include <cstring>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <iostream>

Argv::Argv(int& argc, char**& argv, char const* help_text)
: argc_(argc),
  argv_(argv),
  help_text_(help_text)
{
    if (argc < 1 || !argv[0])
        throw std::runtime_error("invalid arguments");

    name_ = strrchr(argv[0], '/');
    if (name_)
        ++name_;
    else
        name_ = argv[0];

    argi_ = 1;
}

Argv::operator bool()
{
    bool ok;
    if (option('h', "--help", ok)) {
        // Show the help text and then exit.
        // Replace "{name}" the `name_` in the help text.
        static char const pattern[] = "{name}";
        auto text = help_text_;
        while (char const* p = strstr(text, pattern)) {
            std::cout.write(text, p - text);
            std::cout << name_;
            text = p + sizeof(pattern) - 1;
        }
        std::cout << text;
        exit(0);
    }

    if (handling_option_) {
        if (bundle_)
            try_help(std::string("unknown option: -") + *bundle_);
        assert(argi_ < argc_);
        auto arg = argv_[argi_];
        try_help("unknown option: " + std::string(arg));
    }

    if (bundle_)
        return true;

    // Skip past all non-option arguments
    while (auto arg = peek_arg()) {
        if (arg[0] != '-') {
            // Not an option. Skip over it and continue processing
            ++argi_;
            continue;
        }
        if (arg[1] == '\0') {
            // "-" is not an option. Skip over it and continue processing
            ++argi_;
            continue;
        }
        if (arg[1] == '-' && arg[2] == '\0') {
            // Discard "--" and stop processing
            shift_arg();
            break;
        }
        // Argument appears to be an option
        handling_option_ = true;
        return true;
    }
    return false;
}

void Argv::try_help(std::string const& msg)
{
    try_help(msg.c_str());
}

void Argv::try_help(char const* msg)
{
    std::clog << name() << ": " << msg
              << "\nTry '" << name() << " --help' for more information."
              << std::endl;
    throw std::runtime_error(msg);
}

char const* Argv::peek_arg() const
{
    if (argi_ < argc_)
        return argv_[argi_];
    return nullptr;
}

void Argv::shift_arg()
{
    if (argi_ >= argc_)
        throw std::runtime_error("invalid shift_arg");

    /* Remove argument argi_ and shift all others down.

       Example:
        argc=8
        argi=3
        0 1 2 3 4 5 6 7 N
        - - - 4 5 6 7 N
        N is the final nullptr
     */
    std::copy_backward(argv_ + argi_ + 1, // from_begin
                       argv_ + argc_ + 1, // from_end (+1 to include N)
                       argv_ + argc_);    // to_end
    --argc_;
}

bool Argv::get_opt(char short_opt)
{
    if (!bundle_) {
        auto arg = peek_arg();
        if (!arg)
            return false;

        if (*arg != '-')
            return false;

        ++arg;
        if (*arg == '-')        // "--" or long option (not a short option)
            return false;
        if (*arg == '\0')       // "-"
            return false;

        shift_arg();
        bundle_ = arg;
    }
    if (*bundle_ != short_opt)
        return false;
    handling_option_ = false;
    if (*++bundle_ == '\0')
        bundle_ = nullptr;
    return true;
}

char const* Argv::get_opt_with_arg(char short_opt)
{
    if (!bundle_) {
        auto arg = peek_arg();
        if (!arg)
            return nullptr;

        if (*arg != '-')
            return nullptr;

        ++arg;
        if (*arg == '-')        // "--" or long option (not a short option)
            return nullptr;
        if (*arg == '\0')       // "-"
            return nullptr;

        shift_arg();
        bundle_ = arg;
    }

    if (*bundle_ != short_opt)
        return nullptr;
    handling_option_ = false;

    if (*++bundle_ != '\0') {
        auto arg = bundle_;
        bundle_ = nullptr;
        return arg;
    }
    bundle_ = nullptr;

    auto arg = peek_arg();
    if (!arg)
        throw std::runtime_error(std::string("expected argument for option -") + short_opt);
    shift_arg();
    return arg;
}

bool Argv::get_opt(char const* long_opt)
{
    if (bundle_)
        return false;

    if (!long_opt)
        return false;

    auto arg = peek_arg();
    if (!arg)
        return false;

    if (strcmp(arg, long_opt) == 0) {
        shift_arg();
        handling_option_ = false;
        return true;
    }

    return false;
}

char const* Argv::get_opt_with_arg(char const* long_opt)
{
    if (bundle_)
        return nullptr;

    if (!long_opt)
        return nullptr;

    auto arg = peek_arg();
    if (!arg)
        return nullptr;

    if (strcmp(arg, long_opt) == 0) {
        shift_arg();
        arg = peek_arg();
        if (!arg)
            throw std::runtime_error(std::string("expected argument for option ") + long_opt);
        shift_arg();
        handling_option_ = false;
        return arg;
    }

    auto len = strlen(long_opt);
    if (strncmp(arg, long_opt, len) == 0 && arg[len] == '=') {
        shift_arg();
        handling_option_ = false;
        return arg + len + 1;
    }

    return nullptr;
}

bool Argv::option(char short_opt, char const* long_opt, bool& value)
{
    if (get_opt(short_opt)) {
        value = true;
        return true;
    }
    if (get_opt(long_opt)) {
        value = true;
        return true;
    }
    return false;
}

bool Argv::counter(char short_opt, char const* long_opt, int& value)
{
    if (get_opt(short_opt)) {
        ++value;
        return true;
    }
    if (get_opt(long_opt)) {
        ++value;
        return true;
    }
    return false;
}

bool Argv::option(char short_opt, char const* long_opt, char const*& value)
{
    auto arg = get_opt_with_arg(short_opt);
    if (arg) {
        value = arg;
        return true;
    }
    arg = get_opt_with_arg(long_opt);
    if (arg) {
        value = arg;
        return true;
    }
    return false;
}

void Argv::bad_arg(char short_opt, char const* arg)
{
    throw std::runtime_error("invalid argument '" + std::string(arg) + "' for option -" + short_opt);
}

void Argv::bad_arg(char const* long_opt, char const* arg)
{
    throw std::runtime_error("invalid argument '" + std::string(arg) + "' for option " + long_opt);
}

/* ::strtoull allows for a leading minus sign.
   We want to reject that.

   "If the minus sign was part of the input sequence, the numeric value
   calculated from the sequence of digits is negated as if by unary minus
   in the result type, which applies unsigned integer wraparound rules."

   https://en.cppreference.com/w/cpp/string/byte/strtoul */

unsigned long long Argv::positive_strtoull(char const* nptr, char** endptr, int base)
{
    // keep the original nptr value so *endptr gets set right
    auto p = nptr;
    while (isspace((unsigned char) *p))
        ++p;
    if (*p == '-') {
        errno = EINVAL;
        return 0;
    }
    return ::strtoull(nptr, endptr, base);
}
