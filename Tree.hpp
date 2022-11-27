#ifndef TREE_HPP
#define TREE_HPP

#include <cstdio>

//----------------------------------------------------------------------
//CONSTANTS
//----------------------------------------------------------------------

static const char  OPEN_NODE_SYM = '{';
static const char CLOSE_NODE_SYM = '}';
static const char IDENT_DATA_SYM = '"';

//----------------------------------------------------------------------

enum Type
{
    NUM,
    OP,
    VAR
};

enum Operations
{
    ADD = '+',
    SUB = '-',
    MUL = '*',
    DIV = '/'
};

union Data
{
    double value = 0;
    Operations op;
    const char var;
};

enum PrintMode
{
    PRE_ORDER,
    IN_ORDER,
    POST_ORDER
};

struct Node
{
    Type type  = NUM;
    Data data  = {};

    Node *parent = nullptr;
    Node *left   = nullptr;
    Node *right  = nullptr;
};

//----------------------------------------------------------------------

Node *treeCtor   (Type type, Data data);
Node *addToLeft  (Node *node, Type type, Data data );
Node *addToRight (Node *node, Type type, Data data );
void  treeDtor   (Node *node);
void  nodeDtor   (Node *node);

void treePrint     (FILE *stream,         const Node *node, PrintMode mode, int space = 0);
void treePrint     (const char *filename, const Node *node, PrintMode mode, int space = 0);
void treeGraphDump (const Node *node);

#endif //TREE_HPP