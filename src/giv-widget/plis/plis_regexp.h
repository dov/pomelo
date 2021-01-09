/*
 * version 1.6
 * Regexp is a class that encapsulates the Regular expression
 * stuff. Hopefully this means I can plug in different regexp
 * libraries without the rest of my code needing to be changed.
 * Written by Jim Morris,  jegm@sgi.com
 *
 * Ported to encapsulate PCRE by Dov Grobgeld 2001. The original licence
 * seems to allow any us whatsoever of the code.
 */
#ifndef	_REGEXP_H
#define _REGEXP_H
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <pcre.h>

/*! The range class simply handles a ranged expressed as a start and an end.
  It is used internally in the Regexp and the plis classes.
 */
namespace plis
{
  class Range {
  private:
    int st, en;
    
  public:
    Range()
      {
	st=0; en= -1;
      }
    
    Range(int s, int e)
      {
	st= s; en= e;
      }
    
    int start(void) const { return st;}
    int end(void) const { return en;}
    int length(void) const { return (en-st)+1;}
  };
  
  /*! A C++ wrapper for the PCRE engine. It is used internally by the plis
    library for handling regular expressions.
  */
  class Regexp
    {
    private:
      pcre *repat;
      const char *target; // only used as a base address to get an offset
      int res;
      int iflg;
      int ovector[30];
      int number_of_substrings;
      
    public:
      /*! The constructor takes a regexp string as input and immediately
	compiles it. The compiled version will be saved until the class
	is destroyed. */
      Regexp(const char *rege, const char *flag_string = "")
	{
	  int flags = chars_to_flags(flag_string);
	  const char *error;
	  int error_offset;
	  
	  repat = pcre_compile(rege, flags, &error, &error_offset, NULL);
	  if (repat == NULL)
	    {
	      std::cerr << "pcre_compile() failed!\n";
	      return ;
	    }
	  
	}
      
      ~Regexp()
	{
	  free((char *)repat);
	}    
      
      /*! \brief match a regular expression string */
      int match(const std::string& targ)
	{
	  int result;
	  const char *subject = targ.c_str();  // Shortcut
	  
	  result= pcre_exec(repat,             /* result of pcre_compile() */
			    NULL,              /* we didn't study the pattern */
			    subject,           /* the subject string */
			    targ.size(),       /* the length of the subject string */
			    0,                 /* start at offset 0 in the subject */
			    0,                 /* default options */
			    ovector,           /* vector for substring information */
			    30);               /* number of elements in the vector */
	  
	  if (result > 0)
	    {
	      number_of_substrings = result;
	    }
	  else
	    number_of_substrings = 0;
	  
	  return ((result >= 0) ? 1 : 0);
	}
      
      /*! \brief Returns number of substrings in the last match operation */
      int groups()
	{
	  return number_of_substrings;
	}
      
      /*! \brief Get a group from the last match operation */
      Range getgroup(int n) const
	{
	  // assert(n < number_of_substrings);
	  
	  return Range(ovector[2*n],
		       ovector[2*n+1]-1);
	}
      
    private:
      // Currently only recognizes the perl "i" flag for case
      // independant matching.
      int chars_to_flags(const std::string& flag_string)
	{
	  int flags = 0;
	  if (flag_string.find("i") != std::string::npos)
	    flags |= PCRE_CASELESS;
	  return flags;
	}
      
    };
};
#endif
