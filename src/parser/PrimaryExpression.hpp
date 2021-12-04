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
    static std::unique_ptr<Expression> Parse(std::shared_ptr<LuaParser> parser, size_t priority = 0);

    class PostfixExpression : public Expression
    {
    public:
        std::unique_ptr<Expression> left;
    };
    class Index : public PostfixExpression
    {
    public:
        std::unique_ptr<Token> op;
        std::unique_ptr<Expression> right;

        static std::unique_ptr<Index> Parse(std::shared_ptr<LuaParser> parser);
    };

    class SelfCall : public PostfixExpression
    {
    public:
        std::unique_ptr<Token> op;
        std::unique_ptr<Expression> right;

        static std::unique_ptr<SelfCall> Parse(std::shared_ptr<LuaParser> parser);
    };

    class Call : public PostfixExpression
    {
    public:
        std::vector<std::unique_ptr<Expression>> arguments = {};

        std::unique_ptr<Token> tk_arguments_left = nullptr;
        std::unique_ptr<Token> tk_arguments_right = nullptr;
        std::vector<std::unique_ptr<Token>> tk_comma = {};
        std::unique_ptr<Token> tk_type_call = nullptr;

        static std::unique_ptr<Call> Parse(std::shared_ptr<LuaParser> parser);
    };

    class PostfixOperator : public PostfixExpression
    {
    public:
        std::unique_ptr<Token> op;
        static std::unique_ptr<PostfixOperator> Parse(std::shared_ptr<LuaParser> parser);
    };

    class IndexExpression : public PostfixExpression
    {
    public:
        std::unique_ptr<Expression> index = nullptr;

        std::unique_ptr<Token> tk_left_bracket = nullptr;
        std::unique_ptr<Token> tk_right_bracket = nullptr;

        static std::unique_ptr<IndexExpression> Parse(std::shared_ptr<LuaParser> parser);
    };

    class TypeCast : public PostfixExpression
    {
    public:
        std::unique_ptr<Expression> expression;
        std::unique_ptr<Token> tk_operator = nullptr;

        static std::unique_ptr<TypeCast> Parse(std::shared_ptr<LuaParser> parser);
    };
};

class Atomic : public ValueExpression
{
public:
    std::unique_ptr<Token> value;
    static std::unique_ptr<Atomic> Parse(std::shared_ptr<LuaParser> parser);
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
        std::unique_ptr<Expression> val = nullptr;

        std::unique_ptr<Token> tk_equal = nullptr;

        static std::unique_ptr<IdentifierKeyValue> Parse(std::shared_ptr<LuaParser> parser);
    };

    class ExpressionKeyValue : public Child
    {
    public:
        std::unique_ptr<Expression> key = nullptr;
        std::unique_ptr<Expression> val = nullptr;

        std::unique_ptr<Token> tk_equal = nullptr;
        std::unique_ptr<Token> tk_left_bracket = nullptr;
        std::unique_ptr<Token> tk_right_bracket = nullptr;

        static std::unique_ptr<ExpressionKeyValue> Parse(std::shared_ptr<LuaParser> parser);
    };

    class IndexValue : public Child
    {
    public:
        uint64_t key = 0;
        std::unique_ptr<Expression> val = nullptr;

        static std::unique_ptr<IndexValue> Parse(std::shared_ptr<LuaParser> parser);
    };

    std::vector<std::unique_ptr<Child>> children = {};

    std::unique_ptr<Token> tk_left_bracket = nullptr;
    std::unique_ptr<Token> tk_right_bracket = nullptr;
    std::vector<std::unique_ptr<Token>> tk_separators = {};

    static std::unique_ptr<Table> Parse(std::shared_ptr<LuaParser> parser);
};

class PrefixOperator : public ValueExpression
{
public:
    std::unique_ptr<Token> op;
    std::unique_ptr<Expression> right;

    static std::unique_ptr<PrefixOperator> Parse(std::shared_ptr<LuaParser> parser);
};

class BinaryOperator : public ValueExpression
{
public:
    std::unique_ptr<Token> op;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

    static std::unique_ptr<BinaryOperator> Parse(std::shared_ptr<LuaParser> parser);
};
