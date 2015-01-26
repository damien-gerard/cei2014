#include "../../include/util/util.h"
#include <sstream>
#include <algorithm>

using namespace std;



namespace Util {
  string& ltrim(string& str)
  {
    str.erase(str.begin(), find_if(str.begin(), str.end(), not1(ptr_fun<int, int>(isspace))));
    return str;
  }
  string& rtrim(string& str)
  {
    str.erase(std::find_if(str.rbegin(), str.rend(), not1(ptr_fun<int, int>(isspace))).base(), str.end());
    return str;
  }
  string& trim(string& str)
  {
    return ltrim(rtrim(str));
  }

  string& downcase(string& str)
  {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
  }

  string& upcase(string& str)
  {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
  }
  
  double str2dbl(const std::string& str)
  {
    return strtod(str.c_str(), 0);
  }
  
  long str2long(const std::string& str)
  {
    return strtol(str.c_str(), nullptr, 0);
  }
}
