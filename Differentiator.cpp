#include <cassert>
#include <cstring>

#include "Differentiator.hpp"
#include "logs.hpp"
#include "MyGeneralFunctions.hpp"

//----------------------------------------------------------------------------------------------------------------

static Node *CalculateNumbers(Node *node, bool *was_changed);

static Node *CalculateBinaryOperations(Node *node);

static Node *DeleteUselessNodes(Node *node, bool *was_changed);

static Node *DeleteSumSubUslessNode(Node *node, bool *was_changed);

static Node *DeleteMulDivUslessNode(Node *node, bool *was_changed);

static Node *SetChildNodeToThis(Node *node, bool isLeftChild);

static Node *SetLeftNodeToThis(Node *node);

static Node *SetRightNodeToThis(Node *node);

//----------------------------------------------------------------------------------------------------------------

#define cL node->left
#define dL Diff(cL)
#define cR node->right
#define dR Diff(cR)

Node *Diff(const Node *node)
{
    switch (node->type)
    {
    case NUM:
        return CreateNum(0);
    case VAR:
        return CreateNum(1);
    case OP:
        switch (node->data.op)
        {
        case ADD:
            return Add(dL, dR);
        case SUB:
            return Sub(dL, dR);
        case MUL:
            return Add(Mul(dL, cR), Mul(cL, dR));
        case DIV:
            return Div(Sub(Mul(dL, cR), Mul(cL, dR)), Mul(cR, cR));
        }
    }
    return nullptr;
}

#undef cL
#undef dL
#undef cR
#undef dR

Node *OptimizeExpression(Node *node)
{
    bool was_changed_by_calculating = true;
    bool was_changed_by_deleting    = true;

    while (was_changed_by_calculating || was_changed_by_deleting)
    {
        node = CalculateNumbers(node, &was_changed_by_calculating);
        node = DeleteUselessNodes(node, &was_changed_by_deleting);

        treeGraphDump(node);
    }
    
    return node;
}

//----------------------------------------------------------------------------------------------------------------

Node *CreateNode(Type type, Data data , Node *left, Node *right)
{
    Node *node  = treeCtor(type, data);
    node->left  = left;
    node->right = right;

    if (left != nullptr)
    {
        left->parent = node;
    }
    if (right != nullptr)
    {
        right->parent = node;
    }

    return node;
}

Node *CreateNum(double val)
{
    return CreateNode(NUM, {.value = val}, nullptr, nullptr);
}

Node *CreateVar(char var)
{
    return CreateNode(VAR, {.var = var}, nullptr, nullptr);
}

Node *Add(Node *left, Node *right)
{
    return CreateNode(OP, {.op = ADD}, left, right);
}

Node *Sub(Node *left, Node *right)
{
    return CreateNode(OP, {.op = SUB}, left, right);
}

Node *Mul(Node *left, Node *right)
{
    return CreateNode(OP, {.op = MUL}, left, right);
}

Node *Div(Node *left, Node *right)
{
    return CreateNode(OP, {.op = DIV}, left, right);
}

//----------------------------------------------------------------------------------------------------------------

static Node *CalculateNumbers(Node *node, bool *was_changed)
{
    if (node == nullptr || node->type == NUM || node->type == VAR) 
    {
        *was_changed = false;
        return node;
    }

    bool wasLeftChanged  = false;
    bool wasRightChanged = false;
    bool wasCurChanged   = false;

    node->left  = CalculateNumbers(node->left,  &wasLeftChanged );
    node->right = CalculateNumbers(node->right, &wasRightChanged);

    if (node->left->type == NUM && node->right->type == NUM)
    {
        CalculateBinaryOperations(node);
        wasCurChanged = true;
    }

    *was_changed = wasLeftChanged || wasRightChanged || wasCurChanged;

    return node;
}

static Node *CalculateBinaryOperations(Node *node)
{
    assert(node != nullptr && node->left != nullptr && node->right != nullptr && "Error: Expression tree is wrong!\n");

    double first_operand  = node->left->data.value;
    double second_operand = node->right->data.value;

    double result = 0;

    switch (node->data.op)
    {
    case ADD:
        result = first_operand + second_operand;
        break;
    case SUB:
        result = first_operand - second_operand;
        break;
    case MUL:
        result = first_operand * second_operand;
        break;
    case DIV:
        if (!isEqualDoubleNumbers(second_operand, 0))
        {
            result = first_operand / second_operand;
        }
        else
        {
            printf("Calculate error: division by zero\n");
        }
        break;
    default:
        printf("Error operation: %d\n", node->data.op);
        break;
    }
    
    node->type = NUM;
    node->data.value = result;

    nodeDtor(node->left );
    nodeDtor(node->right);

    node->left  = nullptr;
    node->right = nullptr;

    return node;
}

static Node *DeleteUselessNodes(Node *node, bool *was_changed)
{
    if (node == nullptr || node->type == NUM || node->type == VAR) 
    {
        *was_changed = false;
        return node;
    }

    bool wasLeftChanged  = false;
    bool wasRightChanged = false;
    bool wasCurChanged   = false;

    node->left  = DeleteUselessNodes(node->left,  &wasLeftChanged );
    node->right = DeleteUselessNodes(node->right, &wasRightChanged);

    if (node->type == OP)
    {
        Operations op = node->data.op;
        if (op == ADD || op == SUB)
        {
            node = DeleteSumSubUslessNode(node, &wasCurChanged);
        }
        else if (op == MUL || op == DIV)
        {
            node = DeleteMulDivUslessNode(node, &wasCurChanged);
        }
    }

    *was_changed = wasLeftChanged || wasRightChanged || wasCurChanged;
    return node;
}

static Node *DeleteSumSubUslessNode(Node *node, bool *was_changed)
{
    assert(node != nullptr && node->left != nullptr && node->right != nullptr && "Error: Expression tree is wrong!\n");

    if (node->right->type == NUM && node->right->data.value == 0)
    {
        node = SetLeftNodeToThis(node);
        *was_changed = true;
    }
    else if (node->data.op == ADD && node->left->type == NUM && node->left->data.value == 0)
    {
        node = SetRightNodeToThis(node);
        *was_changed = true;
    }

    return node;
}

static Node *DeleteMulDivUslessNode(Node *node, bool *was_changed)
{
    assert(node != nullptr && node->left != nullptr && node->right != nullptr && "Error: Expression tree is wrong!\n");

    if (node->left->type  == NUM && node->left->data.value  == 0 || 
       (node->right->type == NUM && node->right->data.value == 0 && node->data.op == MUL))
    {
        node->type       = NUM;
        node->data.value = 0;

        nodeDtor(node->left);
        nodeDtor(node->right);
        node->left  = nullptr;
        node->right = nullptr;
        *was_changed = true;
    }
    else if (node->right->type == NUM && node->right->data.value == 1)
    {
        node = SetLeftNodeToThis(node);
        *was_changed = true;
    }
    else if (node->left->type  == NUM && node->left->data.value  == 1)
    {
        node = SetRightNodeToThis(node);
        *was_changed = true;
    }

    return node;
}

static Node *SetChildNodeToThis(Node *node, bool isLeftChild)
{
    assert(node != nullptr);

    bool isParentExist  = (node->parent != nullptr);
    bool isThisLeftNode = false;

    Node *target = (isLeftChild) ? node->left  : node->right;
    Node *trash  = (isLeftChild) ? node->right : node->left;
    Node *parent = node->parent;

    if (isParentExist)
    {
        isThisLeftNode = (node == node->parent->left);
    }

    nodeDtor(node);
    nodeDtor(trash);
    
    if (isParentExist)
    {
        target->parent = parent;
        (isThisLeftNode ? parent->left : parent->right) = target;
    }

    return target;
}

static Node *SetLeftNodeToThis(Node *node)
{
    return SetChildNodeToThis(node, true);
}

static Node *SetRightNodeToThis(Node *node)
{
    return SetChildNodeToThis(node, false);
}

//----------------------------------------------------------------------------------------------------------------
