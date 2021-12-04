#include "../parser/LuaParser.hpp"
#include "../parser/PrimaryExpression.hpp"
#include <string>

class BaseType
{
    virtual void lol(){};
};

class Symbol : public BaseType
{
public:
    enum LuaType
    {
        Nil,
        True,
        False,
    };

    size_t value;

    Symbol(size_t value)
    {
        this->value = value;
    }
};

class Number : public BaseType
{
public:
    double value;

    Number(double value)
    {
        this->value = value;
    }
};

class String : public BaseType
{
public:
    std::string value;

    String(std::string value)
    {
        this->value = value;
    }
};

std::shared_ptr<Number> LNumberFromString(std::string str)
{
    std::string lower;
    lower.resize(str.size());

    std::transform(str.begin(), str.end(), lower.begin(), ::tolower);

    auto num = 0.0;

    if (lower.substr(0, 2) == "0b")
    {
        num = std::stoi(lower.substr(2), nullptr, 2);
    }
    else if (lower.substr(0, 2) == "0x")
    {
        num = std::stoi(lower.substr(2), nullptr, 16);
    }
    else if (lower.substr(lower.length() - 2) == "ull")
    {
        num = std::stoi(lower.substr(0, lower.length() - 3), nullptr, 10);
    }
    else if (lower.substr(lower.length() - 2) == "ll")
    {
        num = std::stoi(lower.substr(0, lower.length() - 2), nullptr, 10);
    }
    else if (str == "nan")
    {
        num = std::numeric_limits<double>::quiet_NaN();
    }
    else if (str == "inf")
    {
        num = std::numeric_limits<double>::infinity();
    }
    else if (str == "-inf")
    {
        num = -std::numeric_limits<double>::infinity();
    }
    else
    {
        num = std::stod(str);
    }

    return std::make_shared<Number>(num);
}

std::shared_ptr<String> LStringFromString(std::string str)
{
    if (str.substr(0, 1) == "[")
    {
        auto start = str.find("[=*[", 0);
        if (start != std::string::npos)
        {
            return std::make_shared<String>(str.substr(start + 4, str.length() - start - 5));
        }
        else
        {
            return std::make_shared<String>("");
        }
    }
    else
    {
        return std::make_shared<String>(str.substr(1, str.length() - 2));
    }

    throw std::runtime_error("cannot convert string: " + str);
}

class LuaAnalyzer
{
public:
    std::shared_ptr<BaseType> AnalyzeExpression(std::shared_ptr<Expression> node, ParserNode::Environment env);
};

std::shared_ptr<BaseType> LuaAnalyzer::AnalyzeExpression(std::shared_ptr<Expression> node, ParserNode::Environment env)
{
    if (auto value = dynamic_cast<ValueExpression *>(node.get()))
    {
        if (auto atomic = dynamic_cast<Atomic *>(value))
        {
            auto typ = atomic->token_type;
            if (typ == LuaParser::TokenType::Keyword)
            {
                if (atomic->value->value == "nil")
                {
                    return std::make_shared<Symbol>(Symbol::LuaType::Nil);
                }
                else if (atomic->value->value == "true")
                {
                    return std::make_shared<Symbol>(Symbol::LuaType::True);
                }
                else if (atomic->value->value == "false")
                {
                    return std::make_shared<Symbol>(Symbol::LuaType::False);
                }
            }

            if (typ == LuaParser::TokenType::Number)
            {
                return LNumberFromString(std::string(atomic->value->value));
            }
            else if (typ == LuaParser::TokenType::String)
            {
                return LStringFromString(std::string(atomic->value->value));
            }
        }
    }

    return nullptr;
}