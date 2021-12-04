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

template <class T>
auto Parse(const std::string &code)
{
    auto tokens = Tokenize(code);
    auto parser = std::make_shared<LuaParser>(tokens);
    auto node = cast<T>(ValueExpression::Parse(parser));
    auto table = cast<Table>(node);
    return node;
}

TEST(Parser, EmptyTable)
{
    auto table = Parse<Table>("{}");

    EXPECT_EQ(table->tk_left_bracket->value, "{");
    EXPECT_EQ(table->tk_right_bracket->value, "}");
}

TEST(Parser, TableWithIndexValues)
{
    auto table = Parse<Table>("{1, 2, 3}");

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

TEST(Parser, TableWithIndexExpressions)
{
    auto table = Parse<Table>("{[1337] = 1, [\"foo\"] = 2, [foo] = 3}");

    EXPECT_EQ(table->children.size(), 3);
    EXPECT_EQ(table->tk_separators.size(), 2);

    EXPECT_EQ(table->tk_separators[0]->value, ",");
    EXPECT_EQ(table->tk_separators[1]->value, ",");

    auto first = cast<Table::ExpressionKeyValue>(table->children[0]);

    EXPECT_EQ(first->tk_left_bracket->value, "[");
    EXPECT_EQ(cast<Atomic>(first->key)->value->value, "1337");
    EXPECT_EQ(first->tk_right_bracket->value, "]");
    EXPECT_EQ(first->tk_equal->value, "=");
    EXPECT_EQ(cast<Atomic>(first->val)->value->value, "1");

    auto second = cast<Table::ExpressionKeyValue>(table->children[1]);
    EXPECT_EQ(second->tk_left_bracket->value, "[");
    EXPECT_EQ(cast<Atomic>(second->key)->value->value, "\"foo\"");
    EXPECT_EQ(second->tk_right_bracket->value, "]");
    EXPECT_EQ(second->tk_equal->value, "=");
    EXPECT_EQ(cast<Atomic>(second->val)->value->value, "2");

    auto third = cast<Table::ExpressionKeyValue>(table->children[2]);
    EXPECT_EQ(third->tk_left_bracket->value, "[");
    EXPECT_EQ(cast<Atomic>(third->key)->value->value, "foo");
    EXPECT_EQ(third->tk_right_bracket->value, "]");
    EXPECT_EQ(third->tk_equal->value, "=");
    EXPECT_EQ(cast<Atomic>(third->val)->value->value, "3");
}

TEST(Parser, BinaryOperator)
{
    auto binary = Parse<BinaryOperator>("1 + 2");

    auto left = cast<Atomic>(binary->left);
    auto right = cast<Atomic>(binary->right);

    EXPECT_EQ(left->value->value, "1");
    EXPECT_EQ(binary->op->value, "+");
    EXPECT_EQ(right->value->value, "2");
}

TEST(Parser, Parenthesis)
{
    auto binary = Parse<BinaryOperator>("1 + (5*2)");

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
    auto call = Parse<ValueExpression::Call>("print(1, 2, 3)");

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
TEST(Parser, ChainedCalls)
{
    auto call_3 = Parse<ValueExpression::Call>("foo(1)(2)(3)");

    auto call_2 = cast<ValueExpression::Call>(call_3->left);
    auto call_1 = cast<ValueExpression::Call>(call_2->left);

    EXPECT_EQ(cast<Atomic>(call_3->arguments[0])->value->value, "3");
    EXPECT_EQ(cast<Atomic>(call_2->arguments[0])->value->value, "2");
    EXPECT_EQ(cast<Atomic>(call_1->arguments[0])->value->value, "1");
}

TEST(Parser, CallParenthesis)
{
    auto call = Parse<ValueExpression::Call>("((print(1)))");

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
    auto call = Parse<ValueExpression::Call>("self:print(1, 2, 3)");

    auto self = cast<ValueExpression::SelfCall>(call->left);

    EXPECT_EQ(self->op->value, ":");

    EXPECT_EQ(cast<Atomic>(self->left)->value->value, "self");
    EXPECT_EQ(cast<Atomic>(self->right)->value->value, "print");

    EXPECT_EQ(call->arguments.size(), 3);
}

TEST(Parser, IndexExpression)
{
    auto index = Parse<ValueExpression::IndexExpression>("a[1]");

    EXPECT_EQ(index->tk_left_bracket->value, "[");
    EXPECT_EQ(index->tk_right_bracket->value, "]");
    EXPECT_EQ(cast<Atomic>(index->index)->value->value, "1");
}

TEST(Parser, TypeCast)
{
    auto type_cast = Parse<ValueExpression::TypeCast>("\"foo\" as foo");

    EXPECT_EQ(type_cast->tk_operator->value, "as");

    EXPECT_EQ(cast<Atomic>(type_cast->left)->value->value, "\"foo\"");
    EXPECT_EQ(cast<Atomic>(type_cast->expression)->value->value, "foo");
}