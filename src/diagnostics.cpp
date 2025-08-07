#include "diagnostics/diagnostics.hpp"
using enum Severity;

void Diagnostics::error(Position pos, std::string message) {
  messages.push_back(DiagMessage(Error, std::move(message), pos));
  error_count++;
}

bool Diagnostics::has_errors() const { return error_count > 0; }

void Diagnostics::fatal(std::string message) {
  messages.push_back(DiagMessage(Fatal, std::move(message)));
  fatal_count++;
}

void Diagnostics::print_all() {
  for (const auto &msg : messages) {
    std::string level_string;
    fmt::terminal_color color;

    switch (msg.level) {
    case Note:
      level_string = "note:";
      color = fmt::terminal_color::blue;
      break;
    case Warning:
      level_string = "warning:";
      color = fmt::terminal_color::yellow;
      break;
    case Error:
      level_string = "error:";
      color = fmt::terminal_color::red;
      break;
    case Fatal:
      level_string = "fatal error:";
      color = fmt::terminal_color::red;
      break;
    default:
      level_string = "unknown:";
      color = fmt::terminal_color::white;
      break;
    }
    std::string location = "zzc_cigrid"; // Default location
    // pos is std::optional
    if (msg.pos) {
      int line = msg.pos.value().line;
      int column = msg.pos.value().column;
      // Only accept single file here, so named as input_file
      auto location = fmt::format("{}:{}:{}", "input_file", line, column);
    }
    fmt::print(stderr, "{}: {} {}\n",
               fmt::styled(location, fmt::emphasis::bold),
               fmt::styled(level_string, fmt::fg(color) | fmt::emphasis::bold),
               msg.message);
  }

  if (fatal_count > 0) {
    fmt::print(stderr, "Compileation terminated.\n");
  }
}