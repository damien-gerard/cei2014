#include "../../include/util/logger.h"

using namespace std;
namespace Logger {
  NulOStream null;

  ostream &debug = cerr;
  ostream &info = cout;
  ostream &warning = cerr;
  ostream &error = cerr;
  ostream &critical = cerr;
}
