#pragma once

#include "../lexer/Token.hpp"
#include "./ParserNode.hpp"
#include "./PrimaryExpression.hpp"

class StatementNode : public ParserNode
{
};

class EndOfFile : public StatementNode
{
    Token *tk_main;
};

class AnalyzerDebugCode : public StatementNode
{
    Token *tk_main;
    Value *lua_code;
};

// OPERATORS
