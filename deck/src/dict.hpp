// file: deck/src/dict.hpp
// by  : jooh@cuni.cz
// for : nprg041

////////////////////////////////////////////////////////////////////////
// dictionary


// def
// - Update dictionary with (new) meaning of a (new or old) word.

void
def(string word, vector<string> meaning)
{
  // Check for recursive definition.
  for (const auto w: meaning)
  {
    if (w == DEF_word)
    {
      err("def", "recursion");
      return;
    }
  }

  // New word.
  if (DICT.find(word) == DICT.end())
  {
    vector<vector<string>> meanings = {meaning};
    DICT.insert(pair<string,vector<vector<string>>>(word, meanings));
  }

  // Old word.
  else
  {
    vector<vector<string>> meanings = DICT.at(word);
    vector<string> last = meanings.back();
    size_t size = meaning.size();

    // Check for duplicate definition.
    bool same = true;
    for (size_t i = 0; i < last.size(); ++i)
    {
      if (i >= size || last[i] != meaning[i])
      {
        same = false;
        break;
      }
    }
    if (same)
    {
      err("def", "duplicate");
      return;
    }

    // Update dictionary.
    meanings.push_back(meaning);
    DICT.erase(DICT.find(word));
    DICT.insert(pair<string,vector<vector<string>>>(word, meanings));
  }
}


// forget
// - Forget latest meaning of a word.

void
forget(string word)
{
  // Check if word is defined.
  if (DICT.find(word) == DICT.end())
  {
    ostringstream s;
    s << '"' << word << "\" is not in dictionary";
    err("forget", s.str());
    return;
  }

  // Update dictionary.
  vector<vector<string>> meanings = DICT.at(word);
  meanings.pop_back();
  DICT.erase(DICT.find(word));
  if (meanings.size() > 0)
  {
    DICT.insert(pair<string,vector<vector<string>>>(word, meanings));
  }
}

