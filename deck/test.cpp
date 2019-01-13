// file: deck/test.cpp
// by  : jooh@cuni.cz
// for : nprg041
// lic.: mit

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "deck.hpp"

SCENARIO("Lang description", "[lang]")
{
  GIVEN("nothing")
  {
    WHEN("motd is executed")
    {
      string s = motd();
      THEN("returned motd string is correct")
      {
        REQUIRE(s == "deck v0.1\n(to exit, type \"bye\", \"exit\", or C-c)");
      }
    }

    WHEN("prompt_ans is executed")
    {
      string s = prompt_ans();
      THEN("returned prompt_ans string is correct")
      {
        REQUIRE(s == "=> ");
      }
    }
  }

  GIVEN("lang")
  {
    Lang* lang;

    WHEN("lang is constructed")
    {
      lang = new Lang;

      THEN("lang exposes dictionary, env, history")
      {
        REQUIRE(lang->dictionary != NULL);
        REQUIRE(lang->env != NULL);
        REQUIRE(lang->history != NULL);
        REQUIRE(string(typeid(lang->dictionary).name())
                == string(typeid(Dict*).name()));
        REQUIRE(string(typeid(lang->env).name())
                == string(typeid(Env*).name()));
        REQUIRE(string(typeid(lang->history).name())
                == string(typeid(Hist*).name()));
      }
    }
  }
}

SCENARIO("Dict description", "[dict]")
{
  GIVEN("lang (->dictionary)")
  {
    Lang* lang;

    WHEN("dictionary is constructed")
    {
      lang = new Lang;

      THEN("dictionary is sized 0")
      {
        REQUIRE(lang->dictionary->size() == 0);
      }
    }
  }
}

SCENARIO("Env description", "[env]")
{
  GIVEN("lang (->env)")
  {
    Lang* lang;

    WHEN("env is constructed")
    {
      lang = new Lang;

      THEN("env exposes tokens, parent")
      {
        REQUIRE(string(typeid(lang->env->parent).name())
                == string(typeid(Env*).name()));
      }
    }
  }
}

SCENARIO("Hist description", "[hist]")
{
  GIVEN("lang (->history)")
  {
    Lang* lang;

    WHEN("history is constructed")
    {
      lang = new Lang;

      THEN("history is sized 0")
      {
        REQUIRE(lang->history->size() == 0);
      }
    }
  }
}

SCENARIO("Hist behavior", "[hist]")
{
  GIVEN("history")
  {
    Lang* lang = new Lang;

    REQUIRE(lang->history->size() == 0);

    WHEN("some string is pushed to history")
    {
      lang->history->push("foo");

      THEN("history size increments")
      {
        REQUIRE(lang->history->size() == 1);
      }

      THEN("top of history is that string")
      {
        REQUIRE(lang->history->peek() == "foo");
      }
    }

    WHEN("history is popped")
    {
      lang->history->pop();

      THEN("history size decrements")
      {
        REQUIRE(lang->history->size() == 0);
      }
    }
  }
}

SCENARIO("lex", "[lex]")
{
  string a_def = ": STAR .\" * \" ;";
  GIVEN("Stack*; Dict; string >>> " + a_def)
  {
    Lang* l = new Lang;
    deck_init(l);
    Value* v;

    WHEN("the \"a_def\" string is lexed")
    {
      int res = lex(a_def, l->stack, l->dictionary);

      THEN("Stack* contains the proper Values")
      {
        REQUIRE(res == 0);
        REQUIRE(l->stack.size() == 6);
        size_t c = 0;
        v = l->stack.pop();
        REQUIRE(v->type == Vtype::Ign);
        REQUIRE(v->sym == ":");
        v = l->stack.pop();
        REQUIRE(v->type == Vtype::Ign);
        REQUIRE(v->sym == "STAR");
        v = l->stack.pop();
        REQUIRE(v->type == Vtype::Ign);
        REQUIRE(v->sym == ".\"");
        v = l->stack.pop();
        REQUIRE(v->type == Vtype::Ign);
        REQUIRE(v->sym == "*");
        v = l->stack.pop();
        REQUIRE(v->type == Vtype::Ign);
        REQUIRE(v->sym == "\"");
        v = l->stack->pop();
        REQUIRE(v->type == Vtype::Def);
        REQUIRE(v->sym == ";");
        REQUIRE(v->def->type == Dtype::Native);
      }
    }
  }

  string an_undef = "c++";
  GIVEN("Stack*; Dict; string >>> " + an_undef)
  {
    Lang* l = new Lang;
    deck_init(l);
    Value* v;

    WHEN("the \"an_undef\" string is lexed")
    {
      int res = lex(an_undef, l->stack, l->dictionary);

      THEN("lex fails; Stack* contains nothing")
      {
        REQUIRE(res == 1);
        REQUIRE(l->stack->size() == 0);
      }
    }
  }

  string an_apply = "1 2 +";
  GIVEN("Stack*; Dict; string >>> " + an_apply)
  {
    Lang* l = new Lang;
    deck_init(l);
    Value* v;

    WHEN("the \"an_apply\" string is lexed")
    {
      int res = lex(an_apply, l->stack, l->dictionary);

      THEN("Stack* contains the proper Values")
      {
        REQUIRE(res == 0);
        REQUIRE(l->stack->size() == 3);
        size_t c = 0;
        v = l->stack->pop();
        REQUIRE(v->type == Vtype::Num);
        REQUIRE(v->sym == "1");
        v = l->stack->pop();
        REQUIRE(v->type == Vtype::Num);
        REQUIRE(v->sym == "2");
        v = l->stack->pop();
        REQUIRE(v->type == Vtype::Def);
        REQUIRE(v->sym == "+");
        REQUIRE(v->def->type == Dtype::Native);
      }
    }
  }
}

