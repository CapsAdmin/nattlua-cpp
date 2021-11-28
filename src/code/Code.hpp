#pragma once

#include <cstdint>
#include <string>
#include <optional>

using namespace std;

class Code
{
private:
    string_view buffer;
    string name;

public:
    Code(string buffer, string name);
    ~Code();
    size_t GetSize();
    string_view GetString(size_t start, size_t end);
    optional<size_t> FindNearest(string pattern, size_t from);
    uint8_t GetChar(size_t index);
};