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

    enum TokenType
    {
        Keyword,
        PrefixOperator,
        PostfixOperator,
        BinaryOperator,
        None,
    };

    bool IsTokenValue(Token *token)
    {
        if (token->kind == Token::Kind::Number || token->kind == Token::Kind::String)
            return true;
        if (runtime_syntax->IsKeywordValue(token->value))
            return true;
        if (runtime_syntax->IsKeyword(token->value))
            return false;
        if (token->kind == Token::Kind::Letter)
            return true;
        return false;
    }

    TokenType GetTokenType(Token *token)
    {
        if (token->kind == Token::Kind::Letter && runtime_syntax->IsKeyword(token->value))
        {
            return TokenType::Keyword;
        }
        else if (token->kind == Token::Kind::Symbol)
        {
            if (runtime_syntax->IsPrefixOperator(token->value))
                return TokenType::PrefixOperator;
            else if (runtime_syntax->IsPostfixOperator(token->value))
                return TokenType::PostfixOperator;
            else if (runtime_syntax->IsBinaryOperator(token->value))
                return TokenType::BinaryOperator;
        }

        return TokenType::None;
    }
    LuaParser(std::vector<Token *> tokens)
    {
        this->tokens = tokens;
    }

    bool IsValue(const std::string &val, const uint8_t offset = 0)
    {
        return GetToken(offset)->value == val;
    }

    bool IsType(const Token::Kind val, const uint8_t offset = 0)
    {
        return GetToken(offset)->kind == val;
    }

    Token *ExpectValue(const std::string &val)
    {
        if (!IsValue(val))
            throw Exception("Expected value: " + val, GetToken(), GetToken());

        return ReadToken();
    }

    Token *ExpectType(const Token::Kind val)
    {
        if (!IsType(val))
            throw Exception("Expected something", GetToken(), GetToken());

        return ReadToken();
    }

    Token *ReadToken() { return tokens[index++]; };
    Token *GetToken(size_t offset = 0) { return tokens[index + offset]; };
    void StartNode(ParserNode *node){};
    void EndNode(ParserNode *node){};
    inline size_t GetLength() { return tokens.size(); }
};
