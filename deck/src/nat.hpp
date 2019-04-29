// file: deck/src/nat.hpp
// by  : jooh@cuni.cz
// for : nprg041

////////////////////////////////////////////////////////////////////////
// directives

#include <math.h> // fmod


////////////////////////////////////////////////////////////////////////
// natively defined words

vector<pair<string,function<void()>>>
babble =
{
  {"(",
   []() {
     STATE = S::comm;
   }},

  {"*",
   []() {
     if (!sound(DATA, 2, "*", true)) return;
     double one = stod(pop(DATA));
     double two = stod(pop(DATA));
     dpush(DATA, two * one);
   }},

  {"+",
   []() {
     if (!sound(DATA, 2, "+", true)) return;
     char* end;
     double one = stod(pop(DATA));
     double two = stod(pop(DATA));
     dpush(DATA, two + one);
   }},

  {"-",
   []() {
     if (!sound(DATA, 2, "-", true)) return;
     double one = stod(pop(DATA));
     double two = stod(pop(DATA));
     dpush(DATA, two - one);
   }},

  {"/",
   []() {
     if (!sound(DATA, 2, "/", true)) return;
     double one = stod(pop(DATA));
     double two = stod(pop(DATA));
     dpush(DATA, two / one);
   }},

  {"/mod",
   []() {
     if (!sound(DATA, 2, "/mod", true)) return;
     double one = stod(pop(DATA));
     double two = stod(pop(DATA));
     dpush(DATA, fmod(two, one));
     dpush(DATA, (int)(two / one));
   }},

  {".",
   []() {
     if (!sound(DATA, 1, ".", false)) return;
     std::cout << pop(DATA) << std::endl;
   }},

  {".\"",
   []() {
     STATE = S::str;
   }},

  {"2drop",
   []() {
     if (!sound(DATA, 2, "2drop", false)) return;
     DATA.pop_back();
     DATA.pop_back();
   }},

  {"2dup",
   []() {
     if (!sound(DATA, 2, "2dup", false)) return;
     string one = pop(DATA);
     string two = DATA.back();
     DATA.push_back(one);
     DATA.push_back(two);
     DATA.push_back(one);
   }},

  {"2over",
   []() {
     if (!sound(DATA, 4, "2over", false)) return;
     string one = pop(DATA);
     string two = pop(DATA);
     string three = pop(DATA);
     string four = DATA.back();
     DATA.push_back(three);
     DATA.push_back(two);
     DATA.push_back(one);
     DATA.push_back(four);
     DATA.push_back(three);
   }},

  {"2swap",
   []() {
     if (!sound(DATA, 4, "2swap", false)) return;
     string one = pop(DATA);
     string two = pop(DATA);
     string three = pop(DATA);
     string four = pop(DATA);
     DATA.push_back(two);
     DATA.push_back(one);
     DATA.push_back(four);
     DATA.push_back(three);
   }},

  {":",
   []() {
     STATE = S::def;
   }},

  {"=",
   []() {
     if (!sound(DATA, 2, "=", true)) return;
     double one = stod(pop(DATA));
     double two = stod(pop(DATA));
     DATA.push_back(two == one ? "1" : "0");
   }},

  {"<",
   []() {
     if (!sound(DATA, 2, "<", true)) return;
     double one = stod(pop(DATA));
     double two = stod(pop(DATA));
     DATA.push_back(two < one ? "1" : "0");
   }},

  {">",
   []() {
     if (!sound(DATA, 2, ">", true)) return;
     double one = stod(pop(DATA));
     double two = stod(pop(DATA));
     DATA.push_back(two > one ? "1" : "0");
   }},

  {"cr",
   []() {
     std::cout << std::endl;
   }},

  {"do",
   []() {
     if (!sound(DATA, 2, "do", true)) return;
     STATE = S::loop;
     DO_index = stoi(pop(DATA));
     DO_ctrl = stoi(pop(DATA));
   }},

  {"drop",
   []() {
     if (!sound(DATA, 1, "drop", false)) return;
     DATA.pop_back();
   }},

  {"dup",
   []() {
     if (!sound(DATA, 1, "dup", false)) return;
     DATA.push_back(DATA.back());
   }},

  {"emit",
   []() {
     if (!sound(DATA, 1, "emit", true)) return;
     int n = stoi(pop(DATA));
     if (n < 32 || n > 126)
     {
       err("emit", "only ascii characters allowed");
       return;
     }
     std::cout << (char)n << std::flush;
   }},

  {"empty",
   []() {
     for (const auto entry: DICT)
     {
       forget(entry.first);
     }
   }},

  {"forget",
   []() {
     STATE = S::fgt;
   }},

  {"if",
   []() {
     if (!sound(DATA, 1, "if", true)) return;
     if (pop(DATA) == "0")
     {
       STATE = S::alt;
     }
     else // accept any nonzero
     {
       STATE = S::cond;
     }
   }},

  {"mod",
   []() {
     if (!sound(DATA, 2, "mod", true)) return;
     double one = stod(pop(DATA));
     double two = stod(pop(DATA));
     dpush(DATA, fmod(two, one));
   }},

  {"over",
   []() {
     if (!sound(DATA, 2, "over", false)) return;
     string one = pop(DATA);
     string two = DATA.back();
     DATA.push_back(one);
     DATA.push_back(two);
   }},

  {"rot",
   []() {
     if (!sound(DATA, 3, "rot", false)) return;
     string one = pop(DATA);
     string two = pop(DATA);
     string three = pop(DATA);
     DATA.push_back(two);
     DATA.push_back(one);
     DATA.push_back(three);
   }},

  {"space",
   []() {
     std::cout << " " << std::flush;
   }},

  {"spaces",
   []() {
     if (!sound(DATA, 1, "spaces", true)) return;
     int n = stoi(pop(DATA));
     std::cout << string(n, ' ') << std::flush;
   }},

  {"swap",
   []() {
     if (!sound(DATA, 2, "swap", false)) return;
     string one = pop(DATA);
     string two = pop(DATA);
     DATA.push_back(one);
     DATA.push_back(two);
   }},

  {".?",
   []() {
     std::cout << ".d  print dictionary\n"
               << ".h  print input history\n"
               << ".s  print stack contents\n"
               << ".v  print DECK version\n"
               << ".q  quit DECK" << std::endl;
   }},

  {".c",
   []() {
     clear(DATA);
   }},

  {".d",
   []() {
     string meaning;
     size_t nat_size = NAT.size();
     size_t dict_size = DICT.size();

     std::cout << "NATIVE (" << nat_size << " def"
               << (nat_size != 1 ? "s" : "") << ")\n";
     for (const auto entry: NAT)
     {
       std::cout << entry.first << "\n";
     }

     std::cout << std::endl;

     std::cout << "USER (" << dict_size << " def"
               << (dict_size != 1 ? "s" : "") << ")\n";
     for (const auto entry: DICT)
     {
       meaning = "";
       for (const auto w: entry.second.back())
       {
         meaning += w + " ";
       }
       std::cout << entry.first << " :\t" << meaning << "\n";
     }

     std::cout << std::flush;
   }},

  {".h",
   []() {
     vector<string> history = linenoise::GetHistory();
     size_t size = history.size();
     std::cout << "HISTORY (" << size << " lines)\n";
     for (size_t i = 0; i < size; ++i)
     {
       std::cout << i + 1 << "  " << history[i] << "\n";
     }
     std::cout << std::flush;
   }},

  {".s",
   []() {
     string words = "(";
     for (const auto w: WORK)
     {
       words = words.size() == 1 ? words + w : words + "," + w;
     }
     std::cout << "WORK: " << words << ")" << std::endl;

     words = "(";
     for (const auto w: DATA)
     {
       words = words.size() == 1 ? words + w : words + "," + w;
     }
     std::cout << "DATA: " << words << ")" << std::endl;
   }},

  {".v",
   []() {
     std::cout << "DECK version " << VERSION << std::endl;
   }},
};

