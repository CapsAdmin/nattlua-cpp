
#include "./BaseSyntax.hpp"

using namespace std;

void BaseSyntax::AddPrefixOperators(vector<string> vec)
{
    postfix_operators_lookup.insert(vec.begin(), vec.end());
    AddSymbols(vec);
}

void BaseSyntax::AddPostfixOperators(vector<string> vec)
{
    postfix_operators_lookup.insert(vec.begin(), vec.end());
    AddSymbols(vec);
}

void BaseSyntax::AddPrimaryBinaryOperators(vector<string> vec)
{
    primary_binary_operators_lookup.insert(vec.begin(), vec.end());
    AddSymbols(vec);
}

void BaseSyntax::AddBinaryOperators(vector<vector<string>> operators)
{
    uint8_t priority = 0;
    for (auto &op : operators)
    {

        for (auto &token : op)
        {
            if (token.starts_with("R"))
            {
                binary_operator_info.insert(make_pair(token, BinaryOperatorInfo{
                    left_priority : static_cast<uint8_t>(priority + 1),
                    right_priority : priority,
                }));
            }
            else
            {
                binary_operator_info.insert(make_pair(token, BinaryOperatorInfo{
                    left_priority : priority,
                    right_priority : priority,
                }));
            }
        }
        priority++;
    }
}

void BaseSyntax::AddKeywords(vector<string> vec)
{
    keyword_lookup.insert(vec.begin(), vec.end());
    AddSymbols(vec);
}

void BaseSyntax::AddNonStandardKeywords(vector<string> vec)
{
    non_standard_keyword_lookup.insert(vec.begin(), vec.end());
    AddSymbols(vec);
}

void BaseSyntax::AddKeywordValues(vector<string> vec)
{
    keyword_values_lookup.insert(vec.begin(), vec.end());
    AddSymbols(vec);
}

void BaseSyntax::AddSymbolCharacters(vector<string> vec)
{
    AddSymbols(vec);
}

void BaseSyntax::AddBinaryOperatorTranslation(map<string, string> map)
{
    auto const pattern = std::regex("(.*)A(.*)B(.*)");

    for (auto const &[key, val] : map)
    {
        smatch matches;
        if (std::regex_search(val, matches, pattern))
        {

            auto left = matches[1].str();
            auto mid = matches[2].str();
            auto right = matches[3].str();

            lookup.insert(make_pair(key, vector<string>{" " + left, mid, " " + right}));
        };
    }
}

void BaseSyntax::AddPrefixOperatorTranslation(map<string, string> map)
{
    auto const pattern = std::regex("(.*)A(.*)");

    for (auto const &[key, val] : map)
    {
        smatch matches;
        if (std::regex_search(val, matches, pattern))
        {

            auto left = matches[1].str();
            auto right = matches[2].str();

            lookup.insert(make_pair(key, vector<string>{" " + left, right}));
        };
    }
}

void BaseSyntax::AddPostfixOperatorTranslation(map<string, string> map)
{
    auto const pattern = std::regex("(.*)A(.*)");

    for (auto const &[key, val] : map)
    {
        smatch matches;
        if (std::regex_search(val, matches, pattern))
        {

            auto left = matches[1].str();
            auto right = matches[2].str();

            lookup.insert(make_pair(key, vector<string>{left, " " + right}));
        };
    }
}

void BaseSyntax::AddNumberAnnotation(vector<string> vec)
{
    number_annotations.insert(number_annotations.end(), vec.begin(), vec.end());
}

void BaseSyntax::AddSymbols(vector<string> strings)
{
    auto pattern = std::regex("[^\\p{L}\\d\\s@#]");

    for (auto &str : strings)
    {
        if (std::regex_match(str, pattern))
        {
            symbols.push_back(str);
        }
    }

    sort(symbols.begin(), symbols.end(), [](const string &a, const string &b)
         { return a.size() > b.size(); });
}

void BaseSyntax::AddBinarySymbols(vector<string> strings)
{
    for (auto &str : strings)
    {
        binary_operator_info[str] = BinaryOperatorInfo();
    }
}
