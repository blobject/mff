// file: deck/main.cpp
// by  : jooh@cuni.cz
// for : nprg041
// lic.: mit

////////////////////////////////////////////////////////////////////////
// directives

#include "deck.hpp"
#include "babble.hpp"

#include <fstream>  // ifstream
#include <unistd.h>  // getopt

using std::ifstream;

////////////////////////////////////////////////////////////////////////
// parse

//----------------------------------------------------------------------
// mkdef: define a new word (will be Dtype::Phrase)

void
mkdef(Dict &mut_dict,
      string name,
      string phrase,
      string note,
      int pos,
      Err &mut_e)
{
  pair<bool,double> num = isnum(name);
  if (num.first)
  {
    E::err(mut_e, Etype::Lex, pos, "word cannot be a number");
    return;
  }
  if (name.find_first_of("\r\n\t\v\b^") != string::npos)
  {
    E::err(mut_e, Etype::Lex, pos, "invalid character in word");
    return;
  }
  // definition can be empty (just does absolutely nothing)
  // TODO: parse stack notation
  D::write(mut_dict, name,
           new Def(name, note, Dtype::Phrase, {}, phrase));
  pr("compiled: " + name, true);
}

//----------------------------------------------------------------------
// rmdef: define a new word

void
rmdef(Dict &mut_dict, string word, int pos, Err &mut_e)
{
  if (!D::has(mut_dict, word))
  {
    std::cout << "\"" << word
              << "\" not forgotten as it was never defined\n"
              << std::endl;
    return;
  }
  D::erase(mut_dict, word);
  std::cout << "\"" << word << "\" forgotten from the dictionary\n"
            << std::endl;
}

//----------------------------------------------------------------------
// nom: nominate a string
//
// - ie. Convert it into an actual token recognised by the rest of the
//   program by constructing a Val with the proper Vtype.

Val
nom(const string token, Vtype type, Dict &dictionary)
{
  pair<bool,double> num = isnum(token);
  Val v;

  if (type == Vtype::Str)  // only (.") gets here
  {
    v = Val(type, token, token);
  }
  else if (D::has(dictionary, token))
  {
    v = Val(Vtype::Def, token, D::lookup(dictionary, token));
  }
  else if (num.first)
  {
    v = Val(Vtype::Num, token, num.second);
  }
  else
  {
    v = Val(Vtype::Err, token);
  }

  return v;
}

//----------------------------------------------------------------------
// lex: parse an input string, with the help of nom()
//
// - Simply split on whitespace.
// - Push order: left to right (first phase of global stack push).
// - Handle the following quirks:
//   ( COMMENT )
//   : DEFINITION ;
//   ." STRING "
//   forget WORD

void
lex(const string input, Stack &mut_stack, Dict &dictionary, Err &mut_e)
{
  // TODO: rethink

  Stack tmp;
  pair<string,string> comword = keywords.at("com");
  pair<string,string> defword = keywords.at("def");
  pair<string,string> strword = keywords.at("str");
  pair<string,string> forgetword = keywords.at("forget");
  bool incom = false;
  bool indef = false;
  bool instr = false;
  bool inforget = false;
  string scom = "";
  string sdef = "";
  string sstr = "";
  string space = "";
  string defsym = "";
  string defcom;
  int count = 0;

  // preserve any remaining words (Values) from last iteration
  for (const auto v: mut_stack)
  {
    K::push(tmp, v);
  }

  for (const auto token: split(input))
  {
    if (incom)
    {
      if (token == comword.second)
      {
        // what else shall we do with the comment?
        incom = false;
        if (indef)
        {
          defcom = scom;
          space = "";
        }
      }
      else
      {
        scom += space + token;
        space = " ";
      }
      continue;
    }
    if (indef)
    {
      if (token == defword.second)
      {
        indef = false;
        mkdef(dictionary, defsym, sdef, scom, count + 1, mut_e);
      }
      else
      {
        if (defsym.empty())
        {
          defsym = token;
        }
        else if (token == comword.first && defcom.empty())
        {
          incom = true;
          defcom = "";
        }
        else
        {
          if (token == defword.first)
          {
            E::err(mut_e, Etype::Lex, count,
                   "cannot have another definition inside");
            return;
          }
          sdef += space + token;
          space = " ";
        }
      }
      continue;
    }
    if (instr)
    {
      if (token == strword.second)
      {
        instr = false;
        K::push(tmp, nom(sstr, Vtype::Str, dictionary));
        count++;
      }
      else
      {
        if (token == strword.first)
        {
          E::err(mut_e, Etype::Lex, count,
                 "cannot create another string inside");
          return;
        }
        sstr += space + token;
        space = " ";
      }
      continue;
    }
    if (inforget)
    {
      inforget = false;
      rmdef(dictionary, token, count, mut_e);
      continue;
    }
    if (token == comword.first)
    {
      incom = true;
      scom = "";
      space = "";
      continue;
    }
    if (token == defword.first)
    {
      indef = true;
      defsym = "";
      defcom = "";
      sdef = "";
      space = "";
      continue;
    }
    if (token == strword.first)
    {
      instr = true;
      sstr = "";
      space = "";
      continue;
    }
    if (token == forgetword.first)
    {
      inforget = true;
      continue;
    }
    K::push(tmp, nom(token, Vtype::Any, dictionary));
    count++;
  }

  mut_stack = tmp;
}

////////////////////////////////////////////////////////////////////////
// evaluate

//----------------------------------------------------------------------
// clarify: recursively "dephrase" the stack as preparation for eval()
//
// - Look specifically for Defs of Dtype::Natphr and Dtype::Phrase and
//   recursively expand them out into their corresponding Vals by using
//   lex().
// - Pop order: bottom-to-top & depth-first.
//   - Substitutes exp(ansion) stack for global stack for recursion.
// - Push order: as popped (second phase of global stack push).

void
clarify(const Stack &exp,
        Stack &mut_acc,
        Dict &dictionary,
        Err &e)
{
  Stack phrack;

  for (const auto value: exp)
  {
    if (value.type == Vtype::Def
        && (value.def->type == Dtype::Natphr
            || value.def->type == Dtype::Phrase))
    {
      phrack.clear();
      lex(value.def->phrase, phrack, dictionary, e);
      clarify(phrack, mut_acc, dictionary, e);
    }
    else
    {
      K::push(mut_acc, value);
    }
  }
}

//----------------------------------------------------------------------
// eval: evaluate the result of clarify()
//
// - ie. Apply all native functions within the stack.
// - Success is predicated on the fact that only Vtypes of Num, Str,
//   and Def(->Dtype::Native) remain (ie. only literals and native
//   functions).
// - Pop order: bottom to top.
// - Push order: as popped (third, final phase of global stack push).

void
eval(Stack &mut_stack, Dict &dictionary, Hist &history, Err &mut_e)
{
  const set<Vtype> skip = {Vtype::Num, Vtype::Str};
  Stack tmp;
  Vtype type;
  string sym;

  for (auto value: mut_stack)
  {
    type = value.type;
    sym = value.sym;
    K::push(tmp, value);

    if (skip.find(type) != skip.end())
    {
      continue;
    }
    else if (D::has(dictionary, sym))
    {
      value = K::pop(tmp);
      D::lookup(dictionary, sym)
        ->apply(tmp, dictionary, history, mut_e);
    }
    else
    {
      E::err(mut_e, Etype::Eval, tmp.size() - 1, sym);
    }
  }

  mut_stack = tmp;
}

////////////////////////////////////////////////////////////////////////
// error & debug

//----------------------------------------------------------------------
// indicate: highlight erring word for user convenience

string
indicate(const string line, const set<int> &bads)
{
  pair<string,string> strword = keywords.at("str");
  string indication = "";
  bool instr = false;
  int count = 0;
  string space = "";
  char indicator;

  for (const auto token: split(line))
  {
    indicator = ' ';
    if (instr)
    {
      if (token == strword.second)
      {
        instr = false;
      }
    }
    else
    {
      if (token == strword.first)
      {
        instr = true;
      }
      count++;
    }
    if (bads.find(count - 1) != bads.end())
    {
      indicator = '^';
    }
    indication += space + string(token.length(), indicator);
    space = " ";
  }

  return indication;
}

//----------------------------------------------------------------------
// nag: print errors if any
//
// - TODO: No specification yet decided for error codes and values.

void
nag(Err &e,
    const Stack &stack,
    const string line,
    const size_t carry,
    const size_t prompt_len,
    const bool DBG)
{
  // error precedence
  // 1. eval  - everything seemed good until evaluation
  // 2. param - everything seemed good until application start
  // 3. apply - everything seemed good until actual application
  // 3. lex   - something went awry in the very beginning
  // 4. clar  - something went awry during dephrasing

  const string ep = error_prompt();
  const size_t pre_len = prompt_len - ep.length();
  vector<pair<int,string>> bads;
  set<int> bs;

  if (e[Etype::Eval].size())
  {
    bads = e[Etype::Eval];
    pair<int,string> bad = bads.front();
    // eval failed on word (all words)
    if (bad.first >= 0)
    {
      string plural = "";
      if (bads.size() > 1)
      {
        plural = "s";
      }
      for (const auto b: bads)
      {
        bs.insert(b.first - carry);
      }
      if (DBG)
      {
        er(string(pre_len, ' ') + line + "\n", false);
      }
      er(string(pre_len, ' ') + indicate(line, bs) + "\n", false);
      er("word" + plural + " not defined", true);
    }
  }

  else if (e[Etype::Param].size())
  {
    bads = e[Etype::Param];
    pair<int,string> bad = bads.front();
    // arity check failed
    if (bad.first == -1)
    {
      vector<string> ss = split(bad.second);
      er("\"" + ss[0] + "\" expected to pop " + ss[1]
         + ", but " + ss[2] + " on stack ("
         + ss[3] + ")", true);
    }
    // type check failed
    else
    {
      string word = "";
      vector<string> ss;
      string fail = "";
      string space = "";
      for (const auto b: bads)
      {
        bs.insert(b.first - carry);
        ss = split(b.second);
        word = ss[0];
        fail += space + ss[1];
        space = " ";
      }
      if (DBG)
      {
        er(string(pre_len, ' ') + line + "\n", false);
      }
      er(string(pre_len, ' ') + indicate(line, bs) + "\n", false);
      er("\"" + word + "\" expected different type than: " + fail, true);
    }
  }

  else if (e[Etype::Apply].size())
  {
    bads = e[Etype::Apply];
    pair<int,string> bad = bads.front();
    // apply failed arg-wise (all args)
    if (bad.first >= 0)
    {
      for (const auto b: bads)
      {
        bs.insert(b.first - carry);
      }
      if (DBG)
      {
        er(string(pre_len, ' ') + line + "\n", false);
      }
      er(string(pre_len, ' ') + indicate(line, bs) + "\n", false);
      er(bad.second, true);
    }
    // apply failed generally
    else
    {
      er(bad.second, true);
    }
  }

  else if (e[Etype::Lex].size())
  {
    bads = e[Etype::Lex];
    pair<int,string> bad = bads.front();
    // lex failed
    for (const auto b: bads)
    {
      bs.insert(b.first - carry);
    }
    er(string(pre_len, ' ') + indicate(line, bs) + "\n", false);
    er(bad.second, true);
  }

  else
  {
    if (DBG)
    {
      std::cout << ":-) no errors" << std::endl;
    }
  }
}

////////////////////////////////////////////////////////////////////////
// the program

//----------------------------------------------------------------------
// motd: the string that appears when the program starts

string
motd(int style)
{
  string s;
  if (style == 1)
  {
    return string()
      + " .-----------.\n"
      + " | DECK v" + VERSION + " |\n"
      + " '-----------'\n";
  }
  else if (style == 2)
  {
    s = string()
      + " .----------------------------------------.\n";
  }
  else if (style == 0)
  {
    s = string()
      + " .----------------------------------------.\n"
      + " | DECK v" + VERSION + "                              |\n"
      + " |                                        |\n";
  }
  return s
    + " | - This is a simple FORTH interpreter.  |\n"
    + " | - To exit, type \"bye\", \"exit\", or C-c. |\n"
    + " | - For help, type \".help\" or \".?\".      |\n"
    + " | - The prompt shows sizes of the        |\n"
    + " |   history, stack, and dictionary.      |\n"
    + " '----|--------|----------|---------------'\n"
    + ".-----'        |          |\n"
    + "| .------------'          |\n"
    + "v v v---------------------'\n";
}

//----------------------------------------------------------------------
// prompt: the string that appears before each user input in the REPL

string
prompt(Lang &lang)
{
  return std::to_string(lang.history.size()) + ","
    + std::to_string(lang.stack.size()) + ","
    + std::to_string(lang.dictionary.size()) + "> ";
}

//----------------------------------------------------------------------
// rinse: get ready for the next iteration of loop_body()

void
rinse(Lang &mut_lang, Stack &work)
{
  Err &e = mut_lang.error;
  Stack &k = mut_lang.stack;
  size_t &carry = mut_lang.carry;

  // clear tmp stack
  work.clear();

  // update stack & carry
  if (!E::okay(e))
  {
    E::backtrack(carry, k);
  }
  else
  {
    carry = k.size();
  }

// clear error
  for (auto &phase: e)
  {
    phase.second.clear();
  }
}

//----------------------------------------------------------------------
// loop_body: the REPL
//
// - The flow: INPUT -> LEX -> CLARIFY -> EVAL -> PRINT
//                   -> push to history

bool
loop_body(const string line,
          Lang &mut_lang,
          const size_t prlen,
          const bool DBG)
{
  Stack &stack = mut_lang.stack;
  Dict &dictionary = mut_lang.dictionary;
  Hist &history = mut_lang.history;
  Err &error = mut_lang.error;
  string sym;
  Stack work;

  //----------------
  // update history
  H::push(history, line);

  if (DBG)  // raw user input
  {
    std::cout << "[input] " << line << std::endl;
  }

  //-----
  // lex
  lex(line, work, dictionary, error);

  if (DBG)  // Stack after lex(), before clarify()
  {
    std::cout << "[work]  lex : " << stack_line(work, true)
              << std::endl;
  }

  if (work.size() == 1)
  {
    sym = K::peek(work)->sym;
    if (sym == "bye" || sym == "exit")
    {
      pr("bye!", true);
      return false;
    }
  }

  //---------
  // clarify
  Stack acc;
  clarify(work, acc, dictionary, error);
  for (const auto v: acc)
  {
    K::push(stack, v);
  }

  if (DBG)  // Stack after clarify(), before eval()
  {
    std::cout << "[work]  clar: " << stack_line(acc, true) << std::endl;
  }

  //------
  // eval
  eval(stack, dictionary, history, error);

  if (DBG)  // Stack after eval()
  {
    std::cout << "[stack] eval: " << stack_line(stack, true)
              << std::endl;
  }

  //-------
  // print
  if (DBG)  // Err after everything
  {
    string sep;
    for (const auto phase: error)
    {
      sep = "";
      std::cout << "[error] " << etype(phase.first) << ": "
                << std::flush;
      for (const auto x: phase.second)
      {
        std::cout << sep << x.second << "@" << x.first << std::flush;
        sep = ", ";
      }
      std::cout << std::endl;
    }
  }

  nag(error, stack, line, mut_lang.carry, prlen, DBG);

  //-------
  // rinse
  rinse(mut_lang, work);

  return true;
}

//----------------------------------------------------------------------
// loop: wrapper around the REPL
//
// - Handle optional file option.

void
loop(Lang &lang)
{
  Pref &preference = lang.preference;
  bool DBG = false;
  if (preference.at("debug") == "y")
  {
    DBG = true;
  }
  string line;

  if (DBG)
  {
    std::cout << " . - - - - - - - - - .\n"
              << " . DEBUG MODE IS ON! .\n"
              << "  - - - - - - - - - -" << std::endl;
  }

  // file argument
  if (!preference.at("files").empty())
  {
    std::cout << motd(1) << std::endl;
    vector<string> files = split(preference.at("files"));
    for (auto const f: files)
    {
      ifstream fs{f.c_str()};
      if (!fs.good())
      {
        throw "file does not exist: " + f;
      }
    }
    for (auto const f: files)
    {
      ifstream fs{f.c_str()};
      std::cout << ">> " << f << "\n" << std::endl;
      while (getline(fs, line))
      {
        if (line == "")
        {
          continue;
        }

        if (!loop_body(line, lang, prompt(lang).length(), DBG))
        {
          break;
        }
      }
    }
    if (preference.at("prompt") == "n")
    {
      return;
    }
  }
  // no file argument or "-p"
  if (!preference.at("files").empty())
  {
    std::cout << motd(2) << std::endl;
  }
  else
  {
    std::cout << motd(0) << std::endl;
  }
  string pp;
  do
  {
    pp = prompt(lang);
    std::cout << pp << std::flush;

    getline(std::cin, line);
    if (std::cin.eof())
    {
      pr("bye!", true);
      break;
    }
    if (line == "")
    {
      continue;
    }

    if (!loop_body(line, lang, pp.length(), DBG))
    {
      break;
    }
  }
  while (true);
}

////////////////////////////////////////////////////////////////////////
// Deck, the simple FORTH interpreter

//----------------------------------------------------------------------
// opt: process interpreter options

void
opt(Lang &mut_lang, const int argc, char** argv)
{
  Pref &p = mut_lang.preference;
  int c;
  string space = "";

  while ((c = getopt(argc, argv, "dpf:")) != -1)
  {
    switch (c)
    {
    case 'd':
      P::write(p, "debug", "y");
      break;
    case 'p':
      P::write(p, "prompt", "y");
      break;
    case 'f':
      P::write(p, "files", P::lookup(p, "files") + space + optarg);
      space = " ";
      if (P::lookup(p, "prompt") != "y")
      {
        P::write(p, "prompt", "n");
      }
      break;
    }
  }
}

//----------------------------------------------------------------------
// init_lang: initialise the language
//
// - Set preferences to defaults.
// - Populate the dictionary with some primitives.

void
init_lang(Lang &mut_lang, const int argc, char** argv)
{
  //--------------------------------------------------------------------
  // carry
  mut_lang.carry = 0;

  //--------------------------------------------------------------------
  // dictionary
  Dict &d = mut_lang.dictionary;
  for (auto const word: babble)  // see babble.hpp
  {
    D::write(d, word.first, word.second);
  }

  //--------------------------------------------------------------------
  // error
  mut_lang.error[Etype::Lex] = {};
  mut_lang.error[Etype::Clar] = {};
  mut_lang.error[Etype::Eval] = {};
  mut_lang.error[Etype::Param] = {};
  mut_lang.error[Etype::Apply] = {};

  //--------------------------------------------------------------------
  // preference
  Pref &p = mut_lang.preference;
  for (auto const pref: prefs)
  {
    P::write(p, pref.first, pref.second);
  }

  //--------------------------------------------------------------------
  // options
  opt(mut_lang, argc, argv);
}

int
main(const int argc, char** argv)
{
  Lang deck = Lang();
  init_lang(deck, argc, argv);
  try
  {
    loop(deck);
  }
  catch (string s)
  {
    er(s, true);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

// TODO
// - buff-up tests
// - check specs (= Starting Forth)
// - check comments
// - check documentation
// - check on windows
// - adapt makefile

