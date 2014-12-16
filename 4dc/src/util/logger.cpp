#include "../../include/util/logger.h"

using namespace std;
namespace Logger {
  NulOStream null;

  ostream &debug = null;
  ostream &info = cout;
  ostream &warning = cerr;
  ostream &error = cerr;
  ostream &critical = cerr;
}
