#pragma once

struct Position {
  int line;
  int column;
};

struct CigridFlags {
  bool pretty_print = false;
  bool line_error = false;
  bool name_analysis = false;
  bool type_check = false;
  bool debug = false;
  bool compile = false;
  bool asm_gen = false;
  bool liveness = false;
};

// Overload template to visit std::variant types
template <class... Ts> struct overload : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overload(Ts...) -> overload<Ts...>;