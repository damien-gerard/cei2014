#ifndef LOGGER_H
#define LOGGER_H
#include <string>
#include <iostream>
#include "nulostream.h"

namespace Logger {

  extern NulOStream null; // Simple référence vers une sortie vide (tout disparaît)

  extern std::ostream &debug;     // default -> null
  extern std::ostream &info;      // default -> std::cout
  extern std::ostream &warning;   // default -> std::cerr
  extern std::ostream &error;     // default -> std::cerr
  extern std::ostream &critical;  // default -> std::cerr
}

#endif // LOGGER_H
