#include <cstdio>

#include "Differentiator.hpp"
#include "logs.hpp"
#include "MyGeneralFunctions.hpp"
#include "Syntax_analyzer.hpp"

int main()
{
    initLog();
    FILE *texfile = initLatex();
    
    for (int i = 0; i < 1; ++i)
    {
        stack_id stk = {};
        StackCtor(&stk);

        char exp[1000] = "";

        scanf("%s", exp);

        GetTokens(exp, stk);

        Node *node = GetStarted(stk);

        treeLatex(node, texfile);

        node = Taylor(node, "x", 0, 7, texfile);

        treeLatex(node, texfile, "f(x) = ", true);
        
        node = OptimizeExpression(node);

        treeLatex(node, texfile, "f(x) = ", true);

        StackDtor(&stk);
    }

    closeLatex(texfile);
    closeLog();
}