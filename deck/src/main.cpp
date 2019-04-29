// file: deck/src/main.cpp
// by  : jooh@cuni.cz
// for : nprg041

////////////////////////////////////////////////////////////////////////
// directives

#include "deck.hpp"
#include "dict.hpp"
#include "linenoise.hpp"
#include "nat.hpp"
#include "state.hpp"

#include <fstream> // ifstream
#include <unistd.h> // getopt

using std::ifstream;


////////////////////////////////////////////////////////////////////////
// DECK, a basic (and only partially complete) FORTH interpreter


// opt
// - Process DECK options

int
opt(const int argc, char** argv)
{
  int c;
  string space = "";

  while ((c = getopt(argc, argv, "?hpl:f:")) != -1)
  {
    switch (c)
    {
    case '?':
    case 'h':
      std::cout << "usage: deck [OPTIONS]\n  -?, -h   help\n  -p       disable prompt\n  -f FILE  interpret FILE"
                << std::endl;
      return 1;

    case 'p':
      set(OPT, "prompt", "n");
      break;

    case 'l':
      if (!isnum(optarg))
      {
        err("-l", "argument is not numeric");
        return -1;
      }
      set(OPT, "limit", optarg);
      break;

    case 'f':
      ostringstream s;
      s << OPT.at("files") << space << optarg;
      set(OPT, "files", s.str());
      space = " ";
      set(OPT, "prompt", "n");
      break;
    }
  }

  return 0;
}


// prep
// - DECK preparation.

int
prep(const int argc, char** argv)
{
  int ok;
  if ((ok = opt(argc, argv)) != 0)
  {
    return ok;
  }

  for (auto const entry: babble)
  {
    NAT.insert(entry);
  }

  linenoise::SetHistoryMaxLen(stoi(OPT.at("limit")));

  return 0;
}


// rinse
// - Clean out everything in preparation for next repl iteration.

void
rinse(void)
{
  clear(WORK);
  clear(DEF_meaning);
  DEF_word = "";
  STR = "";
  DO_index = 0;
  DO_ctrl = 0;
}


// motd
// - Text to print when DECK starts.

string
motd(void)
{
  ostringstream s;
  s << " .--------------------------------------.\n"
    << " | DECK v" << VERSION
    << string(31 - VERSION.size(), ' ') << "|\n"
    << " |                                      |\n"
    << " | - DECK is a basic FORTH interpreter. |\n"
    << " | - To quit, type \".q\", C-d, or C-c.   |\n"
    << " | - For help, type \".?\".               |\n"
    << " | - The prompt shows sizes of the      |\n"
    << " |   history, stack, and dictionary.    |\n"
    << " '--------------------------------------'"
    << std::endl;

  return s.str();
}


// prompt
// - Text to print when waiting on user input.

string
prompt(void)
{
  ostringstream s;
  s << linenoise::GetHistory().size()
    << ":s" << DATA.size()
    << ":d" << NAT.size() + DICT.size()
    << "> " << std::flush;
  return s.str();
}


// loop_body
// - The flow: INPUT -> SPLIT -> MATCH -> PRINT
//                   -> add to history

int
loop_body(string input)
{
  if (input == "")
  {
    return 1;
  }

  if (input == ".q")
  {
    return -1;
  }

  linenoise::AddHistory(input.c_str());

  for (const auto word: split(input))
  {
    match(word);
  }

  rinse();

  return 0;
}


// loop
// - Wrapper around repl.

int
loop(void)
{
  if (!OPT.at("files").empty())
  {
    vector<string> files = split(OPT.at("files"));

    // Check files exist.
    for (const auto f: files)
    {
      ifstream fs(f.c_str());
      if (!fs.good())
      {
        ostringstream s;
        s << '"' << f << "\" file does not exist";
        err("-f", s.str());
        return -1;
      }
    }

    // Interpret files.
    for (const auto f: files)
    {
      ifstream fs(f.c_str());
      std::cout << "DECK << " << f << "\n" << std::endl;
      string input;
      int ok;
      while (getline(fs, input))
      {
        if ((ok = loop_body(input)) > 0)
        {
          continue;
        }
        if (ok < 0)
        {
          break;
        }
      }
      string stack = "(";
      for (const auto w: DATA)
      {
        stack = stack.size() == 1 ? stack + w : stack + "," + w;
      }
      std::cout << "\nDECK >> " << f
                << " complete (stack remnant: " << stack << "))"
                << std::endl;
    }

    return 0;
  }

  // Start repl.
  string input;
  int ok;
  string p = "";
  if (OPT.at("prompt") == "y")
  {
    std::cout << motd() << std::endl;
  }
  while (true)
  {
    if (OPT.at("prompt") == "y")
    {
      p = prompt();
    }
    auto quit = linenoise::Readline(p.c_str(), input);
    if (quit)
    {
      break;
    }
    if ((ok = loop_body(input)) > 0)
    {
      continue;
    }
    if (ok < 0)
    {
      break;
    }
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////
// main

int
main(const int argc, char** argv)
{
  int ok;
  if ((ok = prep(argc, argv)) > 0)
  {
    return EXIT_SUCCESS;
  }
  if (ok < 0)
  {
    return EXIT_FAILURE;
  }

  if (loop() < 0)
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

