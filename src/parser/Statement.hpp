#pragma once

#include "../lexer/Token.hpp"
#include "./ParserNode.hpp"
#include "./PrimaryExpression.hpp"

class StatementNode : public ParserNode
{
};

class EndOfFile : public StatementNode
{
    std::unique_ptr<Token> tk_main;
};

class AnalyzerDebugCode : public StatementNode
{
    std::unique_ptr<Token> tk_main;
    Atomic *lua_code;
};

// OPERATORS
