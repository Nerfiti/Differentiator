#ifndef DIFFERENTIATOR_HPP
#define DIFFERENTIATOR_HPP

#include "Tree.hpp"

Node *Diff(const Node *node);
Node *OptimizeExpression(Node *node);

Node *CreateNode(Type type, Data data , Node *left, Node *right);
Node *CreateNum(double val);
Node *CreateVar(char var);
Node *Add(Node *left, Node *right);
Node *Sub(Node *left, Node *right);
Node *Mul(Node *left, Node *right);
Node *Div(Node *left, Node *right);

#endif //DIFFERENTIATOR_HPP