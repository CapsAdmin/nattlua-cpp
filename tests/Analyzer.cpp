#include <assert.h>
#include <gtest/gtest.h>
#include "./Helpers.hpp"
#include "../src/parser/PrimaryExpression.hpp"
#include "../src/analyzer/Analyzer.hpp"

TEST(Analyzer, Atomic)
{
    auto ast = std::shared_ptr<Expression>(cast_uptr<ParserNode, Expression>(Parse("0x123")).release());
    auto analyzer = std::make_shared<LuaAnalyzer>();

    auto num = cast<Number>(analyzer->AnalyzeExpression(ast, ParserNode::Runtime).get());

    EXPECT_EQ(num->value, 0x123);
}
