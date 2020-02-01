// file: _exam_nprg041/main.cpp
// by  : jooh@cuni.cz
// for : nprg041 exam

////////////////////////////////////////////////////////////////////////
// directives

#include <stdlib.h>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>

using std::map;
using std::pair;
using std::regex;
using std::string;
using std::vector;

#define DEBUG 0

////////////////////////////////////////////////////////////////////////
// enums & structures

/*
 * Token
 * - enumeration of types of possible tokens
 */

enum class Token
{
  /* parseable */
  Atomi = 0, // [A-Z]
  Atomf,     // [a-z]
  Bond1,     // -
  Bond2,     // =
  Bond3,     // #
  Ref,       // [0-9]
  Pareno,    // (
  Parenc,    // )

  /* synthesised after parse */
  Atom
};

/*
 * Item
 * - denote "a parsed token"
 */

struct Item
{
  Token type;
  string token;
  int num; // only atoms

  Item(Token t, string k): type(t), token(k) {}
  Item(Token t, string k, int n): type(t), token(k), num(n) {}
};

/*
 * State
 * - a structure to be passed around between parse() and eval()
 */

struct State
{
  string input;                 // current line
  map<Token,string> token_map;  // regex definition of token types
  vector<Item*> items;          // result of parse()
  string name_stack;         // local to parse(); not enough time to fix
  int atom_count;            // local to eval(); not enough time to fix

  State()
  {
    token_map =
      {
        {Token::Atomi,  "[A-Z]"},
        {Token::Atomf,  "[a-z]"},
        {Token::Bond1,  "-"},
        {Token::Bond2,  "="},
        {Token::Bond3,  "#"},
        {Token::Ref,    "[0-9]"},
        {Token::Pareno, "\\("},
        {Token::Parenc, "\\)"}
      };
    atom_count = 0;
  }
};

////////////////////////////////////////////////////////////////////////
// functions

/*
 * eval()
 * - take the result of parse(), interpret it, and print as appropriate
 */
int
eval(State &S)
{
  if (DEBUG)
  {
    string t;
    std::cout << "DEBUG: items:" << std::endl;
    for (const auto item: S.items)
    {
      switch (item->type)
      {
      case Token::Atom:   t = "  atom ";   break;
      case Token::Bond1:  t = "  bond1 ";  break;
      case Token::Bond2:  t = "  bond2 ";  break;
      case Token::Bond3:  t = "  bond3 ";  break;
      case Token::Ref:    t = "  ref ";    break;
      case Token::Pareno: t = "  pareno "; break;
      case Token::Parenc: t = "  parenc "; break;
      default: t = "  bad "; break;
      }
      std::cout << "DEBUG: " << t << item->token << std::endl;
    }
  }

  if (DEBUG)
  {
    std::cout << "\nDEBUG: result:\n" << std::endl;
  }

  std::cout << "\nATOMS:" << std::endl;

  for (const auto item: S.items)
  {
    if (item->type == Token::Atom)
    {
      ++S.atom_count;
      item->num = S.atom_count;
      std::cout << S.atom_count << " " << item->token << std::endl;
    }
  }

  vector<Item*> start_stack;  // stack of atoms to "refer back to"
  vector<string> bond_stack;
  map<int,Item*> ref_stack;

  std::cout << "\nBONDS:" << std::endl;

  for (const auto item: S.items)
  {

    // an atom => check all the stacks
    if (item->type == Token::Atom)
    {
      if (start_stack.empty())
      {
        start_stack.push_back(item);
      }
      else
      {
        if (bond_stack.empty())
        {
          std::cout << start_stack.back()->num << "-" << item->num
                    << std::endl;
        }
        else
        {
          string bond = bond_stack.back();
          bond_stack.pop_back();
          std::cout << start_stack.back()->num << bond << item->num
                    << std::endl;
        }
        start_stack.pop_back();
        start_stack.push_back(item);
      }

      continue;
    }

    // a bond => declare it to be used for the next atom
    if (item->type == Token::Bond1
        || item->type == Token::Bond2
        || item->type == Token::Bond3)
    {
      if (start_stack.empty())
      {
        std::cerr << "ERROR: eval: missing atom startpoint"
                  << std::endl;
        return 1;
      }

      if (!bond_stack.empty())
      {
        std::cerr << "ERROR: eval: previous bond not consumed"
                  << std::endl;
        return 1;
      }

      bond_stack.push_back(item->token);

      continue;
    }

    // "(" => duplicate stack top
    if (item->type == Token::Pareno)
    {
      start_stack.push_back(start_stack.back());

      continue;
    }

    // ")" => remove the duplicated stack top
    if (item->type == Token::Parenc)
    {
      start_stack.pop_back();

      continue;
    }

    // a ref => push a new reference (int,Item pair)
    if (item->type == Token::Ref)
    {
      int refnum = atoi(item->token.c_str());
      map<int,Item*>::iterator found = ref_stack.find(refnum);

      // new ref
      if (found == ref_stack.end())
      {
        ref_stack.insert(pair<int,Item*>(refnum, start_stack.back()));
      }

      // ref already exists, so bond back to it
      else
      {
        Item* ref = (*found).second;
        ref_stack.erase(found);

        if (bond_stack.empty())
        {
          std::cout << start_stack.back()->num << "-" << ref->num
                    << std::endl;
        }
        else
        {
          string bond = bond_stack.back();
          bond_stack.pop_back();
          std::cout << start_stack.back()->num << bond << ref->num
                    << std::endl;
        }
      }

      continue;
    }
  }

  std::cout << "\n--------\n" << std::endl;

  return 0;
}

/*
 * parse()
 * - take the input, and turn all tokens into evaluable items
 */
int
parse(State &S, const string line)
{
  map<Token,string> &m = S.token_map;

  S.items.clear();
  S.atom_count = 0;

  // go thru each char of line
  for (string::const_iterator i = line.begin(); i != line.end(); ++i)
  {
    string s = string(1, *i);

    if (DEBUG)
    {
      std::cout << "DEBUG: " << *i << std::endl;
    }

    // check char against each token type
    for (map<Token,string>::iterator j = m.begin(); j != m.end(); ++j)
    {
      Token t = (*j).first;
      regex r((*j).second);

      if (std::regex_match(s, r))
      {
        if (t == Token::Atomi)
        {
          if (S.name_stack.empty())
          {
            S.name_stack = s;
          }
          else
          {
            if (DEBUG)
            {
              std::cout << "DEBUG: pushing atom " << S.name_stack
                        << std::endl;
            }
            S.items.push_back(new Item(Token::Atom, S.name_stack));
            S.name_stack = s;
          }
          break;
        }

        if (t == Token::Atomf)
        {
          if (S.name_stack.empty())
          {
            std::cerr << "ERROR: parse: atom" << std::endl;
            return 1;
          }
          else
          {
            S.name_stack = S.name_stack + s;
          }
          break;
        }

        if (t == Token::Bond1
            || t == Token::Bond2
            || t == Token::Bond3
            || t == Token::Ref
            || t == Token::Pareno
            || t == Token::Parenc)
        {
          if (!S.name_stack.empty())
          {
            if (DEBUG)
            {
              std::cout << "DEBUG: pushing atom " << S.name_stack
                        << std::endl;
            }
            S.items.push_back(new Item(Token::Atom, S.name_stack));
            S.name_stack = "";
          }

          if (DEBUG)
          {
            std::cout << "DEBUG: pushing " << s << std::endl;
          }
          S.items.push_back(new Item(t, s));
          break;
        }

        std::cerr << "ERROR: parse: bad token [" << s << "]" << std::endl;
        return 1;
      }
    }
  }

  if (!S.name_stack.empty())
  {
    if (DEBUG)
    {
      std::cout << "DEBUG: pushing atom " << S.name_stack
                << std::endl;
    }
    S.items.push_back(new Item(Token::Atom, S.name_stack));
    S.name_stack = "";
  }

  return 0;
}

/*
 * loop()
 * - keep getting user input, then parse()->eval() it
 */
void
loop(State &S)
{
  string line;

  while (getline(std::cin, line))
  {
    // populate S.items with Items
    if (parse(S, line) != 0)
    {
      break;
    }

    // do things with S.items
    if (eval(S) != 0)
    {
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////
// main

int
main(const int argc, char** argv)
{
  (void)argc;
  (void)argv;

  State state {};
  loop(state);

  return EXIT_SUCCESS;
}

