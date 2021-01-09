/**
 * @file   slip.h
 * 
 * @brief  A string class.
 * 
 *  slip is a library of "super-strings". @link ::slip slip @endlink
 *  inherit from strings but adds lots of additional functionality in
 *  the spirit of perl.  This includes regular expression matching as
 *  well as easy transformations between a list @link ::llip
 *  llip @endlink of strings and a strings through operations like
 *  slip::split() and llip::join().
 */
#ifndef	_PLIS_H
#define	_PLIS_H

#include <string.h>
#include <string>
#include <stdarg.h>
#include <deque>
#include "plis_regexp.h"

#define	PLIS_INLINE	inline

namespace plis {
  class substring;
  class llip;
  
  /*! A very featureful perl like string class with regular expression support.
    It is built around the standard std::string which may be accessed
    through the str() method.
    
    The regular expression engine used is the PCRE library, which has been
    wrapped in the Regexp class.
  */
  class slip 
    {
      std::string pstr;
      
    public:
      slip():pstr(){}
      slip(const slip& s) : pstr(s.str()){}     
      slip(const char *s) : pstr(s){}
      slip(const char *s, int n) : pstr(s,n){}
      //    PLIS_INLINE slip(long l);
      slip(const char c) : pstr(1,c) {};
      slip(const substring& sb);
      
      /*! \brief Assignment operator from const char. */
      slip& operator=(const char *s) {
	if (s==NULL)
	  pstr = "";
	else
	  pstr= s;
	return *this;
      }
      /*! \brief Copy assignment. */
      slip& operator=(const slip& s);
      slip& operator=(const substring& sb);
      
      /*! \brief Casting operator to const char* */
      operator const char*() const { return pstr.c_str(); }
      
      /*! \brief Casting operator to std::string */
      operator const std::string() const { return pstr; }
      
      /*! \brief Single character access. */
      const char operator[](int n) const{ return pstr[n]; }
      
      /*! \brief Return the length of the string */
      int length(void) const{ return pstr.size(); }
      
      /*! \brief Erase the last character */
      char chop(void);
      
      /*! \brief Erase the last character if it is a white space character */
      void chomp(void);
      
      /*! \brief Returns position of a string within a string */
      int index(const slip& s, int offset= 0);
      
      /*! \brief Returns position of a string within a from the right string */
      int rindex(const slip& s, int offset= -1);
      
      /*! \brief White space split */
      int wsplit(llip&);
      
      /*! \brief Split a string on boundries of constant string.
	\param str String to split on.
	\retval sl Output slip list
      */
      int strsplit(const slip&str, llip&sl);  // string split
      
      /*! \brief Return a substring of a string */
      substring substr(int offset, int len= -1);
      PLIS_INLINE substring substr(const Range& r);
      
      /*! \brief Like perl tr operator. Translates characters */
      int tr(const char *, const char *, const char *opts="");
      
      /* \brief Match a regular expression.
	 \param r Regular expression
      */
      int m(Regexp& r);
      /* \brief Match a regular expression with optionts
	 \param opts Optional parameters. May be one of:
	 - 'i' - Case insensitive match.
      */
      /*! \brief Match a regular exppression string.
	\param opts Optional parameters. May be one of:
	- 'i' - Case insensitive match.
      */
      int m(const char *, const char *opts=""); // the regexp match m/.../ equiv
      /*! Match a regular expression and return matched groups
	\param opts Matching options
	\retval match_list  The list of the sub groups found. Note that match_list[0]
	is the whole expression and $1 is placed in match_list[1], etc.
      */
      int m(const char *, llip& match_list, const char *opts="");
      
      /* \brief Match a regular expression.
	 \param r Regular expression
	 \retval match_list  The list of the sub groups found. Note that match_list[0]
	 is the whole expression and $1 is placed in match_list[1], etc.
      */
      int m(Regexp&r, llip& match_list);
      /*! \brief like perl/sed s/// operator for regexp substition
       */
      int s(const char *, const char *, const char *opts="");
      
      /*! split a string on a regexp
       */
      llip split(const char *pat= "[ \\t\\n]+", int limit= -1);
      
      int operator<(const slip& s) const { return (strcmp(pstr.c_str(), s) < 0); }
      int operator>(const slip& s) const { return (strcmp(pstr.c_str(), s) > 0); }
      int operator<=(const slip& s) const { return (strcmp(pstr.c_str(), s) <= 0); }
      int operator>=(const slip& s) const { return (strcmp(pstr.c_str(), s) >= 0); }
      int operator==(const slip& s) const { return (strcmp(pstr.c_str(), s) == 0); }
      int operator==(const char *s) const { return (strcmp(pstr.c_str(), s) == 0); }
      int operator!=(const slip& s) const { return (strcmp(pstr.c_str(), s) != 0); }
      int operator!=(const char *s) const { return (strcmp(pstr.c_str(), s) != 0); }
      
      /*! \brief Concatination operator with another slip. */
      slip operator+(const slip& s) const;
      
      /*! \brief Concatination operator with a const char *. */
      slip operator+(const char *s) const;
      
      /*! \brief Concatination operator with a char. */
      slip operator+(char c) const;
      
      /*! \brief Contationation operator */
      friend slip operator+(const char *s1, const slip& s2);
      
      /*! \brief Output to a stream */
      friend std::istream& operator>>(std::istream&, slip&);
      
      /*! \brief Inline concatination with a slip */
      slip& operator+=(const slip& s) { pstr.append(s.str().c_str(), s.length()); return *this;}
      
      /*! \brief Inline concatination with a const char* */
      slip& operator+=(const char *s) { pstr.append(s, strlen(s)); return *this;}
      
      /*! \brief Inline concatination with a char */
      slip& operator+=(char c){ pstr.append(1,c); return *this;}
      friend class substring;
      
      /*! \brief Return the underlying std::string string. */
      const std::string& str() const { return pstr; }
      
      /*! \brief Compatibility function with string. */
      const char *c_str() const { return pstr.c_str(); }
      
      /*! \brief A wrapper for the atoi() function */
      int atoi();
      
      /*! \brief A wrapper for the atof() function */
      double atof();
      
    private:
      void insert(int pos, int len, const char *pt, int nlen);
      
    };

  /*! substrings are a helper class that allow assignement of part of
    slip */
  class substring
    {
    public:
      int pos, len;
      slip& str;
      const char *pt;
      
      friend class slip;
      
    private:
      substring(slip& os, int p, int l) : str(os)
	{
	  if(p > os.length()) p= os.length();
	  if((p+l) > os.length()) l= os.length() - p;
	  pos= p; len= l;
	  if(p == os.length()) pt= 0; // append to end of string
	  else pt= &(((const char*)os)[p]);
	}
    public:
      void operator=(const slip& s)
	{
	  if(&str == &s){ // potentially overlapping
	    std::string tmp((const char*)s);
	    str.insert(pos, len, tmp.c_str(), tmp.length());
	  }
	  else str.insert(pos, len, (const char*)s, s.length());
	}
      
      void operator=(const substring& s)
	{
	  if(&str == &s.str){ // potentially overlapping
	    std::string tmp(s.pt, s.len);
	    str.insert(pos, len, tmp.c_str(), tmp.length());
	  }
	  else str.insert(pos, len, s.pt, s.len);
	}
      
      void operator=(const char *s)
	{
	  str.insert(pos, len, s, strlen(s));
	}
    };

  substring slip::substr(const Range& r)
    {
      return substr(r.start(), r.length());
    }
  
  /*! llip is list of slip with additional string related features.
   Note that it inherits from std::deque and it thus inherits all
   its properties. */
  class llip : public std::deque<slip>
    {
    public:
      llip(int sz=6) {};
      llip(const llip& sl) : std::deque<slip>(sl) {};
      
      /*! \brief join a list on a pattern and return the result */
      slip join(const char *pat= " ");
      
      int split(const char *str, const char *pat= "[ \t\n]+", int limit= -1);
      
      /*! \brief returns the length of the slip list */
      int length() const { return size(); }
      /*! \brief returns the length of the slip list */
      int count() const { return size(); }
      /*! \brief Add a slip at the end of the list */
      void push(const slip& s) { this->push_back(s); }
      /*! \brief Add a llip at the end of the list */
      void push(const llip& sl);
      /*! \brief Add a slip at the beginning of a list */
      void unshift(const slip& s) { this->push_front(s); }
      /*! \brief Add a llip at the beginning of a list */
      void unshift(const llip& sl);
      
      /*! \brief Casting to an integer returns the length of the array. */
      operator const int() const { return size(); }
      
      /*! \brief Chops of the head of the list nand returns it. */
      slip shift() { slip s = this->front(); this->pop_front(); return s; }
      
      /*! \brief Chops of the tail of the list nand returns it. */
      slip pop() { slip s = this->back(); this->pop_back(); return s; }
      
      void replace(int offset, int len, llip& l);
      llip& splice(int offset, int len);
      llip& splice(int offset);
      
      /*! \brief returns a sub list of entries that match the regex */
      llip grep(const char *regex, const char *options = "");
    };

  slip join(llip&, const char *pat=" ");
  void push(llip& sl, const slip& s);

  /** 
   * Create a sups by printf like syntax.
   * 
   * @return 
   */
  slip slipprintf(const char *, ...);

  /** 
   * Create a sups by vprintf like formatting syntax.
   * 
   * @return 
   */
  slip slipvprintf(const char *, va_list args);

  /** 
   * A convenence function for reading a file into a slip.
   * 
   * @param filename File name.
   * @param contents Contains file contents or an error string in case
   *                 of error.
   * 
   * @return Returns 0 on success. -1 otherwise. 
   */
  int slip_read_file(slip filename,
                     // output
                     slip& contents);
};

//std::istream& operator>>(std::istream&, plis::slip&);
//std::ostream& operator<<(std::ostream&, const plis::slip&);
//std::ostream& operator<<(std::ostream&, const plis::llip&);

#define FOREACH(var, array)\
  for (int i=0; i<array.count() && (var=array[i],1);i++) 

#endif

