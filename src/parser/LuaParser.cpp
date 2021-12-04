#include "./LuaParser.hpp"

bool LuaParser::IsTokenValue(PeekedToken token)
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

LuaParser::TokenType LuaParser::GetTokenType(PeekedToken token)
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

LuaParser::LuaParser(std::vector<std::unique_ptr<Token>> tokens)
{
    this->tokens = std::move(tokens);
}

bool LuaParser::IsValue(const std::string &val, const uint8_t offset)
{
    return PeekToken(offset)->value == val;
}

bool LuaParser::IsType(const Token::Kind val, const uint8_t offset)
{
    return PeekToken(offset)->kind == val;
}

std::unique_ptr<Token> LuaParser::ExpectValue(const std::string &val)
{
    if (!IsValue(val))
        throw Exception("Expected value: " + val, PeekToken(), PeekToken());

    return ReadToken();
}

std::unique_ptr<Token> LuaParser::ExpectType(const Token::Kind val)
{
    if (!IsType(val))
        throw Exception("Expected something", PeekToken(), PeekToken());

    return ReadToken();
}

bool LuaParser::IsCallExpression(const uint8_t offset)
{
    return IsValue("(", offset) || IsValue("<|", offset) || IsValue("{", offset) || IsType(Token::Kind::String, offset) || (IsValue("!", offset) && IsValue("(", offset + 1));
}
