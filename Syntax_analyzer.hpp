#ifndef SYNTAX_ANALYZER_HPP
#define SYNTAX_ANALYZER_HPP

int GetStarted(const char *line);

int GetSumSubExpression(const char **str);

int GetMulDivExpression(const char **str);

int GetBrackets(const char **str);

int GetNumber(const char **str);

#endif //SYNTAX_ANALYZER_HPP