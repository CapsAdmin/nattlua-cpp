#pragma once
#include "./BaseLexer.hpp"

class LuaLexer : public BaseLexer
{
public:
    LuaLexer(Code *code)
    {
        this->code = code;
        this->runtime_syntax = new RuntimeSyntax();
        this->typesystem_syntax = new TypesystemSyntax();
    };
    ~LuaLexer() {}

    Token *ReadNonWhitespaceToken();
    Token *ReadWhitespaceToken();

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