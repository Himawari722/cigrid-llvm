#include "diagnostics/diagnostics.hpp"

void Diagnostics::error(int line, int column, std::string message) {
    // fmt args?
    messages.push_back(DiagMessage(Severity::Error, line, column, message));
    error_count++;
}

bool Diagnostics::has_errors() const {
    return error_count > 0;
}

void Diagnostics::fatal(std::string message) {
    messages.push_back(DiagMessage(Severity::Fatal, 0, 0, message));
}

void Diagnostics::print_all() {
    for (const auto& msg : messages) {
        std::string level_string;
        fmt::terminal_color color;

        switch (msg.level) {
            // TODO: Note not implemented
            case Severity::Warning:
                level_string    = "warning:";
                color           = fmt::terminal_color::yellow;
                break;
            case Severity::Error:
                level_string    = "error:";
                color           = fmt::terminal_color::red;
                break;
            case Severity::Fatal:
                level_string    = "fatal error:";
                color           = fmt::terminal_color::red;
                break;
        }
        // Only accept single file here
        auto location = fmt::format("{}:{}:{}", "input_file", msg.line, msg.column);

        fmt::print(stderr,
            "{}: {} {}\n",
            fmt::styled(location, fmt::emphasis::bold),
            fmt::styled(level_string,
                        fmt::fg(color)  | fmt::emphasis::bold),
            msg.message
        );
    }
}