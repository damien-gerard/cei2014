#ifndef TOKEN_H
#define TOKEN_H


#include <string>
/**
 * Token peut s'utiliser exactement comme un enum class
 * Mais possède des attributs ainsi que des méthodes
 *
 * Il par exemple possible d'écrire Token::ID("a")
 * pour avoir un Token ID ayant la valeur "a"
 *
 * Il est aussi possible d'écrire un switch aussi naturellement
 * qu'avec un enum class
 */

class Token;
class TokenType
{
    public:
        constexpr TokenType(const unsigned int type):_type(type){};
        constexpr bool operator==(const Token&);
        constexpr bool operator!=(const Token&);
        constexpr bool operator==(TokenType);
        constexpr bool operator!=(TokenType);
        Token operator()() const;
        Token operator()(const std::string&) const;
        constexpr operator int() {return this->_type;}
        constexpr char chr();
        constexpr unsigned int multichr();
        friend std::ostream& operator<<(std::ostream& out, const TokenType& tokType);
    private:
        const int _type;
};

class Token
{
    public:
        Token();
        Token(const TokenType&);
        Token(const TokenType&, const std::string&);
        Token(const Token&) = default;
        ~Token();
        Token& operator=(const Token&);
        bool operator==(TokenType);
        bool operator!=(TokenType);
        operator int() const;

        inline std::string str() const {return this->_str;}
        inline double dbl() const {return strtod(this->_str.c_str(), 0);}
        inline char chr() const {return this->_str[0];}

        static constexpr TokenType END{'end'};
        static constexpr TokenType DEF{'def'};
        static constexpr TokenType EXTERN{'ext'};
        static constexpr TokenType ID{'id'};
        static constexpr TokenType NUMBER{'num'};
        static constexpr TokenType LEFTP{'('};
        static constexpr TokenType RIGHTP{')'};
        static constexpr TokenType COMMA{','};
        static constexpr TokenType SEMICOL{';'};
        static constexpr TokenType OP{'op'};
        static constexpr TokenType IF{'if'};
        static constexpr TokenType THEN{'then'};
        static constexpr TokenType ELSE{'else'};
        static constexpr TokenType NIL{0};

        friend class TokenType;
        friend std::ostream& operator<<(std::ostream& out, const Token& tok);
    protected:
    private:
        const TokenType *_type;
        std::string _str;
};


#endif // TOKEN_H
