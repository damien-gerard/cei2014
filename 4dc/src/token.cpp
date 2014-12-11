#include "../include/token.h"
#include <iostream>

inline std::string tokenTypeToString(TokenType tokType);



Token::Token()
  : _type(TokenType::NIL)
{}
Token::Token(TokenType type)
  : _type(type), _str(tokenTypeToString(type))
{}
Token::Token(TokenType type, const std::string& str)
  : _type(type), _str(str)
{}
Token::Token(const Token&) = default;
Token::~Token() = default;

Token& Token::operator=(const Token& tok)
{
  if (this == &tok) {
    return *this;
  }
  this->_type = tok._type;
  this->_str = tok._str;
  return *this;
}

Token& Token::operator=(TokenType type)
{
  this->_type = type;
  return *this;
}



std::ostream& operator<<(std::ostream& out, const Token& tok)
{
  return out << "Token::" << tok._type << "(\"" << tok.str() << "\")";
}

std::ostream& operator<<(std::ostream& out, TokenType tokType)
{
  switch (tokType) {
  case TokenType::ENDF:
    out << "ENDF";
    break;
  case TokenType::ENDL:
    out << "ENDL";
    break;
  case TokenType::ID:
    out << "ID";
    break;
  case TokenType::NUM:
    out << "NUM";
    break;
  case TokenType::STRING:
    out << "STRING";
    break;
  case TokenType::DOLLAR:
    out << "DOLLAR";
    break;
  case TokenType::DIAMOND:
    out << "DIAMOND";
    break;
  case TokenType::LEFTP:
    out << "LEFTP";
    break;
  case TokenType::RIGHTP:
    out << "RIGHTP";
    break;
  case TokenType::LEFTB:
    out << "LEFTB";
    break;
  case TokenType::RIGHTB:
    out << "RIGHTB";
    break;
  case TokenType::SEMICOL:
    out << "SEMICOL";
    break;
  case TokenType::COLON:
    out << "COLON";
    break;
  case TokenType::IF:
    out << "IF";
    break;
  case TokenType::ELSE:
    out << "ELSE";
    break;
  case TokenType::ENDIF:
    out << "ENDIF";
    break;
  case TokenType::OP:
    out << "OP";
    break;
  case TokenType::NIL:
    out << "NIL";
    break;
  }
  return out;
}

std::string tokenTypeToString(TokenType tokType) {
  switch (tokType) {
  case TokenType::ENDF:
    return "EOF";
  case TokenType::ENDL:
    return "\\n";
  case TokenType::ID:
    return "ID";
  case TokenType::NUM:
    return "NUM";
  case TokenType::STRING:
    return "STRING";
  case TokenType::DOLLAR:
    return "$";
  case TokenType::DIAMOND:
    return "<>";
  case TokenType::LEFTP:
    return "(";
  case TokenType::RIGHTP:
    return ")";
  case TokenType::LEFTB:
    return "{";
  case TokenType::RIGHTB:
    return "}";
  case TokenType::SEMICOL:
    return ";";
  case TokenType::COLON:
    return ":";
  case TokenType::IF:
    return "If";
  case TokenType::ELSE:
    return "Else";
  case TokenType::ENDIF:
    return "End If";
  case TokenType::OP:
    return "OP";
  case TokenType::NIL:
    return "";
  }
  return "";
}

