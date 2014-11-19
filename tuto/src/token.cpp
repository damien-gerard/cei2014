#include "../include/token.h"


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
constexpr TokenType Token::NIL;


Token::Token()
: _type(&Token::NIL), _str(""+(char)0)
{}
Token::Token(const TokenType& type)
: _type(&type), _str(""+type.sym())
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
