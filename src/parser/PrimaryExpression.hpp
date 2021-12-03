#pragma once

#include "../lexer/Token.hpp"
#include "./LuaParser.hpp"
#include "./ParserNode.hpp"

class Expression : public ParserNode
{
public:
    std::vector<Token *> tk_left_parenthesis = {};
    std::vector<Token *> tk_right_parenthesis = {};

    Token *tk_type_colon_assignment = nullptr;
    Token *tk_type_as_assignment = nullptr;
};

class ValueExpression : public Expression
{
public:
    static Expression *Parse(LuaParser *parser, size_t priority = 0);

    class PostfixExpression : public Expression
    {
    public:
        Expression *left;
    };
    class Index : public PostfixExpression
    {
    public:
        Token *op;
        Expression *right;

        static Index *Parse(LuaParser *parser);
    };

    class SelfCall : public PostfixExpression
    {
    public:
        Token *op;
        Expression *right;

        static SelfCall *Parse(LuaParser *parser);
    };

    class Call : public PostfixExpression
    {
    public:
        std::vector<Expression *> arguments = {};

        Token *tk_arguments_left = nullptr;
        Token *tk_arguments_right = nullptr;
        std::vector<Token *> tk_comma = {};
        Token *tk_type_call = nullptr;

        static Call *Parse(LuaParser *parser);
    };

    class PostfixOperator : public PostfixExpression
    {
    public:
        Token *op;
        static PostfixOperator *Parse(LuaParser *parser);
    };

    class IndexExpression : public PostfixExpression
    {
    public:
        Expression *index = nullptr;

        Token *tk_left_bracket = nullptr;
        Token *tk_right_bracket = nullptr;

        static IndexExpression *Parse(LuaParser *parser);
    };

    class TypeCast : public PostfixExpression
    {
    public:
        Expression *expression;
        Token *tk_operator = nullptr;

        static TypeCast *Parse(LuaParser *parser);
    };
};

class Atomic : public ValueExpression
{
public:
    Token *value;
    static Atomic *Parse(LuaParser *parser);
};

class Table : public ValueExpression
{
public:
    class Child : public Expression
    {
    };

    class KeyValue : public Child
    {
    public:
        Token *key = nullptr;
        Expression *val = nullptr;

        Token *tk_equal = nullptr;

        static KeyValue *Parse(LuaParser *parser);
    };

    class KeyExpressionValue : public Child
    {
    public:
        Expression *key = nullptr;
        Expression *val = nullptr;

        Token *tk_equal = nullptr;
        Token *tk_left_bracket = nullptr;
        Token *tk_right_bracket = nullptr;

        static KeyExpressionValue *Parse(LuaParser *parser);
    };

    class IndexValue : public Child
    {
    public:
        uint64_t key = 0;
        Expression *val = nullptr;

        static IndexValue *Parse(LuaParser *parser);
    };

    std::vector<Child *> children = {};

    Token *tk_left_bracket = nullptr;
    Token *tk_right_bracket = nullptr;
    std::vector<Token *> tk_separators = {};

    static Table *Parse(LuaParser *parser);
};

class PrefixOperator : public ValueExpression
{
public:
    Token *op;
    Expression *right;

    static PrefixOperator *Parse(LuaParser *parser);
};

class BinaryOperator : public ValueExpression
{
public:
    Token *op;
    Expression *left;
    Expression *right;

    static BinaryOperator *Parse(LuaParser *parser);
};
