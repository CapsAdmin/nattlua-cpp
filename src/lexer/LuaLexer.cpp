#include "./LuaLexer.hpp"
#include "../syntax/CharacterClasses.hpp"
#include <sstream>

std::unique_ptr<Token> LuaLexer::ReadSpace()
{
    if (!IsSpace(GetByte()))
        return nullptr;

    while (!TheEnd())
    {
        position += 1;

        if (!IsSpace(GetByte()))
            break;
    }

    return std::make_unique<Token>(Token::Kind::Space);
}

std::unique_ptr<Token> LuaLexer::ReadLetter()
{
    if (!IsLetter(GetByte()))
        return nullptr;

    while (!TheEnd())
    {
        position += 1;

        if (!IsDuringLetter(GetByte()))
            break;
    }

    return std::make_unique<Token>(Token::Kind::Letter);
}

std::unique_ptr<Token> LuaLexer::ReadSymbol()
{
    if (!ReadFirstFromStringArray(runtime_syntax->GetSymbols()) && !ReadFirstFromStringArray(typesystem_syntax->GetSymbols()))
        return nullptr;

    return std::make_unique<Token>(Token::Kind::Symbol);
}

std::unique_ptr<Token> LuaLexer::ReadMultilineCComment()
{
    if (!IsString("/*"))
        return nullptr;

    auto start = position;
    position += 2;

    while (!TheEnd())
    {
        if (IsString("*/"))
        {
            position += 2;
            return std::make_unique<Token>(Token::Kind::MultilineComment);
        }

        position += 1;
    }

    throw BaseLexer::Exception("expected multiline C comment to end, reached end of code", start, position);
}

std::unique_ptr<Token> LuaLexer::ReadLineComment()
{
    if (!IsString("--"))
        return nullptr;

    position += 2;

    while (!TheEnd())
    {
        if (IsString("\n"))
            break;

        position += 1;
    }

    return std::make_unique<Token>(Token::Kind::LineComment);
}

std::unique_ptr<Token> LuaLexer::ReadLineCComment()
{
    if (!IsString("//"))
        return nullptr;

    position += 2;

    while (!TheEnd())
    {
        if (IsString("\n"))
            break;

        position += 1;
    }

    return std::make_unique<Token>(Token::Kind::LineComment);
}

std::string repeat(const std::string &input, size_t num)
{
    std::ostringstream os;
    std::fill_n(std::ostream_iterator<std::string>(os), num, input);
    return os.str();
}

std::unique_ptr<Token> LuaLexer::ReadMultilineComment()
{
    if (!IsString("--[") || (!IsString("[", 3) && !IsString("=", 3)))
        return nullptr;

    auto start = position;

    // skip the --[
    position += 3;

    // skip all the =
    while (IsString("="))
        position += 1;

    // if we have an incomplete multiline comment, it's just a single line comment
    if (!IsString("["))
    {
        position = start;
        return ReadLineComment();
    }

    // skip the last [
    position += 1;
    auto pos = position;

    auto closing = std::string("]" + repeat("=", pos - start - 4) + "]");
    auto pos2 = FindNearest(closing);

    if (pos2.has_value())
    {
        position = pos2.value() + closing.size();
        return std::make_unique<Token>(Token::Kind::MultilineComment);
    }

    position = start + 2;

    throw BaseLexer::Exception("expected multiline comment to end, reached end of code", start, start + 1);
}

std::unique_ptr<Token> LuaLexer::ReadAnalyzerDebugCode()
{
    if (!IsString("??"))
        return nullptr;

    position += 2;

    while (!TheEnd())
    {
        if (IsString("\n"))
            break;

        position += 1;
    }

    return std::make_unique<Token>(Token::Kind::AnalyzerDebugCode);
}

std::unique_ptr<Token> LuaLexer::ReadParserDebugCode()
{
    if (!IsString("??"))
        return nullptr;

    position += 2;

    while (!TheEnd())
    {
        if (IsString("\n"))
            break;

        position += 1;
    }

    return std::make_unique<Token>(Token::Kind::ParserDebugCode);
}

bool ReadNumberExponent(LuaLexer &lexer, std::string_view what)
{
    // skip the 'e', 'E', 'p' or 'P'
    lexer.position += 1;

    if (!lexer.IsString("+") && !lexer.IsString("-"))
        throw BaseLexer::Exception("expected + or - after " + std::string(what) + ", got " + std::string(lexer.GetRelativeStringSlice(0, 1)), lexer.position - 1, lexer.position);

    // skip the '+' or '-'
    lexer.position += 1;

    if (!IsNumber(lexer.GetByte()))
        throw BaseLexer::Exception("malformed '" + std::string(what) + "' expected number, got " + std::string(lexer.GetRelativeStringSlice(0, 1)), lexer.position - 2, lexer.position - 1);

    while (!lexer.TheEnd())
    {
        if (!IsNumber(lexer.GetByte()))
            break;

        lexer.position += 1;
    }

    return true;
}

std::unique_ptr<Token> LuaLexer::ReadHexNumber()
{
    if (!IsString("0") || (!IsString("x", 1) && !IsString("X", 1)))
        return nullptr;

    // skip past 0x
    position += 2;

    while (!TheEnd())
    {
        if (IsString("_"))
            position += 1;

        if (IsString(".") && !IsString(".", 1))
            position += 1;

        if (IsValidHex(GetByte()))
        {
            position += 1;
        }
        else
        {
            // a number is ok to end with space or symbol
            if (IsSpace(GetByte()) || IsSymbol(GetByte()))
                break;

            if (IsString("p") || IsString("P"))
            {
                if (ReadNumberExponent(*this, "pow"))
                    break;
            }

            throw BaseLexer::Exception("malformed hex number, got " + std::string(GetRelativeStringSlice(0, 1)), position - 1, position);
        }
    }

    ReadFirstFromStringArray(runtime_syntax->GetNumberAnnotations());

    return std::make_unique<Token>(Token::Kind::Number);
}

std::unique_ptr<Token> LuaLexer::ReadBinaryNumber()
{
    if (!IsString("0") || (!IsString("b", 1) && !IsString("B", 1)))
        return nullptr;

    // skip past 0b
    position += 2;

    while (!TheEnd())
    {
        if (IsString("_"))
            position += 1;

        if (IsString("1") || IsString("0"))
        {
            position += 1;
        }
        else
        {
            if (IsSpace(GetByte()) || IsSymbol(GetByte()))
                break;

            if (IsString("e") || IsString("E"))
            {
                if (ReadNumberExponent(*this, "exponent"))
                    break;
            }

            throw BaseLexer::Exception("malformed binary number, got " + std::string(GetRelativeStringSlice(0, 1)), position - 1, position);
        }
    }

    ReadFirstFromStringArray(runtime_syntax->GetNumberAnnotations());

    return std::make_unique<Token>(Token::Kind::Number);
}

std::unique_ptr<Token> LuaLexer::ReadDecimalNumber()
{
    if (!IsNumber(GetByte()) && (!IsString(".") || !IsNumber(GetByte(1))))
        return nullptr;

    // if we start with a dot
    // .0
    auto has_dot = false;
    if (IsString("."))
    {
        has_dot = true;
        position += 1;
    }

    while (!TheEnd())
    {
        if (IsString("_"))
            position += 1;

        if (!has_dot && IsString("."))
        {
            // 22..66 would be a number range
            // so we have to return 22 only
            if (IsString(".", 1))
                break;

            has_dot = true;
            position += 1;
        }

        if (IsNumber(GetByte()))
        {
            position += 1;
        }
        else
        {
            if (IsSpace(GetByte()) || IsSymbol(GetByte()))
                break;

            if (IsString("e") || IsString("E"))
            {
                if (ReadNumberExponent(*this, "exponent"))
                    break;
            }

            throw BaseLexer::Exception("malformed decimal number, got " + std::string(GetRelativeStringSlice(0, 1)), position - 1, position);
        }
    }

    ReadFirstFromStringArray(runtime_syntax->GetNumberAnnotations());

    return std::make_unique<Token>(Token::Kind::Number);
}

std::unique_ptr<Token> LuaLexer::ReadMultilineString()
{
    if (!IsString("[", 0) || (!IsString("[", 1) && !IsString("=", 1)))
        return nullptr;

    auto start = position;
    position += 1;

    if (IsString("=", 0))
    {
        while (!TheEnd())
        {
            position += 1;
            if (!IsString("=", 0))
                break;
        }
    }

    if (!IsString("[", 0))
        throw BaseLexer::Exception("malformed multiline string: expected =, got " + std::string(GetRelativeStringSlice(0, 1)), start, position);

    position += 1;
    auto pos = position;

    auto closing = std::string("]" + repeat("=", pos - start - 2) + "]");
    auto pos2 = FindNearest(closing);

    if (pos2.has_value())
    {
        position = pos2.value() + closing.size();
        return std::make_unique<Token>(Token::Kind::String);
    }

    throw BaseLexer::Exception("expected multiline string to end, reached end of code", start, position);
}

std::unique_ptr<Token> ReadQuotedString(LuaLexer &lexer, const char quote)
{
    if (lexer.GetByte() != quote)
        return nullptr;

    auto const start = lexer.position;
    lexer.position += 1;

    while (!lexer.TheEnd())
    {
        auto const byte = lexer.ReadByte();

        if (byte == '\\' && lexer.IsString("z"))
        {
            // skip past \z
            lexer.position += 1;

            lexer.ReadSpace();
        }
        else if (byte == '\n')
        {
            throw BaseLexer::Exception("expected ending " + std::string(1, quote) + " quote, got newline", start, lexer.position);
        }
        else if (byte == quote)
        {
            return std::make_unique<Token>(Token::Kind::String);
        }
    }

    throw BaseLexer::Exception("expected ending " + std::string(1, quote) + " quote, reached end of code", start, lexer.position - 1);
}

std::unique_ptr<Token> LuaLexer::ReadSingleQuotedString()
{
    return ReadQuotedString(*this, '\'');
}

std::unique_ptr<Token> LuaLexer::ReadDoubleQuotedString()
{
    return ReadQuotedString(*this, '"');
}

std::unique_ptr<Token> LuaLexer::ReadNonWhitespaceToken()
{
    if (auto res = ReadAnalyzerDebugCode())
        return res;

    if (auto res = ReadParserDebugCode())
        return res;

    if (auto res = ReadHexNumber())
        return res;

    if (auto res = ReadBinaryNumber())
        return res;

    if (auto res = ReadDecimalNumber())
        return res;

    if (auto res = ReadMultilineString())
        return res;

    if (auto res = ReadSingleQuotedString())
        return res;

    if (auto res = ReadDoubleQuotedString())
        return res;

    if (auto res = ReadLetter())
        return res;

    if (auto res = ReadSymbol())
        return res;

    return nullptr;
}

std::unique_ptr<Token> LuaLexer::ReadWhitespaceToken()
{
    if (auto res = ReadRemainingCommentEscape())
        return res;

    if (auto res = ReadSpace())
        return res;

    if (auto res = ReadCommentEscape())
        return res;

    if (auto res = ReadMultilineCComment())
        return res;

    if (auto res = ReadLineCComment())
        return res;

    if (auto res = ReadMultilineComment())
        return res;

    if (auto res = ReadLineComment())
        return res;

    return nullptr;
}
