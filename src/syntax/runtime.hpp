#pragma once
#include "./base.hpp"

class RuntimeSyntax : public BaseSyntax
{
public:
    RuntimeSyntax()
    {
        AddNumberAnnotation({"ull",
                             "ll",
                             "ul",
                             "i"});
        AddPrefixOperatorTranslation({{"~", "bit.bnot(A)"}});
        AddPostfixOperatorTranslation({{"++", "A = A + 1"},
                                       {"ÆØÅ", "(A)"},
                                       {"ÆØÅÆ", "(A)"}});
        AddBinaryOperatorTranslation({
            {">>", "bit.rshift(A, B)"},
            {"<<", "bit.lshift(A, B)"},
            {"|", "bit.bor(A, B)"},
            {"&", "bit.band(A, B)"},
            {"//", "math.floor(A / B)"},
            {"~", "bit.bxor(A, B)"},
        });

        AddKeywordValues({"...",
                          "nil",
                          "true",
                          "false"});
        AddSymbolCharacters({",",
                             ";",
                             "(",
                             ")",
                             "{",
                             "}",
                             "[",
                             "]",
                             "=",
                             "::",
                             "\"",
                             "'",
                             "<|",
                             "|>"});
        AddNonStandardKeywords({"continue",
                                "import",
                                "literal",
                                "mutable"});
        AddKeywords({"do",
                     "end",
                     "if",
                     "then",
                     "else",
                     "elseif",
                     "for",
                     "in",
                     "while",
                     "repeat",
                     "until",
                     "break",
                     "return",
                     "local",
                     "function",
                     "and",
                     "not",
                     "or",
                     "ÆØÅ",
                     "ÆØÅÆ"});
        AddPrefixOperators({"-",
                            "#",
                            "not",
                            "!",
                            "~",
                            "supertype"});
        AddPrimaryBinaryOperators({".",
                                   ":"});
        AddPostfixOperators({"++",
                             "ÆØÅ",
                             "ÆØÅÆ"});
        AddBinaryOperators({{"or", "||"},
                            {"and", "&&"},
                            {"<", ">", "<=", ">=", "~=", "==", "!="},
                            {"|"},
                            {"~"},
                            {"&"},
                            {"<<", ">>"},
                            {"R.."},
                            {"+", "-"},
                            {"*", "/", "/idiv/", "%"},
                            {"R^"}});
    }
};
