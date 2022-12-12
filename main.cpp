#include <cstdio>

#include "Differentiator.hpp"
#include "logs.hpp"
#include "MyGeneralFunctions.hpp"
#include "Syntax_analyzer.hpp"

int main()
{
    initLog();
    FILE *texfile = initLatex();
    
    stack_id stk = {};
    StackCtor(&stk);

    char exp[1000] = "e^(sin(x))";

    //scanf("%s", exp);

    GetTokens(exp, stk);

    Node *node = GetStarted(stk);

    treeLatex(node, texfile);

    node = Taylor(node, "x", 0, 4);

    treeLatex(node, texfile);
    
    node = OptimizeExpression(node);

    treeGraphDump(node);

    treeLatex(node, texfile);

    closeLatex(texfile);
    closeLog();
}