#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "Differentiator.hpp"
#include "Syntax_analyzer.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------

static bool GetOperationToken(const char **str, token *Token); 

static void GetVariableToken(const char **str, token *Token);

//--------------------------------------------------------------------------------------------------------------------------------------------------------

void GetTokens(const char *str, stack_id stk)
{
    token Token = {};
    char *endptr = nullptr;

    while (*str != '\0')
    {
        Token.init_symbol = str;
        
        if ('0' <= *str && *str <= '9')
        {
            Token.data.value = strtod(str, &endptr);
            Token.type       = NUM;

            str = endptr;
        }
        else if (!GetOperationToken(&str, &Token))
        {
            GetVariableToken(&str, &Token);
        }

        StackPush(stk, Token);
    }
    Token.type = END_EXPRESSION;
    Token.init_symbol = str;
    Token.data = {};
    StackPush(stk, Token);
}

Node *GetStarted(stack_id stk)
{   
    int reader = 0;

    Node *node = GetSumSubExpression(stk, &reader);
    token Token = GetItem(stk, reader);

    if (Token.type != END_EXPRESSION)
    {
        assert(Token.init_symbol);
        printf("Syntax error in symbol %c. Expected: '\\0'\n", *Token.init_symbol);
        fflush(stdout);
        abort();
    }
    return node;
}

Node *GetSumSubExpression(stack_id stk, int *reader)
{
    Node *node = GetMulDivExpression(stk, reader);

    token Token = GetItem(stk, *reader);

    while (Token.type == OP && (Token.data.op == ADD || Token.data.op == SUB))
    {
        Operations op = Token.data.op;
        (*reader)++;

        Node *second_node = GetMulDivExpression(stk, reader);

        if (op == ADD)
        {
            node = Add(node, second_node);
        }
        else
        {
            node = Sub(node, second_node);
        }

        Token = GetItem(stk, *reader);
    }

    return node;
}

Node *GetMulDivExpression(stack_id stk, int *reader)
{
    Node *node = GetUnary(stk, reader);

    token Token = GetItem(stk, *reader);

    while (Token.type == OP && (Token.data.op == MUL || Token.data.op == DIV))
    {
        char op = Token.data.op;
        (*reader)++;
        
        Node *second_node = GetUnary(stk, reader);

        if (op == MUL)
        {
            node = Mul(node, second_node);
        }
        else
        {
            node = Div(node, second_node);
        }

        Token = GetItem(stk, *reader);
    }

    return node;

}

Node *GetUnary(stack_id stk, int *reader)
{
    int sign = 1;

    token Token = GetItem(stk, *reader);

    if (Token.type == OP && (Token.data.op == SUB || Token.data.op == ADD)) 
    {
        sign = Token.data.op == SUB ? -1 : 1;
        (*reader)++;
    }

    Node *node = GetFunction(stk, reader);

    if (sign == -1) 
    {
        return Mul(CreateNum(-1), node);
    }
    return node;
}

Node *GetFunction(stack_id stk, int *reader)
{
    Node *node = nullptr;

    token Token = GetItem(stk, *reader);

    Operations op = Token.data.op;

    if (Token.type == OP && (op == SIN    || op == COS    || op == TAN    || op == COT    ||
                             op == ARCSIN || op == ARCCOS || op == ARCTAN || op == ARCCOT ||
                             op == LN     || op == SQRT))
    {
        (*reader)++;
        
        Token = GetItem(stk, *reader);
        (*reader)++;

        if (!(Token.type == OP && Token.data.op == OPEN_BRACKET))
        {
            printf("Syntax error. Functions arguments must be started with '('\n");
            fflush(stdout);
            abort();
        }
        node = CreateNode(OP, {.op = op}, nullptr, GetSumSubExpression(stk, reader));
        
        Token = GetItem(stk, *reader);
        (*reader)++;

        if (!(Token.type == OP && Token.data.op == CLOSE_BRACKET))
        {
            printf("Syntax error. Functions arguments must be ended with ')'\n");
            fflush(stdout);
            abort();
        }
    }
    else
    {
        node = GetPow(stk, reader);
    }

    return node;
}

Node *GetPow(stack_id stk, int *reader)
{
    Node *node = GetBrackets(stk, reader);

    token Token = GetItem(stk, *reader);
    
    if (Token.type == OP && Token.data.op == POW)
    {
        (*reader)++;

        Node *second_node = GetUnary(stk, reader);

        node = Pow(node, second_node);
    }

    return node;
}

Node *GetBrackets(stack_id stk, int *reader)
{
    Node *node = nullptr;

    token Token = GetItem(stk, *reader);


    if (Token.type == OP && Token.data.op == OPEN_BRACKET)
    {
        (*reader)++;

        node = GetSumSubExpression(stk, reader);

        Token = GetItem(stk, *reader);
        (*reader)++;
        
        if (!(Token.type == OP && Token.data.op == CLOSE_BRACKET))
        {
            printf("Error in symbol %c. Expected ')'\n", *Token.init_symbol);
            fflush(stdout);
            abort();
        }
    }
    else if (Token.type == VAR)
    {
        node = GetVariable(stk, reader);
    }
    else
    {
        node = GetNumber(stk, reader);
        //treeGraphDump(node);

    }
    
    return node;
}

Node *GetVariable(stack_id stk, int *reader)
{
    token Token = GetItem(stk, *reader);
    (*reader)++;

    if (Token.type != VAR)
    {
        printf("Syntax error in symbol %c. Expected variable.\n", *Token.init_symbol);
        fflush(stdout);
        abort();
    }

    return CreateVar(Token.data.var);
}

Node *GetNumber(stack_id stk, int *reader)
{
    token Token = GetItem(stk, *reader);
    (*reader)++;

    if (Token.type != NUM)
    {
        printf("Syntax error in symbol %c. Expected number.\n", *Token.init_symbol);
        fflush(stdout);
        abort();
    }

    return CreateNum(Token.data.value);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------

#define CHECK_OP(OP)                        \
{                                           \
    len = strlen(#OP);                      \
    if (strncasecmp(*str, #OP, len) == 0)   \
    {                                       \
        Token->data.op = OP;                \
        *str += len;                        \
        return true;                        \
    }                                       \
}

static bool GetOperationToken(const char **str, token *Token)
{
    *Token = {};
    Token->type        = OP;
    Token->init_symbol = *str;
    
    switch (**str)
    {
        case '+':
            Token->data.op = ADD;
            (*str)++;
            return true;

        case '-':
            Token->data.op = SUB;
            (*str)++;
            return true;

        case '*':
            Token->data.op = MUL;
            (*str)++;
            return true;

        case '/':
            Token->data.op = DIV;
            (*str)++;
            return true;

        case '(':
            Token->data.op = OPEN_BRACKET;
            (*str)++;
            return true;

        case ')':
            Token->data.op = CLOSE_BRACKET;
            (*str)++;
            return true;

        case '^':
            Token->data.op = POW;
            (*str)++;
            return true;

        default:
            int len = 0;

            CHECK_OP(SIN);
            CHECK_OP(COS);
            CHECK_OP(TAN);
            CHECK_OP(COT);
            CHECK_OP(ARCSIN);
            CHECK_OP(ARCCOS);
            CHECK_OP(ARCTAN);
            CHECK_OP(ARCCOT);
            CHECK_OP(LN);
            CHECK_OP(SQRT);
            break;
    }
    return false;
}

#undef CHECK_OP

static void GetVariableToken(const char **str, token *Token)
{
    Token->init_symbol = *str;
    Token->type = VAR;

    int i = 0;
    while (i < 7 && (('a' <= **str && **str <= 'z') || ('A' <= **str && **str <= 'Z')))
    {
        Token->data.var[i] = **str;
        i++;
        (*str)++;
    }
    while (i < 8)
    {
        Token->data.var[i] = '\0';
        i++;
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------