#include <iostream>

#include "include/token.h"
#include "include/lexer.h"
#include "include/parser.h"
#include <fstream>

using namespace std;

int main()
{
    Token t;
    ifstream myfile;
    myfile.open("/tmp/pouet");
    Lexer lex(cin);
    Parser parser(lex);
/*
    while ((t = lex.nextToken()) != Token::END) {
        cout << "->" << t.str().c_str() << endl << endl;
    }*/
    parser.parse();
    return 0;
}
