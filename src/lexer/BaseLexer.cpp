#include "./BaseLexer.hpp"
#include "../code/Code.hpp"

string_view BaseLexer::GetString(size_t start, size_t stop)
{
    return code->GetString(start, stop);
}

uint8_t BaseLexer::GetChar(size_t offset)
{
    return code->GetChar(offset);
}
bool BaseLexer::IsString(string value, size_t relative_offset)
{
    auto l = GetString(position + relative_offset, position + relative_offset + value.size());

    return l == value;
}

void BaseLexer::ResetState()
{
    comment_escape = false;
    position = 0;
}

optional<size_t> BaseLexer::FindNearest(string pattern)
{
    return code->FindNearest(pattern, position);
}

void BaseLexer::Advance(size_t offset)
{
    position += offset;
}

uint8_t BaseLexer::ReadChar()
{
    auto byte = GetChar(0);
    Advance(1);
    return byte;
}

bool BaseLexer::TheEnd()
{
    return position >= code->GetSize();
}

bool BaseLexer::ReadFromArray(vector<string> array)
{
    for (auto &s : array)
    {
        if (IsString(s, 0))
        {
            Advance(s.size());
            return true;
        }
    }
    return false;
}

Token *BaseLexer::ReadCommentEscape()
{
    if (!IsString("--[[#", 0))
    {
        return nullptr;
    }

    Advance(5);
    comment_escape = true;

    return new Token(TokenType::CommentEscape);
}

Token *BaseLexer::ReadRemainingCommentEscape()
{
    if (!comment_escape || IsString("]]", 0))
    {
        return nullptr;
    }

    Advance(2);
    comment_escape = false;

    return new Token(TokenType::CommentEscape);
}

Token *BaseLexer::ReadEndOfFile()
{
    if (!TheEnd())
    {
        return nullptr;
    }

    return new Token(TokenType::EndOfFile);
}

Token *BaseLexer::ReadUnknown()
{
    Advance(1);

    return new Token(TokenType::Unknown);
}

/*
    fn read_shebang(&mut self) -> TokenResult {
        if self.position != 0 || !self.is_string("#", 0) {
            return Ok(None);
        }

        while !self.the_end() {
            self.advance(1);

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
    {
        return nullptr;
    }

    while (!TheEnd())
    {
        Advance(1);

        if (IsString("\n", 0))
        {
            break;
        }
    }

    return new Token(TokenType::Shebang);
}

Token *BaseLexer::ReadSingleToken()
{
    auto start = position;

    Token *token;

    if (auto res = ReadShebang())
        token = res;
    else if (auto res = ReadRemainingCommentEscape())
        token = res;
    else if (auto res = ReadWhitespaceToken())
        token = res;
    else if (auto res = ReadNonWhitespaceToken())
        token = res;
    else if (auto res = ReadEndOfFile())
        token = res;
    else if (auto res = ReadUnknown())
        token = res;

    token->start = start;
    token->stop = position;

    return token;
}

Token *BaseLexer::ReadToken()
{
    auto whitespace_tokens = vector<Token *>();

    while (true)
    {
        auto token = ReadSingleToken();

        if (token->IsWhitespace())
        {
            whitespace_tokens.push_back(token);
        }
        else
        {
            for (auto &token : whitespace_tokens)
            {
                token->value = GetString(token->start, token->stop);
            }

            token->value = GetString(token->start, token->stop);
            token->whitespace = whitespace_tokens;

            whitespace_tokens.clear();

            return token;
        }
    }
}

pair<vector<Token *>, vector<LexerException>> BaseLexer::GetTokens()
{
    ResetState();

    auto tokens = vector<Token *>();
    auto errors = vector<LexerException>();

    while (true)
    {
        try
        {

            auto token = ReadToken();

            if (token->kind == TokenType::EndOfFile)
            {
                break;
            }
        }
        catch (LexerException err)
        {
            errors.push_back(err);
        }
    }

    return make_pair(tokens, errors);
}
