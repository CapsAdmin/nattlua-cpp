#pragma once

#include <cstdint>
#include "../code/Code.hpp"
#include "../syntax/RuntimeSyntax.hpp"
#include "../syntax/TypesystemSyntax.hpp"
#include "./Token.hpp"

using namespace std;

// lexer exception

class LexerException : public exception
{
private:
    string message;
    size_t start;
    size_t stop;

public:
    explicit LexerException(const string &message, size_t start, size_t stop)
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

class BaseLexer
{
public:
    Code *code;
    size_t position = 0;
    RuntimeSyntax *runtime_syntax = new RuntimeSyntax();
    TypesystemSyntax *typesystem_syntax = new TypesystemSyntax();

    virtual Token *ReadNonWhitespaceToken() = 0;
    virtual Token *ReadWhitespaceToken() = 0;

    Token *ReadToken();
    Token *ReadShebang();
    Token *ReadUnknown();
    Token *ReadEndOfFile();
    Token *ReadCommentEscape();
    Token *ReadRemainingCommentEscape();
    pair<vector<Token *>, vector<LexerException>> GetTokens();

    string_view GetRelativeStringSlice(size_t start, size_t stop);
    uint8_t GetByte(size_t offset = 0);
    bool IsString(const string value, const size_t relative_offset = 0);
    void ResetState();
    optional<size_t> FindNearest(string pattern);
    uint8_t ReadByte();
    bool TheEnd();
    bool ReadFirstFromStringArray(vector<string> array);

private:
    bool comment_escape = false;
    Token *ReadSingleToken();
};