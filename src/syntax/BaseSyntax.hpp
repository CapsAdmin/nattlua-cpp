#pragma once

#include <vector>
#include <map>
#include <set>
#include <regex>

using namespace std;

struct BinaryOperatorInfo
{
    uint8_t left_priority;
    uint8_t right_priority;
};

class BaseSyntax
{
public:
    vector<string> symbols;
    vector<string> number_annotations;
    map<string, vector<string>> lookup;
    map<string, BinaryOperatorInfo> binary_operator_info;
    set<string> primary_binary_operators_lookup;
    set<string> postfix_operators_lookup;
    set<string> keyword_values_lookup;
    set<string> keyword_lookup;
    set<string> non_standard_keyword_lookup;

    void AddPrefixOperators(vector<string> vec);
    void AddPostfixOperators(vector<string> vec);
    void AddPrimaryBinaryOperators(vector<string> vec);
    void AddBinaryOperators(vector<vector<string>> operators);
    void AddKeywords(vector<string> vec);
    void AddNonStandardKeywords(vector<string> vec);
    void AddKeywordValues(vector<string> vec);
    void AddSymbolCharacters(vector<string> vec);
    void AddBinaryOperatorTranslation(map<string, string> map);
    void AddPrefixOperatorTranslation(map<string, string> map);
    void AddPostfixOperatorTranslation(map<string, string> map);
    void AddNumberAnnotation(vector<string> vec);

private:
    void AddSymbols(vector<string> strings);
    void AddBinarySymbols(vector<string> strings);
};
