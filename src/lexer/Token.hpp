#pragma once

#include <cstdint>

enum TokenType
{
    AnalyzerDebugCode,
    ParserDebugCode,
    Letter,
    String,
    Number,
    Symbol,
    EndOfFile, // sort of whitespace
    Shebang,   // sort of whitespace
    Unknown,

    LineComment,
    MultilineComment,
    CommentEscape,
    Space,
};

class Token
{
public:
    TokenType kind;
    size_t start;
    size_t stop;
    string_view value;
    vector<Token *> whitespace;
    inline Token(TokenType kind)
    {
        this->kind = kind;
    };
    ~Token() {}
    inline bool IsWhitespace()
    {
        return kind == TokenType::Space ||
               kind == TokenType::LineComment ||
               kind == TokenType::MultilineComment ||
               kind == TokenType::CommentEscape;
    }
};