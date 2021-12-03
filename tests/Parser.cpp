#include <assert.h>
#include <gtest/gtest.h>
#include "./Helpers.hpp"
#include "../src/parser/PrimaryExpression.hpp"

template <class TO>
auto cast(auto *node)
{
    auto val = dynamic_cast<TO *>(node);
    EXPECT_TRUE(val != nullptr);
    return val;
}

TEST(Parser, EmptyTable)
{
    auto tokens = Tokenize("{}");
    auto parser = new LuaParser(tokens);

    auto table = cast<Table>(ValueExpression::Parse(parser));

    EXPECT_EQ(table->tk_left_bracket->value, "{");
    EXPECT_EQ(table->tk_right_bracket->value, "}");
}

TEST(Parser, TableWithValues)
{
    auto tokens = Tokenize("{1, 2, 3}");
    auto parser = new LuaParser(tokens);

    auto table = cast<Table>(ValueExpression::Parse(parser));

    EXPECT_EQ(table->children.size(), 3);
    EXPECT_EQ(table->tk_separators.size(), 2);

    EXPECT_EQ(table->tk_separators[0]->value, ",");
    EXPECT_EQ(table->tk_separators[1]->value, ",");

    for (int i = 0; i < 3; i++)
    {
        auto child = cast<Table::IndexValue>(table->children[i]);
        auto value = cast<Atomic>(child->val);
        EXPECT_EQ(value->value->value, std::to_string(i + 1));
    }
}

TEST(Parser, BinaryOperator)
{
    auto tokens = Tokenize("1 + 2");
    auto parser = new LuaParser(tokens);

    auto binary = cast<BinaryOperator>(ValueExpression::Parse(parser));
    auto left = cast<Atomic>(binary->left);
    auto right = cast<Atomic>(binary->right);

    EXPECT_EQ(left->value->value, "1");
    EXPECT_EQ(binary->op->value, "+");
    EXPECT_EQ(right->value->value, "2");
}

TEST(Parser, Parenthesis)
{
    auto tokens = Tokenize("1 + (5*2)");
    auto parser = new LuaParser(tokens);

    auto binary = cast<BinaryOperator>(ValueExpression::Parse(parser));
    auto left = cast<Atomic>(binary->left);
    auto right = cast<BinaryOperator>(binary->right);

    EXPECT_EQ(left->value->value, "1");
    EXPECT_EQ(binary->op->value, "+");
    EXPECT_EQ(right->tk_left_parenthesis[0]->value, "(");
    EXPECT_EQ(cast<Atomic>(right->left)->value->value, "5");
    EXPECT_EQ(right->op->value, "*");
    EXPECT_EQ(cast<Atomic>(right->right)->value->value, "2");
    EXPECT_EQ(right->tk_right_parenthesis[0]->value, ")");
}

TEST(Parser, Call)
{
    auto tokens = Tokenize("print(1, 2, 3)");
    auto parser = new LuaParser(tokens);

    auto call = cast<ValueExpression::Call>(ValueExpression::Parse(parser));

    EXPECT_EQ(call->arguments.size(), 3);
    EXPECT_EQ(call->tk_comma.size(), 2);

    EXPECT_EQ(call->tk_comma[0]->value, ",");
    EXPECT_EQ(call->tk_comma[1]->value, ",");

    for (int i = 0; i < 3; i++)
    {
        auto value = cast<Atomic>(call->arguments[i]);
        EXPECT_EQ(value->value->value, std::to_string(i + 1));
    }
}

TEST(Parser, CallParenthesis)
{
    auto tokens = Tokenize("((print(1)))");
    auto parser = new LuaParser(tokens);

    auto call = cast<ValueExpression::Call>(ValueExpression::Parse(parser));

    EXPECT_EQ(call->arguments.size(), 1);
    EXPECT_EQ(call->tk_comma.size(), 0);
    EXPECT_EQ(cast<Atomic>(call->arguments[0])->value->value, "1");

    EXPECT_EQ(call->tk_left_parenthesis[0]->value, "(");
    EXPECT_EQ(call->tk_right_parenthesis[0]->value, ")");

    EXPECT_EQ(call->tk_left_parenthesis[1]->value, "(");
    EXPECT_EQ(call->tk_right_parenthesis[1]->value, ")");
}

TEST(Parser, SelfCall)
{
    auto tokens = Tokenize("self:print(1, 2, 3)");

    auto parser = new LuaParser(tokens);

    auto call = cast<ValueExpression::Call>(ValueExpression::Parse(parser));
    auto self = cast<ValueExpression::SelfCall>(call->left);

    EXPECT_EQ(self->op->value, ":");

    EXPECT_EQ(cast<Atomic>(self->left)->value->value, "self");
    EXPECT_EQ(cast<Atomic>(self->right)->value->value, "print");

    EXPECT_EQ(call->arguments.size(), 3);
}

TEST(Parser, IndexExpression)
{
    auto tokens = Tokenize("a[1]");
    auto parser = new LuaParser(tokens);

    auto index = cast<ValueExpression::IndexExpression>(ValueExpression::Parse(parser));

    EXPECT_EQ(index->tk_left_bracket->value, "[");
    EXPECT_EQ(index->tk_right_bracket->value, "]");
    EXPECT_EQ(cast<Atomic>(index->index)->value->value, "1");
}

TEST(Parser, TypeCast)
{
    auto tokens = Tokenize("\"foo\" as foo");

    auto parser = new LuaParser(tokens);

    auto type_cast = cast<ValueExpression::TypeCast>(ValueExpression::Parse(parser));

    EXPECT_EQ(type_cast->tk_operator->value, "as");

    EXPECT_EQ(cast<Atomic>(type_cast->left)->value->value, "\"foo\"");
    EXPECT_EQ(cast<Atomic>(type_cast->expression)->value->value, "foo");
}