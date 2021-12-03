#include "./PrimaryExpression.hpp"

Value *Value::Parse(LuaParser *parser)
{
    if (!parser->IsTokenValue(parser->GetToken()))
        return nullptr;

    auto node = new Value();
    parser->StartNode(node);
    node->value = parser->ReadToken();
    parser->EndNode(node);
    return node;
};

Table::KeyValue *Table::KeyValue::Parse(LuaParser *parser)
{
    if (!parser->IsType(Token::Kind::Letter) || !parser->IsValue("=", 1))
        return nullptr;

    auto node = new KeyValue();
    parser->StartNode(node);
    node->key = parser->ExpectType(Token::Kind::Letter);
    node->tk_equal = parser->ExpectValue("=");
    node->val = ExpressionNode::Parse(parser);
    parser->EndNode(node);
    return node;
}

Table::KeyExpressionValue *Table::KeyExpressionValue::Parse(LuaParser *parser)
{
    if (!parser->IsValue("["))
        return nullptr;

    auto node = new KeyExpressionValue();
    parser->StartNode(node);
    node->tk_left_bracket = parser->ExpectValue("[");
    node->key = ExpressionNode::Parse(parser);
    node->tk_right_bracket = parser->ExpectValue("]");
    node->tk_equal = parser->ExpectValue("=");
    node->val = ExpressionNode::Parse(parser);
    parser->EndNode(node);

    return node;
};

Table::IndexValue *Table::IndexValue::Parse(LuaParser *parser)
{
    auto node = new IndexValue();
    parser->StartNode(node);
    node->val = ExpressionNode::Parse(parser);
    parser->EndNode(node);

    return node;
}

Table *Table::Parse(LuaParser *parser)
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

        if (auto res = KeyExpressionValue::Parse(parser))
        {
            child = res;
        }
        else if (auto res = KeyValue::Parse(parser))
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

PrefixOperator *PrefixOperator::Parse(LuaParser *parser)
{
    if (!parser->runtime_syntax->IsPrefixOperator(parser->GetToken()->value))
        return nullptr;

    auto node = new PrefixOperator();
    parser->StartNode(node);
    node->op = parser->ReadToken();
    node->right = ExpressionNode::Parse(parser);
    parser->EndNode(node);
    return node;
};

PostfixOperator *PostfixOperator::Parse(LuaParser *parser)
{
    if (!parser->runtime_syntax->IsPostfixOperator(parser->GetToken()->value))
        return nullptr;

    auto node = new PostfixOperator();
    parser->StartNode(node);
    node->left = ExpressionNode::Parse(parser);
    node->op = parser->ReadToken();
    parser->EndNode(node);
    return node;
};

BinaryOperator *BinaryOperator::Parse(LuaParser *parser)
{
    if (!parser->runtime_syntax->IsBinaryOperator(parser->GetToken()->value))
        return nullptr;

    auto node = new BinaryOperator();
    parser->StartNode(node);
    node->left = ExpressionNode::Parse(parser);
    node->op = parser->ReadToken();
    node->right = ExpressionNode::Parse(parser);
    parser->EndNode(node);
    return node;
};

ExpressionNode *ExpressionNode::Parse(LuaParser *parser, size_t priority)
{
    // local x = foo()[2]

    ParserNode *node = nullptr;

    if (auto res = Value::Parse(parser))
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
        else if (auto res = Postfix::Parse(parser))
        {
            sub = res;
        }
        else if (auto res = PostfixIndex::Parse(parser))
        {
            sub = res;
        }
        else if (auto res = Cast::Parse(parser))
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

        binary->right = ExpressionNode::Parse(parser, info->right_priority);

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

    return static_cast<ExpressionNode *>(node);
}

ExpressionNode::Index *ExpressionNode::Index::Parse(LuaParser *parser)
{
    if (!parser->IsValue(".") || !parser->IsType(Token::Kind::Letter, 1))
        return nullptr;

    auto *node = new Index();
    parser->StartNode(node);
    node->op = parser->ReadToken();
    node->right = Value::Parse(parser);
    parser->EndNode(node);

    return node;
}

ExpressionNode::SelfCall *ExpressionNode::SelfCall::Parse(LuaParser *parser)
{
    if (!(parser->IsValue(":") && parser->IsType(Token::Kind::Letter, 1) && parser->IsCallExpression(2)))
        return nullptr;

    auto *node = new SelfCall();
    parser->StartNode(node);
    node->op = parser->ReadToken();

    node->right = Value::Parse(parser);

    parser->EndNode(node);

    return node;
}

ExpressionNode::Call *ExpressionNode::Call::Parse(LuaParser *parser)
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
        node->arguments.push_back(Value::Parse(parser));
    }
    else if (parser->IsValue("("))
    {
        node->tk_arguments_left = parser->ReadToken();
        auto values = parser->ReadMultipleValues<ExpressionNode>(
            1000,
            [parser]()
            {
                return Value::Parse(parser);
            },
            node->tk_comma);

        node->arguments.insert(node->arguments.end(), values.begin(), values.end());

        node->tk_arguments_right = parser->ReadToken();
    }

    return node;
}

ExpressionNode::Postfix *ExpressionNode::Postfix::Parse(LuaParser *parser)
{
    if (!parser->runtime_syntax->IsPostfixOperator(parser->GetToken()->value))
        return nullptr;

    auto node = new Postfix();
    parser->StartNode(node);
    node->op = parser->ReadToken();
    parser->EndNode(node);

    return node;
}

ExpressionNode::PostfixIndex *ExpressionNode::PostfixIndex::Parse(LuaParser *parser)
{
    if (!parser->IsValue("["))
        return nullptr;

    auto *node = new PostfixIndex();
    parser->StartNode(node);

    node->tk_left_bracket = parser->ReadToken();
    node->index = ExpressionNode::Parse(parser);
    node->tk_right_bracket = parser->ReadToken();

    parser->EndNode(node);

    return node;
}

ExpressionNode::Cast *ExpressionNode::Cast::Parse(LuaParser *parser)
{
    if (!parser->IsValue(":") || (parser->IsType(Token::Kind::Letter, 1) || parser->IsCallExpression(2)))
    {
        return nullptr;
    }

    if (!parser->IsValue("as"))
    {
        return nullptr;
    }

    auto *node = new Cast();
    parser->StartNode(node);
    node->tk_operator = parser->ReadToken(); // either as or :
    node->expression = ExpressionNode::Parse(parser);
    parser->EndNode(node);

    return node;
}