#pragma once

#include "../lexer/Token.hpp"
#include "../syntax/RuntimeSyntax.hpp"
#include "../syntax/TypesystemSyntax.hpp"
#include "./ParserNode.hpp"

class LuaParser
{
public:
    class Exception : public std::exception
    {
    private:
        std::string message;
        Token *start;
        Token *stop;

    public:
        explicit Exception(const std::string &message, Token *start, Token *stop)
        {
            this->message = message;
            this->start = start;
            this->stop = stop;
        }
        const char *what() const noexcept override
        {
            return message.c_str();
        }
    };

    std::vector<Token *> tokens;
    size_t index = 0;
    RuntimeSyntax *runtime_syntax = new RuntimeSyntax();
    TypesystemSyntax *typesystem_syntax = new TypesystemSyntax();

    LuaParser(std::vector<Token *> tokens);

    enum TokenType
    {
        Keyword,
        PrefixOperator,
        PostfixOperator,
        BinaryOperator,
        None,
    };
    TokenType GetTokenType(Token *token);

    bool IsTokenValue(Token *token);
    bool IsValue(const std::string &val, const uint8_t offset = 0);
    bool IsType(const Token::Kind val, const uint8_t offset = 0);
    Token *ExpectValue(const std::string &val);
    Token *ExpectType(const Token::Kind val);

    template <class T>
    inline std::vector<T *> ReadMultipleValues(
        const size_t max,
        std::function<T *()> reader,
        std::vector<Token *> &comma_tokens)
    {
        std::vector<T *> out;

        for (size_t i = 0; i < (max != 0 ? max : GetLength()); i++)
        {
            T *node = reader();
            if (!node)
                break;
            out.push_back(node);

            if (!IsValue(","))
                break;

            comma_tokens.push_back(ExpectValue(","));
        }

        return out;
    };

    bool IsCallExpression(const uint8_t offset = 0);
    Token *ReadToken() { return tokens[index++]; };
    Token *GetToken(size_t offset = 0) { return tokens[index + offset]; };
    void StartNode(ParserNode *node){};
    void EndNode(ParserNode *node){};
    inline size_t GetLength() { return tokens.size(); }
};
