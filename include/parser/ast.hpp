#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "common.hpp"

// Forward
class ASTPrinter;

enum class Bop {
  NOT,
  BITWISE_NOT,
  PLUS,
  MINUS,
  MULTIPLY,
  DIVIDE,
  EXPONENTIAL,
  MODULUS,
  LESS_THAN,
  LARGER_THAN,
  LESS_EQUAL,
  LARGER_EQUAL,
  EQUAL,
  ASSIGN,
  NOT_EQUAL,
  BITWISE_AND,
  BITWISE_OR,
  LOGICAL_AND,
  LOGICAL_OR,
  SHIFT_LEFT,
  SHIFT_RIGHT
};

enum class Uop { NEG, NOT, BITWISE_NOT };

// --- TypeNode（T） ---
// T ::= TVoid | TInt | TChar | TIdent(r) | TPoint(T) (35)
struct TVoid;
struct TInt;
struct TChar;
struct TIdent;
struct TPoint;
using TypeNode = std::variant<TVoid, TInt, TChar, TIdent, TPoint>;

struct TVoid {
  Position pos;
  void print(ASTPrinter &P) const;
};
struct TInt {
  Position pos;
  void print(ASTPrinter &P) const;
};
struct TChar {
  Position pos;
  void print(ASTPrinter &P) const;
};
struct TIdent {
  Position pos;
  std::string name;
  void print(ASTPrinter &P) const;
};
struct TPoint {
  Position pos;
  std::unique_ptr<TypeNode> point_type;
  void print(ASTPrinter &P) const;
};

// --- ExprNode (e) ---
// e ::= EVar(r) | EInt(i) | EChar(c) | EString(r) (36)
// | EBinOp(bop, e, e) | EUnOp(uop, e) | ECall(r, e) (37)
// | ENew(T, e) | EArrayAccess(r, e, rˆ) (38)
struct EVar;
struct EInt;
struct EChar;
struct EString;
struct EBinOp;
struct EUnOp;
struct ECall;
struct ENew;
struct EArrayAccess;
using ExprNode = std::variant<EVar, EInt, EChar, EString, EBinOp, EUnOp, ECall,
                              ENew, EArrayAccess>;
struct EVar {
  Position pos;
  std::string name;
  void print(ASTPrinter &P) const;
};
struct EInt {
  Position pos;
  int value;
  void print(ASTPrinter &P) const;
};
struct EChar {
  Position pos;
  char value;
  void print(ASTPrinter &P) const;
};
struct EString {
  Position pos;
  std::string value;
  void print(ASTPrinter &P) const;
};
struct EBinOp {
  Position pos;
  Bop op;
  std::unique_ptr<ExprNode> lhs, rhs;
  void print(ASTPrinter &P) const;
};
struct EUnOp {
  Position pos;
  Uop op;
  std::unique_ptr<ExprNode> rhs;
  void print(ASTPrinter &P) const;
};
struct ECall {
  Position pos;
  std::string name;
  std::vector<std::unique_ptr<ExprNode>> args;
  void print(ASTPrinter &P) const;
};
struct ENew {
  Position pos;
  std::unique_ptr<TypeNode> type;
  std::unique_ptr<ExprNode> expr;
  void print(ASTPrinter &P) const;
};
struct EArrayAccess {
  Position pos;
  std::string name;
  std::unique_ptr<ExprNode> index;
  std::optional<std::string> label;
  void print(ASTPrinter &P) const;
};

// --- StmtNode（s） ---
// s ::= SExpr(e) | SVarDef(T, r, e) | SVarAssign(r, e) (39)
//     | SArrayAssign(r, e, r, e ˆ ) | SScope(s) | SIf(e, s, sˆ) (40)
//     | SWhile(e, s) | SBreak | SReturn(ˆe) | SDelete(r) (41)
struct SExpr;
struct SVarDef;
struct SVarAssign;
struct SArrayAssign;
struct SArrayPlusAssign;
struct SArrayMinusAssign;
struct SScope;
struct SIf;
struct SWhile;
struct SBreak;
struct SReturn;
struct SDelete;
using StmtNode = std::variant<SExpr, SVarDef, SVarAssign, SArrayAssign,
                              SArrayPlusAssign, SArrayMinusAssign, SScope, SIf,
                              SWhile, SBreak, SReturn, SDelete>;
struct SExpr {
  Position pos;
  std::unique_ptr<ExprNode> expr;
  void print(ASTPrinter &P) const;
};
struct SVarDef {
  Position pos;
  std::unique_ptr<TypeNode> type;
  std::string name;
  std::unique_ptr<ExprNode> value;
  void print(ASTPrinter &P) const;
};
struct SVarAssign {
  Position pos;
  std::string name;
  std::unique_ptr<ExprNode> value;
  void print(ASTPrinter &P) const;
};
struct SArrayAssign {
  Position pos;
  std::string name;
  std::unique_ptr<ExprNode> index;
  std::optional<std::string> label;
  std::unique_ptr<ExprNode> value;
  void print(ASTPrinter &P) const;
};
struct SArrayPlusAssign {
  Position pos;
  std::string name;
  std::unique_ptr<ExprNode> index;
  std::optional<std::string> label;
  std::unique_ptr<ExprNode> value;
  void print(ASTPrinter &P) const;
};
struct SArrayMinusAssign {
  Position pos;
  std::string name;
  std::unique_ptr<ExprNode> index;
  std::optional<std::string> label;
  std::unique_ptr<ExprNode> value;
  void print(ASTPrinter &P) const;
};
struct SScope {
  Position pos;
  std::vector<std::unique_ptr<StmtNode>> stmts;
  void print(ASTPrinter &P) const;
};
struct SIf {
  Position pos;
  std::unique_ptr<ExprNode> cond;
  std::unique_ptr<StmtNode> then_branch;
  std::unique_ptr<StmtNode> else_branch;
  void print(ASTPrinter &P) const;
};
struct SWhile {
  Position pos;
  std::unique_ptr<ExprNode> cond;
  std::unique_ptr<StmtNode> stmt;
  void print(ASTPrinter &P) const;
};
struct SBreak {
  Position pos;
  void print(ASTPrinter &P) const;
};
struct SReturn {
  Position pos;
  std::unique_ptr<ExprNode> expr;
  void print(ASTPrinter &P) const;
};
struct SDelete {
  Position pos;
  std::string name;
  void print(ASTPrinter &P) const;
};

// --- GlobalNode（g） ---
// g ::= GFuncDef(T, r,(T, r), s) | GFuncDecl(T, r,(T, r)) (42)
//     | GVarDef(T, r, e) | GVarDecl(T, r) | GStruct(r,(T, r)) (43)
struct GFuncDef;
struct GFuncDecl;
struct GVarDef;
struct GVarDecl;
struct GStruct;
using GlobalNode =
    std::variant<GFuncDef, GFuncDecl, GVarDef, GVarDecl, GStruct>;

struct Parameter {
  std::unique_ptr<TypeNode> type;
  std::string name;
  void print(ASTPrinter &P) const;
};

struct GFuncDef {
  Position pos;
  std::unique_ptr<TypeNode> return_type;
  std::string name;
  std::vector<Parameter> params;
  std::unique_ptr<StmtNode> stmt;
  void print(ASTPrinter &P) const;
};
struct GFuncDecl {
  Position pos;
  std::unique_ptr<TypeNode> return_type;
  std::string name;
  std::vector<Parameter> params;
  void print(ASTPrinter &P) const;
};
struct GVarDef {
  Position pos;
  std::unique_ptr<TypeNode> type;
  std::string name;
  std::unique_ptr<ExprNode> value;
  void print(ASTPrinter &P) const;
};
struct GVarDecl {
  Position pos;
  std::unique_ptr<TypeNode> type;
  std::string name;
  void print(ASTPrinter &P) const;
};
struct GStruct {
  Position pos;
  std::string name;
  std::vector<Parameter> fields;
  void print(ASTPrinter &P) const;
};

// --- Root of the AST（p） ---
struct Prog {
  // TODO: in future extension, there might be other things in program
  // other than only globals
  std::vector<std::unique_ptr<GlobalNode>> globals;
  void print(ASTPrinter &P) const;
};
