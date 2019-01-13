// file: deck/deck.hpp
// by  : jooh@cuni.cz
// for : nprg041
// lic.: mit

////////////////////////////////////////////////////////////////////////
// directives

#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using std::function;
using std::istringstream;
using std::map;
using std::pair;
using std::set;
using std::string;
using std::vector;

////////////////////////////////////////////////////////////////////////
// globals, macros, enums

// Def types
enum class Dtype {Native, Natphr, Phrase};
// Val types (Any is used only by check_param() and lex())
enum class Vtype {Any = 0, Def, Num, Str, Err};
// Err types
enum class Etype {Lex = 0, Clar, Eval, Param, Apply};

const string VERSION = "0.1";
const map<string,pair<string,string>> keywords =
{{"com", {"(", ")"}},
 {"def", {":", ";"}},
 {"str", {".\"", "\""}},
 {"forget", {"forget", ""}}};
const vector<pair<string,string>> prefs =  // "y", "n", or other string
{{"debug", "n"},
 {"files", ""},
 {"prompt", ""}};
// see also babble.hpp for a global constant

////////////////////////////////////////////////////////////////////////
// declarations

struct Def;
struct Val;
typedef map<string,Def*> Dict;
typedef vector<string> Hist;
typedef vector<Val> Stack;
typedef map<string,string> Pref;
typedef map<Etype,vector<pair<int,string>>> Err;
struct Lang;

// for test.cpp
void mkdef(Dict &d, string s, string p, string n, int i, Err &e);
void rmdef(Dict &d, string s, int i, Err &e);
Val nom(const string s, Vtype t, Dict &d);
void lex(const string s, Stack &k, Dict &d, Err &e);
void clarify(const Stack &a, Stack &b, Dict &d, Err &e);
void eval(Stack &k, Dict &d, Hist &h, Err &e);
void nag(Err &e,
         const Stack &k,
         const string s,
         const size_t c,
         const size_t n,
         const bool b);
string indicate(const string s, const set<int> &t);
string motd(const int n);
string prompt(Lang &l);
void rinse(Lang &l, Stack &k);
void init_lang(Lang &l, const int n, char** a);

// forward declarations
string vtype(Vtype t);
string etype(Etype t);
vector<string> split(string s);
pair<bool,double> isnum(string s);
string stack_line(const Stack &k, const bool b);
bool check_param(const Def* f, const Stack &k, Err &e);
string ans_prompt();
string error_prompt();
void pr(string s, bool b);
void er(string s, bool b);

////////////////////////////////////////////////////////////////////////
// types & classes & structures

//----------------------------------------------------------------------
// Def(inition) class
//
// - Encapsulates "native" and "phrase" def types.
// - Think of Def as a "function".
//   - eg. "+" is a Def of type Native and includes a lambda that adds
//     2 numbers.
//   - eg. a user-defined word "foo" is a Def of type Natphr that refers
//     to whatever string the user chose. The referred string eventually
//     gets parsed into its constituent literal Val and/or native Defs.
//   - "param" is for argument checking.
//     - For Phrases, variadic Natives, and zero-arity Natives, param
//       should be empty.
// - Think of Def also as the "definition" of a "word".
//   - The Def type populates the dictionary.

struct Def
{
public:
  string sym;
  Dtype type;
  string note;  // forth stack notation
  vector<Vtype> param;
  function<void(Stack&, Dict&, Hist&, Err&)> apply;  // lambda
  string phrase;  // forth word composite

  Def(string y, string n, Dtype t, vector<Vtype> a,
      function<void(Stack&, Dict&, Hist&, Err&)> f)
    : sym(y), type(Dtype::Native), note(n), param(a), apply(f) {}
  Def(string y, string n, Dtype t, vector<Vtype> a, string p)
    : sym(y), type(t), note(n), param(a), phrase(p) {}
};

//----------------------------------------------------------------------
// Val(ue) class
//
// - Encapsulates "def", "num", and "str" value types.
// - "num" and "str" represent literals and "def" can be either a
//   "native" function (lambda) or a "phrase" (ie. a word composite).
// - Think of Val as the atom that the lexer, evaluator, and stack
//   would be primarily concerned with.

struct Val
{
  Vtype type;
  string sym;
  Def* def;
  double num;
  string str;

  Val() {}
  Val(Vtype t): type(t) {}
  Val(Vtype t, string y): type(t), sym(y) {}
  Val(Vtype t, string y, Def* f): type(t), sym(y), def(f) {}
  Val(Vtype t, string y, double n): type(t), sym(y), num(n) {}
  Val(Vtype t, string y, string s): type(t), sym(y), str(s) {}
  Val(const Val &v)
    : type(v.type), sym(v.sym), def(v.def), num(v.num), str(v.str) {}
};

//----------------------------------------------------------------------
// Dict(ionary) typedef [map(string->Def*)]
// - Table of defined words (either primitively, by the user, or by the
//   environment).

pair<string,string>
note_spaces(string s)
{
  string a = "";
  string b = "";
  if (s.substr(0, 2) == "--")
  {
    a = " ";
  }
  if (s.substr(s.length() - 2) == "--")
  {
    b = " ";
  }
  return {a, b};
}

namespace D
{
  bool
  has(const Dict &d, string s)
  {
    return d.find(s) != d.end();
  }

  Def*
  lookup(Dict &d, string s)
  {
    return d.at(s);
  }

  void
  erase(Dict &d, string s)
  {
    if (has(d, s))
    {
      d.erase(d.find(s));
    }
  }

  void
  write(Dict &d, string s, Def* f)
  {
    if (has(d, s))
    {
      d.erase(d.find(s));
    }
    d.insert(pair<string,Def*>(s, f));
  }

  void
  print(Dict &d)
  {
    Def* def;
    string s;
    pair<string,string> spc;

    std::cout << "---- native:" << std::endl;
    for (const auto entry: d)
    {
      def = entry.second;
      spc = note_spaces(def->note);
      s += entry.first + ":\t(" + spc.first + def->note + spc.second
        + ")\t";
      if (def->type == Dtype::Native)
      {
        s += "native";
      }
      else if (def->type == Dtype::Natphr)
      {
        s += def->phrase;
      }
      s += "\n";
    }

    std::cout << s << "---- user:" << std::endl;
    s = "";
    for (const auto entry: d)
    {
      def = entry.second;
      spc = note_spaces(def->note);
      if (def->type == Dtype::Phrase)
      {
        s += entry.first + ":\t(" + spc.first + def->note + spc.second
          + ")\t" + def->phrase + "\n";
      }
    }
    std::cout << s << std::endl;
  }
}

//----------------------------------------------------------------------
// Hist(ory) typedef [vec(string)]
//
// - Record of every line of user input.

namespace H
{
  string
  peek(const Hist &h)
  {
    return h.back();
  }

  void
  push(Hist &h, string s)
  {
    h.push_back(s);
  }

  string
  pop(Hist &h)
  {
    string s = "";
    if (!h.empty())
    {
      s = h.back();
      h.pop_back();
    }
    return s;
  }

  void
  print(Hist &h)
  {
    size_t count = 1;
    for (const auto line: h)
    {
      std::cout << count << ": " << line << std::endl;
      count++;
    }
  }
}

//----------------------------------------------------------------------
// Stack typedef [vec(Val)]

namespace K
{
  Val*
  peek(Stack &k)
  {
    return &(k.back());
  }

  void
  push(Stack &k, Val v)
  {
    k.push_back(v);
  }

  Val
  pop(Stack &k)
  {
    Val v;
    if (!k.empty())
    {
      v = k.back();
      k.pop_back();
    }
    return v;
  }

  void
  print(Stack &k)
  {
    string s;
    if (k.empty())
    {
      s = "stack empty";
    }
    else {
      s = "stack: " + stack_line(k, false);
    }
    std::cout << s << std::endl;
  }
}

//----------------------------------------------------------------------
// Pref(erence) typedef [map(string->string)]
//
// - Table of interpreter's runtime preferences.

namespace P
{
  bool
  has(const Pref &p, string s)
  {
    return p.find(s) != p.end();
  }

  string
  lookup(Pref &p, string s)
  {
    return p.at(s);
  }

  void
  write(Pref &p, string s, string v)
  {
    if (has(p, s))
    {
      p.erase(p.find(s));
    }
    p.insert(pair<string,string>(s, v));
  }
}

//----------------------------------------------------------------------
// Err(or) typedef [map(Etype,vec(pair(int,string)))]
//
// - Table of interpreter's runtime preferences.
// - int: the position of the erroneous token/value.
// - string: the token/value that erred.
// - For flexibility, all values can also be some other kind of
//   pre-promised value.
// - TODO: But should decide on a specification for error codes and
//         values.

namespace E
{
  void
  backtrack(size_t size, Stack &k)
  {
    while (k.size() > size)
    {
      K::pop(k);
    }
  }

  void
  err(Err &e, Etype t, int i, string s)
  {
    e[t].push_back(pair<int,string>(i, s));
  }

  bool
  okay(Err &e)
  {
    for (const auto x: e)
    {
      if (x.second.size())
      {
        return false;
      }
    }
    return true;
  }
}

//----------------------------------------------------------------------
// Lang class
//
// - The root structure of the language, containing instances of all the
//   other structures.
// - In its current implementation, the language is line-based, ie. each
//   input line must be complete and sane.

struct Lang
{
  Dict dictionary;  // map(string->Def*)
  Hist history;     // vec(string)
  Stack stack;      // vec(Val)
  Pref preference;  // map(string->string)
  Err error;        // vec(vec(pair(int,string)))
  size_t carry;     // size of stack carried over from previous eval
};

////////////////////////////////////////////////////////////////////////
// helpers

//----------------------------------------------------------------------
// vtype: convert Vtype to string

string
vtype(Vtype type)
{
  map<Vtype,string> vs =
    {{Vtype::Any, "any"},
     {Vtype::Def, "def"},
     {Vtype::Num, "num"},
     {Vtype::Str, "str"},
     {Vtype::Err, "err"}};
  return vs.at(type);
}

//----------------------------------------------------------------------
// etype: convert Etype to string

string
etype(Etype type)
{
  map<Etype,string> es =
    {{Etype::Lex, "lex"},
     {Etype::Clar, "clar"},
     {Etype::Eval, "eval"},
     {Etype::Param, "param"},
     {Etype::Apply, "apply"}};
  return es.at(type);
}

//----------------------------------------------------------------------
// split: split a string on whitespace

vector<string>
split(string s)
{
  istringstream is(s);
  vector<string> ss = {std::istream_iterator<string>{is},
                       std::istream_iterator<string>{}};
  return ss;
}

//----------------------------------------------------------------------
// isnum: check if a string is a number

pair<bool,double>
isnum(string s)
{
  bool b = true;
  char* p;
  double n = std::strtod(s.c_str(), &p);
  if (*p)
  {
    b = false;
  }
  return pair<bool,double>(b, n);
}

//----------------------------------------------------------------------
// stack_line: return contents of the stack

string
stack_line(const Stack &k, const bool verbose)
{
  map<Vtype,string> vs =
    {{Vtype::Any, "any"},
     {Vtype::Def, "def"},
     {Vtype::Num, "num"},
     {Vtype::Str, "str"},
     {Vtype::Err, "err"}};
  string s = "";
  string space = "";
  string type = "";

  for (const auto value: k)
  {
    if (verbose)
    {
      type = "(" + vs[value.type] + ")";
    }
    s += space + type + value.sym;
    space = " ";
  }

  return s;
}

//----------------------------------------------------------------------
// check_param: check a native def's arguments

bool
check_param(const Def* def, const Stack &stack, Err &e)
{
  // TODO: use def notation to check types

  string word = def->sym;
  vector<Vtype> param = def->param;

  // arity
  if (stack.size() < param.size())
  {
    E::err(e, Etype::Param, -1,
           word + " "
           + std::to_string(param.size()) + " "
           + std::to_string(stack.size()) + " "
           + stack_line(stack, false));
    return false;
  }

  // types
  bool fail = false;
  size_t i = stack.size();
  Val v;
  for (size_t count = 0; count < param.size(); count++)
  {
    i--;
    if (param[count] == Vtype::Any)
    {
      continue;
    }
    v = stack[i];
    if (v.type != param[count])
    {
      E::err(e, Etype::Param, count,
             word + " " + vtype(v.type) + "(" + v.sym + ")");
      fail = true;
    }
  }
  return !fail;
}

//----------------------------------------------------------------------
// ans_prompt: the string that appears before each line of answering

string
ans_prompt()
{
  return "=> ";
}

//----------------------------------------------------------------------
// error_prompt: the string that appears before each line of error

string
error_prompt()
{
  return "E> ";
}

//----------------------------------------------------------------------
// pr: std::cout wrapper
//
// - Used by loop_body() and maybe some defs.

void
pr(string s, bool nl)
{
  std::cout << ans_prompt() << s;
  if (nl)
  {
    std::cout << "\n" << std::endl;
  }
  else
  {
    std::cout << std::flush;
  }
}

//----------------------------------------------------------------------
// er: std::cerr wrapper

void
er(string s, bool nl)
{
  std::cerr << error_prompt() << s;
  if (nl)
  {
    std::cerr << "\n" << std::endl;
  }
  else
  {
    std::cerr << std::flush;
  }
}

