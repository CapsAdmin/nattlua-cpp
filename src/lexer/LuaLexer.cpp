#include "./LuaLexer.hpp"
#include "../syntax/CharacterClasses.hpp"

Token *LuaLexer::ReadSpace()
{
    if (!IsSpace(GetChar(0)))
    {
        return nullptr;
    }

    while (!TheEnd())
    {
        Advance(1);

        if (!IsSpace(GetChar(0)))
        {
            break;
        }
    }

    return new Token(TokenType::Space);
}

Token *LuaLexer::ReadLetter()
{
    if (!IsLetter(GetChar()))
    {
        return nullptr;
    }

    while (!TheEnd())
    {
        Advance(1);

        if (!IsDuringLetter(GetChar(0)))
        {
            break;
        }
    }

    return new Token(TokenType::Letter);
}

Token *LuaLexer::ReadSymbol()
{
    if (!ReadFromArray(runtime_syntax->symbols) && !ReadFromArray(typesystem_syntax->symbols))
    {
        return nullptr;
    }

    return new Token(TokenType::Symbol);
}

Token *LuaLexer::ReadMultilineCComment()
{
    if (!IsString("/*"))
    {
        return nullptr;
    }

    auto start = position;
    Advance(2);

    while (!TheEnd())
    {
        if (IsString("*/"))
        {
            Advance(2);
            return new Token(TokenType::MultilineComment);
        }

        Advance(1);
    }

    throw LexerException("tried to find end of multiline c comment, reached end of code", start, position);
}

Token *LuaLexer::ReadLineComment()
{
    if (!IsString("--"))
    {
        return nullptr;
    }

    Advance(2);

    while (!TheEnd())
    {
        if (IsString("\n"))
        {
            break;
        }

        Advance(1);
    }

    return new Token(TokenType::LineComment);
}

Token *LuaLexer::ReadLineCComment()
{
    if (!IsString("//"))
    {
        return nullptr;
    }

    Advance(2);

    while (!TheEnd())
    {
        if (IsString("\n"))
        {
            break;
        }

        Advance(1);
    }

    return new Token(TokenType::LineComment);
}

std::string repeat(const std::string &input, size_t num)
{
    std::ostringstream os;
    std::fill_n(std::ostream_iterator<std::string>(os), num, input);
    return os.str();
}

Token *LuaLexer::ReadMultilineComment()
{
    if (!IsString("--[") || (!IsString("[", 3) && !IsString("=", 3)))
    {
        return nullptr;
    }

    auto start = position;
    Advance(3);

    while (IsString("="))
    {
        Advance(1);
    }

    // if we have an incomplete multiline comment, it's just a single line comment
    if (!IsString("["))
    {
        position = start;
        return ReadLineComment();
    }

    Advance(1);
    auto pos = position;

    auto closing = string("]" + repeat("=", pos - start - 4) + "]");
    auto pos2 = FindNearest(closing);

    if (pos2.has_value())
    {
        position = pos + pos2.value() + closing.size();
        return new Token(TokenType::MultilineComment);
    }

    position = start + 2;

    throw LexerException("unclosed multiline comment", start, start + 1);
}

Token *LuaLexer::ReadAnalyzerDebugCode()
{
    if (!IsString("§"))
    {
        return nullptr;
    }

    Advance(2);

    while (!TheEnd())
    {
        if (IsString("\n"))
        {
            break;
        }

        Advance(1);
    }

    return new Token(TokenType::AnalyzerDebugCode);
}

Token *LuaLexer::ReadParserDebugCode()
{
    if (!IsString("£"))
    {
        return nullptr;
    }

    Advance(2);

    while (!TheEnd())
    {
        if (IsString("\n"))
        {
            break;
        }

        Advance(1);
    }

    return new Token(TokenType::ParserDebugCode);
}

bool ReadNumberExponent(LuaLexer &lexer, string what)
{
    // skip the 'e', 'E', 'p' or 'P'
    lexer.Advance(1);

    if (lexer.IsString("+") || lexer.IsString("-"))
    {
        lexer.Advance(1);
    }
    else
    {
        throw LexerException("expected '+' or '-' after '" + what + "'", lexer.position - 1, lexer.position);
    }

    if (!IsNumber(lexer.GetChar()))
    {
        throw LexerException("malformed '" + what + "' expected number, got " + string(lexer.GetString(0, 1)), lexer.position - 2, lexer.position - 1);
    }

    while (!lexer.TheEnd())
    {
        if (!IsNumber(lexer.GetChar()))
        {
            break;
        }

        lexer.Advance(1);
    }

    return true;
}

Token *LuaLexer::ReadHexNumber()
{
    if (!IsString("0") || (!IsString("x", 1) && !IsString("X", 1)))
    {
        return nullptr;
    }

    // skip past 0x
    Advance(2);

    while (!TheEnd())
    {
        if (IsString("_"))
        {
            Advance(1);
        }

        if (IsString(".") && !IsString(".", 1))
        {
            Advance(1);
        }

        if (IsValidHex(GetChar()))
        {
            Advance(1);
        }
        else
        {

            if (IsSpace(GetChar()) || IsSymbol(GetChar()))
            {
                break;
            }

            if ((IsString("p") || IsString("P")) && ReadNumberExponent(*this, "pow"))
            {
                break;
            }

            throw LexerException("malformed hex number " + string(GetString(0, 1)) + " in hex notation", position - 1, position);
        }
    }

    ReadFromArray(runtime_syntax->number_annotations);

    return new Token(TokenType::Number);
}

Token *LuaLexer::ReadBinaryNumber()
{
    if (!IsString("0") || (!IsString("b", 1) && !IsString("B", 1)))
    {
        return nullptr;
    }

    // skip past 0b
    Advance(2);

    while (!TheEnd())
    {
        if (IsString("_"))
        {
            Advance(1);
        }

        if (IsString("1") || IsString("0"))
        {
            Advance(1);
        }
        else
        {
            if (IsSpace(GetChar()) || IsSymbol(GetChar()))
            {
                break;
            }

            if ((IsString("p") || IsString("P")) && ReadNumberExponent(*this, "pow"))
            {
                break;
            }

            throw LexerException("malformed binary number " + string(GetString(0, 1)) + " in binary notation", position - 1, position);
        }
    }

    ReadFromArray(runtime_syntax->number_annotations);

    return new Token(TokenType::Number);
}

Token *LuaLexer::ReadDecimalNumber()
{
    if (!IsNumber(GetChar()) && (!IsString(".") || !IsNumber(GetChar(1))))
    {
        return nullptr;
    }

    auto has_dot = false;
    if (IsString("."))
    {
        has_dot = true;
        Advance(1);
    }

    while (!TheEnd())
    {
        if (IsString("_"))
        {
            Advance(1);
        }

        if (!has_dot && IsString("."))
        {
            if (IsString(".", 1))
            {
                break;
            }
            has_dot = true;
            Advance(1);
        }

        if (IsNumber(GetChar()))
        {
            Advance(1);
        }
        else
        {
            if (IsSpace(GetChar()) || IsSymbol(GetChar()))
            {
                break;
            }

            if ((IsString("e") || IsString("E")) && ReadNumberExponent(*this, "exponent"))
            {
                break;
            }

            throw LexerException("malformed decimal number " + string(GetString(0, 1)) + " in decimal notation", position - 1, position);
        }
    }

    ReadFromArray(runtime_syntax->number_annotations);

    return new Token(TokenType::Number);
}

Token *LuaLexer::ReadNumber()
{
    if (auto res = ReadHexNumber())
    {
        return res;
    }
    if (auto res = ReadBinaryNumber())
    {
        return res;
    }
    if (auto res = ReadDecimalNumber())
    {
        return res;
    }

    return nullptr;
}

Token *LuaLexer::ReadMultilineString()
{
    if (!IsString("[", 0) || (!IsString("[", 1) && !IsString("=", 1)))
    {
        return nullptr;
    }

    auto start = position;
    Advance(1);

    if (IsString("=", 0))
    {
        while (!TheEnd())
        {
            Advance(1);
            if (!IsString("=", 0))
            {
                break;
            }
        }
    }

    if (!IsString("[", 0))
    {
        throw LexerException("malformed multiline string, expected =", start, position);
    }

    Advance(1);

    auto pos = position;

    auto closing = string("]" + repeat("=", pos - start - 2) + "]");
    auto pos2 = FindNearest(closing);

    if (pos2.has_value())
    {
        position = pos + pos2.value() + closing.size();
        return new Token(TokenType::String);
    }

    throw LexerException("expected multiline string reached end of code", start, position);
}

Token *ReadQuotedString(LuaLexer &lexer, char quote)
{
    if (lexer.GetChar() != quote)
    {
        return nullptr;
    }

    auto start = lexer.position;
    lexer.Advance(1);

    while (!lexer.TheEnd())
    {
        auto byte = lexer.ReadChar();

        if (byte == '\\')
        {
            auto byte = lexer.ReadChar();

            if (byte == 'z' && lexer.GetChar() != quote)
            {
                // TODO: space is lost
                if (lexer.ReadSpace() == nullptr)
                {
                    return nullptr;
                }
            }
        }
        else if (byte == '\n')
        {
            throw LexerException("expected quote to end", start, lexer.position);
        }
        else if (byte == quote)
        {
            return new Token(TokenType::String);
        }
    }

    throw LexerException("expected quote to end: reached end of file", start, lexer.position - 1);
}

Token *LuaLexer::ReadSingleQuotedString()
{
    return ReadQuotedString(*this, '\'');
}

Token *LuaLexer::ReadDoubleQuotedString()
{
    return ReadQuotedString(*this, '"');
}

Token *LuaLexer::ReadNonWhitespaceToken()
{
    if (auto res = ReadAnalyzerDebugCode())
    {
        return res;
    }
    if (auto res = ReadParserDebugCode())
    {
        return res;
    }
    if (auto res = ReadNumber())
    {
        return res;
    }
    if (auto res = ReadMultilineString())
    {
        return res;
    }
    if (auto res = ReadSingleQuotedString())
    {
        return res;
    }
    if (auto res = ReadDoubleQuotedString())
    {
        return res;
    }
    if (auto res = ReadLetter())
    {
        return res;
    }
    if (auto res = ReadSymbol())
    {
        return res;
    }
    return nullptr;
}

Token *LuaLexer::ReadWhitespaceToken()
{
    if (auto res = ReadSpace())
    {
        return res;
    }
    if (auto res = ReadCommentEscape())
    {
        return res;
    }
    if (auto res = ReadMultilineCComment())
    {
        return res;
    }
    if (auto res = ReadLineCComment())
    {
        return res;
    }
    if (auto res = ReadMultilineComment())
    {
        return res;
    }
    if (auto res = ReadLineComment())
    {
        return res;
    }

    return nullptr;
}
