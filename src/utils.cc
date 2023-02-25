// A few shared utilities throughout pomelo
#include <string>
#include <fstream>
#include "utils.h"

using namespace std;

void string_to_file(const string& text,
                    const string& filename)
{
  ofstream out(filename);
  out << text;
  out.close();
}

string load_string_from_file(const string& filename)
{
  ifstream inf(filename);
  return string(istreambuf_iterator<char>(inf),
                istreambuf_iterator<char>());
}

