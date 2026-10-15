#pragma once

// Portable helpers for tests that re-launch their own executable through
// std::system. POSIX shells accept single-quoted arguments; Windows runs the
// command through `cmd.exe /c`, which ignores single quotes and strips the
// first and last double quote of a command containing more than two quotes.

#include <cstdlib>
#include <string>
#include <string_view>

namespace openemperor::test {

// Quotes one argument for the platform shell used by std::system.
inline std::string shell_quote(std::string_view text) {
#ifdef _WIN32
    std::string quoted = "\"";
    for (const char c : text) {
        if (c == '"') quoted += "\\\"";
        else quoted += c;
    }
    return quoted + "\"";
#else
    std::string quoted = "'";
    for (const char c : text) quoted += c == '\'' ? "'\"'\"'" : std::string(1, c);
    return quoted + "'";
#endif
}

// Runs a command whose arguments are already quoted with shell_quote (or with
// plain double quotes). On Windows the whole line is wrapped once more so that
// cmd.exe's outer-quote stripping leaves the individual quotes intact.
inline int run_command(const std::string& command) {
#ifdef _WIN32
    const std::string wrapped = "\"" + command + "\"";
    return std::system(wrapped.c_str());
#else
    return std::system(command.c_str());
#endif
}

} // namespace openemperor::test
