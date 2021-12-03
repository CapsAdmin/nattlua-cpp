#include <assert.h>
#include <gtest/gtest.h>
#include "./Helpers.hpp"
#include "../src/parser/PrimaryExpression.hpp"

template <class FROM, class TO>
auto cast(FROM *node)
{
    auto val = dynamic_cast<TO *>(node);
    EXPECT_TRUE(val != nullptr);
    return val;
}

TEST(Parser, EmptyTable)
{
    auto tokens = Tokenize("{}");
    auto parser = new LuaParser(tokens);

    auto table = cast<ExpressionNode, Table>(ExpressionNode::Parse(parser));

    EXPECT_EQ(table->tk_left_bracket->value, "{");
    EXPECT_EQ(table->tk_right_bracket->value, "}");
}

TEST(Parser, TableWithValues)
{
    auto tokens = Tokenize("{1, 2, 3}");
    auto parser = new LuaParser(tokens);

    auto table = cast<ExpressionNode, Table>(ExpressionNode::Parse(parser));

    EXPECT_EQ(table->children.size(), 3);
    EXPECT_EQ(table->tk_separators.size(), 2);

    EXPECT_EQ(table->tk_separators[0]->value, ",");
    EXPECT_EQ(table->tk_separators[1]->value, ",");

    for (int i = 0; i < 3; i++)
    {
        auto child = cast<Table::Child, Table::IndexValue>(table->children[i]);
        auto value = cast<ExpressionNode, Value>(child->val);
        EXPECT_EQ(value->value->value, std::to_string(i + 1));
    }
}

TEST(Parser, BinaryOperator)
{
    auto tokens = Tokenize("1 + 2");
    auto parser = new LuaParser(tokens);

    auto binary = cast<ExpressionNode, BinaryOperator>(ExpressionNode::Parse(parser));
    auto left = cast<ParserNode, Value>(binary->left);
    auto right = cast<ExpressionNode, Value>(binary->right);

    EXPECT_EQ(binary->op->value, "+");
    EXPECT_EQ(left->value->value, "1");
    EXPECT_EQ(right->value->value, "2");
}