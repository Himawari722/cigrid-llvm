#pragma once

#include <string>
#include <vector>

#include <fmt/core.h>   
#include <fmt/color.h>

// Could add severity level in the future
enum class Severity { Note, Warning, Error, Fatal };


class Diagnostics {
public:
    void error(int line, int column, std::string message);
    bool has_errors() const;
    // TODO: Only used for file not found for now
    void fatal(std::string file_name);
    void print_all();

private:
    // TODO: just realized that not all diagmessage has line and column: file not found e.g.
    struct DiagMessage {
        Severity level;
        int line;
        int column;
        std::string message;
    };

    std::vector<DiagMessage> messages;
    int note_count = 0;
    int error_count = 0;
    int warning_count = 0;
    int fatal_count = 0;
};