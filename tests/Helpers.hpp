#pragma once

#include "../src/lexer/LuaLexer.hpp"
#include "../src/parser/LuaParser.hpp"

inline std::vector<std::unique_ptr<Token>> Tokenize(std::string_view code)
{
    auto code_obj = std::make_shared<Code>(code, "test");
    auto lexer = std::make_shared<LuaLexer>(code_obj);
    auto [tokens, errors] = lexer->GetTokens();

    for (auto &error : errors)
    {
        printf("%s", error.what());
    }

    return std::move(tokens);
}

inline std::unique_ptr<Token> OneToken(std::vector<std::unique_ptr<Token>> tokens)
{
    if (tokens.size() != 2)
    {
        throw std::runtime_error("expected one token");
    }

    assert(tokens[1]->kind == Token::Kind::EndOfFile);

    return std::move(tokens[0]);
}

inline std::string TokensToString(std::vector<std::unique_ptr<Token>> tokens)
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

inline void ExpectError(std::string_view lua_code, std::string error_message)
{
    auto code = std::make_shared<Code>(lua_code, "test");
    auto lexer = std::make_unique<LuaLexer>(code);
    auto [tokens, errors] = lexer->GetTokens();

    EXPECT_TRUE(errors.size() != 0);
    EXPECT_EQ(errors[0].what(), error_message);
}

inline void Check(std::string_view lua_code)
{
    auto tokens = Tokenize(lua_code);
    auto actual_tokens = TokensToString(std::move(tokens));

    EXPECT_EQ(actual_tokens, lua_code);
}