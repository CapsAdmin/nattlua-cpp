#pragma once

#include "../src/lexer/LuaLexer.hpp"
#include "../src/parser/LuaParser.hpp"

inline std::vector<Token *> Tokenize(std::string_view code)
{
    auto code_obj = new Code(code, "test");
    LuaLexer lexer(code_obj);
    auto [tokens, errors] = lexer.GetTokens();

    for (auto &error : errors)
    {
        printf("%s", error.what());
    }

    return tokens;
}

inline Token *OneToken(std::vector<Token *> tokens)
{
    if (tokens.size() != 2)
    {
        throw std::runtime_error("expected one token");
    }

    assert(tokens[1]->kind == Token::Kind::EndOfFile);

    return tokens[0];
}

inline std::string TokensToString(std::vector<Token *> tokens)
{
    std::stringstream ss;

    for (auto &token : tokens)
    {
        for (auto &whitespace_token : token->whitespace)
        {
            ss << whitespace_token->value;
        }

        ss << token->value;
    }

    return ss.str();
}

inline void ExpectError(std::string_view code, std::string error_message)
{
    auto code_obj = new Code(code, "test");
    LuaLexer lexer(code_obj);
    auto [tokens, errors] = lexer.GetTokens();

    EXPECT_TRUE(errors.size() != 0);
    EXPECT_EQ(errors[0].what(), error_message);
}

inline void Check(std::string_view code)
{
    auto tokens = Tokenize(code);
    auto actual_tokens = TokensToString(tokens);

    EXPECT_EQ(actual_tokens, code);
}