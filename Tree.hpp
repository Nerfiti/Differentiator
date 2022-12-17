#ifndef TREE_HPP
#define TREE_HPP

#include <cstdio>

//----------------------------------------------------------------------
//CONSTANTS
//----------------------------------------------------------------------

static const char  OPEN_NODE_SYM   = '{';
static const char CLOSE_NODE_SYM   = '}';
static const char IDENT_DATA_SYM   = '"';

static const int  MAX_VAR_NAME_LEN = 8;

static const char *OUT_TEX_FILE = "./TexFiles/Differentiator.tex";

//--------------------------------------------------------------

enum Type
{
    NUM,
    OP,
    VAR,
    END_EXPRESSION
};

enum Operations
{
    ADD,
    SUB,
    MUL,
    DIV,
    SIN,
    COS,
    TAN,
    COT,
    ARCSIN,
    ARCCOS,
    ARCTAN,
    ARCCOT,
    LN,
    SQRT,
    POW,
    OPEN_BRACKET,
    CLOSE_BRACKET,
    NUMBER_OF_OPERATIONS
};

union Data
{
    double value = 0;
    Operations op;
    char var[MAX_VAR_NAME_LEN];
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

void treePrint     (FILE *stream,         const Node *node, bool needBrackets = false);
void treePrint     (const char *filename, const Node *node, bool needBrackets = false);
void treeLatex     (const Node *node, FILE *out, const char *prefix = "f(x) = ", bool withPhrases = false, const char *postfix = "");
void treeGraphDump (const Node *node);

FILE *initLatex  (const char *filename = OUT_TEX_FILE);
FILE *initLatex  (FILE *stream);
void  closeLatex (FILE *stream);

Node *copyNode (Node *node);

void LatexPlot        (Node *node, int width, int height, FILE *texfile, const char *funcname);

FILE *OpenGnuPlotFile (int width, int height);
void AddToGnuplotFile (FILE *plotfile, Node *node, const char *mode, int width, const char *funcname);
void CreatePlot       (FILE *plotfile, FILE *texfile);

//----------------------------------------------------------------------

#endif //TREE_HPP