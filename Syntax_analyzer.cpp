#include <cstdio>
#include <cstdlib>

#include "Differentiator.hpp"
#include "Syntax_analyzer.hpp"

void GetTokens();

Node *GetStarted(const char *str)
{   
    Node *node = GetSumSubExpression(&str);
    if (*str != '\0')
    {
        printf("Syntax error in symbol %c. Expected: '\\0'\n", *str);
        fflush(stdout);
        abort();
    }
    return node;
}

Node *GetSumSubExpression(const char **str)
{
    Node *node = GetMulDivExpression(str);

    while (**str == '+' || **str == '-')
    {
        char op = **str;
        (*str)++;

        Node *second_node = GetMulDivExpression(str);

        if (op == '+')
        {
            node = Add(node, second_node);
        }
        else
        {
            node = Sub(node, second_node);
        }
    }
    return node;
}

Node *GetMulDivExpression(const char **str)
{
    Node *node = GetBrackets(str);

    while (**str == '*' || **str == '/')
    {
        char op = **str;
        (*str)++;

        Node *second_node = GetBrackets(str);

        if (op == '*')
        {
            node = Mul(node, second_node);
        }
        else
        {
            node = Div(node, second_node);
        }
    }
    return node;

}

Node *GetBrackets(const char **str)
{
    Node *node = nullptr;
    if (**str == '(')
    {
        (*str)++;
        node = GetSumSubExpression(str);
        if (**str != ')')
        {
            printf("Syntax error in symbol %c. Expexted: ')'\n", **str);
            fflush(stdout);
            abort();
        }
        (*str)++;
    }
    else if ('a' <= **str && **str <= 'z')
    {
        node = GetVariable(str);
    }
    else 
    {
        node = GetNumber(str);
    }
    return node;
}

Node *GetVariable(const char **str)
{
    char var = **str;
    (*str)++;
    return CreateVar(var);
}

Node *GetNumber(const char **str)
{
    int val = 0;
    const char *strOld = *str;

    while ('0' <= **str && **str <= '9')
    {
        val = val * 10 + **str - '0';
        (*str)++;
    }

    if (*str <= strOld)
    {
        printf("Syntax error: %c is not a number\n", **str);
        fflush(stdout);
        abort();
    }
    return CreateNum(val);
}
