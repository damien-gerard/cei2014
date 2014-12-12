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

    //friend class Parser;
  protected:
  private:
    std::istream& _in;
    std::string _str;
    int _chr;
    bool _virtualChr;


    inline int eatChr(bool virt = false);
    inline int ensureChr();

    Token eatIdToken();
    Token eatNumToken();
    Token eatStringToken();
    Token eatSymbolToken();
    Token eatOpToken();
    Token eatSingleLineComment();
    Token eatMultilineLineComment();

};

#endif // LEXER_H
