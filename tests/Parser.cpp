#include <assert.h>
#include <gtest/gtest.h>
#include "./Helpers.hpp"
#include "../src/parser/PrimaryExpression.hpp"

template <typename T, typename U>
inline std::unique_ptr<T> cast2(std::unique_ptr<U> &&ptr)
{
    U *const stored_ptr = ptr.release();
    T *const converted_stored_ptr = dynamic_cast<T *>(stored_ptr);
    if (converted_stored_ptr)
    {
        std::cout << "Cast did succeeded\n";
        return std::unique_ptr<T>(converted_stored_ptr);
    }
    else
    {
        std::cout << "Cast did not succeeded\n";
        ptr.reset(stored_ptr);
        return std::unique_ptr<T>();
    }
}

template <class T>
auto cast(ParserNode *node)
{
    auto val = dynamic_cast<T *>(node);
    EXPECT_TRUE(val != nullptr);
    return val;
}

template <class T>
auto Parse(std::string_view code)
{
    auto tokens = Tokenize(code);
    auto parser = std::make_shared<LuaParser>(std::move(tokens));
    auto node = cast<T>(ValueExpression::Parse(parser).release());
    return node;
}

template <class T>
auto Parse2(std::string_view code)
{
    auto tokens = Tokenize(code);
    auto parser = std::make_shared<LuaParser>(std::move(tokens));
    auto node = cast2<T, Expression>(std::move(ValueExpression::Parse(parser)));
    return node;
}

TEST(Parser, EmptyTable)
{
    auto table = Parse2<Table>("{}");

    EXPECT_EQ(table->tk_left_bracket->value, "{");
    EXPECT_EQ(table->tk_right_bracket->value, "}");
}

TEST(Parser, TableWithIndexValues)
{
    auto table = Parse2<Table>("{1, 2, 3}");

    EXPECT_EQ(table->children.size(), 3);
    EXPECT_EQ(table->tk_separators.size(), 2);

    EXPECT_EQ(table->tk_separators[0]->value, ",");
    EXPECT_EQ(table->tk_separators[1]->value, ",");

    for (int i = 0; i < 3; i++)
    {
        auto child = cast2<Table::IndexValue>(std::move(table->children[i]));
        auto value = cast2<Atomic>(std::move(child->val));
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

    auto first = cast<Table::ExpressionKeyValue>(table->children[0].release());

    EXPECT_EQ(first->tk_left_bracket->value, "[");
    EXPECT_EQ(cast<Atomic>(first->key.release())->value->value, "1337");
    EXPECT_EQ(first->tk_right_bracket->value, "]");
    EXPECT_EQ(first->tk_equal->value, "=");
    EXPECT_EQ(cast<Atomic>(first->val.release())->value->value, "1");

    auto second = cast<Table::ExpressionKeyValue>(table->children[1].release());
    EXPECT_EQ(second->tk_left_bracket->value, "[");
    EXPECT_EQ(cast<Atomic>(second->key.release())->value->value, "\"foo\"");
    EXPECT_EQ(second->tk_right_bracket->value, "]");
    EXPECT_EQ(second->tk_equal->value, "=");
    EXPECT_EQ(cast<Atomic>(second->val.release())->value->value, "2");

    auto third = cast<Table::ExpressionKeyValue>(table->children[2].release());
    EXPECT_EQ(third->tk_left_bracket->value, "[");
    EXPECT_EQ(cast<Atomic>(third->key.release())->value->value, "foo");
    EXPECT_EQ(third->tk_right_bracket->value, "]");
    EXPECT_EQ(third->tk_equal->value, "=");
    EXPECT_EQ(cast<Atomic>(third->val.release())->value->value, "3");
}

TEST(Parser, BinaryOperator)
{
    auto binary = Parse<BinaryOperator>("1 + 2");

    auto left = cast<Atomic>(binary->left.release());
    auto right = cast<Atomic>(binary->right.release());

    EXPECT_EQ(left->value->value, "1");
    EXPECT_EQ(binary->op->value, "+");
    EXPECT_EQ(right->value->value, "2");
}

TEST(Parser, Parenthesis)
{
    auto binary = Parse<BinaryOperator>("1 + (5*2)");

    auto left = cast<Atomic>(binary->left.release());
    auto right = cast<BinaryOperator>(binary->right.release());

    EXPECT_EQ(left->value->value, "1");
    EXPECT_EQ(binary->op->value, "+");
    EXPECT_EQ(right->tk_left_parenthesis[0]->value, "(");
    EXPECT_EQ(cast<Atomic>(right->left.release())->value->value, "5");
    EXPECT_EQ(right->op->value, "*");
    EXPECT_EQ(cast<Atomic>(right->right.release())->value->value, "2");
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
        auto value = cast<Atomic>(call->arguments[i].release());
        EXPECT_EQ(value->value->value, std::to_string(i + 1));
    }
}
TEST(Parser, ChainedCalls)
{
    auto call_3 = Parse<ValueExpression::Call>("foo(1)(2)(3)");

    auto call_2 = cast<ValueExpression::Call>(call_3->left.release());
    auto call_1 = cast<ValueExpression::Call>(call_2->left.release());

    EXPECT_EQ(cast<Atomic>(call_3->arguments[0].release())->value->value, "3");
    EXPECT_EQ(cast<Atomic>(call_2->arguments[0].release())->value->value, "2");
    EXPECT_EQ(cast<Atomic>(call_1->arguments[0].release())->value->value, "1");
}

TEST(Parser, CallParenthesis)
{
    auto call = Parse<ValueExpression::Call>("((print(1)))");

    EXPECT_EQ(call->arguments.size(), 1);
    EXPECT_EQ(call->tk_comma.size(), 0);
    EXPECT_EQ(cast<Atomic>(call->arguments[0].release())->value->value, "1");

    EXPECT_EQ(call->tk_left_parenthesis[0]->value, "(");
    EXPECT_EQ(call->tk_right_parenthesis[0]->value, ")");

    EXPECT_EQ(call->tk_left_parenthesis[1]->value, "(");
    EXPECT_EQ(call->tk_right_parenthesis[1]->value, ")");
}

TEST(Parser, SelfCall)
{
    auto call = Parse<ValueExpression::Call>("self:print(1, 2, 3)");

    auto self = cast<ValueExpression::SelfCall>(call->left.release());

    EXPECT_EQ(self->op->value, ":");

    EXPECT_EQ(cast<Atomic>(self->left.release())->value->value, "self");
    EXPECT_EQ(cast<Atomic>(self->right.release())->value->value, "print");

    EXPECT_EQ(call->arguments.size(), 3);
}

TEST(Parser, IndexExpression)
{
    auto index = Parse<ValueExpression::IndexExpression>("a[1]");

    EXPECT_EQ(index->tk_left_bracket->value, "[");
    EXPECT_EQ(index->tk_right_bracket->value, "]");
    EXPECT_EQ(cast<Atomic>(index->index.release())->value->value, "1");
}

TEST(Parser, TypeCast)
{
    auto type_cast = Parse<ValueExpression::TypeCast>("\"foo\" as foo");

    EXPECT_EQ(type_cast->tk_operator->value, "as");

    EXPECT_EQ(cast<Atomic>(type_cast->left.release())->value->value, "\"foo\"");
    EXPECT_EQ(cast<Atomic>(type_cast->expression.release())->value->value, "foo");
}