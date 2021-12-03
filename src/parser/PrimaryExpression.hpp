#pragma once

#include "../lexer/Token.hpp"
#include "./LuaParser.hpp"
#include "./ParserNode.hpp"

class ExpressionNode : public ParserNode
{
public:
    std::vector<Token *> tk_left_parenthesis = {};
    std::vector<Token *> tk_right_parenthesis = {};

    Token *tk_type_colon_assignment = nullptr;
    Token *tk_type_as_assignment = nullptr;

    static ExpressionNode *Parse(LuaParser *parser, size_t priority = 0);

    class PostfixExpression : public ParserNode
    {
    public:
        // this can be a PostfixExpression or an ExpressionNode
        ParserNode *left;
    };
    class Index : public PostfixExpression
    {
    public:
        Token *op;
        ExpressionNode *right;

        static Index *Parse(LuaParser *parser);
    };

    class SelfCall : public PostfixExpression
    {
    public:
        Token *op;
        ExpressionNode *right;

        static SelfCall *Parse(LuaParser *parser);
    };

    class Call : public PostfixExpression
    {
    public:
        std::vector<ExpressionNode *> arguments = {};

        Token *tk_arguments_left = nullptr;
        Token *tk_arguments_right = nullptr;
        std::vector<Token *> tk_comma = {};
        Token *tk_type_call = nullptr;

        static Call *Parse(LuaParser *parser);
    };

    class Postfix : public PostfixExpression
    {
    public:
        Token *op;
        static Postfix *Parse(LuaParser *parser);
    };

    class PostfixIndex : public PostfixExpression
    {
    public:
        ExpressionNode *index = nullptr;

        Token *tk_left_bracket = nullptr;
        Token *tk_right_bracket = nullptr;

        static PostfixIndex *Parse(LuaParser *parser);
    };

    class Cast : public PostfixExpression
    {
    public:
        ExpressionNode *expression;
        Token *tk_operator = nullptr;

        static Cast *Parse(LuaParser *parser);
    };
};

class Value : public ExpressionNode
{
public:
    Token *value;
    ParserNode *standalone_letter = nullptr;

    static Value *Parse(LuaParser *parser);
};

// table

class Table : public ExpressionNode
{
public:
    class Child : public ParserNode
    {
    };

    class KeyValue : public Child
    {
    public:
        Token *key = nullptr;
        ExpressionNode *val = nullptr;

        Token *tk_equal = nullptr;

        static KeyValue *Parse(LuaParser *parser);
    };

    class KeyExpressionValue : public Child
    {
    public:
        ExpressionNode *key = nullptr;
        ExpressionNode *val = nullptr;

        Token *tk_equal = nullptr;
        Token *tk_left_bracket = nullptr;
        Token *tk_right_bracket = nullptr;

        static KeyExpressionValue *Parse(LuaParser *parser);
    };

    class IndexValue : public Child
    {
    public:
        uint64_t key = 0;
        ExpressionNode *val = nullptr;

        static IndexValue *Parse(LuaParser *parser);
    };

    std::vector<Child *> children = {};

    Token *tk_left_bracket = nullptr;
    Token *tk_right_bracket = nullptr;
    std::vector<Token *> tk_separators = {};

    static Table *Parse(LuaParser *parser);
};

class PrefixOperator : public ExpressionNode
{
public:
    Token *op;
    ExpressionNode *right;

    static PrefixOperator *Parse(LuaParser *parser);
};

class PostfixOperator : public ExpressionNode
{
public:
    Token *op;
    ExpressionNode *left;

    static PostfixOperator *Parse(LuaParser *parser);
};

class BinaryOperator : public ExpressionNode
{
public:
    Token *op;
    ParserNode *left;
    ExpressionNode *right;

    static BinaryOperator *Parse(LuaParser *parser);
};
