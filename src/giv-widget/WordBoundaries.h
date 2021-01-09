//======================================================================
//  WordBoundaries.h - Non copy parsing of line based syntaxes.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Fri Apr  6 09:23:51 2018
//----------------------------------------------------------------------
#ifndef WORDBOUNDARIES_H
#define WORDBOUNDARIES_H

#include <string>
#include <vector>

// A class containing word boundaries on string.
class WordBoundaries : public std::vector<std::pair<int,int> >
{
private:
  // String under inspection. Now owned and not copied! Must
  // remaind under usage!
  const char *InspectionString;

public:
  WordBoundaries() {
    reserve(100);
  }
  void ParseBoundaries(const char *s);

  // Answer whether the string TestString matches the Index'th word.
  bool CheckMatch(int Index, const char* Candidate) const;

  // Get the rest of a string starting at the position.
  const char *GetRestAsString(int Index);

  // Get a word as a floating point value.
  double GetFloat(int Index);

  // Get a word as an integer value.
  int GetInt(int Index);

  // Get a word as string
  std::string GetWordAsString(int Index) const;
};

#endif /* WORDBOUNDARIES */
