#pragma once

#include "../lexer/Token.hpp"
#include "../syntax/RuntimeSyntax.hpp"
#include "../syntax/TypesystemSyntax.hpp"
#include "./ParserNode.hpp"

using PeekedToken = Token *;

class LuaParser
{
public:
    class Exception : public std::exception
    {
    private:
        std::string message;
        PeekedToken start;
        PeekedToken stop;

    public:
        explicit Exception(const std::string &message, PeekedToken start_token, PeekedToken stop_token)
        {
            this->message = message;

            // TODO: copy?
            this->start = start_token;
            this->stop = stop_token;
        }
        const char *what() const noexcept override
        {
            return message.c_str();
        }
    };

    std::vector<std::unique_ptr<Token>> tokens;
    size_t index = 0;
    RuntimeSyntax *runtime_syntax = new RuntimeSyntax();
    TypesystemSyntax *typesystem_syntax = new TypesystemSyntax();

    LuaParser(std::vector<std::unique_ptr<Token>> tokens);
    ~LuaParser()
    {
        delete runtime_syntax;
        delete typesystem_syntax;
    };

    enum TokenType
    {
        Keyword,
        PrefixOperator,
        PostfixOperator,
        BinaryOperator,
        None,
    };
    TokenType GetTokenType(PeekedToken token);
    bool IsTokenValue(PeekedToken token);

    bool IsValue(const std::string &val, const uint8_t offset = 0);
    bool IsType(const Token::Kind val, const uint8_t offset = 0);
    std::unique_ptr<Token> ExpectValue(const std::string &val);
    std::unique_ptr<Token> ExpectType(const Token::Kind val);

    template <class T>
    inline std::vector<T *> ReadMultipleValues(
        const size_t max,
        std::function<T *()> reader,
        std::vector<std::unique_ptr<Token>> &comma_tokens)
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
    std::unique_ptr<Token> ReadToken() { return std::move(tokens[index++]); };
    PeekedToken PeekToken(size_t offset = 0) { return tokens[index + offset].get(); };
    void StartNode(ParserNode *node){};
    void EndNode(ParserNode *node){};
    inline size_t GetLength() { return tokens.size(); }
};
