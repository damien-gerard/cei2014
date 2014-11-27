#include "../include/token.h"
#include <sstream>

std::string char2string(char c) {
  std::stringstream ss;
  ss.put(c);
  return ss.str();
}

constexpr TokenType Token::END;
constexpr TokenType Token::DEF;
constexpr TokenType Token::EXTERN;
constexpr TokenType Token::ID;
constexpr TokenType Token::NUMBER;
constexpr TokenType Token::LEFTP;
constexpr TokenType Token::RIGHTP;
constexpr TokenType Token::COMMA;
constexpr TokenType Token::SEMICOL;
constexpr TokenType Token::OP;
constexpr TokenType Token::IF;
constexpr TokenType Token::THEN;
constexpr TokenType Token::ELSE;
constexpr TokenType Token::NIL;


Token::Token()
: _type(&Token::NIL), _str("")
{}
Token::Token(const TokenType& type)
: _type(&type), _str(char2string(type.sym()))
{}
Token::Token(const TokenType& type, const std::string& str)
: _type(&type), _str(str)
{}

Token::~Token()
{}

bool Token::operator==(TokenType type) {
    return type == *this;
}

bool Token::operator!=(TokenType type) {
    return type != *this;
}


Token& Token::operator=(const Token& tok) {
    if (this == &tok) {return *this;}
    this->_type = tok._type;
    this->_str = tok._str;
    return *this;
}

Token::operator int() const {
    return int(*this->_type);
}


std::ostream& operator<<(std::ostream& out, const Token& tok) {
    return out << tok._type << "(\"" << tok.str() << "\")";
}
std::ostream& operator<<(std::ostream& out, const TokenType& tokType) {
  out << "Token::";
  switch (tokType) {
  case Token::END:
      out << "END";
      break;
  case Token::DEF:
      out << "DEF";
      break;
  case Token::EXTERN:
      out << "EXTERN";
      break;
  case Token::ID:
      out << "ID";
      break;
  case Token::NUMBER:
      out << "NUMBER";
      break;
  case Token::LEFTP:
      out << "LEFTP";
      break;
  case Token::RIGHTP:
      out << "RIGHTP";
      break;
  case Token::COMMA:
      out << "COMMA";
      break;
  case Token::SEMICOL:
      out << "SEMICOL";
      break;
  case Token::OP:
      out << "OP";
      break;
  case Token::IF:
      out << "IF";
      break;
  case Token::THEN:
      out << "THEN";
      break;
  case Token::ELSE:
      out << "ELSE";
      break;
  case Token::NIL:
      out << "NIL";
      break;
  default:
      out << "'" << char2string(tokType.sym()) << "'";
  }
  return out;
}



/*
constexpr TokenType::TokenType(const int n)
: _type(type)
{}
*/
constexpr bool TokenType::operator==(const Token& tok) const {
    return *tok._type == *this;
}

constexpr bool TokenType::operator!=(const Token& tok) const {
    return *tok._type != *this;
}

constexpr bool TokenType::operator==(TokenType tokType) const {
    return tokType._type == this->_type;
}

constexpr bool TokenType::operator!=(TokenType tokType) const {
    return tokType._type != this->_type;
}

Token TokenType::operator()() const {
    return Token(*this);
}
Token TokenType::operator()(const std::string& str) const {
    return Token(*this, str);
}

constexpr char TokenType::sym() {
    return (this->_type < 0) ? -this->_type : this->_type;
}
