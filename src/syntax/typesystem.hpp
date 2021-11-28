#pragma once
#include "./runtime.hpp"

class TypesystemSyntax : public RuntimeSyntax
{
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
};