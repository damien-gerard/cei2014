#ifndef TOKEN_H
#define TOKEN_H


#include <string>
#include <cstdlib>


enum class TokenType
{
  ENDF,     // End of File
  ENDL,     // End of Line

  ID,       // Identifier
  NUM,      // Numerical literal
  STRING,   // String literal

  DOLLAR,   // $
  DIAMOND,  // <>

  LEFTP,    // (
  RIGHTP,   // )
  LEFTB,    // {
  RIGHTB,   // }
  SEMICOL,  // ;
  COLON,    // :

  IF,       // If
  ELSE,     // Else
  ENDIF,    // End If

  OP,       // operators

  NIL       // undefined token
};

std::ostream& operator<<(std::ostream& out, TokenType);

class Token
{
  public:
    Token();
    Token(TokenType);
    Token(TokenType, const std::string&);
    Token(const Token&);
    ~Token();
    Token& operator=(const Token&);
    Token& operator=(TokenType);
    inline bool operator==(TokenType t) const { return this->_type == t; }
    inline bool operator!=(TokenType t) const { return this->_type != t; }
    inline operator int() const { return int(this->_type); }

    inline std::string str() const { return this->_str; }
    inline double dbl() const { return strtod(this->_str.c_str(), 0); }

    friend std::ostream& operator<<(std::ostream& out, const Token&);
  protected:
  private:
    TokenType _type;
    std::string _str;
};


#endif // TOKEN_H
