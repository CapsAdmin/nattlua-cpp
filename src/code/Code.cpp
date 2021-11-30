#include "./Code.hpp"

Code::Code(std::string_view buffer, const std::string name)
{
    this->buffer = buffer;
    this->name = name;
};

Code::~Code(){};

size_t Code::GetByteSize()
{
    return buffer.size();
}

std::string_view Code::GetStringSlice(size_t start, size_t end)
{
    return buffer.substr(start, end - start);
}

std::optional<size_t> Code::FindNearest(std::string pattern, size_t from)
{
    auto pos = buffer.find_first_of(pattern, from);

    if (pos == std::string::npos)
        return std::nullopt;

    return pos;
}

uint8_t Code::GetByte(size_t index)
{
    if (index >= buffer.size())
        return 0;

    return buffer.at(index);
}