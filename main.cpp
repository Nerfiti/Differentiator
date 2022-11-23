#include <cstdio>

#include "Differentiator.hpp"
#include "Syntax_analyzer.hpp"

int main()
{
    while (true)
    {
        char exp[1000] = "";
        scanf("%s", exp);
        int val = GetStarted(exp);
        printf("Value = %d\n", val);
    }
}