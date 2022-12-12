#include <cassert>
#include <malloc.h>
#include <cstdlib>
#include <random>
#include <unistd.h>

#include "logs.hpp"
#include "MyGeneralFunctions.hpp"
#include "Tree.hpp"

#define DEBUG

//--------------------------------------------------------------
//FOR GRAPH DUMP
//--------------------------------------------------------------

static int Dump_counter = 1;

static const int  MAX_PATH_LEN   = 30;
static const char *DUMP_PATH     = "./DumpFiles/Dump%d.dot";
static const char *SVG_DUMP_PATH = "./DumpFiles/Dump%d.svg";

static const char *ADD_DUMP_TO_HTML_CODE =  "<details open>\n"
                                                "\t<summary>Dump%d</summary>\n"
                                                "\t<img src = \".%s\">\n"
                                            "</details>\n\n";

static const char *NODE_COLOR = "cornflowerblue";
static const char *LEAF_COLOR = "springGreen";

static const char *LEFT_EDGE_COLOR  = "red";
static const char *RIGHT_EDGE_COLOR = "green";

static const char *START_GRAPH =    "digraph {\n"
                                        "\tbgcolor=\"invis\"\n"
                                        "\tordering = out\n\n"
                                    "node[style = filled, shape = record]\n\n";

//----------------------------------------------------------------------
//FOR LATEX
//----------------------------------------------------------------------

//Path of the TEX files is in the Tree.hpp

static const char *START_LATEX  =   "\\documentclass{article}\n"
                                    "\\usepackage[T2A]{fontenc}\n"
                                    "\\usepackage[utf8]{inputenc}\n"
                                    "\\usepackage[russian, english]{babel}\n"
                                    "\\begin{document}\n\n";

static const char *END_LATEX    =   "\\end{document}";

#include "phrases.hpp"

//----------------------------------------------------------------------

struct operation
{
    const char *label       = nullptr;
    const char *latex_label = nullptr;
    const int   priority    = 0;
};

static const operation OPS[NUMBER_OF_OPERATIONS] = 
{
    {"+"     , "+"        , 1},
    {"-"     , "-"        , 1},
    {"*"     , "\\cdot "  , 2},
    {"/"     , "\\frac"   , 2},
    {"sin"   , "\\sin "   , 3},
    {"cos"   , "\\cos "   , 3},
    {"tan"   , "\\tan "   , 3},
    {"cot"   , "\\cot "   , 3},
    {"arcsin", "\\arcsin ", 3},
    {"arccos", "\\arccos ", 3},
    {"arctan", "\\arctan ", 3},
    {"arccot", "\\arccot ", 3},
    {"ln"    , "\\ln "    , 3},
    {"sqrt"  , "\\sqrt "  , 3},
    {"pow"   , "^"        , 4},
    {"oBr"   , "("        , 5},
    {"cBr"   , ")"        , 5}
};

//--------------------------------------------------------------

static Node *addNode              (Node *node, Type type, Data data, bool toLeft);
static int  creatGraphvizTreeCode (const Node *node, int nodeNum, FILE *dump_file);
static void get_dump_filenames    (char *dump_filename, char *svg_dump_filename);
static void printNodeData         (FILE *stream, Type type, Data data);
static bool IsLeaf                (const Node *node);

//--------------------------------------------------------------

Node *treeCtor(Type type, Data data)
{
    Node *node = (Node *)calloc(1, sizeof(Node));
    
    node->type   = type;
    node->data   = data;
    node->left   = nullptr;
    node->right  = nullptr;
    node->parent = nullptr;

    return node;
}

Node *addToLeft(Node *node, Type type, Data data)
{
    return addNode(node, type, data, true);
}

Node *addToRight(Node *node, Type type, Data data)
{
    return addNode(node, type, data, false);
}

void treeDtor(Node *node)
{
    if (node == nullptr) {return;}

    treeDtor(node->left );
    treeDtor(node->right);

    nodeDtor(node);
}

void nodeDtor(Node *node)
{
    if (node == nullptr) {return;}

    #ifdef DEBUG
        node->type   = NUM;
        node->data   = {};
        node->left   = nullptr;
        node->right  = nullptr;
        node->parent = nullptr;
    #endif //DEBUG

    if (node != JUST_FREE_PTR) 
    {
        free(node);
        node = (Node *)JUST_FREE_PTR;
    }
}

void treePrint(FILE *stream, const Node *node, bool needBrackets)
{    
    if (node != nullptr)
    {
        fprintf(stream, "%c", OPEN_NODE_SYM);

        if (needBrackets)
        {
            fprintf(stream, "%s", OPS[OPEN_BRACKET].latex_label);
        }        

        if (node->type == OP && node->data.op == DIV)
        {
            fprintf  (stream, "%s", OPS[DIV].latex_label);
            treePrint(stream, node->left );
            treePrint(stream, node->right);
        }
        else
        { 
            Node *left  = node->left;
            Node *right = node->right;
            
            bool  needLeftBrackets =  left != nullptr &&  left->type == OP && OPS[ left->data.op].priority < OPS[node->data.op].priority;
            bool needRightBrackets = right != nullptr && right->type == OP && OPS[right->data.op].priority < OPS[node->data.op].priority;

            needLeftBrackets  = needLeftBrackets  || (node->type == OP && node->data.op == MUL && left->type  == NUM && left->data.value  < 0);
            needRightBrackets = needRightBrackets || (node->type == OP && node->data.op == MUL && right->type == NUM && right->data.value < 0);

            treePrint(stream, node->left, needLeftBrackets);

            printNodeData(stream, node->type, node->data);

            treePrint(stream, node->right, needRightBrackets);

        }

        if (needBrackets)
        {
            fprintf(stream, "%s", OPS[CLOSE_BRACKET].latex_label);
        }  

        fprintf(stream, "%c", CLOSE_NODE_SYM);
    }
}

void treePrint(const char *filename, const Node *node, bool needBrackets)
{
    FILE *stream = fopen(filename, "w");
    if (stream == nullptr)
    {
        printf("Error opening file for print: \"%s\".\n", filename);
        return;
    }

    treePrint(stream, node, needBrackets);
}

void treeLatex(const Node *node, FILE *out, const char *prefix, bool withPhrases)
{
    assert(out);

    int phrase = rand() % number_of_phrases;
    if (withPhrases)
    {
        fprintf(out, "\n%s", phrases[phrase]);
    }
    fprintf(out, "\n$$%s", prefix);

    treePrint(out, node);
    fprintf(out, "$$\n\n");
}

void treeLatex(const Node *node, const char *filename, const char *prefix, bool withPhrases)
{
    FILE *out = fopen(filename, "a");

    if (out == nullptr)
    {
        printf("Error opening tex file: %s\n", filename);
        return;
    }

    treeLatex(node, out, prefix, withPhrases);

    fclose(out);
}

void treeGraphDump(const Node *node)
{
    char     dump_filename[MAX_PATH_LEN] = "";
    char svg_dump_filename[MAX_PATH_LEN] = "";

    get_dump_filenames(dump_filename, svg_dump_filename);

    FILE *dump_file = fopen(dump_filename, "w");
    if (dump_file == nullptr)
    {
        log("Error opening dump file: %s\n", dump_filename);
        return;
    }

    fprintf(dump_file, "%s", START_GRAPH);
    int nodeNum = 0;
    creatGraphvizTreeCode(node, nodeNum, dump_file);

    fprintf(dump_file, "}");
    if (fclose(dump_file) != 0)
    {
        log("<p>Error closing dump_file</p>\n");
    }

    const int MAX_CMD_LEN = 100;
    char CMD[MAX_CMD_LEN] = "";
    sprintf(CMD, "dot %s -T svg -o %s", dump_filename, svg_dump_filename);
    
    system(CMD);

    log(ADD_DUMP_TO_HTML_CODE, Dump_counter, svg_dump_filename);
    Dump_counter++;
}

FILE *initLatex(const char *filename)
{
    FILE *out = fopen(filename, "w");

    if (out == nullptr)
    {
        printf("Error opening tex file: %s\n", filename);
        return nullptr;
    }

    return initLatex(out);
}

FILE *initLatex(FILE *stream)
{
    fprintf(stream, "%s", START_LATEX);

    return stream;
}

void closeLatex(const char *filename)
{
    FILE *out = fopen(filename, "a");

    if (out == nullptr)
    {
        printf("Error opening tex file to end it: %s\n", filename);
        return;
    }

    closeLatex(out);
}

void closeLatex(FILE *stream)
{
    fprintf(stream, "%s", END_LATEX);
    fclose(stream);

    pid_t PID = fork();
    if (PID == 0)
    {
        execlp("pdflatex", "pdflatex", OUT_TEX_FILE, (char *)0);
        perror("Error running espeak: ");
        exit(1);
    }
}

Node *copyNode(Node *node)
{
    if (node == nullptr) {return node;}

    Node *newNode = treeCtor(node->type, node->data);
    
    newNode->parent = node->parent;
    
    newNode->left  = copyNode(node->left);
    newNode->right = copyNode(node->right);

    return newNode;
}

//--------------------------------------------------------------

static Node *addNode(Node *node, Type type, Data data, bool toLeft)
{
    Node *newNode   = treeCtor(type, data);
    newNode->parent = node;

    if (toLeft) {node->left  = newNode;}
    else        {node->right = newNode;}

    return newNode;
}

static int creatGraphvizTreeCode(const Node *node, int nodeNum, FILE *dump_file)//TODO:
{
    int number_of_nodes = 0;
    if (node == nullptr) {return number_of_nodes;}

    number_of_nodes ++;

    bool nullLeft  = true;
    bool nullRight = true;

    if (node->left != nullptr)
    {
        fprintf(dump_file, "node%d -> node%d [color = \"%s\"]\n\n", nodeNum, nodeNum + number_of_nodes, LEFT_EDGE_COLOR);        
        number_of_nodes += creatGraphvizTreeCode(node->left, nodeNum + number_of_nodes, dump_file);

        number_of_nodes++;
    }

    if (node->right != nullptr)
    {
        fprintf(dump_file, "node%d -> node%d [color = \"%s\"]\n\n", nodeNum, nodeNum + number_of_nodes, RIGHT_EDGE_COLOR);        
        number_of_nodes += creatGraphvizTreeCode(node->right, nodeNum + number_of_nodes, dump_file);

        number_of_nodes++;
    }

    const char *color = IsLeaf(node) ? LEAF_COLOR : NODE_COLOR;

    fprintf(dump_file, "node%d [fillcolor = %s, label = \"", nodeNum, color);

    if (node->type == NUM)
    {
        fprintf(dump_file, "%.2f\"]\n", node->data.value);
    }
    else if (node->type == OP)
    {
        fprintf(dump_file, "OP|%s\"]\n", OPS[node->data.op].label);
    }
    else if (node->type == VAR)
    {
        fprintf(dump_file, "VAR|%s\"]\n", node->data.var);
    }
    else 
    {
        fprintf(dump_file, "ERROR\"]\n");

    }

    return number_of_nodes;
}

static void get_dump_filenames(char *dump_filename, char *svg_dump_filename)
{
    sprintf(    dump_filename,     DUMP_PATH, Dump_counter);
    sprintf(svg_dump_filename, SVG_DUMP_PATH, Dump_counter);
}

static void printNodeData(FILE *stream, Type type, Data data)
{ 
    if (type == NUM)
    {
        fprintf(stream, "%lg", data.value);
    }
    else if (type == VAR)
    {
        fprintf(stream, "%s", data.var);
    }
    else //if (Type == OP)
    { 
        fprintf(stream, "%s", OPS[data.op].latex_label);
    }
}

static bool IsLeaf(const Node *node)
{
    return (node->left == nullptr && node->right == nullptr);
}

//--------------------------------------------------------------