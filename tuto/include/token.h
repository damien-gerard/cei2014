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
        constexpr TokenType(const int type):_type(type){};
        constexpr bool operator==(const Token&);
        constexpr bool operator!=(const Token&);
        constexpr bool operator==(TokenType);
        constexpr bool operator!=(TokenType);
        Token operator()() const;
        Token operator()(const std::string&) const;
        constexpr operator int() {return this->_type;}
        constexpr char sym();
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

        inline std::string str() {return this->_str;}
        inline double dbl() {return strtod(this->_str.c_str(), 0);}
        inline char chr() {return this->_str[0];}

        static constexpr TokenType END{'z'};
        static constexpr TokenType DEF{'d'};
        static constexpr TokenType EXTERN{'e'};
        static constexpr TokenType ID{'i'};
        static constexpr TokenType NUMBER{'n'};
        static constexpr TokenType LEFTP{'('};
        static constexpr TokenType RIGHTP{')'};
        static constexpr TokenType COMMA{','};
        static constexpr TokenType SEMICOL{';'};
        static constexpr TokenType OP{'o'};
        static constexpr TokenType NIL{0};

        friend class TokenType;
    protected:
    private:
        const TokenType *_type;
        std::string _str;
};

#endif // TOKEN_H
