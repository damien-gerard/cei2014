#include <iostream>

#include "include/builder.h"
#include "include/util/logger.h"
#include "include/util/file.h"
#include "include/util/util.h"
#include <fstream>
#include <vector>
#include <tuple>

using namespace std;

void compileFile(istream&, Builder&);

int main(int argc, char *argv[])
{
  vector<pair<string, File>> files;
  
  bool readSTDIN = true;
  for(int i =1; i < argc; i++){
    readSTDIN = false;
    std::string current_exec_name = argv[i];
    if (current_exec_name == "-") {
      readSTDIN = true;
      break;
    }
    files.emplace_back(Util::basename(current_exec_name), File(current_exec_name));

  }

  if (readSTDIN) {
    files.emplace_back("stdin", File(""));
  }
  
  Builder::buildAll(files);
  
  return 0;
}

