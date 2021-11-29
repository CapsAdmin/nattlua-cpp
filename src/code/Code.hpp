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
    Code(string_view buffer, string name);
    ~Code();
    size_t GetByteSize();
    string_view GetStringSlice(size_t start, size_t end);
    optional<size_t> FindNearest(string pattern, size_t from);
    uint8_t GetByte(size_t index);
};