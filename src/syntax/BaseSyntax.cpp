
#include "./BaseSyntax.hpp"
#include "./CharacterClasses.hpp"

void BaseSyntax::AddPrefixOperators(std::vector<std::string> vec)
{
    prefix_operators_lookup.insert(vec.begin(), vec.end());
    AddSymbols(vec);
}

void BaseSyntax::AddPostfixOperators(std::vector<std::string> vec)
{
    postfix_operators_lookup.insert(vec.begin(), vec.end());
    AddSymbols(vec);
}

void BaseSyntax::AddPrimaryBinaryOperators(std::vector<std::string> vec)
{
    primary_binary_operators_lookup.insert(vec.begin(), vec.end());
    AddSymbols(vec);
}

void BaseSyntax::AddBinaryOperators(std::vector<std::vector<std::string>> groups)
{
    uint8_t priority = 0;
    for (auto &operators : groups)
    {
        for (auto &op : operators)
        {
            if (op.starts_with("R"))
            {
                std::string op_ = std::string(op).substr(1);

                binary_operator_info.insert(make_pair(op_, BinaryOperatorInfo{
                    left_priority : static_cast<uint8_t>(priority + 1),
                    right_priority : priority,
                }));

                AddSymbols({op_});
            }
            else
            {
                binary_operator_info.insert(make_pair(op, BinaryOperatorInfo{
                    left_priority : priority,
                    right_priority : priority,
                }));

                AddSymbols({op});
            }
        }
        priority++;
    }
}

void BaseSyntax::AddKeywords(std::vector<std::string> vec)
{
    keyword_lookup.insert(vec.begin(), vec.end());
    AddSymbols(vec);
}

void BaseSyntax::AddNonStandardKeywords(std::vector<std::string> vec)
{
    non_standard_keyword_lookup.insert(vec.begin(), vec.end());
    AddSymbols(vec);
}

void BaseSyntax::AddKeywordValues(std::vector<std::string> vec)
{
    keyword_values_lookup.insert(vec.begin(), vec.end());
    AddSymbols(vec);
}

void BaseSyntax::AddSymbolCharacters(const std::vector<std::string> vec)
{
    AddSymbols(vec);
}

void BaseSyntax::AddBinaryOperatorTranslation(std::map<std::string, std::string> map)
{
    auto const pattern = std::regex("(.*)A(.*)B(.*)");

    for (auto const &[key, val] : map)
    {
        std::smatch matches;
        if (std::regex_search(val, matches, pattern))
        {

            auto left = matches[1].str();
            auto mid = matches[2].str();
            auto right = matches[3].str();

            translation_lookup.insert(make_pair(key, std::vector<std::string>{" " + left, mid, " " + right}));
        };
    }
}

void BaseSyntax::AddPrefixOperatorTranslation(std::map<std::string, std::string> map)
{
    auto const pattern = std::regex("(.*)A(.*)");

    for (auto const &[key, val] : map)
    {
        std::smatch matches;
        if (std::regex_search(val, matches, pattern))
        {

            auto left = matches[1].str();
            auto right = matches[2].str();

            translation_lookup.insert(make_pair(key, std::vector<std::string>{" " + left, right}));
        };
    }
}

void BaseSyntax::AddPostfixOperatorTranslation(std::map<std::string, std::string> map)
{
    auto const pattern = std::regex("(.*)A(.*)");

    for (auto const &[key, val] : map)
    {
        std::smatch matches;
        if (std::regex_search(val, matches, pattern))
        {

            auto left = matches[1].str();
            auto right = matches[2].str();

            translation_lookup.insert(make_pair(key, std::vector<std::string>{left, " " + right}));
        };
    }
}

void BaseSyntax::AddNumberAnnotation(std::vector<std::string> vec)
{
    number_annotations.insert(number_annotations.end(), vec.begin(), vec.end());
}

void BaseSyntax::AddSymbols(const std::vector<std::string> strings)
{
    for (auto &str : strings)
    {
        for (auto c : str)
        {
            if (IsSymbol(c))
            {
                symbols.push_back(str);
                break;
            }
        }
    }

    sort(symbols.begin(), symbols.end(), [](const std::string &a, const std::string &b)
         { return a.size() > b.size(); });
}

void BaseSyntax::AddBinarySymbols(const std::vector<std::string> strings)
{
    for (auto &str : strings)
    {
        binary_operator_info[str] = BinaryOperatorInfo();
    }
}
