#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <malloc.h>
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
                                    "\\usepackage[pdftex]{graphicx}\n"
                                    "\\usepackage[left=0.1 cm, right=0.1cm]{geometry}\n\n"
                                    "\\geometry{papersize={100 cm,50 cm}}\n\n"
                                    "\\begin{document}\n"
                                    "\\begin{center}\n\n";

static const char *END_LATEX    =   "\\end{center}\n"
                                    "\\end{document}";

static const char *FUNC_PLOT_FILENAME_PNG = "Plot%d.png";
static const int MAX_PLOT_FILENAME_LEN = 60;
static const char *PLOTDATAFILENAME = "./TexFiles/plot%d.data";
static int PlotDataCounter = 1;
static const char *PLOTFILENAME = "./TexFiles/plot.gnu";
static int PlotCounter = 1;

#include "phrases.hpp"

//----------------------------------------------------------------------

struct operation
{
    const char *label       = nullptr;
    const char *latex_label = nullptr;
    const int   priority    = 0;
};

static const operation OPS[NUMBER_OF_OPERATIONS] = //TODO: add info about tokens
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

static double calculateValueForPlot (Node *node, const char *var, double value);

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

    if (node->parent != nullptr)
    {
        if (node->parent->left == node)
        {
            node->parent->left = nullptr;
        }
        else if (node->parent->right == node)
        {
            node->parent->right = nullptr;
        }
    }

    #ifdef DEBUG
        node->type   = NUM;
        node->data   = {};
        node->left   = nullptr;
        node->right  = nullptr;
        node->parent = nullptr;
    #endif //DEBUG

    if (node != nullptr) 
    {
        free(node);
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

            bool  needLeftBrackets =  ((left != nullptr) &&  (left->type == OP) && (OPS[ left->data.op].priority < OPS[node->data.op].priority));
            bool needRightBrackets = ((right != nullptr) && (right->type == OP) && (OPS[right->data.op].priority < OPS[node->data.op].priority));

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

void treeLatex(const Node *node, FILE *out, const char *prefix, bool withPhrases, const char *postfix)
{
    assert(out);

    int phrase = rand() % number_of_phrases;
    if (withPhrases)
    {
        fprintf(out, "\n%s", phrases[phrase]);
    }
    fprintf(out, "\n$$%s", prefix);

    treePrint(out, node);
    fprintf(out, "%s$$\n\n", postfix);
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

void closeLatex(FILE *stream)
{
    fprintf(stream, "%s", END_LATEX);
    fclose(stream);

    const int MAX_CMD_LEN = 100;
    char cmd[MAX_CMD_LEN] = "";

    sprintf(cmd, "pdflatex -output-directory=./TexFiles \"%s\" > TEXLOG.txt", OUT_TEX_FILE);
    system(cmd);
}

Node *copyNode(Node *node)
{
    if (node == nullptr) {return nullptr;}

    Node *newNode = treeCtor(node->type, node->data);
    
    newNode->left  = copyNode(node->left);
    newNode->right = copyNode(node->right);

    return newNode;
}

void LatexPlot(Node *node, int width, int height, FILE *texfile, const char *funcname)
{
    // const double accuracy = ((double)width)/10000;

    // FILE *plotdatafile = fopen(PLOTDATAFILENAME, "w");
    // if (plotdatafile == nullptr)
    // {
    //     printf("Error opening file for plot data\n");
    // }

    // for (double x = -width; x < width; x += accuracy)
    // {
    //     fprintf(plotdatafile, "%lg, %lg\n", x, calculateValueForPlot(node, "x", x));
    // }

    // assert(!fclose(plotdatafile));
    

    // FILE *plotfile = fopen(PLOTFILENAME, "w");
    // if (plotfile == nullptr)
    // {
    //     printf("Error opening file for plot\n");
    // }

    // char plotfilename_JPG[MAX_PLOT_FILENAME_LEN] = "";
    // sprintf(plotfilename_JPG, FUNC_PLOT_FILENAME_JPG, PlotCounter);
    // PlotCounter++;

    // fprintf(plotfile,   "set xrange [%d:%d]\n"
    //                     "set yrange [%d:%d]\n"
    //                     "set terminal png\n"
    //                     "set output \"./TexFiles/%s\"\n"
    //                     "set grid\n"
    //                     "plot \"%s\" title \"%s\" with lines\n"
    //                     "exit\n", -width, width, -height, height, plotfilename_JPG, PLOTDATAFILENAME, funcname);

    // assert(!fclose(plotfile));
    
    // pid_t PID = fork();
    // if (PID == 0) 
    // {
    //     execlp("gnuplot", "gnuplot", PLOTFILENAME, (char *)0);
    //     perror("Error running gnuplot: ");
    //     exit(1);
    // }

    // fprintf(texfile, "\n\\includegraphics{\"%s\"}\n\n", plotfilename_JPG);
}

FILE *OpenGnuPlotFile(int width, int height)
{
    FILE *plotfile = fopen(PLOTFILENAME, "w");
    if (plotfile == nullptr)
    {
        printf("Error opening file for plot\n");
    }

    char plotfilename_PNG[MAX_PLOT_FILENAME_LEN] = "";
    sprintf(plotfilename_PNG, FUNC_PLOT_FILENAME_PNG, PlotCounter);

    fprintf(plotfile,   "set xrange [%d:%d]\n"
                        "set yrange [%d:%d]\n"
                        "set terminal png\n"
                        "set output \"./TexFiles/%s\"\n"
                        "set grid\n"
                        "plot ", -width, width, -height, height, plotfilename_PNG);

    return plotfile;   
}

void AddToGnuplotFile(FILE *plotfile, Node *node, const char *mode, int width, const char *funcname)
{
    assert(plotfile);
    
    const double accuracy = ((double)width)/10000;

    const int max_filename_len = 100;
    char plotDataFilename[max_filename_len] = "";
    sprintf(plotDataFilename, PLOTDATAFILENAME, PlotDataCounter);
    PlotDataCounter++;

    FILE *plotdatafile = fopen(plotDataFilename, "w");
    if (plotdatafile == nullptr)
    {
        printf("Error opening file for plot data\n");
    }

    for (double x = -width; x < width; x += accuracy)
    {
        fprintf(plotdatafile, "%lg, %lg\n", x, calculateValueForPlot(node, "x", x));
    }

    assert(!fclose(plotdatafile));

    fprintf(plotfile, "\"%s\" title \"%s\" with lines %s, ", plotDataFilename, funcname, mode);
}

void CreatePlot(FILE *plotfile, FILE *texfile)
{
    fprintf(plotfile, "\nexit");
    fclose(plotfile);

    pid_t PID = fork();
    if (PID == 0) 
    {
        execlp("gnuplot", "gnuplot", PLOTFILENAME, (char *)0);
        perror("Error running gnuplot: ");
        exit(1);
    }

    char plotfilename_PNG[MAX_PLOT_FILENAME_LEN] = "";
    sprintf(plotfilename_PNG, FUNC_PLOT_FILENAME_PNG, PlotCounter);
    PlotCounter++;

    fprintf(texfile, "\n\\includegraphics{\"%s\"}\n\n", plotfilename_PNG);

    PlotDataCounter = 1;
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

static int creatGraphvizTreeCode(const Node *node, int nodeNum, FILE *dump_file)
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

static double calculateValueForPlot(Node *node, const char *var, double value)
{
    if (node == nullptr) {return 0;}

    double left  = calculateValueForPlot(node->left,  var, value);
    double right = calculateValueForPlot(node->right, var, value);

    switch (node->type)
    {
    case NUM:
        return node->data.value;
        break;
    case VAR:
        if (strcmp(node->data.var, var) == 0)
        {
            return value;
        }
        else
        {
            return 0;
        }
        break;
    case OP:
        switch (node->data.op)
        {
        case ADD:
            return left + right;
        case SUB:
            return left - right;
        case MUL:
            return left * right;
        case DIV:
            if (right == 0) {return 0;}
            return left / right;
        case SIN:
            return sin(right);
        case COS:
            return cos(right);
        case TAN:
            return tan(right);
        case COT:
            return 1/tan(right);
        case ARCSIN:
            return asin(right);
        case ARCCOS:
            return acos(right);
        case ARCTAN:
            return atan(right);
        case ARCCOT:
            return M_PI_2 - atan(right);
        case LN:
            return log(right);
        case SQRT:
            return sqrt(right);
        case POW:
            return pow(left, right);
        break;
        }
    }
    return 0;
}

//--------------------------------------------------------------