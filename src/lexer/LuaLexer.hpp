#pragma once
#include "./BaseLexer.hpp"

class LuaLexer : public BaseLexer
{
public:
    explicit LuaLexer(std::shared_ptr<Code> code)
    {
        this->code = code;
    }
    ~LuaLexer()
    {
        printf("LuaLexer::~LuaLexer()\n");
    }

    std::unique_ptr<Token> ReadNonWhitespaceToken() override;
    std::unique_ptr<Token> ReadWhitespaceToken() override;
    std::unique_ptr<Token> ReadMultilineComment();
    std::unique_ptr<Token> ReadLineCComment();
    std::unique_ptr<Token> ReadMultilineCComment();
    std::unique_ptr<Token> ReadLineComment();
    std::unique_ptr<Token> ReadParserDebugCode();
    std::unique_ptr<Token> ReadHexNumber();
    std::unique_ptr<Token> ReadBinaryNumber();
    std::unique_ptr<Token> ReadDecimalNumber();
    std::unique_ptr<Token> ReadMultilineString();
    std::unique_ptr<Token> ReadSingleQuotedString();
    std::unique_ptr<Token> ReadDoubleQuotedString();
    std::unique_ptr<Token> ReadLetter();
    std::unique_ptr<Token> ReadSymbol();
    std::unique_ptr<Token> ReadSpace();
    std::unique_ptr<Token> ReadAnalyzerDebugCode();
};