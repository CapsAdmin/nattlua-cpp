#include "./Code.hpp"

Code::Code(string_view buffer, string name)
{
    this->buffer = buffer;
    this->name = name;
};

Code::~Code(){};

size_t Code::GetSize()
{
    return buffer.size();
}

string_view Code::GetString(size_t start, size_t end)
{
    return buffer.substr(start, end - start);
}

optional<size_t> Code::FindNearest(string pattern, size_t from)
{
    auto pos = buffer.find_first_of(pattern, from);

    if (pos == string::npos)
        return nullopt;

    return pos;
}

uint8_t Code::GetChar(size_t index)
{
    if (index >= buffer.size())
        return 0;

    return buffer.at(index);
}