#include <iostream>

#include "include/token.h"
#include "include/lexer.h"
#include "include/parser.h"
#include "include/builder.h"
#include <fstream>

using namespace std;

int main()
{
    Token t;
    ifstream myfile;
    //myfile.open("/tmp/test");
    Lexer lex(cin);
    Parser parser(lex);
    Builder builder(parser);
    builder.createJIT();
    builder.setOptimizer(builder.getStandardOptimizer());

/*
    while ((t = lex.nextToken()) != Token::END) {
        cout << "->" << t.str().c_str() << endl << endl;
    }*/
    builder.build();
    return 0;
}
