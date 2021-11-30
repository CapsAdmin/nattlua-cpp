#include <assert.h>
#include <gtest/gtest.h>
#include "./Helpers.hpp"

TEST(Parser, Table)
{
    auto tokens = Tokenize("{}");
    auto parser = new LuaParser(tokens);
}