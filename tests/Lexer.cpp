#include <assert.h>
#include <gtest/gtest.h>
#include "./Helpers.hpp"

TEST(Lexer, TokensToString)
{
    Check("local foo =   5 + 2..2");
}
TEST(Lexer, Smoke)
{
    EXPECT_EQ(Tokenize("")[0]->kind, Token::Kind::EndOfFile);
    EXPECT_EQ(OneToken(Tokenize("a"))->kind, Token::Kind::Letter);
    EXPECT_EQ(OneToken(Tokenize("1"))->kind, Token::Kind::Number);
    EXPECT_EQ(OneToken(Tokenize("("))->kind, Token::Kind::Symbol);
}

TEST(Lexer, Symbols)
{
    std::vector<std::string> symbols = {
        "+", "-", "*", "/", "%", "^", /*"#", this would become shebang*/
        "==", "~=", "<=", ">=", "<", ">", "=",
        "(", ")", "{", "}", "[", "]",
        ";", ":", ",", ".", "..", "..."};

    for (auto symbol : symbols)
    {
        auto kind = OneToken(Tokenize(symbol))->kind;
        if (kind != Token::Kind::Symbol)
        {
            EXPECT_EQ(kind, Token::Kind::Symbol);
            std::cout << "Symbol: " << symbol << std::endl;
        }
        EXPECT_EQ(OneToken(Tokenize(symbol))->value, symbol);
    }
}

TEST(Lexer, Shebang)
{
    EXPECT_EQ(Tokenize("#!/usr/bin/env lua\nprint(1)")[0]->kind, Token::Kind::Shebang);
}

TEST(Lexer, SingleQuoteString)
{
    EXPECT_EQ(OneToken(Tokenize("'foo'"))->kind, Token::Kind::String);
}

TEST(Lexer, ZEscapedString)
{
    EXPECT_EQ(OneToken(Tokenize("\"a\\z\na\""))->kind, Token::Kind::String);
    EXPECT_EQ(OneToken(Tokenize("\"a\\z\n\n   \na\""))->kind, Token::Kind::String);
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
    EXPECT_EQ(Tokenize("--[[a]]")[0]->kind, Token::Kind::EndOfFile);
    EXPECT_EQ(Tokenize("--[=[a]=]")[0]->kind, Token::Kind::EndOfFile);
    EXPECT_EQ(Tokenize("--[==[a]==]")[0]->kind, Token::Kind::EndOfFile);
    EXPECT_EQ(Tokenize("/*a*/")[0]->kind, Token::Kind::EndOfFile);
}

TEST(Lexer, LineComment)
{
    EXPECT_EQ(Tokenize("-- a")[0]->kind, Token::Kind::EndOfFile);
    EXPECT_EQ(Tokenize("// a")[0]->kind, Token::Kind::EndOfFile);
    EXPECT_EQ(Tokenize("--[= a")[0]->kind, Token::Kind::EndOfFile);
}

TEST(Lexer, CommentEscape)
{
    EXPECT_EQ(Tokenize("--[[# 1337 ]]")[0]->kind, Token::Kind::Number);
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
    EXPECT_EQ(Tokenize("??foo = true")[0]->kind, Token::Kind::AnalyzerDebugCode);
    EXPECT_EQ(Tokenize("??foo = true")[0]->kind, Token::Kind::ParserDebugCode);
}
