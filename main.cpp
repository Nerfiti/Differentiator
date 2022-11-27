#include <cstdio>

#include "Differentiator.hpp"
#include "logs.hpp"
#include "MyGeneralFunctions.hpp"
#include "Syntax_analyzer.hpp"

int main()
{
    initLog();

    int start = 1;
    int finish = 1;

    while (start <= finish)
    {
        const int max_expression_len = 1000;
        
        char exp[max_expression_len] = "";
        GetLine(exp);
        exp[max_expression_len - 1] = '\0';

        Node *node = GetStarted(exp);
        treeGraphDump(node);

        node = Diff(node);
        treeGraphDump(node);

        node = OptimizeExpression(node);

        start++;
    }

    closeLog();
}