#pragma once
#include "./RuntimeSyntax.hpp"

class TypesystemSyntax : public RuntimeSyntax
{
public:
    TypesystemSyntax() : RuntimeSyntax()
    {
        AddPrefixOperators({"-",
                            "#",
                            "not",
                            "~",
                            "typeof",
                            "$",
                            "unique",
                            "mutable",
                            "literal",
                            "supertype",
                            "expand"});

        AddPrimaryBinaryOperators({".", ":"});

        AddBinaryOperators({{"or", "||"},
                            {"and", "&&"},
                            {"extends"},
                            {"subsetof"},
                            {"supersetof"},
                            {"<", ">", "<=", ">=", "~=", "=="},
                            {"|"},
                            {"~"},
                            {"&"},
                            {"<<", ">>"},
                            {"R.."},
                            {"+", "-"},
                            {"*", "/", "/idiv/", "%"},
                            {"R^"}});
    }
    ~TypesystemSyntax() {}
};