#include <iostream>

#include "include/token.h"
#include "include/lexer.h"
#include <fstream>

using namespace std;

int main()
{
  Token t;
  ifstream myfile;
  //myfile.open("/tmp/test");
  Lexer lex(cin);

  while ((t = lex.nextToken()) != TokenType::ENDF) {
    cout << "->" << t << endl;
  }
  return 0;
}
