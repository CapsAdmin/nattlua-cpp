#include "./PrimaryExpression.hpp"

Atomic *Atomic::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->IsTokenValue(parser->GetToken()))
        return nullptr;

    auto node = new Atomic();
    parser->StartNode(node);
    node->value = parser->ReadToken();
    parser->EndNode(node);
    return node;
};

Table::IdentifierKeyValue *Table::IdentifierKeyValue::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->IsType(Token::Kind::Letter) || !parser->IsValue("=", 1))
        return nullptr;

    auto node = new IdentifierKeyValue();
    parser->StartNode(node);
    node->key = parser->ExpectType(Token::Kind::Letter);
    node->tk_equal = parser->ExpectValue("=");
    node->val = ValueExpression::Parse(parser);
    parser->EndNode(node);
    return node;
}

Table::ExpressionKeyValue *Table::ExpressionKeyValue::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->IsValue("["))
        return nullptr;

    auto node = new ExpressionKeyValue();
    parser->StartNode(node);
    node->tk_left_bracket = parser->ExpectValue("[");
    node->key = ValueExpression::Parse(parser);
    node->tk_right_bracket = parser->ExpectValue("]");
    node->tk_equal = parser->ExpectValue("=");
    node->val = ValueExpression::Parse(parser);
    parser->EndNode(node);

    return node;
};

Table::IndexValue *Table::IndexValue::Parse(std::shared_ptr<LuaParser> parser)
{
    auto node = new IndexValue();
    parser->StartNode(node);
    node->val = ValueExpression::Parse(parser);
    parser->EndNode(node);

    return node;
}

Table *Table::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->IsValue("{"))
        return nullptr;

    auto tree = new Table();
    parser->StartNode(tree);

    tree->tk_left_bracket = parser->ExpectValue("{");

    size_t index = 0;

    while (true)
    {
        if (parser->IsValue("}"))
            break;

        Child *child = nullptr;

        if (auto res = ExpressionKeyValue::Parse(parser))
        {
            child = res;
        }
        else if (auto res = IdentifierKeyValue::Parse(parser))
        {
            child = res;
        }
        else if (auto res = IndexValue::Parse(parser))
        {
            res->key = index;
            child = res;
        }

        tree->children.push_back(child);

        if (!parser->IsValue(",") && !parser->IsValue(";") && !parser->IsValue("}"))
        {
            throw LuaParser::Exception("Expected something", parser->GetToken(), parser->GetToken());
        }

        if (!parser->IsValue("}"))
            tree->tk_separators.push_back(parser->ExpectValue(","));

        index++;
    }

    tree->tk_right_bracket = parser->ExpectValue("}");

    parser->EndNode(tree);

    return tree;
};

PrefixOperator *PrefixOperator::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->runtime_syntax->IsPrefixOperator(parser->GetToken()->value))
        return nullptr;

    auto node = new PrefixOperator();
    parser->StartNode(node);
    node->op = parser->ReadToken();
    node->right = ValueExpression::Parse(parser);
    parser->EndNode(node);
    return node;
};

BinaryOperator *BinaryOperator::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->runtime_syntax->IsBinaryOperator(parser->GetToken()->value))
        return nullptr;

    auto node = new BinaryOperator();
    parser->StartNode(node);
    node->left = ValueExpression::Parse(parser);
    node->op = parser->ReadToken();
    node->right = ValueExpression::Parse(parser);
    parser->EndNode(node);
    return node;
};

Expression *ValueExpression::Parse(std::shared_ptr<LuaParser> parser, size_t priority)
{
    if (parser->IsValue("("))
    {
        auto left_paren = parser->ExpectValue("(");
        auto node = ValueExpression::Parse(parser);
        auto right_paren = parser->ExpectValue(")");

        if (!node)
        {
            throw LuaParser::Exception("Empty parentheses group", parser->GetToken(), parser->GetToken());
        }

        node->tk_left_parenthesis.push_back(std::move(left_paren)); // TODO: unshift
        node->tk_right_parenthesis.push_back(std::move(right_paren));

        return node;
    }

    Expression *node = nullptr;

    if (auto res = Atomic::Parse(parser))
    {
        node = res;
    }
    else if (auto res = Table::Parse(parser))
    {
        node = res;
    }

    while (node)
    {
        PostfixExpression *sub = nullptr;

        if (auto res = Index::Parse(parser))
        {
            sub = res;
        }
        else if (auto res = SelfCall::Parse(parser))
        {
            sub = res;
        }
        else if (auto res = Call::Parse(parser))
        {
            sub = res;
        }
        else if (auto res = PostfixOperator::Parse(parser))
        {
            sub = res;
        }
        else if (auto res = IndexExpression::Parse(parser))
        {
            sub = res;
        }
        else if (auto res = TypeCast::Parse(parser))
        {
            sub = res;
        }

        if (!sub)
        {
            break;
        }

        sub->left = node;
        node = sub;
    }

    // check integer

    while (node)
    {
        auto info = parser->runtime_syntax->GetBinaryOperatorInfo(parser->GetToken()->value);
        if (!info || info->left_priority < priority)
            break;

        auto left_node = node;

        auto binary = new BinaryOperator();
        parser->StartNode(binary);
        binary->op = parser->ReadToken();
        binary->left = left_node;
        binary->left->parent = node;

        binary->right = ValueExpression::Parse(parser, info->right_priority);

        if (!binary->right)
        {
            auto token = parser->GetToken();
            throw LuaParser::Exception(
                "expected right side to be an expression, got $1",
                token,
                token);
        }

        parser->EndNode(binary);

        node = binary;
    }

    return node;
}

ValueExpression::Index *ValueExpression::Index::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->IsValue(".") || !parser->IsType(Token::Kind::Letter, 1))
        return nullptr;

    auto *node = new Index();
    parser->StartNode(node);
    node->op = parser->ReadToken();
    node->right = Atomic::Parse(parser);
    parser->EndNode(node);

    return node;
}

ValueExpression::SelfCall *ValueExpression::SelfCall::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!(parser->IsValue(":") && parser->IsType(Token::Kind::Letter, 1) && parser->IsCallExpression(2)))
        return nullptr;

    auto *node = new SelfCall();
    parser->StartNode(node);
    node->op = parser->ReadToken();

    node->right = Atomic::Parse(parser);

    parser->EndNode(node);

    return node;
}

ValueExpression::Call *ValueExpression::Call::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->IsCallExpression(0))
        return nullptr;

    auto node = new Call();
    parser->StartNode(node);

    if (parser->IsValue("{"))
    {
        node->arguments.push_back(Table::Parse(parser));
    }
    else if (parser->IsType(Token::Kind::String))
    {
        node->arguments.push_back(Atomic::Parse(parser));
    }
    else if (parser->IsValue("("))
    {
        node->tk_arguments_left = parser->ReadToken();
        auto values = parser->ReadMultipleValues<ValueExpression>(
            1000,
            [parser]()
            {
                return Atomic::Parse(parser);
            },
            node->tk_comma);

        node->arguments.insert(node->arguments.end(), values.begin(), values.end());

        node->tk_arguments_right = parser->ReadToken();
    }

    return node;
}

ValueExpression::PostfixOperator *ValueExpression::PostfixOperator::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->runtime_syntax->IsPostfixOperator(parser->GetToken()->value))
        return nullptr;

    auto node = new PostfixOperator();
    parser->StartNode(node);
    node->op = parser->ReadToken();
    parser->EndNode(node);

    return node;
}

ValueExpression::IndexExpression *ValueExpression::IndexExpression::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->IsValue("["))
        return nullptr;

    auto *node = new IndexExpression();
    parser->StartNode(node);

    node->tk_left_bracket = parser->ReadToken();
    node->index = ValueExpression::Parse(parser);
    node->tk_right_bracket = parser->ReadToken();

    parser->EndNode(node);

    return node;
}

ValueExpression::TypeCast *ValueExpression::TypeCast::Parse(std::shared_ptr<LuaParser> parser)
{
    if ((!parser->IsValue(":") || (parser->IsType(Token::Kind::Letter, 1) || parser->IsCallExpression(2))) && !parser->IsValue("as"))
    {
        return nullptr;
    }

    auto *node = new TypeCast();
    parser->StartNode(node);
    node->tk_operator = parser->ReadToken(); // either as or :
    node->expression = ValueExpression::Parse(parser);
    parser->EndNode(node);

    return node;
}