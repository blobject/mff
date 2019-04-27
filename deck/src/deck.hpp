// file: deck/src/deck.hpp
// by  : jooh@cuni.cz
// for : nprg041

////////////////////////////////////////////////////////////////////////
// directives

#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <vector>

using std::function;
using std::map;
using std::ostringstream;
using std::pair;
using std::string;
using std::vector;


////////////////////////////////////////////////////////////////////////
// globals

enum class     S {none, comm, def, comp, fgt, str, cond, alt, loop};
S              STATE; // stack behavior & lexical control
vector<string> DATA;  // main stack
vector<string> WORK;  // tmp stack for cond and loop

map<string,function<void()>> NAT; // words with native functionality
map<string,vector<vector<string>>> DICT; // "worded" words

string         DEF_word;    // : WORD ... ;
vector<string> DEF_meaning; // : w MEANING ;
string         STR;         // ." STR "
size_t         DO_index;    // u INDEX do ... loop
size_t         DO_ctrl;     // CTRL  f do ... loop

const string VERSION = "0.2";
map<string,string> OPT = // DECK options
{{"files", ""},
 {"limit", "4096"},
 {"prompt", "y"}};


////////////////////////////////////////////////////////////////////////
// utility


// split
// - Split a string into vector of strings, delimited by whitespace.

vector<string>
split(string &s)
{
  std::istringstream is(s);
  vector<string> ss = {std::istream_iterator<string>(is),
                       std::istream_iterator<string>()};
  return ss;
}


// isnum
// - Check if a string is numeric

bool
isnum(const char* s)
{
  char* end;
  double n = strtod(s, &end);
  if (*end)
  {
    return false;
  }
  return true;
}


// err
// - Print error

void
err(string s, string t)
{
  std::cerr << "ERR: " << s << ": " << t << std::endl;
}


// sound
// - Check stack size and numericity in preparation for pop(s).

bool
sound(vector<string> &stack, size_t n, string word, bool check_num)
{
  size_t size = stack.size();

  if (size < n)
  {
    err(word, "not enough items on stack");
    return false;
  }

  if (check_num)
  {
    string at;
    for (size_t i = 0; i < n; ++i)
    {
      at = stack[size - i - 1];
      if (!isnum(at.c_str()))
      {
        ostringstream s;
        s << "argument on stack is not numeric (" << at << ")";
        err(word, s.str());
        return false;
      }
    }
  }
  return true;
}


// pop
// - Pop from stack and also return popped.

string
pop(vector<string> &stack)
{
  string s = "";
  if (!stack.empty())
  {
    s = stack.back();
    stack.pop_back();
  }
  return s;
}


// dpush
// - Convert double to string and push onto stack

void
dpush(vector<string> &stack, double d)
{
  ostringstream s;
  s << d;
  stack.push_back(s.str());
}


// clear
// - Clear a stack.

void
clear(vector<string> &stack)
{
  for (const auto item: stack)
  {
    stack.pop_back();
  }
}

// set
// - Set option.

void
set(map<string,string> &opt, string key, string val)
{
  if (opt.find(key) != opt.end())
  {
    opt.erase(opt.find(key));
  }
  opt.insert(pair<string,string>(key, val));
}

