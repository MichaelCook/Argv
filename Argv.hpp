/*
  Easy command-line option parsing.

  Copyright (c) 2019, Michael Cook <michael@waxrat.com>
*/

#pragma once

#include <type_traits>
#include <limits>
#include <string>
#include <cstdlib>

class Argv {

    int& argc_;
    char** argv_;
    char const* help_text_ = nullptr;
    char const* name_ = nullptr;
    int argi_ = 0;
    bool handling_option_ = false;

    // Non-null if we're working our way through a bundle of short options
    char* bundle_ = nullptr;

    char* peek_arg() const;
    void shift_arg();
    bool get_opt(char short_opt);
    char* get_opt_with_arg(char short_opt);
    bool get_opt(char const* long_opt);
    char* get_opt_with_arg(char const* long_opt);
    void bad_arg(char short_opt, char const*);
    void bad_arg(char const* long_opt, char const*);

    template <typename T,
              typename MINT,
              typename MAXT,
              typename std::enable_if<std::is_integral<T>::value &&
                                      std::is_signed<T>::value>::type* = nullptr>
    static bool string_to(char const* s, T& value, MINT minimum, MAXT maximum)
    {
        errno = 0;
        char* endp;
        auto v = ::strtoll(s, &endp, 10);
        if (errno != 0 || endp == s || *endp != '\0')
            return false;
        T r = v;
        if (r != v)
            return false;
        if (r < minimum || r > maximum)
            return false;
        value = r;
        return true;
    }

    static unsigned long long nonnegative_strtoull(char const*, char **, int);

    template <typename T,
              typename MINT,
              typename MAXT,
              typename std::enable_if<std::is_integral<T>::value &&
                                      std::is_unsigned<T>::value>::type* = nullptr>
    static bool string_to(char const* s, T& value, MINT minimum, MAXT maximum)
    {
        errno = 0;
        char* endp;
        auto v = nonnegative_strtoull(s, &endp, 10);
        if (errno != 0 || endp == s || *endp != '\0')
            return false;
        T r = v;
        if (r != v)
            return false;
        if (r < minimum || r > maximum)
            return false;
        value = r;
        return true;
    }

    template <typename T,
              typename MINT,
              typename MAXT,
              typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr>
    static bool string_to(char const* s, T& value, MINT minimum, MAXT maximum)
    {
        errno = 0;
        char* endp;
        auto v = ::strtold(s, &endp);
        if (errno != 0 || endp == s || *endp != '\0')
            return false;
        if (v < minimum || v > maximum)
            return false;
        value = v;
        return true;
    }

public:

    Argv(int& argc, char** argv, char const* help_text);

    // true if there are any more option arguments to process.
    // May modify `argv`.
    // Handles any `--help`, `-h` or `--` arguments.
    explicit operator bool();

    // The program name (the base name of argv[0])
    char const* name() const {
        return name_;
    }

    // Write the given text to stderr along with "Try --help"
    // and then throw std::runtime_error.
    void try_help(std::string const&) const __attribute__((noreturn));
    void try_help(char const*) const __attribute__((noreturn));
    void show_help() const __attribute__((noreturn));

    // Consume a `bool` or `char*` option.
    //
    // If successful, remove the option from the argument list, assign the
    // given `value` argument, and return true.  Otherwise, merely return
    // false
    //
    // `short_opt` may be '\0' to indicate there is no short form.
    // `long_opt` may be nullptr to indicate no long form.
    bool option(char short_opt, char const* long_opt, bool& value);
    bool option(char short_opt, char const* long_opt, char const*& value);
    bool option(char short_opt, char const* long_opt, char*& value);

    // Like `option` but increment `value`
    bool counter(char short_opt, char const* long_opt, int& value);

    // Consume an arithmetic option (integer or floating point).
    //
    // The converted arithmetic value must be between `minimum` and `maximum`
    // (inclusive), or else this function throws std::runtime_error.
    template <typename T,
              typename MINT = T,
              typename MAXT = T,
              typename std::enable_if<std::is_arithmetic<T>::value>* = nullptr>
    bool option(char short_opt, char const* long_opt, T& value,
                MINT minimum = std::numeric_limits<T>::min(),
                MAXT maximum = std::numeric_limits<T>::max())
    {
        auto arg = get_opt_with_arg(short_opt);
        if (arg) {
            if (!string_to(arg, value, minimum, maximum))
                bad_arg(short_opt, arg);
            return true;
        }
        arg = get_opt_with_arg(long_opt);
        if (arg) {
            if (!string_to(arg, value, minimum, maximum))
                bad_arg(long_opt, arg);
            return true;
        }
        return false;
    }
};
