#pragma once

#include <cstdint>
#include "../code/Code.hpp"
#include "../syntax/RuntimeSyntax.hpp"
#include "../syntax/TypesystemSyntax.hpp"
#include "./Token.hpp"

class BaseLexer
{
public:
    class Exception : public std::exception
    {
    private:
        std::string message;
        size_t start;
        size_t stop;

    public:
        explicit Exception(const std::string &message, size_t start, size_t stop)
        {
            this->message = message;
            this->start = start;
            this->stop = stop;
        }
        const char *what() const noexcept override
        {
            return message.c_str();
        }
    };

    std::shared_ptr<Code> code;
    size_t position = 0;
    RuntimeSyntax *runtime_syntax = new RuntimeSyntax();
    TypesystemSyntax *typesystem_syntax = new TypesystemSyntax();

    virtual std::unique_ptr<Token> ReadNonWhitespaceToken() = 0;
    virtual std::unique_ptr<Token> ReadWhitespaceToken() = 0;

    std::unique_ptr<Token> ReadToken();
    std::unique_ptr<Token> ReadShebang();
    std::unique_ptr<Token> ReadUnknown();
    std::unique_ptr<Token> ReadEndOfFile();
    std::unique_ptr<Token> ReadCommentEscape();
    std::unique_ptr<Token> ReadRemainingCommentEscape();
    std::pair<std::vector<std::unique_ptr<Token>>, std::vector<BaseLexer::Exception>> GetTokens();

    std::string_view GetRelativeStringSlice(size_t start, size_t stop);
    uint8_t GetByte(size_t offset = 0);
    bool IsString(std::string_view value, const size_t relative_offset = 0);
    void ResetState();
    std::optional<size_t> FindNearest(std::string_view pattern);
    uint8_t ReadByte();
    bool TheEnd();
    bool ReadFirstFromStringArray(std::vector<std::string> array);

private:
    bool comment_escape = false;
    std::unique_ptr<Token> ReadSingleToken();
};