#pragma once

#include "../lexer/Token.hpp"
#include "./LuaParser.hpp"
#include "./ParserNode.hpp"

class SubExpressionNode : public ParserNode
{
};

class PrimaryExpressionNode : public ParserNode
{
public:
    std::vector<Token *> tk_left_parenthesis = {};
    std::vector<Token *> tk_right_parenthesis = {};

    Token *tk_type_colon_assignment = nullptr;
    Token *tk_type_as_assignment = nullptr;

    static PrimaryExpressionNode *Parse(LuaParser *parser);
};

class Value : public PrimaryExpressionNode
{
public:
    Token *value;

    static Value *Parse(LuaParser *parser)
    {
        if (!parser->IsTokenValue(parser->GetToken()))
            return nullptr;

        auto node = new Value();
        parser->StartNode(node);
        node->value = parser->ReadToken();
        parser->EndNode(node);
        return node;
    };
};

// table

class Table : public PrimaryExpressionNode
{
public:
    class Child : public SubExpressionNode
    {
    };

    class KeyValue : public Child
    {
    public:
        Token *key = nullptr;
        PrimaryExpressionNode *val = nullptr;

        Token *tk_equal = nullptr;

        static KeyValue *Parse(LuaParser *parser)
        {
            if (!parser->IsType(Token::Kind::Letter) || !parser->IsValue("=", 1))
                return nullptr;

            auto node = new KeyValue();
            parser->StartNode(node);
            node->key = parser->ExpectType(Token::Kind::Letter);
            node->tk_equal = parser->ExpectValue("=");
            node->val = PrimaryExpressionNode::Parse(parser);
            parser->EndNode(node);
            return node;
        }
    };

    class KeyExpressionValue : public Child
    {
    public:
        PrimaryExpressionNode *key = nullptr;
        PrimaryExpressionNode *val = nullptr;

        Token *tk_equal = nullptr;
        Token *tk_left_bracket = nullptr;
        Token *tk_right_bracket = nullptr;

        static KeyExpressionValue *Parse(LuaParser *parser)
        {
            if (!parser->IsValue("["))
                return nullptr;

            auto node = new KeyExpressionValue();
            parser->StartNode(node);
            node->tk_left_bracket = parser->ExpectValue("[");
            node->key = PrimaryExpressionNode::Parse(parser);
            node->tk_right_bracket = parser->ExpectValue("]");
            node->tk_equal = parser->ExpectValue("=");
            node->val = PrimaryExpressionNode::Parse(parser);
            parser->EndNode(node);

            return node;
        };
    };

    class IndexValue : public Child
    {
    public:
        uint64_t key = 0;
        PrimaryExpressionNode *val = nullptr;

        static IndexValue *Parse(LuaParser *parser)
        {
            auto node = new IndexValue();
            parser->StartNode(node);
            node->val = PrimaryExpressionNode::Parse(parser);
            parser->EndNode(node);

            return node;
        }
    };

    std::vector<Child *> children = {};

    Token *tk_left_bracket = nullptr;
    Token *tk_right_bracket = nullptr;
    std::vector<Token *> tk_separators = {};

    static Table *Parse(LuaParser *parser)
    {
        if (!parser->IsValue("{"))
            return nullptr;

        auto tree = new Table();
        parser->StartNode(tree);

        tree->tk_left_bracket = parser->ExpectValue("{");

        size_t index = 0;

        while (true)
        {
            if (parser->IsValue("}"))
                break;

            Child *child = nullptr;

            if (auto res = KeyExpressionValue::Parse(parser))
            {
                child = res;
            }
            else if (auto res = KeyValue::Parse(parser))
            {
                child = res;
            }
            else if (auto res = IndexValue::Parse(parser))
            {
                res->key = index;
                child = res;
            }

            tree->children.push_back(child);

            if (!parser->IsValue(",") && !parser->IsValue(";") && !parser->IsValue("}"))
            {
                throw LuaParser::Exception("Expected something", parser->GetToken(), parser->GetToken());
            }

            if (!parser->IsValue("}"))
                tree->tk_separators.push_back(parser->ExpectValue(","));

            index++;
        }

        tree->tk_right_bracket = parser->ExpectValue("}");

        parser->EndNode(tree);

        return tree;
    };
};

class BinaryOperator : public PrimaryExpressionNode
{
    Token *op;
    PrimaryExpressionNode *left;
    PrimaryExpressionNode *right;
};

class PrefixOperator : public PrimaryExpressionNode
{
    Token *op;
    PrimaryExpressionNode *right;
};

class PostfixOperator : public PrimaryExpressionNode
{
    Token *op;
    PrimaryExpressionNode *left;
};

PrimaryExpressionNode *PrimaryExpressionNode::Parse(LuaParser *parser)
{
    if (auto res = Value::Parse(parser))
        return res;

    if (auto res = Table::Parse(parser))
        return res;

    return nullptr;
}
