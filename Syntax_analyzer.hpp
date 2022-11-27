#ifndef SYNTAX_ANALYZER_HPP
#define SYNTAX_ANALYZER_HPP

#include "Tree.hpp"

Node *GetStarted(const char *line);

Node *GetSumSubExpression(const char **str);

Node *GetMulDivExpression(const char **str);

Node *GetBrackets(const char **str);

Node *GetVariable(const char **str);

Node *GetNumber(const char **str);

#endif //SYNTAX_ANALYZER_HPP