#pragma once

#include "../lexer/Token.hpp"
#include "./LuaParser.hpp"
#include "./ParserNode.hpp"

class Expression : public ParserNode
{
public:
    std::vector<std::unique_ptr<Token>> tk_left_parenthesis = {};
    std::vector<std::unique_ptr<Token>> tk_right_parenthesis = {};

    std::unique_ptr<Token> tk_type_colon_assignment = nullptr;
    std::unique_ptr<Token> tk_type_as_assignment = nullptr;
};

class ValueExpression : public Expression
{
public:
    static Expression *Parse(std::shared_ptr<LuaParser> parser, size_t priority = 0);

    class PostfixExpression : public Expression
    {
    public:
        Expression *left;
    };
    class Index : public PostfixExpression
    {
    public:
        std::unique_ptr<Token> op;
        Expression *right;

        static Index *Parse(std::shared_ptr<LuaParser> parser);
    };

    class SelfCall : public PostfixExpression
    {
    public:
        std::unique_ptr<Token> op;
        Expression *right;

        static SelfCall *Parse(std::shared_ptr<LuaParser> parser);
    };

    class Call : public PostfixExpression
    {
    public:
        std::vector<Expression *> arguments = {};

        std::unique_ptr<Token> tk_arguments_left = nullptr;
        std::unique_ptr<Token> tk_arguments_right = nullptr;
        std::vector<std::unique_ptr<Token>> tk_comma = {};
        std::unique_ptr<Token> tk_type_call = nullptr;

        static Call *Parse(std::shared_ptr<LuaParser> parser);
    };

    class PostfixOperator : public PostfixExpression
    {
    public:
        std::unique_ptr<Token> op;
        static PostfixOperator *Parse(std::shared_ptr<LuaParser> parser);
    };

    class IndexExpression : public PostfixExpression
    {
    public:
        Expression *index = nullptr;

        std::unique_ptr<Token> tk_left_bracket = nullptr;
        std::unique_ptr<Token> tk_right_bracket = nullptr;

        static IndexExpression *Parse(std::shared_ptr<LuaParser> parser);
    };

    class TypeCast : public PostfixExpression
    {
    public:
        Expression *expression;
        std::unique_ptr<Token> tk_operator = nullptr;

        static TypeCast *Parse(std::shared_ptr<LuaParser> parser);
    };
};

class Atomic : public ValueExpression
{
public:
    std::unique_ptr<Token> value;
    static Atomic *Parse(std::shared_ptr<LuaParser> parser);
};

class Table : public ValueExpression
{
public:
    class Child : public Expression
    {
    };

    class IdentifierKeyValue : public Child
    {
    public:
        std::unique_ptr<Token> key = nullptr;
        Expression *val = nullptr;

        std::unique_ptr<Token> tk_equal = nullptr;

        static IdentifierKeyValue *Parse(std::shared_ptr<LuaParser> parser);
    };

    class ExpressionKeyValue : public Child
    {
    public:
        Expression *key = nullptr;
        Expression *val = nullptr;

        std::unique_ptr<Token> tk_equal = nullptr;
        std::unique_ptr<Token> tk_left_bracket = nullptr;
        std::unique_ptr<Token> tk_right_bracket = nullptr;

        static ExpressionKeyValue *Parse(std::shared_ptr<LuaParser> parser);
    };

    class IndexValue : public Child
    {
    public:
        uint64_t key = 0;
        Expression *val = nullptr;

        static IndexValue *Parse(std::shared_ptr<LuaParser> parser);
    };

    std::vector<Child *> children = {};

    std::unique_ptr<Token> tk_left_bracket = nullptr;
    std::unique_ptr<Token> tk_right_bracket = nullptr;
    std::vector<std::unique_ptr<Token>> tk_separators = {};

    static Table *Parse(std::shared_ptr<LuaParser> parser);
};

class PrefixOperator : public ValueExpression
{
public:
    std::unique_ptr<Token> op;
    Expression *right;

    static PrefixOperator *Parse(std::shared_ptr<LuaParser> parser);
};

class BinaryOperator : public ValueExpression
{
public:
    std::unique_ptr<Token> op;
    Expression *left;
    Expression *right;

    static BinaryOperator *Parse(std::shared_ptr<LuaParser> parser);
};
