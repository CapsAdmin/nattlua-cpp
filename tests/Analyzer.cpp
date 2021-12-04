#include <assert.h>
#include <gtest/gtest.h>
#include "./Helpers.hpp"
#include "../src/parser/PrimaryExpression.hpp"
#include "../src/analyzer/Analyzer.hpp"

template <typename T>
inline std::unique_ptr<T> cast_uptr(std::unique_ptr<ParserNode> &&ptr)
{
    ParserNode *const stored_ptr = ptr.release();
    T *const converted_stored_ptr = dynamic_cast<T *>(stored_ptr);
    if (converted_stored_ptr)
    {
        return std::unique_ptr<T>(converted_stored_ptr);
    }
    else
    {
        ptr.reset(stored_ptr);
        return std::unique_ptr<T>();
    }
}

template <typename T>
inline std::unique_ptr<T> cast_uptr2(std::unique_ptr<BaseType> &&ptr)
{
    BaseType *const stored_ptr = ptr.release();
    T *const converted_stored_ptr = dynamic_cast<T *>(stored_ptr);
    if (converted_stored_ptr)
    {
        return std::unique_ptr<T>(converted_stored_ptr);
    }
    else
    {
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
auto cast2(BaseType *node)
{
    auto val = dynamic_cast<T *>(node);
    EXPECT_TRUE(val != nullptr);
    return val;
}

inline auto Parse(std::string_view code)
{
    auto tokens = Tokenize(code);
    auto parser = std::make_shared<LuaParser>(std::move(tokens));
    auto node = cast_uptr<ParserNode>(std::move(ValueExpression::Parse(parser)));
    return std::move(node);
}

TEST(Parser, Atomic)
{
    auto ast = std::shared_ptr<Expression>(cast_uptr<Expression>(Parse("0x123")).release());
    auto analyzer = std::make_shared<LuaAnalyzer>();

    auto num = cast2<Number>(analyzer->AnalyzeExpression(ast, ParserNode::Runtime).get());

    EXPECT_EQ(num->value, 0x123);
}
