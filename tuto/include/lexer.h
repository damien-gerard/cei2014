#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <iostream>
#include <iterator>

class Token;
class Lexer
{
    public:
        Lexer(std::istream&);
        Token nextToken();

        friend class Parser;
    protected:
    public:
        std::istream& _in;
        std::string _str;
        int _chr;

        inline int eatChr();
};

#endif // LEXER_H
