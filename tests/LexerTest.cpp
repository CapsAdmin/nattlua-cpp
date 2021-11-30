#include "../src/lexer/LuaLexer.hpp"
#include <assert.h>
#include <gtest/gtest.h>

std::vector<Token *> Tokenize(std::string_view code)
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

Token *OneToken(std::vector<Token *> tokens)
{
    if (tokens.size() != 2)
    {
        throw std::runtime_error("expected one token");
    }

    assert(tokens[1]->kind == TokenType::EndOfFile);

    return tokens[0];
}

std::string TokensToString(std::vector<Token *> tokens)
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

void ExpectError(std::string_view code, std::string error_message)
{
    auto code_obj = new Code(code, "test");
    LuaLexer lexer(code_obj);
    auto [tokens, errors] = lexer.GetTokens();

    EXPECT_TRUE(errors.size() != 0);
    EXPECT_EQ(errors[0].what(), error_message);
}

void Check(std::string_view code)
{
    auto tokens = Tokenize(code);
    auto actual_tokens = TokensToString(tokens);

    EXPECT_EQ(actual_tokens, code);
}

TEST(Lexer, TokensToString)
{
    Check("local foo =   5 + 2..2");
}
TEST(Lexer, Smoke)
{
    EXPECT_EQ(Tokenize("")[0]->kind, TokenType::EndOfFile);
    EXPECT_EQ(OneToken(Tokenize("a"))->kind, TokenType::Letter);
    EXPECT_EQ(OneToken(Tokenize("1"))->kind, TokenType::Number);
    EXPECT_EQ(OneToken(Tokenize("("))->kind, TokenType::Symbol);
}

TEST(Lexer, Shebang)
{
    EXPECT_EQ(Tokenize("#!/usr/bin/env lua\nprint(1)")[0]->kind, TokenType::Shebang);
}

TEST(Lexer, SingleQuoteString)
{
    EXPECT_EQ(OneToken(Tokenize("'foo'"))->kind, TokenType::String);
}

TEST(Lexer, ZEscapedString)
{
    EXPECT_EQ(OneToken(Tokenize("\"a\\z\na\""))->kind, TokenType::String);
    EXPECT_EQ(OneToken(Tokenize("\"a\\z\n\n   \na\""))->kind, TokenType::String);
    ExpectError("\"a\\z\n\n--foo  \na\"", "expected ending \" quote, got newline");
}

TEST(Lexer, NumberRange)
{
    EXPECT_EQ(Tokenize("1..20").size(), 4);
}

TEST(Lexer, NumberDelimiter)
{
    EXPECT_EQ(Tokenize("1_000_000").size(), 2);
    EXPECT_EQ(Tokenize("0xdead_beef").size(), 2);
    EXPECT_EQ(Tokenize("0b0101_0101").size(), 2);
}

TEST(Lexer, NumberAnnotations)
{
    EXPECT_EQ(Tokenize("50ull").size(), 2);
    EXPECT_EQ(Tokenize("50uLL").size(), 2);
    EXPECT_EQ(Tokenize("50ULL").size(), 2);
    EXPECT_EQ(Tokenize("50LL").size(), 2);
    EXPECT_EQ(Tokenize("50lL").size(), 2);
    EXPECT_EQ(Tokenize("1.5e+20").size(), 2);
    EXPECT_EQ(Tokenize(".0").size(), 2);
}

TEST(Lexer, MalformedNumber)
{
    ExpectError("12LOL", "malformed decimal number, got L");
    ExpectError("0xbLOL", "malformed hex number, got L");
    ExpectError("0b101LOL01", "malformed binary number, got L");
    ExpectError("1.5eD", "expected + or - after exponent, got D");
    ExpectError("1.5e+D", "malformed 'exponent' expected number, got D");
}

TEST(Lexer, MultilineCommentError)
{
    ExpectError("/*", "expected multiline C comment to end, reached end of code");
    ExpectError("--[[", "expected multiline comment to end, reached end of code");
}

TEST(Lexer, StringError)
{
    ExpectError("\"woo\nfoo", "expected ending \" quote, got newline");
    ExpectError("'aaa", "expected ending ' quote, reached end of code");
}

TEST(Lexer, MultilineString)
{
    EXPECT_EQ(Tokenize("a = [[a]]").size(), 4);
    EXPECT_EQ(Tokenize("a = [=[a]=]").size(), 4);
    EXPECT_EQ(Tokenize("a = [==[a]==]").size(), 4);

    ExpectError("a = [=a", "malformed multiline string: expected =, got a");
    ExpectError("a = [[a", "expected multiline string to end, reached end of code");
}

TEST(Lexer, MultilineComment)
{
    EXPECT_EQ(Tokenize("--[[a]]")[0]->kind, TokenType::EndOfFile);
    EXPECT_EQ(Tokenize("--[=[a]=]")[0]->kind, TokenType::EndOfFile);
    EXPECT_EQ(Tokenize("--[==[a]==]")[0]->kind, TokenType::EndOfFile);
    EXPECT_EQ(Tokenize("/*a*/")[0]->kind, TokenType::EndOfFile);
}

TEST(Lexer, LineComment)
{
    EXPECT_EQ(Tokenize("-- a")[0]->kind, TokenType::EndOfFile);
    EXPECT_EQ(Tokenize("// a")[0]->kind, TokenType::EndOfFile);
    EXPECT_EQ(Tokenize("--[= a")[0]->kind, TokenType::EndOfFile);
}

TEST(Lexer, CommentEscape)
{
    EXPECT_EQ(Tokenize("--[[# 1337 ]]")[0]->kind, TokenType::Number);
}

TEST(Lexer, TypesystemSymbols)
{
    EXPECT_EQ(Tokenize("$'foo'").size(), 3);
}

TEST(Lexer, UnknownSymbols)
{
    EXPECT_EQ(Tokenize("```").size(), 4);
}

TEST(Lexer, DebugCode)
{
    EXPECT_EQ(Tokenize("§foo = true")[0]->kind, TokenType::AnalyzerDebugCode);
    EXPECT_EQ(Tokenize("£foo = true")[0]->kind, TokenType::ParserDebugCode);
}