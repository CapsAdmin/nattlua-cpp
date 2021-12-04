#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

class Token
{
public:
    enum Kind : uint8_t
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

    Kind kind;
    size_t start;
    size_t stop;
    std::string_view value;
    std::vector<std::unique_ptr<Token>> whitespace;
    Token &operator=(Token &&) = default;
    inline Token(Token::Kind kind)
    {
        this->kind = kind;
    };
    ~Token()
    {
    }
    inline bool IsWhitespace()
    {
        return kind == Token::Kind::Space ||
               kind == Token::Kind::LineComment ||
               kind == Token::Kind::MultilineComment ||
               kind == Token::Kind::CommentEscape;
    }
};