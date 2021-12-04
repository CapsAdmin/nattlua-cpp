#include "./PrimaryExpression.hpp"

std::unique_ptr<Atomic> Atomic::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->IsTokenValue(parser->PeekToken()))
        return nullptr;

    auto node = std::make_unique<Atomic>();
    parser->StartNode(node.get());
    node->value = parser->ReadToken();
    parser->EndNode(node.get());
    return node;
};

std::unique_ptr<Table::IdentifierKeyValue> Table::IdentifierKeyValue::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->IsType(Token::Kind::Letter) || !parser->IsValue("=", 1))
        return nullptr;

    auto node = std::make_unique<IdentifierKeyValue>();
    parser->StartNode(node.get());
    node->key = parser->ExpectType(Token::Kind::Letter);
    node->tk_equal = parser->ExpectValue("=");
    node->val = ValueExpression::Parse(parser);
    parser->EndNode(node.get());
    return node;
}

std::unique_ptr<Table::ExpressionKeyValue> Table::ExpressionKeyValue::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->IsValue("["))
        return nullptr;

    auto node = std::make_unique<ExpressionKeyValue>();
    parser->StartNode(node.get());
    node->tk_left_bracket = parser->ExpectValue("[");
    node->key = ValueExpression::Parse(parser);
    node->tk_right_bracket = parser->ExpectValue("]");
    node->tk_equal = parser->ExpectValue("=");
    node->val = ValueExpression::Parse(parser);
    parser->EndNode(node.get());

    return node;
};

std::unique_ptr<Table::IndexValue> Table::IndexValue::Parse(std::shared_ptr<LuaParser> parser)
{
    auto node = std::make_unique<IndexValue>();
    parser->StartNode(node.get());
    node->val = ValueExpression::Parse(parser);
    parser->EndNode(node.get());

    return node;
}

std::unique_ptr<Table> Table::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->IsValue("{"))
        return nullptr;

    auto node = std::make_unique<Table>();
    parser->StartNode(node.get());

    node->tk_left_bracket = parser->ExpectValue("{");

    size_t index = 0;

    while (true)
    {
        if (parser->IsValue("}"))
            break;

        std::unique_ptr<Child> child = nullptr;

        if (auto res = ExpressionKeyValue::Parse(parser))
        {
            child = std::move(res);
        }
        else if (auto res = IdentifierKeyValue::Parse(parser))
        {
            child = std::move(res);
        }
        else if (auto res = IndexValue::Parse(parser))
        {
            res->key = index;
            child = std::move(res);
        }

        node->children.push_back(std::move(child));

        if (!parser->IsValue(",") && !parser->IsValue(";") && !parser->IsValue("}"))
        {
            throw LuaParser::Exception("Expected something", parser->PeekToken(), parser->PeekToken());
        }

        if (!parser->IsValue("}"))
            node->tk_separators.push_back(parser->ExpectValue(","));

        index++;
    }

    node->tk_right_bracket = parser->ExpectValue("}");

    parser->EndNode(node.get());

    return node;
};

std::unique_ptr<PrefixOperator> PrefixOperator::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->runtime_syntax->IsPrefixOperator(parser->PeekToken()->value))
        return nullptr;

    auto node = std::make_unique<PrefixOperator>();
    parser->StartNode(node.get());
    node->op = parser->ReadToken();
    node->right = ValueExpression::Parse(parser);
    parser->EndNode(node.get());
    return node;
};

std::unique_ptr<BinaryOperator> BinaryOperator::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->runtime_syntax->IsBinaryOperator(parser->PeekToken()->value))
        return nullptr;

    auto node = std::make_unique<BinaryOperator>();
    parser->StartNode(node.get());
    node->left = ValueExpression::Parse(parser);
    node->op = parser->ReadToken();
    node->right = ValueExpression::Parse(parser);
    parser->EndNode(node.get());
    return node;
};

std::unique_ptr<Expression> ValueExpression::Parse(std::shared_ptr<LuaParser> parser, size_t priority)
{
    if (parser->IsValue("("))
    {
        auto left_paren = parser->ExpectValue("(");
        auto node = ValueExpression::Parse(parser);
        auto right_paren = parser->ExpectValue(")");

        if (!node)
        {
            throw LuaParser::Exception("Empty parentheses group", parser->PeekToken(), parser->PeekToken());
        }

        node->tk_left_parenthesis.push_back(std::move(left_paren)); // TODO: unshift
        node->tk_right_parenthesis.push_back(std::move(right_paren));

        return node;
    }

    std::unique_ptr<Expression> node = nullptr;

    if (auto res = Atomic::Parse(parser))
    {
        node = std::move(res);
    }
    else if (auto res = Table::Parse(parser))
    {
        node = std::move(res);
    }

    while (node)
    {
        std::unique_ptr<PostfixExpression> sub = nullptr;

        if (auto res = Index::Parse(parser))
        {
            sub = std::move(res);
        }
        else if (auto res = SelfCall::Parse(parser))
        {
            sub = std::move(res);
        }
        else if (auto res = Call::Parse(parser))
        {
            sub = std::move(res);
        }
        else if (auto res = PostfixOperator::Parse(parser))
        {
            sub = std::move(res);
        }
        else if (auto res = IndexExpression::Parse(parser))
        {
            sub = std::move(res);
        }
        else if (auto res = TypeCast::Parse(parser))
        {
            sub = std::move(res);
        }

        if (!sub)
        {
            break;
        }

        sub->left = std::move(node);
        node = std::move(sub);
    }

    // check integer

    while (node)
    {
        auto info = parser->runtime_syntax->GetBinaryOperatorInfo(parser->PeekToken()->value);
        if (!info || info->left_priority < priority)
            break;

        auto left_node = std::move(node);

        auto binary = std::make_unique<BinaryOperator>();
        parser->StartNode(binary.get());
        binary->op = parser->ReadToken();
        binary->left = std::move(left_node);
        binary->left->parent = node.get();

        binary->right = ValueExpression::Parse(parser, info->right_priority);

        if (!binary->right)
        {
            auto token = parser->PeekToken();
            throw LuaParser::Exception(
                "expected right side to be an expression, got $1",
                token,
                token);
        }

        parser->EndNode(binary.get());

        node = std::move(binary);
    }

    return node;
}

std::unique_ptr<ValueExpression::Index> ValueExpression::Index::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->IsValue(".") || !parser->IsType(Token::Kind::Letter, 1))
        return nullptr;

    auto node = std::make_unique<Index>();
    parser->StartNode(node.get());
    node->op = parser->ReadToken();
    node->right = Atomic::Parse(parser);
    parser->EndNode(node.get());

    return node;
}

std::unique_ptr<ValueExpression::SelfCall> ValueExpression::SelfCall::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!(parser->IsValue(":") && parser->IsType(Token::Kind::Letter, 1) && parser->IsCallExpression(2)))
        return nullptr;

    auto node = std::make_unique<SelfCall>();
    parser->StartNode(node.get());
    node->op = parser->ReadToken();

    node->right = Atomic::Parse(parser);

    parser->EndNode(node.get());

    return node;
}

std::unique_ptr<ValueExpression::Call> ValueExpression::Call::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->IsCallExpression(0))
        return nullptr;

    auto node = std::make_unique<Call>();
    parser->StartNode(node.get());

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

        for (size_t i = 0; i < 1000; i++)
        {
            auto value = Atomic::Parse(parser);
            if (!value)
                break;

            node->arguments.push_back(std::move(value));

            if (!parser->IsValue(","))
                break;

            node->tk_comma.push_back(parser->ExpectValue(","));
        }

        node->tk_arguments_right = parser->ReadToken();
    }

    return node;
}

std::unique_ptr<ValueExpression::PostfixOperator> ValueExpression::PostfixOperator::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->runtime_syntax->IsPostfixOperator(parser->PeekToken()->value))
        return nullptr;

    auto node = std::make_unique<PostfixOperator>();
    parser->StartNode(node.get());
    node->op = parser->ReadToken();
    parser->EndNode(node.get());

    return node;
}

std::unique_ptr<ValueExpression::IndexExpression> ValueExpression::IndexExpression::Parse(std::shared_ptr<LuaParser> parser)
{
    if (!parser->IsValue("["))
        return nullptr;

    auto node = std::make_unique<IndexExpression>();
    parser->StartNode(node.get());

    node->tk_left_bracket = parser->ReadToken();
    node->index = ValueExpression::Parse(parser);
    node->tk_right_bracket = parser->ReadToken();

    parser->EndNode(node.get());

    return node;
}

std::unique_ptr<ValueExpression::TypeCast> ValueExpression::TypeCast::Parse(std::shared_ptr<LuaParser> parser)
{
    if ((!parser->IsValue(":") || (parser->IsType(Token::Kind::Letter, 1) || parser->IsCallExpression(2))) && !parser->IsValue("as"))
    {
        return nullptr;
    }

    auto node = std::make_unique<TypeCast>();
    parser->StartNode(node.get());
    node->tk_operator = parser->ReadToken(); // either as or :
    node->expression = ValueExpression::Parse(parser);
    parser->EndNode(node.get());

    return node;
}