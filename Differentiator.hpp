#ifndef DIFFERENTIATOR_HPP
#define DIFFERENTIATOR_HPP

//----------------------------------------------------------------------------------------------------------------

#include "Tree.hpp"

//----------------------------------------------------------------------------------------------------------------

Node *Diff(Node *node, const char *var);
Node *FuncValue(Node *node, const char *var, double value);
Node *OptimizeExpression(Node *node);
Node *Taylor(Node *node, const char *var, double point, int count);

//----------------------------------------------------------------------------------------------------------------

Node *CreateNode (Type type, Data data , Node *left, Node *right);
Node *CreateNum  (double val);
Node *CreateVar  (const char *var);

//----------------------------------------------------------------------------------------------------------------

Node *Add    (Node *left, Node *right);
Node *Sub    (Node *left, Node *right);
Node *Mul    (Node *left, Node *right);
Node *Div    (Node *left, Node *right);
Node *Pow    (Node *left, Node *right);
Node *Sin    (Node *right);
Node *Cos    (Node *right);
Node *Tan    (Node *right);
Node *Cot    (Node *right);
Node *Arcsin (Node *right);
Node *Arccos (Node *right);
Node *Arctan (Node *right);
Node *Arccot (Node *right);
Node *Ln     (Node *right);
Node *Sqrt   (Node *right);

//----------------------------------------------------------------------------------------------------------------

#endif //DIFFERENTIATOR_HPP