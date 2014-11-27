#include "../include/token.h"
#include <sstream>
#include <endian.h>

inline std::string char2string(char c);
inline std::string multichar2string(uint16_t c);
inline std::string multichar2string(uint32_t c);
inline std::string multichar2string(uint64_t c);

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
: _type(&type), _str(multichar2string(type.multichr()))
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
    return out << *tok._type << "(\"" << tok.str() << "\")";
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
      out << "'" << multichar2string(tokType.multichr()) << "'";
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

constexpr char TokenType::chr() const {
    return this->_type;
}
constexpr unsigned int TokenType::multichr() const {
    return this->_type;
}





std::string char2string(char c) {
  char str[1+1] = {0};
  str[0] = c;
  return str;
}

std::string multichar2string(uint16_t mchr) {
  mchr = htobe16(mchr);
  char *in = &reinterpret_cast<char&>(mchr);
  char out[sizeof(uint16_t)+1] = {0};
  for (uint16_t i = 0, j = 0; i < sizeof(uint16_t); ++i) {
    if (in[i]) {
        out[j++] = in[i];
    }
  }
  return out;
}

std::string multichar2string(uint32_t mchr) {
  mchr = htobe32(mchr);
  char *in = &reinterpret_cast<char&>(mchr);
  char out[sizeof(uint32_t)+1] = {0};
  for (uint32_t i = 0, j = 0; i < sizeof(uint32_t); ++i) {
    if (in[i]) {
        out[j++] = in[i];
    }
  }
  return out;
}

std::string multichar2string(uint64_t mchr) {
  mchr = htobe64(mchr);
  char *in = &reinterpret_cast<char&>(mchr);
  char out[sizeof(uint64_t)+1] = {0};
  for (uint64_t i = 0, j = 0; i < sizeof(uint64_t); ++i) {
    if (in[i]) {
        out[j++] = in[i];
    }
  }
  return out;
}
