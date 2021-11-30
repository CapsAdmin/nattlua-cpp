#pragma once

class ParserNode
{
public:
    enum Environment
    {
        Runtime,
        Typesystem,
    };
    ParserNode *parent = nullptr;
    Environment environment = Runtime;
};
