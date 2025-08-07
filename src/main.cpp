// main.cpp
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

#include <fmt/color.h>
#include <fmt/core.h>

#include "common.hpp"
#include "diagnostics/diagnostics.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "printer/ast_printer.hpp"

bool handle_flags(int argc, char *argv[], CigridFlags &flags,
                  Diagnostics &diag) {
  if (argc < 2) {
    diag.fatal("No arguments or input file provided");
    return false;
  }

  for (int i = 1; i < argc - 1; ++i) {
    std::string arg = argv[i];
    if (arg == "--pretty-print")
      flags.pretty_print = true;
    else if (arg == "--line-error")
      flags.line_error = true;
    else if (arg == "--name-analysis")
      flags.name_analysis = true;
    else if (arg == "--type-check")
      flags.type_check = true;
    else if (arg == "--debug")
      flags.debug = true;
    else if (arg == "--compile")
      flags.compile = true;
    else if (arg == "--asm-gen")
      flags.asm_gen = true;
    else if (arg == "--liveness")
      flags.liveness = true;
    else {
      diag.fatal(fmt::format("Unknown flag: {}", arg));
      return false;
    }
  }
  return true;
}

int main(int argc, char *argv[]) {
  fmt::print("Hello, World!\n");
  CigridFlags flags;
  Diagnostics diag;
  if (!handle_flags(argc, argv, flags, diag)) {
    diag.print_all();
    return 1;
  }
  std::string filename = argv[argc - 1]; // The last arg should be filename
  std::ifstream file_stream(filename);
  if (!file_stream) {
    diag.fatal(fmt::format("{}: No such file or directory", filename));
    diag.print_all();
    return 1;
  }
  Parser parser(file_stream, diag, flags);
  auto prog = parser.parse();
  diag.print_all();

  // TODO: handle flags
  if (flags.pretty_print) {
    ASTPrinter printer;
    (*prog).print(printer);
  }

  return 0;
}
