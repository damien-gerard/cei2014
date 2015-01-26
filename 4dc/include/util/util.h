#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <iostream>
#include <sstream>
#include <vector>


namespace Util {
  std::string& ltrim(std::string&);
  std::string& rtrim(std::string&);
  std::string& trim(std::string&);
  std::string& downcase(std::string&);
  std::string& upcase(std::string&);
  double str2dbl(const std::string&);
  long str2long(const std::string&);
  template<class T>
  std::string join(const std::vector<T>&, const std::string&);
  template<class T>
  std::ostream& join(std::ostream&, const std::vector<T>&, const std::string&);
}



template<class T>
std::string Util::join(const std::vector<T>& vec, const std::string& sep)
{
  std::stringstream ss;
  return Util::join(ss, vec, sep).str();
}
template<class T>
std::ostream& Util::join(std::ostream& out, const std::vector<T>& vec, const std::string& sep)
{
  bool first = true;
  for (auto& printable : vec) {
    if (first) {
      first = false;
    } else {
      out << sep;
    }
    out << printable;
  }
  return out;
}

#endif // UTIL_H
