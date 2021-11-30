#include "./BaseLexer.hpp"
#include "../code/Code.hpp"

std::string_view BaseLexer::GetRelativeStringSlice(size_t start, size_t stop)
{
    return code->GetStringSlice(position + start, position + stop);
}

uint8_t BaseLexer::GetByte(size_t offset)
{
    return code->GetByte(position + offset);
}

bool BaseLexer::IsString(const std::string &value, const size_t relative_offset)
{
    auto l = code->GetStringSlice(position + relative_offset, position + relative_offset + value.size());

    return l == value;
}

void BaseLexer::ResetState()
{
    comment_escape = false;
    position = 0;
}

std::optional<size_t> BaseLexer::FindNearest(std::string pattern)
{
    return code->FindNearest(pattern, position);
}

uint8_t BaseLexer::ReadByte()
{
    auto byte = GetByte();
    position += 1;
    return byte;
}

bool BaseLexer::TheEnd()
{
    return position >= code->GetByteSize();
}

bool BaseLexer::ReadFirstFromStringArray(std::vector<std::string> strings)
{
    for (auto &str : strings)
    {
        if (IsString(str))
        {
            position += str.size();
            return true;
        }
    }

    return false;
}

Token *BaseLexer::ReadCommentEscape()
{
    if (!IsString("--[[#"))
        return nullptr;

    position += 5;
    comment_escape = true;

    return new Token(TokenType::CommentEscape);
}

Token *BaseLexer::ReadRemainingCommentEscape()
{
    if (!comment_escape || IsString("]]"))
        return nullptr;

    position += 2;
    comment_escape = false;

    return new Token(TokenType::CommentEscape);
}

Token *BaseLexer::ReadEndOfFile()
{
    if (!TheEnd())
        return nullptr;

    return new Token(TokenType::EndOfFile);
}

Token *BaseLexer::ReadUnknown()
{
    position += 1;

    return new Token(TokenType::Unknown);
}

/*
    fn read_shebang(&mut self) -> TokenResult {
        if self.position != 0 || !self.is_string("#", 0) {
            return Ok(None);
        }

        while !self.the_end() {
            self.position += 1;

            if self.is_string("\n", 0) {
                break;
            }
        }

        Ok(Some(TokenType::Shebang))
    }
*/

Token *BaseLexer::ReadShebang()
{
    if (position != 0 || !IsString("#", 0))
        return nullptr;

    while (!TheEnd())
    {
        position += 1;

        if (IsString("\n", 0))
            break;
    }

    return new Token(TokenType::Shebang);
}

Token *BaseLexer::ReadSingleToken()
{
    auto start = position;

    Token *token;

    if (auto res = ReadShebang())
        token = res;
    else if (auto res = ReadEndOfFile())
        token = res;
    else if (auto res = ReadWhitespaceToken())
        token = res;
    else if (auto res = ReadNonWhitespaceToken())
        token = res;
    else
        token = ReadUnknown();

    token->start = start;
    token->stop = position;

    return token;
}

Token *BaseLexer::ReadToken()
{
    auto whitespace_tokens = std::vector<Token *>();

    while (true)
    {
        auto token = ReadSingleToken();

        if (token->IsWhitespace())
        {
            whitespace_tokens.push_back(token);
        }
        else
        {
            for (auto &whitespace_token : whitespace_tokens)
            {
                whitespace_token->value = code->GetStringSlice(whitespace_token->start, whitespace_token->stop);
            }

            token->value = code->GetStringSlice(token->start, token->stop);
            token->whitespace = whitespace_tokens;

            whitespace_tokens.clear();

            return token;
        }
    }
}

std::pair<std::vector<Token *>, std::vector<LexerException>> BaseLexer::GetTokens()
{
    ResetState();

    auto tokens = std::vector<Token *>();
    auto errors = std::vector<LexerException>();

    while (true)
    {
        try
        {
            auto token = ReadToken();

            tokens.push_back(token);

            if (token->kind == TokenType::EndOfFile)
            {
                break;
            }
        }
        catch (LexerException &err)
        {
            errors.push_back(err);
        }
    }

    return make_pair(tokens, errors);
}
