#ifndef UTIL_H
#define UTIL_H

#include <string>


namespace Util {
  std::string& ltrim(std::string&);
  std::string& rtrim(std::string&);
  std::string& trim(std::string&);
  std::string& downcase(std::string&);
  std::string& upcase(std::string&);
}

#endif // UTIL_H
