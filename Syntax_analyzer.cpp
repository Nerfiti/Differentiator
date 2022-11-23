#include <cstdio>
#include <cstdlib>

#include "Syntax_analyzer.hpp"

int GetStarted(const char *str)
{   
    int val = GetSumSubExpression(&str);
    if (*str != '\0')
    {
        printf("Syntax error in symbol %c. Expected: '\\0'\n", *str);
        fflush(stdout);
        abort();
    }
    return val;
}

int GetSumSubExpression(const char **str)
{
    int val = GetMulDivExpression(str);

    while (**str == '+' || **str == '-')
    {
        char op = **str;
        (*str)++;

        int second_val = GetMulDivExpression(str);

        if (op == '+')
        {
            val += second_val;
        }
        else
        {
            val -= second_val;
        }
    }
    return val;
}

int GetMulDivExpression(const char **str)
{
    int val = GetBrackets(str);

    while (**str == '*' || **str == '/')
    {
        char op = **str;
        (*str)++;

        int second_val = GetBrackets(str);

        if (op == '*')
        {
            val *= second_val;
        }
        else
        {
            if (second_val != 0)
            {
                val /= second_val;
            }
            else
            {
                printf("Calculate error. Division by zero");
                fflush(stdout);
                abort();
            }
        }
    }
    return val;

}

int GetBrackets(const char **str)
{
    int val = 0;
    if (**str == '(')
    {
        (*str)++;
        val = GetSumSubExpression(str);
        if (**str != ')')
        {
            printf("Syntax error in symbol %c. Expexted: ')'\n", **str);
            fflush(stdout);
            abort();
        }
        (*str)++;
    }
    else
    {
        val = GetNumber(str);
    }
    return val;
}

int GetNumber(const char **str)
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
    return val;
}
