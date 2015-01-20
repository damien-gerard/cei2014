#include "../include/token.h"
#include "../include/lexer.h"
#include <cstdio>
#include <cassert>
#include "../include/util/logger.h"
#include "../include/util/util.h"

/**
 * Helpers
 **/
namespace {
  inline bool isSpace(int chr);
  inline bool isEndLine(int chr);
  inline bool isAlpha(int chr);
  inline bool isAlphaNum(int chr);
  inline bool isDigit(int chr);
}


Lexer::Lexer(std::istream& in)
  : _in(in), _virtualChr(true)
{}


Token Lexer::nextToken()
{
  this->ensureChr();

  // Enlève tous les espaces
  // Les espaces à l'avant d'un token peuvent toujours être ignorés
  while (isSpace(this->_chr)) {
    this->eatChr();
  }
  this->_str = "";

  // Renvoie un token fin de ligne si on arrive en bout de ligne
  assert(this->_chr != '\r');
  if (isEndLine(this->_chr)) {
    this->eatChr(true);
    return TokenType::ENDL;
  }

  // Identificateur (nom de variable, de fonctions)
  if (isAlpha(this->_chr) || this->_chr == '_') return this->eatIdToken();

  // Nombre
  if (isDigit(this->_chr) || this->_chr == '.') return this->eatNumToken();

  if (this->_chr == EOF) {
    return TokenType::ENDF;
  }

  // String
  if (this->_chr == '"') return this->eatStringToken();

  return this->eatSymbolToken();
}


Token Lexer::eatIdToken()
{
  // Récupère l'identificateur complet
  while (isAlphaNum(this->_chr) || isSpace(this->_chr) || this->_chr == '_') {
    this->eatChr();
  }

  Util::downcase(Util::trim(this->_str));

  if (this->_str == "if") return TokenType::IF;
  if (this->_str == "else") return TokenType::ELSE;
  if (this->_str == "end if") return TokenType::ENDIF;
  
  if (this->_str == "for") return TokenType::FOR;
  if (this->_str == "end for") return TokenType::ENDFOR;

  if (this->_str == "while") return TokenType::WHILE;
  if (this->_str == "end while") return TokenType::ENDWHILE;
  
  if (this->_str == "repeat") return TokenType::REPEAT;
  if (this->_str == "until") return TokenType::UNTIL;
  
  return Token(TokenType::ID, this->_str);
}




Token Lexer::eatNumToken()
{
  // Partie entière
  while (isDigit(this->_chr)) {
    this->eatChr();
  }

  // Partie décimale
  if (this->_chr == '.') {
    this->eatChr();
    do {
      this->eatChr();
    } while (isDigit(this->_chr));
  }

  return Token(TokenType::NUM, this->_str);
}

Token Lexer::eatStringToken()
{
  this->eatChr();
  this->_str = "";
  while (this->_chr != '"') {
    if (this->_chr == EOF) {
      Logger::error << "Lex Error: Expected '\"' but found EOF" << std::endl;
      exit(EXIT_FAILURE);
      return TokenType::ENDF;
    }
    this->eatChr();
  }
  Token t(TokenType::STRING, this->_str);
  this->eatChr();
  return t;
}

Token Lexer::eatSymbolToken()
{
  int chr0, chr1;
  chr0 = this->_chr;
  chr1 = this->eatChr();

  // Symboles spéciaux
  switch (chr0) {
  case '(':
    return TokenType::LEFTP;
  case ')':
    return TokenType::RIGHTP;
  case '{':
    return TokenType::LEFTB;
  case '}':
    return TokenType::RIGHTB;
  case ';':
    return TokenType::SEMICOL;
  case ':':
    if(chr1=='='){  // opérateur :=
	  this->eatChr();
      return TokenType::AFFECT;
	}
    return TokenType::COLON;
  case '$':
    return TokenType::DOLLAR;
  case '<':
  case '>':
    if(chr1=='='){ //operator '<=' or '>='
	  this->eatChr();
	}
	return this->eatOpToken();
  }

  // Diamond
  if (chr0 == '<' && chr1 == '>') {
    this->eatChr();
    return TokenType::DIAMOND;
  }

  // Single Line commenta
  if (chr0 == '/' && chr1 == '/') return this->eatSingleLineComment();

  // Multi Line comment
  if (chr0 == '/' && chr1 == '*') return this->eatMultilineLineComment();

  return this->eatOpToken();
}
Token Lexer::eatOpToken()
{
  return Token(TokenType::OP, this->_str);
}

Token Lexer::eatSingleLineComment()
{
  Logger::warning << "Lex Warning: Comments not stored" << std::endl;
  while (!isEndLine(this->_chr)) {
    if (this->_chr == EOF) {
      return TokenType::ENDF;
    }
    this->eatChr();
  }
  return this->nextToken();
}


Token Lexer::eatMultilineLineComment()
{
  Logger::warning << "Lex Warning: Comments not stored" << std::endl;
  int chr0, chr1;
  chr1 = this->eatChr();
  do {
    chr0 = chr1;
    chr1 = this->eatChr();
    if (this->_chr == EOF) {
      return TokenType::ENDF;
    }
  } while (chr0 != '*' && chr1 != '/');
  this->eatChr();
  return this->nextToken();
}




int Lexer::eatChr(bool virt)
{
  this->_str += (char) this->_chr;
  if (virt) {
    this->_virtualChr = true;
    return 0;
  }

  this->_virtualChr = false;

  this->_chr = this->_in.get();

  // Convertit le symbole de fin de ligne en '\n'
  if (this->_chr == '\r') {
    if (this->_in.get() != '\n') {
      this->_in.unget();
    }
    this->_chr = '\n';
  }


  return this->_chr;
}

int Lexer::ensureChr()
{
  if (this->_virtualChr) {
    return this->eatChr();
  }
  return this->_chr;
}



namespace {
  bool isSpace(int chr) {
    return isspace(chr) && !isEndLine(chr);
  }
  bool isEndLine(int chr) {
    return chr == '\r' || chr == '\n';
  }
  bool isAlpha(int chr) {
    return isalpha(chr);
  }
  bool isAlphaNum(int chr) {
    return isalnum(chr);
  }
  bool isDigit(int chr) {
    return isdigit(chr);
  }
}
