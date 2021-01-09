/*
 * Code taken from splash
 */
#include <stdarg.h>
#include <iostream>
#include <cstring>
#include <malloc.h>
#include <fstream>
#include "plis.h"

using namespace std;
//
// plis stuff
//

namespace plis {
  
// assignments
slip& slip::operator=(const slip& n)
{
  if(this == &n) return *this;
  pstr= n.pstr;
  return *this;
}

slip& slip::operator=(const substring& sb)
{
  if (!sb.pt) {
      pstr = "";
      return *this;
  }
  std::string tmp(sb.pt, sb.len);
  pstr= tmp;
  return *this;
}

// concatenations
slip slip::operator+(const slip& s) const
{
  slip ts(*this);
  ts.pstr+= s.str(); 
  return ts; 
}

slip slip::operator+(const char *s) const
{
  slip ts(*this);
  ts.pstr += s;
  return ts; 
}

slip slip::operator+(char c) const
{
  slip ts(*this);
  ts.pstr += c;
  return ts; 
}

slip operator+(const char *s1, const slip& s2)
{
  slip ts(s1);
  ts = ts + s2;
  //    cout << "s2[0] = " << s2[0] << endl; // gives incorrect error
  return ts; 
}

// other stuff

char slip::chop(void)
{
  int n= length();
  if(n <= 0) return '\0'; // empty
  char tmp= pstr[n-1];
  pstr.erase(n-1);
  return tmp;
}

void slip::chomp(void)
{
  int n= length();
  if (n <= 0)
    return; // empty
  char tmp= pstr[n-1];
  if (tmp == '\n')
    pstr.erase(n-1);
}

int slip::index(const slip& s, int offset)
{
  if(offset < 0) offset= 0;
  for(int i=offset;i<length();i++)
    {
      if(strncmp(&pstr[i], s, s.length()) == 0) return i;
    }

  return -1;
}

int slip::rindex(const slip& s, int offset)
{
  if(offset == -1) offset= length()-s.length();
  else offset= offset-s.length()+1;
  if(offset > length()-s.length()) offset= length()-s.length();
      
  for(int i=offset;i>=0;i--)
    {
      if(strncmp(&pstr[i], s, s.length()) == 0) return i;
    }
  return -1;
}

int slip::wsplit(llip& sl)
{
  int i,cnt;
  slip blank=" ";
  int in_word = 0;
  int word_start_idx;
    
  sl.clear();
  i=0; cnt=0;

  for (i=0; i<(int)pstr.length(); i++)  {
    char ch = pstr[i];
	
    /* printf("%c\n", *p); fflush(stdout); */
    if (!in_word)
      {
	if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r')
	  {
	    in_word = 1;
	    cnt++;
	    word_start_idx = i;
	  }
      }
    else if (in_word)
      {
	if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
	  {
	    sl.push(substr(word_start_idx, i-word_start_idx));
	    in_word = 0;
	  }
      }
  }
  if (in_word)
    sl.push(substr(word_start_idx, pstr.length()-word_start_idx));

  return cnt;
}

int slip::strsplit(const slip& s, llip& sl)
{
  int i,j,cnt;
    
  sl.clear();
  i=0; cnt=0;
  while((j=index(s,i))>=0)
    {
      cnt++;
      push(sl, substr(i, j-i));
      i=j+s.length();
    }
  push(sl, substr(i));
  return cnt+1;
}

llip slip::split(const char *pat, int limit)
{
  llip l;
 
  l.split(*this, pat, limit);
  return l;
}

substring slip::substr(int offset, int len)
{
  if(len == -1)
    len= length() - offset; // default use rest of string
  
  if(offset < 0)
    {
      offset= length() + offset;  // count from end of string
      if(offset < 0) offset= 0;   // went too far, adjust to start
    }
  return substring(*this, offset, len);
}

void slip::insert(int pos, int len, const char *s, int nlen)
{
  pstr.replace(pos,len, s, nlen);
}

//
// I know! This is not fast, but it works!!
//
int slip::tr(const char *sl, const char *rl, const char *opts)
{
  if(length() == 0 || strlen(sl) == 0) return 0;

  int cflg= strchr(opts, 'c') != NULL;
  int dflg= strchr(opts, 'd') != NULL;
  int sflg= strchr(opts, 's') != NULL;

  int cnt= 0, flen= 0;
  slip t;
  unsigned char lstc= '\0', fr[256];
  int i;
    
  // build search array, which is a 256 byte array that stores the index+1
  // in the search string for each character found, == 0 if not in search
  memset(fr, 0, 256);
  for(i=0;i<(int)strlen(sl);i++)
    {
      if(i && sl[i] == '-')
	{ // got a range
          assert(i+1 < (int)strlen(sl) && lstc <= sl[i+1]); // sanity check
	  for(unsigned char c=lstc+1;c<=sl[i+1];c++)
	    {
	      fr[c]= ++flen;
	    }
	  i++; lstc= '\0';
	}
      else
	{
	  lstc= sl[i];
	  fr[(int)sl[i]]= ++flen;
	}
    }

  int rlen;
  // build replacement list
  if((rlen=strlen(rl)) != 0)
    {
      for(i=0;i<rlen;i++)
	{
	  if(i && rl[i] == '-') // got a range
	    { 
	      assert(i+1 < rlen && t[t.length()-1] <= rl[i+1]); // sanity check
	      for(char c=t[i-1]+1;c<=rl[i+1];c++) t += c;
	      i++;
	    }
	  else
	    t += rl[i];
	}
    }
  
  // replacement string that is shorter uses last character for rest of string
  // unless the delete option is in effect or it is empty
  while(!dflg && rlen && flen > t.length())
    {
      t += t[t.length()-1]; // duplicate last character
    }

  rlen= t.length(); // length of translation string   
   
  // do translation, and deletion if dflg (actually falls out of length of t)
  // also squeeze translated characters if sflg

  slip tmp; // need this in case dflg, and string changes size
  for(i=0;i<length();i++)
    {
      int off;
      if (cflg) // complement, ie if NOT in f
	{ 
	  char rc= !dflg ? t[t.length()-1] : '\0'; // always use last character for replacement
	  if((off=fr[(int)((*this)[i])]) == 0) // not in map
	    { 
	      cnt++;
	      if(!dflg && (!sflg || tmp.length() == 0 || tmp[tmp.length()-1] != rc))
		tmp += rc;
	    }
	  else tmp += (*this)[i]; // just stays the same
	}
      else
	{ // in fr so substitute with t, if no equiv in t then delete
          if((off=fr[(int)((*this)[i])]) > 0)
	    {
	      off--; cnt++;
	      if(rlen==0 && !dflg && (!sflg || tmp.length() == 0 || tmp[tmp.length()-1] != (*this)[i])) tmp += (*this)[i]; // stays the same
	      else if(off < rlen && (!sflg || tmp.length() == 0 || tmp[tmp.length()-1] != t[off]))
		tmp += t[off]; // substitute
	    }
	  else tmp += (*this)[i]; // just stays the same
	}
    }

  *this= tmp;
  return cnt;
}

int slip::m(Regexp& r)
{
  return r.match(this->str());
}
 
int slip::m(const char *pat, const char *opts)
{
  Regexp r(pat, opts);
  return m(r);
}
 
int slip::m(Regexp& r, llip& psl)
{
  if(!r.match(this->str())) return 0;
  psl.clear();    // clear it first

  for (int i=0; i<r.groups(); i++)
    {
      Range rng= r.getgroup(i);
      psl.push(substr(rng.start(), rng.length()));
    }
  return r.groups();
}
 
int slip::m(const char *pat, llip& psl, const char *opts)
{
  Regexp r(pat, opts);
  return m(r, psl);
}
 
int slip::s(const char *exp, const char *repl, const char *opts)
{
  int gflg= strchr(opts, 'g') != NULL;
  int mflg= strchr(opts, 'm') != NULL;

  int cnt= 0;
  Regexp re(exp, opts);
  Range rg;
 
  if(re.match(this->str()))
    {
      // OK I know, this is a horrible hack, but it seems to work
      if(gflg)
	{
          // Special case for beginning of line
          if ((slip)exp == "^")
            {
              this->substr(0,0) = repl;
              cnt = 1;
              if (mflg)
                {
                  // recurse for rest of line endings
                  cnt+= s("\n", (slip)"$&"+repl, opts);
                }
              return cnt;
            }
          else
            {
              // recursively call s() until applied to whole string
              rg= re.getgroup(0);
              slip st(substr(rg.end()+1));
              cnt += st.s(exp, repl, opts);
              substr(rg.end()+1)= st;
            }
        }
      if(!strchr(repl, '$')) // straight, simple substitution
	{ 
	  rg= re.getgroup(0);
	  substr(rg.start(), rg.length())= repl;
	  cnt++;      
	}
      else
	{ // need to do subexpression substitution
	  char c;
	  const char *src= repl;
	  slip dst;
	  int no;
	  while ((c = *src++) != '\0')
	    {
	      if(c == '$' && *src == '&')
		{
		  no = 0; src++;
		}else if(c == '$' && '0' <= *src && *src <= '9')
		  no = *src++ - '0';
	      else no = -1;
 
	      if(no < 0)
		{     /* Ordinary character. */
		  if(c == '\\' && (*src == '\\' || *src == '$'))
		    c = *src++;
		  dst += c;
		}else{
		  rg= re.getgroup(no);
		  dst += substr(rg.start(), rg.length());
		}
	    }
	  rg= re.getgroup(0);
	  substr(rg.start(), rg.length())= dst;
	  cnt++;
	}
 
      return cnt;
    }
  return cnt;
}
 
int slip::atoi()
{
  return ::atoi(*this);
}
 
double slip::atof()
{
  return ::atof(*this);
}
 
slip llip::join(const char *pat)
{
  slip joined_string = "";
  for (int i=0;i<count();i++)
    {
      joined_string += (*this)[i];
      if (i<count()-1)
        joined_string += pat;
    }
  return joined_string;
}

int llip::split(const char *str, const char *pat, int limit)
{
  Regexp re(pat);
  Range rng;
  slip s(str);
  int cnt_char= 1;

  clear();
  if(*pat == '\0')
    { // special empty string case splits entire thing
      while(*str)
	{
	  s= *str++;
	  push(s);
	}
      return count();
    }
 
  if(strcmp(pat, "' '") == 0)
    { // special awk case
      char *p;
      const char ws[]= " \t\n";
      char *t = strdup(str); // can't hack users data
      p= strtok(t, ws);
      while(p)
	{
	  push(p);
	  p= strtok(NULL, ws);
	}
      free(t);
      return count();
    }
 
  while(re.match(s.str()) && (limit < 0 || cnt_char < limit))
    { // find separator
      rng= re.getgroup(0); // full matched string (entire separator)
      if (rng.start() > 0)
          push(s.substr(0, rng.start()));
      for(int i=1;i<re.groups();i++)
	{
	  push(s.substr(re.getgroup(i))); // add subexpression matches
	}
        
      s= s.substr(rng.end()+1);
      cnt_char++;
    }
  if(s.length()) push(s);

#if 0  
  if(limit < 0)
    { // strip trailing null entries
      int off= count()-1;
      while(off >= 0 && (*this)[off].length() == 0)
	{
	  off--;
	}
      pop_back();
    }
#endif
  return count();
}

llip llip::grep(const char *rege, const char *opts)
{
  llip return_list;
  
  Regexp rexp(rege, opts);    // compile once
  for(int i=0;i<(int)size();i++)
    {
      if(rexp.match((*this)[i].str()))
	{
	  return_list.push((*this)[i]);
	}
    }
  return return_list;
}

void llip::push(const llip& sl)
{
  for (int i=0; i<sl.length(); i++) {
    this->push(sl[i]);
  }
}

void llip::unshift(const llip& sl)
{
  for (int i=sl.length()-1; i>=0; i--) {
    this->unshift(sl[i]);
  }
}

slip join(llip& sl, const char *pat)
{
  slip ts;

  for (int i=0;i<sl.count();i++)
    {
      ts+= sl[i];
      if (i<sl.count()-1) ts += pat;
    }
  return ts;
}

slip slipprintf(const char *fmt, ...)
{
  va_list args;
  
  va_start(args,fmt);
  return slipvprintf(fmt, args);
}

slip slipvprintf(const char *fmt, va_list args) 
{
  int len;
  char *buffer;
  va_list args_tmp;

    /* Estimate the result as twice the fmt size. It would be much more
       efficient here to search for %s arguments and allocate according to t
       he size of the corresponding argument...
    */
    len = strlen(fmt)*2;
    
    if (len < 32)  // No point in allocating very small strings...
      len = 32;
    
    
    buffer = (char*)malloc(len);
    while (1)
      {
        /* According to the stdarg man page, the value of va_list parameter is
           undefined after the call of the function which processes this list.
           To preserve its value, we need a working copy. */
        va_copy(args_tmp, args);
        /* Try to print in the allocated space. */
        int nchars = vsnprintf(buffer, len, fmt, args_tmp);
        /* Free the working copy */
        va_end(args_tmp);

        /* If that worked, return the string. */
        if (nchars >= 0 && nchars < len)
          break;
        
	/* Else try again with twice as much space. */
	len *= 2;
	buffer = (char *)realloc(buffer, len);
      }
    
    slip s(buffer);
    free(buffer);
    
    return s;
}

int slip_read_file(slip filename,
                   // output
                   slip& contents)
{
    slip s;
    ifstream inf;

    inf.open(filename.c_str());

    if (!inf.is_open()) {
        contents = "Failed opening file!\n";
        return -1;
    }

    contents = "";
    while(inf >> s) 
        contents += s;

    inf.close();

    return 0;
}


void push(llip& sl, const slip& s)
{
  sl.push(s);
}

// streams stuff
istream& operator>>(istream& ifs, slip& s)
{
    char c;
    char buf[255];

    s= ""; // empty string

    // special treatment of empty lines as ifs enters a good()=false
    // state if we use get below on an empty line.
    if (ifs.peek() == '\n') {
        ifs.ignore();
        s = "\n";
        return ifs;
    }
    
    ifs.get(buf, sizeof buf);
    
    // This is tricky because a line teminated by end of file that is not terminated
    // with a '\n' first is considered an OK line, but ifs.good() will fail.
    // This will correctly return the last line if it is terminated by eof with the
    // stream still in a non-fail condition, but at eof, so next call will fail as

    // expected
    if(ifs) { 		// previous operation was ok
        s += buf; 	// append buffer to string
        //	cout << "<" << buf << ">" << endl;
        // if its a long line continue appending to string
        while(ifs.good() && (c=ifs.get()) != '\n') {
            ifs.putback(c);
            //	    cout << "eof1= " << ifs.eof() << endl;
            //          cout << "eof2= " << ifs.eof() << endl;
            if(ifs.get(buf, sizeof buf)) s += buf; // append to line
        }
    }
    
    s+= '\n';  // This shouldn't be done if there is no newline at the
               // end of the file.

    return ifs;    
}

ostream& operator<<(ostream& os,  const slip&s)
{
#ifdef TEST
  os << "(" << arr.length() << ")" << "\"";
  os << (const char *)s;
  os << "\"";
#else
  os << (const char *)s;
#endif
  return os;   
}

slip::slip(const substring& sb) : pstr(sb.pt, sb.len) {}

std::ostream& operator<<(std::ostream& os, const llip& arr)
{
  os << "[";
  for (int i=0; i<arr.length(); i++)
    {
      os << arr[i];
      if (i<arr.length()-1)
	{ os << " "; }
    }
  os << "]";
  return os;
}

}
