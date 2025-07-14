// main.cpp
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

#include <fmt/color.h> 
#include <fmt/core.h>

#include "lexer/lexer.hpp"
#include "diagnostics/diagnostics.hpp"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "No arguments provided.\n"
              << "Usage:" << argv[0] << " [--flags] <filename>\n";
    return 1;
  }

  Diagnostics diag;

  std::vector<std::string> flags;
  std::string filename = argv[argc - 1]; // The last arg should be filename
  
  std::ifstream file(filename);
  if (!file) {
    diag.fatal(fmt::format("{}: No such file or directory", filename));
    diag.print_all();
    return 1;
  }

  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

  Lexer lexer(content, diag);
  auto token_list = lexer.gen_token();
  lexer.print_token_list();
  diag.print_all();

  // TODO: handle flags

  return 0;
}
