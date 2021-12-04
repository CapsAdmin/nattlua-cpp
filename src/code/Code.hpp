#pragma once

#include <cstdint>
#include <string>
#include <optional>

class Code
{
private:
    std::string_view buffer;
    std::string name;

public:
    Code(std::string_view buffer, std::string name);
    ~Code();
    size_t GetByteSize();
    std::string_view GetStringSlice(size_t start, size_t end);
    std::optional<size_t> FindNearest(std::string_view pattern, size_t from);
    uint8_t GetByte(size_t index);
};