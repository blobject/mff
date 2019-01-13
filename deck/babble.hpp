// file: deck/babble.hpp
// by  : jooh@cuni.cz
// for : nprg041
// lic.: mit

////////////////////////////////////////////////////////////////////////
// directives

#include <math.h>  // fmod

////////////////////////////////////////////////////////////////////////
// babble: builtin words
//
// - Defs that are entries in Dict.
// - See also lang_init().
// - Defined numeric constants default to 15-digit precision.

vector<pair<string,Def*>> babble =
{
  // ignores
  {"bye", new Def("bye", "--", Dtype::Native, {},
                  [](Stack &k, Dict &d, Hist &h, Err &e) {})},
  {"exit", new Def("exit", "--", Dtype::Native, {},
                   [](Stack &k, Dict &d, Hist &h, Err &e) {})},

  // native defs
  {"+",
   new Def("+", "n1 n2 -- sum", Dtype::Native,
           {Vtype::Num, Vtype::Num},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (!check_param(D::lookup(d, "+"), k, e))
             {
               return;
             }
             double sum = K::pop(k).num + K::pop(k).num;
             K::push(k, Val(Vtype::Num, std::to_string(sum), sum));
           })},
  {"*",
   new Def("*", "n1 n2 -- prod", Dtype::Native,
           {Vtype::Num, Vtype::Num},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (!check_param(D::lookup(d, "*"), k, e))
             {
               return;
             }
             double prod = K::pop(k).num * K::pop(k).num;
             K::push(k, Val(Vtype::Num, std::to_string(prod), prod));
           })},
  {"-",
   new Def("-", "n1 n2 -- diff", Dtype::Native,
           {Vtype::Num, Vtype::Num},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (!check_param(D::lookup(d, "-"), k, e))
             {
               return;
             }
             double diff = K::pop(k).num - K::pop(k).num;
             K::push(k, Val(Vtype::Num, std::to_string(diff), diff));
           })},
  {"/",
   new Def("/", "n1 n1 -- quot", Dtype::Native,
           {Vtype::Num, Vtype::Num},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (!check_param(D::lookup(d, "/"), k, e))
             {
               return;
             }
             double quot = K::pop(k).num / K::pop(k).num;
             K::push(k, Val(Vtype::Num, std::to_string(quot), quot));
           })},
  {"/mod",
   new Def("/mod", "u1 u2 -- u-rem u-quot", Dtype::Native,
           {Vtype::Num, Vtype::Num},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (!check_param(D::lookup(d, "/mod"), k, e))
             {
               return;
             }
             double a = K::pop(k).num;
             double b = K::pop(k).num;
             double quot = a / b;
             double rem = fmod(a, b);
             K::push(k, Val(Vtype::Num,
                            std::to_string(rem), rem));
             K::push(k, Val(Vtype::Num,
                            std::to_string(quot), quot));
           })},
  {".",
   new Def(".", "n --", Dtype::Native, {},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (k.size() == 0)
             {
               std::cout << "stack empty" << std::endl;
               return;
             }
             Val* v = K::peek(k);
             string sym = v->sym;
             string s;
             switch (v->type)
             {
             case Vtype::Def:
               // actually should never get here:
               // - Native Defs cannot exist as an entity on the stack.
               //   They get immediately applied.
               // - Phrase Defs cannot exist on the stack. They get
               //   dephrased during clarify().
               if (v->def->type == Dtype::Native)
               {
                 s = "function: " + sym;
               }
               else if (v->def->type == Dtype::Phrase)
               {
                 s = "phrase: " + sym;
               }
               else
               {
                 s = sym;  // should never get here
               }
               break;
             case Vtype::Num:
               s = sym;
               break;
             case Vtype::Str:
               s = "\"" + sym + "\"";
               break;
             default:
               E::err(e, Etype::Apply, k.size(),
                      " word type not recognised");
               return;
             }
             K::pop(k);
             pr(s, true);
           })},
  {"2drop",
   new Def("2drop", "d --", Dtype::Native,
           {Vtype::Any, Vtype::Any},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (!check_param(D::lookup(d, "2drop"), k, e))
             {
               return;
             }
             K::pop(k);
             K::pop(k);
           })},
  {"2dup",
   new Def("2dup", "d -- d d", Dtype::Native,
           {Vtype::Any},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (!check_param(D::lookup(d, "2dup"), k, e))
             {
               return;
             }
             Val d1 = K::pop(k);
             Val d2 = Val(*K::peek(k));
             K::push(k, d1);
             K::push(k, d2);
             K::push(k, Val(d1));
           })},
  {"2over",
   new Def("2over", "d1 d2 -- d1 d2 d1", Dtype::Native,
           {Vtype::Any},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (!check_param(D::lookup(d, "2over"), k, e))
             {
               return;
             }
             Val d1a = K::pop(k);
             Val d1b = K::pop(k);
             Val d2a = K::pop(k);
             Val d2b = Val(*K::peek(k));
             K::push(k, d2a);
             K::push(k, d1b);
             K::push(k, d1a);
             K::push(k, d2b);
             K::push(k, Val(d2a));
           })},
  {"2swap",
   new Def("2swap", "d1 d2 -- d2 d1", Dtype::Native,
           {Vtype::Any, Vtype::Any, Vtype::Any, Vtype::Any},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (!check_param(D::lookup(d, "2swap"), k, e))
             {
               return;
             }
             Val d1a = K::pop(k);
             Val d1b = K::pop(k);
             Val d2a = K::pop(k);
             Val d2b = K::pop(k);
             K::push(k, d1b);
             K::push(k, d1a);
             K::push(k, d2b);
             K::push(k, d2a);
           })},
  {"cr",
   new Def("cr", "--", Dtype::Native, {},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)d;
             (void)h;
             std::cout << std::endl;
           })},
  {"drop",
   new Def("drop", "n --", Dtype::Native,
           {Vtype::Any},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (!check_param(D::lookup(d, "drop"), k, e))
             {
               return;
             }
             K::pop(k);
           })},
  {"dup",
   new Def("dup", "n -- n n", Dtype::Native,
           {Vtype::Any},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (!check_param(D::lookup(d, "dup"), k, e))
             {
               return;
             }
             K::push(k, Val(*K::peek(k)));
           })},
  {"emit",
   new Def("emit", "c --", Dtype::Native,
           {Vtype::Num},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (!check_param(D::lookup(d, "emit"), k, e))
             {
               return;
             }
             int num = atoi(K::pop(k).sym.c_str());
             if (num < 32 || num > 126)
             {
               E::err(e, Etype::Apply, k.size(),  // assume is at top
                      "character not emittable");
               return;
             }
             std::cout << (char)num;
           })},
  {"empty",
   new Def("empty", "--", Dtype::Native, {},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)k;
             (void)h;
             (void)e;
             for (auto &entry: d)
             {
               if (entry.second->type == Dtype::Phrase)
               {
                 D::erase(d, entry.first);
               }
             }
           })},
  {"mod",
   new Def("mod", "u1 u2 -- u-rem", Dtype::Native,
           {Vtype::Num, Vtype::Num},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (!check_param(D::lookup(d, "mod"), k, e))
             {
               return;
             }
             double rem = fmod(K::pop(k).num, K::pop(k).num);
             K::push(k, Val(Vtype::Num,
                            std::to_string(rem), rem));
           })},
  {"over",
   new Def("over", "n1 n2 -- n1 n2 n1", Dtype::Native,
           {Vtype::Any},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (!check_param(D::lookup(d, "over"), k, e))
             {
               return;
             }
             Val v1 = K::pop(k);
             Val* v2 = K::peek(k);
             K::push(k, v1);
             K::push(k, Val(*v2));
           })},
  {"rot",
   new Def("rot", "n1 n2 n3 -- n2 n3 n1", Dtype::Native,
           {Vtype::Any, Vtype::Any, Vtype::Any},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (!check_param(D::lookup(d, "rot"), k, e))
             {
               return;
             }
             Val v1 = K::pop(k);
             Val v2 = K::pop(k);
             Val v3 = K::pop(k);
             K::push(k, v2);
             K::push(k, v1);
             K::push(k, v3);
           })},
  {"space",
   new Def("space", "--", Dtype::Native, {},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)k;
             (void)d;
             (void)h;
             (void)e;
             std::cout << " ";
           })},
  {"spaces",
   new Def("spaces", "n --", Dtype::Native, {Vtype::Num},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (!check_param(D::lookup(d, "spaces"), k, e))
             {
               return;
             }
             int num = atoi(K::pop(k).sym.c_str());
             std::cout << string(num, ' ');
           })},
  {"swap",
   new Def("swap", "n1 n2 -- n2 n1", Dtype::Native,
           {Vtype::Any, Vtype::Any},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)h;
             if (!check_param(D::lookup(d, "swap"), k, e))
             {
               return;
             }
             Val v1 = K::pop(k);
             Val v2 = K::pop(k);
             K::push(k, v1);
             K::push(k, v2);
           })},

  // custom defs
  {".dictionary",
   new Def(".dictionary", "--", Dtype::Native, {},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)k;
             (void)h;
             (void)e;
             D::print(d);
           })},
  {".d", new Def(".d", "--", Dtype::Natphr, {}, ".dictionary")},
  {".history",
   new Def(".history", "--", Dtype::Native, {},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)k;
             (void)d;
             (void)e;
             H::print(h);
           })},
  {".h", new Def(".h", "--", Dtype::Natphr, {}, ".history")},
  {".stack",
   new Def(".stack", "--", Dtype::Native, {},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)d;
             (void)h;
             (void)e;
             K::print(k);
           })},
  {".s", new Def(".s", "--", Dtype::Natphr, {}, ".stack")},
  {".help",
   new Def(".help", "--", Dtype::Native, {},
           [](Stack &k, Dict &d, Hist &h, Err &e)
           {
             (void)k;
             (void)d;
             (void)h;
             (void)e;
             std::cout << ".stack      : print contents of the stack"
                       << "\n.dictionary : print all entries in the dictionary"
                       << "\n.history    : print entire input history"
                       << "\n.version    : print Deck version"
                       << std::endl;
           })},
  {".?", new Def(".?", "--", Dtype::Natphr, {}, ".help")},
  {".version", new Def(".version", "--", Dtype::Natphr, {},
                       ".\" deck version " + VERSION + " \" .")},
  {".v", new Def(".v", "--", Dtype::Natphr, {}, ".version")},
  {"PHI", new Def("PHI", "--", Dtype::Natphr, {}, "1.618033988749894")},
  {"PI", new Def("PI", "--", Dtype::Natphr, {}, "3.141592653589793")},
};

