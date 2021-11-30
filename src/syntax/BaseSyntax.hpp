#pragma once

#include <vector>
#include <map>
#include <set>
#include <regex>

struct BinaryOperatorInfo
{
    uint8_t left_priority;
    uint8_t right_priority;
};

class BaseSyntax
{

public:
    void AddPrefixOperators(std::vector<std::string> vec);
    void AddPostfixOperators(std::vector<std::string> vec);
    void AddPrimaryBinaryOperators(std::vector<std::string> vec);
    void AddBinaryOperators(std::vector<std::vector<std::string>> operators);
    void AddKeywords(std::vector<std::string> vec);
    void AddNonStandardKeywords(std::vector<std::string> vec);
    void AddKeywordValues(std::vector<std::string> vec);
    void AddSymbolCharacters(std::vector<std::string> vec);
    void AddBinaryOperatorTranslation(std::map<std::string, std::string> map);
    void AddPrefixOperatorTranslation(std::map<std::string, std::string> map);
    void AddPostfixOperatorTranslation(std::map<std::string, std::string> map);
    void AddNumberAnnotation(std::vector<std::string> vec);

    auto GetSymbols() { return symbols; }
    auto GetNumberAnnotations() { return number_annotations; }
    bool IsPrefixOperator(const std::string_view &op)
    {
        return prefix_operators_lookup.contains(std::string(op));
    }
    bool IsPostfixOperator(const std::string_view &op)
    {
        return postfix_operators_lookup.contains(std::string(op));
    }
    bool IsBinaryOperator(const std::string_view &op)
    {
        return primary_binary_operators_lookup.contains(std::string(op));
    }
    bool IsKeyword(const std::string_view &kw)
    {
        return keyword_lookup.contains(std::string(kw));
    }
    bool IsKeywordValue(const std::string_view &kw)
    {
        return keyword_values_lookup.contains(std::string(kw));
    }

private:
    std::vector<std::string> symbols;
    std::vector<std::string> number_annotations;
    std::map<std::string, std::vector<std::string>> translation_lookup;
    std::map<std::string, BinaryOperatorInfo> binary_operator_info;
    std::set<std::string> primary_binary_operators_lookup;
    std::set<std::string> postfix_operators_lookup;
    std::set<std::string> prefix_operators_lookup;
    std::set<std::string> keyword_values_lookup;
    std::set<std::string> keyword_lookup;
    std::set<std::string> non_standard_keyword_lookup;

    void AddSymbols(std::vector<std::string> strings);
    void AddBinarySymbols(std::vector<std::string> strings);
};
