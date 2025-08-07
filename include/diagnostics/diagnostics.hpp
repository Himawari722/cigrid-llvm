#pragma once

#include <optional>
#include <string>
#include <vector>

#include <fmt/color.h>
#include <fmt/core.h>

#include "common.hpp"

// Could add severity level in the future
enum class Severity { Note, Warning, Error, Fatal };

class Diagnostics {
public:
  void error(Position pos, std::string message);
  bool has_errors() const;
  void fatal(std::string message);
  void print_all();

private:
  // TODO: just realized that not all diagmessage has line and column: file not
  // found e.g.
  struct DiagMessage {
    Severity level;
    std::string message;
    std::optional<Position> pos;
  };

  std::vector<DiagMessage> messages;
  int note_count = 0;
  int error_count = 0;
  int warning_count = 0;
  int fatal_count = 0;
};