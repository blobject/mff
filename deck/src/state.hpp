// file: deck/src/state.hpp
// by  : jooh@cuni.cz
// for : nprg041

////////////////////////////////////////////////////////////////////////
// handle words one at a time


void consume(vector<string>&);


// match
// - Determine what to do with a word, based on global state and the
//   word itself.

void
match(string word)
{
  // Match state.

  switch (STATE)
  {

  case S::comm:
    // Ignore until ")".
    if (word == ")")
    {
      STATE = S::none;
      return;
    }
    return;

  case S::def:
    // Recognise next word and change to compile state.
    STATE = S::comp;
    DEF_word = word;
    DEF_meaning = vector<string>();
    return;

  case S::comp:
    // Push words onto new meaning vector until ";".
    if (word == ";")
    {
      STATE = S::none;
      def(DEF_word, DEF_meaning);
      return;
    }
    DEF_meaning.push_back(word);
    return;

  case S::fgt:
    // Forget next word.
    STATE = S::none;
    forget(word);
    return;

  case S::str:
    // Build str until '"', then push result onto data stack.
    // TODO: multiline strings
    if (word.back() == '"')
    {
      STATE = S::none;
      if (word.size() > 1)
      {
        word.pop_back(); // remove '"' suffix
        STR = STR.empty() ? word : STR + " " + word;
      }
      DATA.push_back(STR);
      STR = "";
      return;
    }
    STR = STR.empty() ? word : STR + " " + word;
    return;

  case S::cond:
    // Push if-branch onto work stack until "else" or "then".
    // If "else", ignore. If "then", consume.
    if (word == "else")
    {
      STATE = S::alt;
      return;
    }
    if (word == "then")
    {
      STATE = S::none;
      consume(WORK);
      return;
    }
    WORK.push_back(word);
    return;

  case S::alt:
    // Ignore until "else" or "then".
    // If "else", set to recognise. If "then", consume.
    if (word == "else")
    {
      STATE = S::cond;
      return;
    }
    if (word == "then")
    {
      STATE = S::none;
      consume(WORK);
      return;
    }
    return;

  case S::loop:
    // Push onto work stack until "loop", then loop over result.
    if (word == "loop")
    {
      STATE = S::none;
      for (size_t i = DO_index; i < DO_ctrl; ++i)
      {
        for (const auto w: WORK)
        {
          match(w == "i" ? std::to_string(i) : w);
        }
      }
      for (const auto w: WORK)
      {
        WORK.pop_back();
      }
      return;
    }
    WORK.push_back(word);
    return;

  case S::none:
    // Do nothing, and let word matching handle this word.
    break;
  }

  // Match word.

  // If word is native, apply its predefined function.
  if (NAT.find(word) != NAT.end())
  {
    NAT.at(word)();
    return;
  }

  // If word is compiled, recursively match its definition.
  if (DICT.find(word) != DICT.end())
  {
    for (const auto w: DICT.at(word).back())
    {
      match(w);
    }
    return;
  }

  // Push this undefined, unstateful word onto data stack.

  DATA.push_back(word);
}


// consume
// - Match each word in a stack then clear the stack.

void
consume(vector<string> &stack)
{
  for (const auto item: stack)
  {
    match(item);
  }
  clear(stack);
}

