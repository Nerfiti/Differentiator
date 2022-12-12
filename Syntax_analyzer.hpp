#ifndef SYNTAX_ANALYZER_HPP
#define SYNTAX_ANALYZER_HPP

//----------------------------------------------------------------------------------------------------------------

#include "advanced_stack.hpp"
#include "Tree.hpp"

//----------------------------------------------------------------------------------------------------------------

void GetTokens(const char *str, stack_id stk);

Node *GetStarted(stack_id stk);

Node *GetSumSubExpression(stack_id stk, int *reader);

Node *GetMulDivExpression(stack_id stk, int *reader);

Node *GetUnary(stack_id stk, int *reader);

Node *GetFunction(stack_id stk, int *reader);

Node *GetPow(stack_id stk, int *reader);

Node *GetBrackets(stack_id stk, int *reader);

Node *GetVariable(stack_id stk, int *reader);

Node *GetNumber(stack_id stk, int *reader);

//----------------------------------------------------------------------------------------------------------------

#endif //SYNTAX_ANALYZER_HPP