#pragma once
#include "./BaseLexer.hpp"

class LuaLexer : public BaseLexer
{
public:
    explicit LuaLexer(Code *code)
    {
        this->code = code;
    }
    ~LuaLexer() {}

    Token *ReadNonWhitespaceToken() override;
    Token *ReadWhitespaceToken() override;

    Token *ReadMultilineComment();
    Token *ReadLineCComment();
    Token *ReadMultilineCComment();
    Token *ReadLineComment();
    Token *ReadParserDebugCode();
    Token *ReadHexNumber();
    Token *ReadBinaryNumber();
    Token *ReadDecimalNumber();
    Token *ReadMultilineString();
    Token *ReadSingleQuotedString();
    Token *ReadDoubleQuotedString();
    Token *ReadLetter();
    Token *ReadSymbol();
    Token *ReadSpace();
    Token *ReadAnalyzerDebugCode();
};