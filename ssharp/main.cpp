// file: ssharp/main.cpp
// by  : jooh@cuni.cz
// for : nprg041
// lic.: mit

////////////////////////////////////////////////////////////////////////
// directives

#include <algorithm>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <vector>

using std::map;
using std::pair;
using std::regex;
using std::set;
using std::string;
using std::vector;

#define DEBUG true
#define REP_LIM 256

////////////////////////////////////////////////////////////////////////
// "S"ymbol class
//
// - A symbol has "type", classifying its grammatic repetition behavior,
//   and an "id" that refers either to an atomic identifier or another
//   symbol.
// - Used in grammar rules.

struct S
{
  int type; // 0 = non-terminal; 1 = terminal
  int rep; // 0 = s; 1 = s*; 2 = s+; -1 = err
  string id;
  S(int t, int r, string i): type(t), rep(r), id(i) {}
};

struct Node
{
  string id;
  vector<Node*> children;
  Node(string i, vector<Node*> cs): id(i), children(cs) {}
};

////////////////////////////////////////////////////////////////////////
// "Atoms" typedef
//
// - A mapping from an atomic identifier to a regex pattern.
// - eg. "N" -> "[0-9]+"

typedef map<string,regex> Atoms;

namespace A
{
  bool
  has(const Atoms &a, const string k)
  {
    return a.find(k) != a.end();
  }

  regex
  lookup(const Atoms &a, const string k)
  {
    return a.at(k);
  }

  void
  add(Atoms &mut_a, const string k, const string pat)
  {
    if (has(mut_a, k))
    {
      std::cerr << "'" << k << "' atom already exists" << std::endl;
      return;
    }
    mut_a.insert(pair<string,regex>(k, regex(pat)));
  }

  void
  print(const Atoms &a)
  {
    string sep = "";
    if (DEBUG)
    {
      std::cout << "DBG ATOMS:\nDBG ";
    }
    for (const auto entry: a)
    {
      // ignore whitespace atoms
      if (!isspace(entry.first.front()))
      {
        if (DEBUG)
        {
          std::cout << sep << entry.first;
          sep = " ";
        }
      }
    }
    if (DEBUG)
    {
      std::cout << "\nDBG" << std::endl;
    }
  }
}

////////////////////////////////////////////////////////////////////////
// "Grammar" typedef
//
// - A list of grammatical rule definitions.
// - A definition is a pair of <string> and <vector<vector<S>>>, where
//   the <string> represents the class being defined, and <vector...>
//   represents a list of alternative rules.
// - eg.
//   foo := bar baz* | qux
//   ^   ^  ^   ^    ^ ^
//   |   |  `---|    | `-alternative rule containing single a symbol
//   |   |      |    `-separates alternatives
//   |   |      `-alternative rule containing two symbols
//   |   `-this definition is therefore a list of list of symbols
//   `-class being defined

typedef vector<pair<string,vector<vector<S>>>> Grammar;

namespace G
{
  bool
  has(const Grammar &g, const string k)
  {
    for (const auto rule: g)
    {
      if (k == rule.first)
      {
        return true;
      }
    }
    return false;
  }

  pair<string,vector<vector<S>>>
  lookup(const Grammar &g, const string k)
  {
    for (const auto rule: g)
    {
      if (k == rule.first)
      {
        return rule;
      }
    }
    return pair<string,vector<vector<S>>> {};
  }

  void
  add(Grammar &mut_g, const string k, const vector<vector<S>> rule)
  {
    if (has(mut_g, k))
    {
      std::cerr << "grammar already has rule for '" << k << "'"
                << std::endl;
      return;
    }
    mut_g.push_back(pair<string,vector<vector<S>>>(k, rule));
  }

  void
  print(const Grammar &g, const Atoms &a)
  {
    string rule;
    string altsep;
    string alt;
    string symsep;
    string s;

    if (DEBUG)
    {
      std::cout << "DBG GRAMMAR RULES:" << std::endl;
    }
    for (const auto entry: g)
    {
      rule = "";
      altsep = "";
      for (const auto syms: entry.second)
      {
        alt = "";
        symsep = "";
        for (const auto sym: syms)
        {
          s = sym.id;
          if (A::has(a, sym.id))
          {
            s = "'" + s + "'";
          }
          alt += symsep + s;
          if (sym.rep == 1)
          {
            alt += "*";
          }
          else if (sym.rep == 2)
          {
            alt += "+";
          }
          symsep = " ";
        }
        rule += altsep + alt;
        altsep = " | ";
      }
      if (DEBUG)
      {
        std::cout << "DBG " << entry.first << ": " << rule << std::endl;
      }
    }
    if (DEBUG)
    {
      std::cout << "DBG " << std::endl;
    }
  }
}

////////////////////////////////////////////////////////////////////////
// "Transpiler" class
//
// - The main program structure that holds grammatical information.

struct Transpiler
{
  Atoms atoms;
  Grammar grammar;
};

////////////////////////////////////////////////////////////////////////
// functions

// lex
// - Convert a string into a list of strings, where each string is
//   either a lexable token (ie. an entry in Atoms) or some
//   non-whitespace string.
void
lex(const Transpiler &tr, string mut_s, vector<string> &tokens)
{
  int found;
  size_t min;
  string lexable;
  string lexed;

  if (DEBUG)
  {
    std::cout << "DBG --------lex" << std::endl;
    std::cout << "DBG " << mut_s << std::endl;
  }

  // keep chopping the front off of input
  while (!mut_s.empty())
  {
    found = -1;           // remains -1 if finds no lexable
    min = mut_s.length(); // record earliest position of a lexable
    lexed = "";           // the earliest lexable

    // try searching for every lexable
    for (const auto atom: tr.atoms)
    {
      lexable = atom.first;
      found = mut_s.find(lexable);
      // a lexable found at the front
      if (found == 0)
      {
        lexed = lexable;
        min = found;
        break;
      }
      // the earliest lexable found
      else if (found > 0 && found < min)
      {
        lexed = lexable;
        min = found;
      }
    }

    // if no lexable found
    if (lexed.empty())
    {
      tokens.push_back(mut_s);
      mut_s = "";
      continue;
    }

    // if lexable is at the front
    if (min == 0)
    {
      if (!isspace(lexed.front()))
      {
        tokens.push_back(lexed);
      }
      mut_s = mut_s.substr(lexed.length());
    }

    // if earliest lexable not at the front
    else if (min > 0)
    {
      tokens.push_back(mut_s.substr(0, min));
      if (!isspace(lexed.front()))
      {
        tokens.push_back(mut_s.substr(min, lexed.length()));
      }
      mut_s = mut_s.substr(min + lexed.length());
    }
  }

  if (DEBUG)
  {
    for (const auto token: tokens)
    {
      std::cout << "DBG --" << token << "--" << std::endl;
    }
  }
}

// Global cursor over lexed tokens. Used for parsing.
static vector<string>::iterator look;
static vector<pair<string,string>> acc;

bool
match(const Atoms &a, const string s)
{
  if (DEBUG)
  {
    std::cout << "DBG matching " << *look << " - " << s << " ... ";
  }
  if (*look == s)
  {
    if (DEBUG)
    {
      std::cout << "matched" << std::endl;
    }
    ++look;
    return true;
  }

  if (A::has(a, s)
      && std::regex_match(*look, A::lookup(a, s)))
  {
    if (DEBUG)
    {
      std::cout << "matched" << std::endl;
    }
    ++look;
    return true;
  }

  if (DEBUG)
  {
    std::cout << std::endl;
  }
  return false;
}

bool
nomatch(const string s)
{
  return *look != s;
}

// Various predictive parsing helper functions follow.
// Basic strategy:
// - If the token under the "look" cursor ought to refer to a
//   non-terminal, invoke the corresponding parse_*(). Otherwise, it
//   ought to refer to a terminal, so invoke match().
// - Recursively return success or failure and every success pushes
//   to a "success accummulator"
// TODO: Redundant with the Transpiler.grammar structure. Should use
//       one or the other.
// TODO: Currently has problems dealing with repeated binary operators.

bool parse_stmt  (vector<string>::iterator b,
                  const Atoms &a, const Grammar &g);
bool parse_cond  (vector<string>::iterator b,
                  const Atoms &a, const Grammar &g);
bool parse_call  (vector<string>::iterator b,
                  const Atoms &a, const Grammar &g);
bool parse_def   (vector<string>::iterator b,
                  const Atoms &a, const Grammar &g);
bool parse_grps  (vector<string>::iterator b,
                  const Atoms &a, const Grammar &g);
bool parse_grpc  (vector<string>::iterator b,
                  const Atoms &a, const Grammar &g);
bool parse_grpn  (vector<string>::iterator b,
                  const Atoms &a, const Grammar &g);
bool parse_stmts (vector<string>::iterator b,
                  const Atoms &a, const Grammar &g);
bool parse_exprc (vector<string>::iterator b,
                  const Atoms &a, const Grammar &g);
bool parse_namec (vector<string>::iterator b,
                  const Atoms &a, const Grammar &g);
bool parse_expr  (vector<string>::iterator b,
                  const Atoms &a, const Grammar &g);
bool parse_unary (vector<string>::iterator b,
                  const Atoms &a, const Grammar &g);
bool parse_binary(vector<string>::iterator b,
                  const Atoms &a, const Grammar &g);
bool parse_name  (vector<string>::iterator b,
                  const Atoms &a, const Grammar &g);
bool parse_fact   (vector<string>::iterator b,
                  const Atoms &a, const Grammar &g);

bool
parse_stmt(vector<string>::iterator b,
           const Atoms &a,
           const Grammar &g)
{
  look = b;
  if (parse_cond(b, a, g)
      || parse_call(b, a, g)
      || parse_def(b, a, g)
      || parse_expr(b, a, g))
  {
    acc.push_back({*look, "stmt"});
    return true;
  }
  return false;
}

bool
parse_cond(vector<string>::iterator b,
           const Atoms &a,
           const Grammar &g)
{
  look = b;
  if (match(a, "if")
      && match(a, "(")
      && parse_expr(look, a, g)
      && match(a, ")")
      && match(a, "{")
      && parse_grps(look, a, g)
      && match(a, "}")
      && match(a, "{")
      && parse_grps(look, a, g)
      && match(a, "}"))
  {
    acc.push_back({*look, "cond"});
    return true;
  }
  return false;
}

bool
parse_call(vector<string>::iterator b,
           const Atoms &a,
           const Grammar &g)
{
  look = b;
  if (nomatch("main")
      && parse_name(look, a, g)
      && match(a, "(")
      && parse_grpc(look, a, g)
      && match(a, ")"))
  {
    acc.push_back({*look, "call"});
    return true;
  }
  return false;
}

bool
parse_def(vector<string>::iterator b,
          const Atoms &a,
          const Grammar &g)
{
  look = b;
  if (match(a, "main")
      && match(a, "{")
      && parse_grps(look, a, g)
      && match(a, "}"))
  {
    // disjunction
    acc.push_back({*look, "def main"});
    return true;
  }
  if (parse_name(b, a, g)
      && parse_grpn(look, a, g)
      && match(a, "{")
      && parse_grps(look, a, g)
      && match(a, "}"))
  {
    acc.push_back({*look, "def name"});
    return true;
  }
  return false;
}

bool
parse_grps(vector<string>::iterator b,
           const Atoms &a,
           const Grammar &g)
{
  look = b;
  size_t rep_count = 0;
  vector<string>::iterator innerback;
  while (*look != "}" && rep_count < REP_LIM)
  {
    innerback = look;
    parse_stmts(look, a, g);
  }
  if (parse_stmt(innerback, a, g))
  {
    acc.push_back({*look, "grps"});
    return true;
  }
  return false;
}

bool
parse_grpc(vector<string>::iterator b,
           const Atoms &a,
           const Grammar &g)
{
  look = b;
  size_t rep_count = 0;
  vector<string>::iterator innerback;
  while (*look != ")" && rep_count < REP_LIM)
  {
    innerback = look;
    parse_exprc(look, a, g);
  }
  if (parse_expr(innerback, a, g))
  {
    acc.push_back({*look, "grpc"});
    return true;
  }
  return false;
}

bool
parse_grpn(vector<string>::iterator b,
           const Atoms &a,
           const Grammar &g)
{
  look = b;
  size_t rep_count = 0;
  vector<string>::iterator innerback;
  while (*look != "{" && rep_count < REP_LIM)
  {
    innerback = look;
    parse_namec(look, a, g);
  }
  if (parse_name(innerback, a, g))
  {
    acc.push_back({*look, "grpn"});
    return true;
  }
  return false;
}

bool
parse_stmts(vector<string>::iterator b,
            const Atoms &a,
            const Grammar &g)
{
  look = b;
  if (parse_stmt(look, a, g)
      && match(a, ";"))
  {
    acc.push_back({*look, "stmts"});
    return true;
  }
  return false;
}

bool
parse_exprc(vector<string>::iterator b,
            const Atoms &a,
            const Grammar &g)
{
  look = b;
  if (parse_expr(look, a, g)
      && match(a, ","))
  {
    acc.push_back({*look, "exprc"});
    return true;
  }
  return false;
}

bool
parse_namec(vector<string>::iterator b,
            const Atoms &a,
            const Grammar &g)
{
  look = b;
  if (parse_name(look, a, g)
      && match(a, ","))
  {
    acc.push_back({*look, "namec"});
    return true;
  }
  return false;
}

bool
parse_expr(vector<string>::iterator b,
           const Atoms &a,
           const Grammar &g)
{
  look = b;
  if ((parse_unary(b, a, g)
       && parse_fact(look, a, g))
      || (parse_fact(b, a, g)
          && parse_binary(look, a, g)
          && parse_fact(look, a, g))
      || parse_fact(b, a, g))
  {
    acc.push_back({*look, "expr"});
    return true;
  }
  return false;
}

bool
parse_unary(vector<string>::iterator b,
            const Atoms &a,
            const Grammar &g)
{
  look = b;
  if (match(a, "~"))
  {
    acc.push_back({*look, "unary"});
    return true;
  }
  return false;
}

bool
parse_binary(vector<string>::iterator b,
             const Atoms &a,
             const Grammar &g)
{
  look = b;
  if (match(a, "+")
      || match(a, "-")
      || match(a, "*")
      || match(a, "/")
      || match(a, "%")
      || match(a, "<")
      || match(a, ">")
      || match(a, "==")
      || match(a, "!=")
      || match(a, "&&")
      || match(a, "||"))
  {
    acc.push_back({*look, "binary"});
    return true;
  }
  return false;
}

bool
parse_name(vector<string>::iterator b,
           const Atoms &a,
           const Grammar &g)
{
  look = b;
  if (match(a, "write")
      || match(a, "read")
      || match(a, "S"))
  {
    acc.push_back({*look, "name"});
    return true;
  }
  return false;
}

bool
parse_fact(vector<string>::iterator b,
           const Atoms &a,
           const Grammar &g)
{
  look = b;
  if (match(a, "N")
      || match(a, "S"))
  {
    acc.push_back({*look, "fact"});
    return true;
  }
  return false;
}

// parse
// - Start the parsing descent.
void
parse(const Transpiler &tr)
{
  // grammar: vector< pair< string, vector< vector< {int, int, str} >>>>
  //                        ^       ^       ^        ^     ^     ^
  //                        class   rule   alt  symtype  symrep  symid

  const Atoms &a = tr.atoms;
  const Grammar &g = tr.grammar;

  // assume only stmt's on level zero
  parse_stmt(look, a, g);
}

// outro
// - Print out transpilation preamble.
void
outro()
{
  if (DEBUG)
  {
    std::cout << "DBG --------outro" << std::endl;
  }
  std::cout << "#include <stdio.h>" << std::endl;
  std::cout << "#include <stdint.h>" << std::endl;
  std::cout << "typedef uint64_t u;" << std::endl;
  std::cout << "u read() { u x; scanf(\"%lu\", &x); return x; }" << std::endl;
}

// out
// - Using the result of parse(), print out transpilation according
//   to assignment requirements.
void
out(vector<string>::iterator look, vector<string>::iterator end)
{
  vector<string> stack;
  string tok;
  string id;

  std::reverse(acc.begin(), acc.end());

  // TODO: must finish parse() first
}

// init
// - Initialise the transpilation with grammar and atomic definitions.
void
init(Transpiler &mut_tr)
{
  Atoms &a = mut_tr.atoms;
  Grammar &g = mut_tr.grammar;

  // populate list of atomic regex patterns
  A::add(a, "S", "^[A-Za-z]+$");
  A::add(a, "N", "^[0-9]+$");
  A::add(a, "main", "^main$");
  A::add(a, "if", "^if$");
  A::add(a, "read", "^read$");
  A::add(a, "write", "^write$");
  A::add(a, "~", "^~$");
  A::add(a, "+", "^\\+$");
  A::add(a, "-", "^-$");
  A::add(a, "*", "^\\*$");
  A::add(a, "/", "^/$");
  A::add(a, "%", "^%$");
  A::add(a, "<", "^<$");
  A::add(a, ">", "^>$");
  A::add(a, "==", "^==$");
  A::add(a, "!=", "^!=$");
  A::add(a, "&&", "^&&$");
  A::add(a, "||", "^\\|\\|$");
  A::add(a, ";", "^;$");
  A::add(a, ",", "^,$");
  A::add(a, "{", "^\\{$");
  A::add(a, "}", "^\\}$");
  A::add(a, "(", "^\\($");
  A::add(a, ")", "^\\)$");

  // add whitespace atoms, just for lexing purposes
  A::add(a, " ", "");
  A::add(a, "\t", "");
  A::add(a, "\v", "");
  A::add(a, "\f", "");
  A::add(a, "\r", "");
  A::add(a, "\n", "");

  // TODO: The following "g" is not being used completely as intended.
  //       The parsing descent uses hardcoded "parse_*" instead.
  //       Could use reflection to refactor all the parse_*.

  // populate grammar with rules (order matters)
  G::add(g, "stmt", {{S(0, 0, "cond")},
                     {S(0, 0, "call")},
                     {S(0, 0, "def")},
                     {S(0, 0, "expr")} });
  G::add(g, "cond", {{S(1, 0, "if"), S(1, 0, "("), S(0, 0, "expr"), S(1, 0, ")"), S(1, 0, "{"), S(0, 0, "grps"), S(1, 0, "}"), S(1, 0, "{"), S(0, 0, "grps"), S(1, 0, "}")}});
  G::add(g, "call", {{S(0, 0, "name"), S(1, 0, "("), S(0, 0, "grpc"), S(1, 0, ")")}});
  G::add(g, "def", {{S(1, 0, "main"), S(1, 0, "{"), S(0, 0, "grps"), S(1, 0, "}")},
                    {S(0, 0, "name"), S(0, 1, "name"), S(1, 0, "{"), S(0, 0, "grps"), S(1, 0, "}")}});
  G::add(g, "grps", {{S(0, 1, "stmts"), S(0, 0, "stmt")}});
  G::add(g, "grpc", {{S(0, 1, "exprc"), S(0, 0, "expr")}});
  G::add(g, "stmts", {{S(0, 0, "stmt"), S(1, 0, ";")}});
  G::add(g, "exprc", {{S(0, 0, "expr"), S(1, 0, ",")}});
  G::add(g, "expr", {{S(0, 0, "unary"), S(0, 0, "fact")},
                     {S(0, 0, "fact"), S(0, 0, "binary"), S(0, 0, "fact")},
                     {S(1, 0, "fact")}});
  G::add(g, "unary", {{S(1, 0, "~")}});
  G::add(g, "binary", {{S(1, 0, "+")},
                       {S(1, 0, "-")},
                       {S(1, 0, "*")},
                       {S(1, 0, "/")},
                       {S(1, 0, "%")},
                       {S(1, 0, "<")},
                       {S(1, 0, ">")},
                       {S(1, 0, "==")},
                       {S(1, 0, "!=")},
                       {S(1, 0, "&&")},
                       {S(1, 0, "||")}});
  G::add(g, "name", {{S(1, 0, "write")},
                     {S(1, 0, "read")},
                     {S(1, 0, "S")}});
  G::add(g, "fact", {{S(1, 0, "N")},
                     {S(1, 0, "S")}});

  // debug
  A::print(a);
  G::print(g, a);
}

////////////////////////////////////////////////////////////////////////
// S# -> C transpiler

int
main(const int argc, char** argv)
{
  string test = "fun x {write(x); if(x>0) {fun(x-1)} {0} }\n\
main {fun(10)}";

  Transpiler tr = Transpiler();
  vector<string> tokens;
  string start;

  init(tr);
  //outro(); // TODO
  lex(tr, test, tokens);
  if (DEBUG)
  {
    std::cout << "DBG --------parse" << std::endl;
  }
  look = tokens.begin(); // global
  while (look != tokens.end())
  {
    parse(tr);
  }
  //out(tokens.begin(), tokens.end()); // TODO

  return EXIT_SUCCESS;
}

