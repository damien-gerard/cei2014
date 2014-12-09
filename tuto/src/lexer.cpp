
#include "../include/token.h"
#include "../include/lexer.h"
#include <cstdio>


Lexer::Lexer(std::istream& in)
: _in(in)
{
    this->eatChr();
}

int Lexer::eatChr() {
    this->_str += (char) this->_chr;
    this->_chr = this->_in.get();
    return this->_chr;
}

Token Lexer::nextToken() {
    std::string& str = this->_str;
    int&         chr = this->_chr;

    // Enlève tous les espaces
    while (isspace(chr)) {
        this->eatChr();
    }
    str = "";


    switch (chr) {
    case '(':
        this->eatChr();
        return Token::LEFTP;
    case ')':
        this->eatChr();
        return Token::RIGHTP;
    case ',':
        this->eatChr();
        return Token::COMMA;
    case ';':
        this->eatChr();
        return Token::SEMICOL;
    }

    // Identificateur (nom de variable, de fonctions)
    if (isalpha(chr)) {
        // Récupère l'identificateur complet
        while (isalnum(chr)) {
            this->eatChr();
        }

        // Si _str est un mot clé
        if (str == "def") return Token::DEF;
        if (str == "extern") return Token::EXTERN;
        if (str == "if") return Token::IF;
        if (str == "then") return Token::THEN;
        if (str == "else") return Token::ELSE;

        return Token::ID(str);
    }

    // Nombre
    if (isdigit(chr) || chr == '.') {
        // Partie entière
        while (isdigit(chr)) {
            this->eatChr();
        }

        // Partie décimale
        if (chr == '.') {
            this->eatChr();
            do {
                this->eatChr();
            } while (isdigit(chr));
        }

        //dbl = strtod(str.c_str(), 0);
        return Token::NUMBER(str);
    }

    // Commentaires
    if (chr == '#') {
        do {
            this->eatChr();
        } while (chr != EOF && chr != '\n' && chr != '\r');

        // Si on a n'a pas encore atteint la fin de fichier
        // On récupère le token suivant
        if (chr != EOF) {
            return this->nextToken();
        }
    }

    if (chr == EOF) {
        return Token::END;
    }

    this->eatChr();
    return Token::OP(str);
}
