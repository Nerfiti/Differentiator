#include <cassert>
#include <cmath>
#include <cstring>

#include "advanced_stack.hpp"
#include "Differentiator.hpp"
#include "logs.hpp"
#include "MyGeneralFunctions.hpp"
#include "Syntax_analyzer.hpp"

//----------------------------------------------------------------------------------------------------------------

#define CALCULATE_DIVISIONS

//----------------------------------------------------------------------------------------------------------------

static Node *CalculateNumbers(Node *node, bool *was_changed);

static Node *CalculateBinaryOperations(Node *node, bool *was_changed);

static Node *CalculateDivision(Node *node, bool *was_changed);

static Node *DeleteUselessNodes(Node *node, bool *was_changed);

static Node *DeleteSumSubUslessNode(Node *node, bool *was_changed);

static Node *DeleteMulDivUslessNode(Node *node, bool *was_changed);

static Node *DeleteFuncUslessNode(Node *node, bool *was_changed);

static Node *SetChildNodeToThis(Node *node, bool isLeftChild);

static Node *SetLeftNodeToThis(Node *node);

static Node *SetRightNodeToThis(Node *node);

static bool  isConstant(Node *node, const char *var);

static void  Set_o_add(char *o_add, double point, int power);

//----------------------------------------------------------------------------------------------------------------

#define cThis   copyNode(node)
#define cL      copyNode(node->left)
#define dL      Diff(node->left, var)
#define cR      copyNode(node->right)
#define dR      Diff(node->right, var)

Node *Diff(Node *node, const char *var)
{
    assert(node && var);

    switch (node->type)
    {
    case NUM:
        return CreateNum(0);
    case VAR:
        if (strcmp(var, node->data.var) == 0)
        {
            return CreateNum(1);
        }
        else
        {
            return CreateNum(0);
        }
    case OP:
        switch (node->data.op)
        {
        case ADD:
            return Add(dL, dR);
        case SUB:
            return Sub(dL, dR);
        case MUL:
            return Add(Mul(dL, cR), Mul(cL, dR));
        case DIV:
            return Div(Sub(Mul(dL, cR), Mul(cL, dR)), Mul(cR, cR));
        case SIN:
            return Mul(Cos(cR), dR);
        case COS:
            return Mul(Mul(CreateNum(-1), Sin(cR)), dR);
        case TAN:
            return Mul(Div(CreateNum(1),  Pow(Cos(cR), CreateNum(2))), dR);
        case COT:
            return Mul(Div(CreateNum(-1), Pow(Sin(cR), CreateNum(2))), dR);
        case ARCSIN:
            return Mul(Div(CreateNum(1),  Sqrt(Sub(CreateNum(1), Pow(cR, CreateNum(2))))), dR);
        case ARCCOS:
            return Mul(Div(CreateNum(-1), Sqrt(Sub(CreateNum(1), Pow(cR, CreateNum(2))))), dR);
        case ARCTAN:
            return Mul(Div(CreateNum(1),  Add(CreateNum(1), Pow(cR, CreateNum(2)))), dR);
        case ARCCOT:
            return Mul(Div(CreateNum(-1), Add(CreateNum(1), Pow(cR, CreateNum(2)))), dR);
        case LN:
            return Mul(Div(CreateNum(1), cR), dR);
        case SQRT:
            return Mul(Div(CreateNum(1), Mul(CreateNum(2), Sqrt(cR))), dR);
        case POW:
            bool isLeftConstant  = isConstant(node->left , var);
            bool isRightConstant = isConstant(node->right, var);
            if (isLeftConstant && isRightConstant)
            {
                return CreateNum(0);
            }
            else if (isLeftConstant)
            {
                return Mul(Mul(Pow(cL, cR), Ln(cL)), dR);
            }
            else if (isRightConstant)
            {
                return Mul(Mul(cR, Pow(cL, Sub(cR, CreateNum(1)))), dL);
            }
            else
            {
                Node *power = Mul(cR, Ln(cL));
                Node *ans = Mul(cThis, Diff(power, var)); 
                treeDtor(power);
                return ans;    
            }
                       
        }
    }

    return nullptr;
}

#undef cThis
#undef cL
#undef dL
#undef cR
#undef dR

Node *FuncValue(Node *node, const char *var, double value)
{
    if (node->type == VAR && strcmp(var, node->data.var) == 0)
    {
        node->type = NUM;
        node->data.value = value;
    }

    if (node->left != nullptr)
    {
        FuncValue(node->left, var, value);
    }
    if (node->right != nullptr)
    {
        FuncValue(node->right, var, value);
    }

    return node;
}

Node *OptimizeExpression(Node *node)
{
    bool was_changed_by_calculating = true;
    bool was_changed_by_deleting    = true;

    while (was_changed_by_calculating || was_changed_by_deleting)
    {
        node =   CalculateNumbers(node, &was_changed_by_calculating);
        node = DeleteUselessNodes(node, &was_changed_by_deleting   );
    }
    
    return node;
}

Node *Taylor(Node *node, const char *var, double point, int count, FILE *texfile)
{   
    fprintf(texfile, "Разложение функции f(%s) по Тейлору в точке '%lg' до %d-й степени.\n\n"
                     "Обозначим i-й моном многочлена Тейлора за $P_i$.\n\n", var, point, count);

    Node *Derivative = copyNode(node);
    Derivative = OptimizeExpression(Derivative);

    Node *Taylor = FuncValue(copyNode(Derivative), var, point);
    Taylor = OptimizeExpression(Taylor);

    char o_add[50] = "";

    for (int i = 1; i <= count; i++)
    {
        treeGraphDump(Taylor);
        const int max_func_name = 50;
        char der_name[max_func_name] = "";
        char monomial[max_func_name] = "";

        sprintf(der_name, "f^{(%d)}(%s) = ", i, var);
        sprintf(monomial, "P_{%d}(%s) = ", i, var);

        Node *tmp_derivative = Diff(Derivative, var);
        tmp_derivative = OptimizeExpression(tmp_derivative);
        treeLatex(tmp_derivative, texfile, der_name, true);
        
        Node *TaylorNext = Mul(Div(FuncValue(copyNode(tmp_derivative), var, point), CreateNum(factorial(i))), Pow(Sub(CreateVar(var), CreateNum(point)), CreateNum(i)));        
        TaylorNext = OptimizeExpression(TaylorNext);
        treeLatex(TaylorNext, texfile, monomial, true);

        Taylor = Add(Taylor, TaylorNext);
        Taylor = OptimizeExpression(Taylor);

        nodeDtor(Derivative);
        Derivative = tmp_derivative;
    }

    Set_o_add(o_add, point, count);
    treeLatex(Taylor, texfile, "f(x) = ", true, o_add);

    return Taylor;
}

bool GetFuncForAnalyze(char *data, char *function, double *point, int *count, int *width, int *height)
{
    int position = 0;
    int second_position = 0;

    int errflag = sscanf(data, "func: %[^\n]\n %n", function, &position);
    if (!errflag)
    {
        printf("Error in input file. Func is not found.\n");
        return false;
    }

    errflag = sscanf(data + position, "point: %lg %n", point, &second_position);
    if (!errflag)
    {
        printf("Error in input file. Point is not found.\n");
        return false;
    }
    position += second_position;

    errflag = sscanf(data + position, "count: %d %n", count, &second_position);
    if (!errflag)
    {
        printf("Error in input file. Count is not found.\n");
        return false;
    }
    position += second_position;

    errflag = sscanf(data + position, "width: %d %n", width, &second_position);
    if (!errflag)
    {
        printf("Error in input file. Width is not found.\n");
        return false;
    }
    position += second_position;

    errflag = sscanf(data + position, "height: %d %n", height, &second_position);
    if (!errflag)
    {
        printf("Error in input file. Height is not found.\n");
        return false;
    }
    position += second_position;

    return true;
}

void AnalyseFunction(FILE *input)//TODO: width from file
{
    int file_size = getFileSize(input);
    if (file_size == 0) 
    {
        printf("Error opening input file.\n");
        return;
    }

    char *data = (char *)calloc(file_size, sizeof(char));
    fread(data, sizeof(char), file_size, input);

    const int MAX_FUNC_NAME_LEN      = 100;
    char function[MAX_FUNC_NAME_LEN] = "";
    double point   = 0;
    int count      = 0;
    int width      = 0;
    int height     = 0;
    stack_id stk   = {};

    if (GetFuncForAnalyze(data, function, &point, &count, &width, &height))
    {
        FILE *texfile = initLatex();

        StackCtor(&stk);

        printf("Getting input function...\n\n");

        GetTokens(function, stk);
        Node *node = GetStarted(stk);
        treeLatex(node, texfile);

        treeGraphDump(node);

        node = OptimizeExpression(node);
        treeGraphDump(node);

        printf("Function is ready for analysys\n\n");

        Node *taylor = Taylor(node, "x", point, count, texfile);
        Node *tangent = Add(FuncValue(copyNode(node), "x", point), Mul(FuncValue(Diff(node, "x"), "x", point), Sub(CreateVar("x"), CreateNum(point))));

        FILE *gnuplotfile = OpenGnuPlotFile(width, height);
        AddToGnuplotFile(gnuplotfile, node, "", width, "f(x)");
        AddToGnuplotFile(gnuplotfile, taylor, "lt 4", width, "P(x)");
        AddToGnuplotFile(gnuplotfile, tangent, "", width, "tangent");
        CreatePlot(gnuplotfile, texfile);
        
        
        printf("Analysis finished.\n\n");

        treeDtor(node);
        treeDtor(taylor);
        StackDtor(&stk);

        closeLatex(texfile);
    }
    free(data);
}

//----------------------------------------------------------------------------------------------------------------

Node *CreateNode(Type type, Data data, Node *left, Node *right)
{
    Node *node  = treeCtor(type, data);
    node->left  = left;
    node->right = right;

    if (left != nullptr)
    {
        left->parent = node;
    }
    if (right != nullptr)
    {
        right->parent = node;
    }

    return node;
}

Node *CreateNum(double val)
{
    return CreateNode(NUM, {.value = val}, nullptr, nullptr);
}

Node *CreateVar(const char *var)
{
    Node *node = CreateNode(VAR, {.value = 0}, nullptr, nullptr);

    strcpy(node->data.var, var);

    return node;
}

Node *Add(Node *left, Node *right)
{
    return CreateNode(OP, {.op = ADD}, left, right);
}

Node *Sub(Node *left, Node *right)
{
    return CreateNode(OP, {.op = SUB}, left, right);
}

Node *Mul(Node *left, Node *right)
{
    return CreateNode(OP, {.op = MUL}, left, right);
}

Node *Div(Node *left, Node *right)
{
    return CreateNode(OP, {.op = DIV}, left, right);
}

Node *Pow(Node *left, Node *right)
{
    return CreateNode(OP, {.op = POW}, left, right);
}

Node *Sin(Node *right)
{
    return CreateNode(OP, {.op = SIN}, nullptr, right);
}

Node *Cos(Node *right)
{
    return CreateNode(OP, {.op = COS}, nullptr, right);
}

Node *Tan(Node *right)
{
    return CreateNode(OP, {.op = TAN}, nullptr, right);
}

Node *Cot(Node *right)
{
    return CreateNode(OP, {.op = COT}, nullptr, right);
}

Node *Arcsin(Node *right)
{
    return CreateNode(OP, {.op = ARCSIN}, nullptr, right);
}

Node *Arccos(Node *right)
{
    return CreateNode(OP, {.op = ARCCOS}, nullptr, right);
}

Node *Arctan(Node *right)
{
    return CreateNode(OP, {.op = ARCTAN}, nullptr, right);
}

Node *Arccot(Node *right)
{
    return CreateNode(OP, {.op = ARCCOT}, nullptr, right);
}

Node *Ln(Node *right)
{
    return CreateNode(OP, {.op = LN}, nullptr, right);
}

Node *Sqrt(Node *right)
{
    return CreateNode(OP, {.op = SQRT}, nullptr, right);
}

//----------------------------------------------------------------------------------------------------------------

static Node *CalculateNumbers(Node *node, bool *was_changed)
{
    if (node == nullptr || node->type == NUM || node->type == VAR) 
    {
        *was_changed = false;
        return node;
    }

    bool wasLeftChanged  = false;
    bool wasRightChanged = false;
    bool wasCurChanged   = false;

    node->left  = CalculateNumbers(node->left,  &wasLeftChanged );
    node->right = CalculateNumbers(node->right, &wasRightChanged);
    
    if (node->type == OP)
    {
        if (node->left != nullptr && node->right != nullptr)
        {
            if (node->left->type == NUM && node->right->type == NUM)
            {
                node = CalculateBinaryOperations(node, &wasCurChanged);
            }
        }
    }
    else
    {
        printf("Error type: %d\n", node->type);
    }
    *was_changed = wasCurChanged || wasLeftChanged || wasRightChanged;
    return node;
}

static Node *CalculateBinaryOperations(Node *node, bool *was_changed)
{
    assert(node != nullptr && node->left != nullptr && node->right != nullptr && "Error: Expression tree is wrong!\n");

    double first_operand  = node->left->data.value;
    double second_operand = node->right->data.value;

    double result = 0;

    switch (node->data.op)
    {
    case ADD:
        result = first_operand + second_operand;
        break;
    case SUB:
        result = first_operand - second_operand;
        break;
    case MUL:
        result = first_operand * second_operand;
        break;
    case DIV:
        {
        return CalculateDivision(node, was_changed);
        }
    case POW:
        result = pow(first_operand, second_operand);
        break;
    default:
        printf("Error operation: %d\n", node->data.op);
        break;
    }
    
    node->type = NUM;
    node->data.value = result;

    nodeDtor(node->left );
    nodeDtor(node->right);

    node->left  = nullptr;
    node->right = nullptr;

    *was_changed = true;

    return node;
}

static Node *CalculateDivision(Node *node, bool *was_changed)
{
    if (isEqualDoubleNumbers(node->right->data.value, 0))
    {
        printf("Calculate error: Division by zero!\n");
    }
    if (isEqualDoubleNumbers(node->left->data.value, 0))
    {
        nodeDtor(node->left);
        nodeDtor(node->right);

        node->left  = nullptr;
        node->right = nullptr;

        node->type       = NUM;
        node->data.value = 0;

        *was_changed = true;

        return node;
    }
    if (isEqualDoubleNumbers(fabs(node->left->data.value), 1))
    {
        return node;
    }
    if (fabs(node->left->data.value) - fabs(node->right->data.value) > -MIN_POSITIVE_DOUBLE_VALUE)
    {
        double result = node->left->data.value/node->right->data.value;
        if (isEqualDoubleNumbers(result, (int)result))
        {
            node->type       = NUM;
            node->data.value = result;

            nodeDtor(node->left);
            nodeDtor(node->right);

            node->left  = nullptr;
            node->right = nullptr;

            *was_changed = true;

            return node;
        }
    }
    if (fabs(node->right->data.value) - fabs(node->left->data.value) > -MIN_POSITIVE_DOUBLE_VALUE)
    {
        double result = node->right->data.value/node->left->data.value;
        if (isEqualDoubleNumbers(result, (int)result) && !isEqualDoubleNumbers(node->left->data.value, 1))
        {
            node->left->type = NUM;
            node->left->data.value = 1;

            node->right->type = NUM;
            node->right->data.value = result;

            if (result < -MIN_POSITIVE_DOUBLE_VALUE)
            {
                node->left->data.value  *= -1;
                node->right->data.value *= -1;
            }

            *was_changed = true;

            return node;
        }
    }

    return node;
}

static Node *DeleteUselessNodes(Node *node, bool *was_changed)
{
    if (node == nullptr || node->type == NUM || node->type == VAR) 
    {
        *was_changed = false;
        return node;
    }

    bool wasLeftChanged  = false;
    bool wasRightChanged = false;
    bool wasCurChanged   = false;

    node->left  = DeleteUselessNodes(node->left,  &wasLeftChanged );
    node->right = DeleteUselessNodes(node->right, &wasRightChanged);

    if (node->type == OP)
    {
        Operations op = node->data.op;
        if (op == ADD || op == SUB)
        {
            node = DeleteSumSubUslessNode(node, &wasCurChanged);
        }
        else if (op == MUL || op == DIV)
        {
            node = DeleteMulDivUslessNode(node, &wasCurChanged);
        }
        else
        {
            node = DeleteFuncUslessNode(node, &wasCurChanged);
        }
    }

    *was_changed = wasLeftChanged || wasRightChanged || wasCurChanged;
    return node;
}

static Node *DeleteSumSubUslessNode(Node *node, bool *was_changed)
{
    assert(node != nullptr && node->left != nullptr && node->right != nullptr && "Error: Expression tree is wrong!\n");

    if (node->right->type == NUM && node->right->data.value == 0)
    {
        node = SetLeftNodeToThis(node);
        *was_changed = true;
    }
    else if (node->data.op == ADD && node->left->type == NUM && node->left->data.value == 0)
    {
        node = SetRightNodeToThis(node);
        *was_changed = true;
    }

    return node;
}

static Node *DeleteMulDivUslessNode(Node *node, bool *was_changed)//TODO: DSL
{
    assert(node != nullptr && node->left != nullptr && node->right != nullptr && "Error: Expression tree is wrong!\n");
    
    if (node->left->type  == NUM && isEqualDoubleNumbers(node->left->data.value, 0) || 
       (node->right->type == NUM && isEqualDoubleNumbers(node->right->data.value, 0) && node->data.op == MUL))
    {
        node->type       = NUM;
        node->data.value = 0;

        nodeDtor(node->left);
        nodeDtor(node->right);
        node->left  = nullptr;
        node->right = nullptr;
        *was_changed = true;
    }
    else if (node->right->type == NUM && isEqualDoubleNumbers(node->right->data.value, 1))
    {
        node = SetLeftNodeToThis(node);
        *was_changed = true;
    }
    else if (node->left->type  == NUM && isEqualDoubleNumbers(node->left->data.value, 1) && node->data.op == MUL)
    {
        node = SetRightNodeToThis(node);
        *was_changed = true;
    }
    else if (node->right->type == NUM && isEqualDoubleNumbers(node->right->data.value, 0) && node->data.op == MUL)
    {
        printf("Calculate error! Division by zero!\n");
    }

    return node;
}

static Node *DeleteFuncUslessNode(Node *node, bool *was_changed)
{
    assert(node != nullptr && node->right != nullptr && "Error: Expression tree is wrong!\n");

    if (node->type != OP) {return node;}

    Operations op = node->data.op;

    if ((op == TAN || op == ARCTAN || op == SIN) && node->right->type == NUM && node->right->data.value == 0)
    {
        nodeDtor(node->right);

        node->type       = NUM;
        node->data.value = 0;
        node->left       = nullptr;
        node->right      = nullptr;
        *was_changed     = true;
    }
    else if ((op == COT || op == COS) && node->right->type == NUM && node->right->data.value == 0)
    {
        nodeDtor(node->right);

        node->type       = NUM;
        node->data.value = 1;
        node->left       = nullptr;
        node->right      = nullptr;
        *was_changed     = true;
    }
    else if (op == LN)
    {
        if (node->right->type == VAR && strcmp(node->right->data.var, "e") == 0)
        {
            nodeDtor(node->right);

            node->type       = NUM;
            node->data.value = 1;
            node->left       = nullptr;
            node->right      = nullptr;
            *was_changed     = true;
        }
        else if (node->right->type == NUM && node->right->data.value == 1)
        {
            nodeDtor(node->right);

            node->type       = NUM;
            node->data.value = 0;
            node->left       = nullptr;
            node->right      = nullptr;
            *was_changed     = true;
        }
    }
    else if (op == POW)
    {
        if (node->left->type == NUM && (node->left->data.value == 0 || node->left->data.value == 1))
        {
            nodeDtor(node->right);

            node->data.value = node->left->data.value;
            nodeDtor(node->left);
            
            node->type       = NUM;
            node->left       = nullptr;
            node->right      = nullptr;
            *was_changed     = true;
        }
        else if (node->right->type == NUM)
        {
            if (node->right->data.value == 0)
            {
                nodeDtor(node->right);
                nodeDtor(node->left );

                node->data.value = 1;
                node->type       = NUM;
                node->left       = nullptr;
                node->right      = nullptr;
                *was_changed     = true;
            }
            else if (node->right->data.value == 1)
            {
                node = SetLeftNodeToThis(node);
                *was_changed = true;
            }
        }
    }
    else if (op == SQRT && node->right->type == NUM)
    {
        if (isEqualDoubleNumbers((int)sqrt(node->right->data.value), sqrt(node->right->data.value)))
        {
            node->data.value = sqrt(node->right->data.value);
            nodeDtor(node->right);
            
            node->type       = NUM;
            node->left       = nullptr;
            node->right      = nullptr;
            *was_changed     = true;
        }
    }

    return node;
}

static Node *SetChildNodeToThis(Node *node, bool isLeftChild)
{
    assert(node != nullptr);

    bool isParentExist  = (node->parent != nullptr);
    bool isThisLeftNode = false;

    Node *target = (isLeftChild) ? node->left  : node->right;
    Node *trash  = (isLeftChild) ? node->right : node->left;
    Node *parent = node->parent;

    if (isParentExist)
    {
        isThisLeftNode = (node == node->parent->left);
    }

    nodeDtor(node);
    nodeDtor(trash);
    
    if (isParentExist)
    {
        target->parent = parent;
        (isThisLeftNode ? parent->left : parent->right) = target;
    }

    return target;
}

static Node *SetLeftNodeToThis(Node *node)
{
    return SetChildNodeToThis(node, true);
}

static Node *SetRightNodeToThis(Node *node)
{
    return SetChildNodeToThis(node, false);
}

static bool isConstant(Node *node, const char *var)
{
    if (node == nullptr) {return true;}

    bool isThisConstant = !(node->type == VAR && strcmp(var, node->data.var) == 0);

    return isThisConstant && isConstant(node->left, var) && isConstant(node->right, var);
}

static void Set_o_add(char *o_add, double point, int power)
{
    if (isEqualDoubleNumbers(power, 0)) 
        sprintf(o_add, "+o(1)");
    else if (isEqualDoubleNumbers(point, 0) && isEqualDoubleNumbers(power, 1)) 
        sprintf(o_add, "+o(x)");
    else if (isEqualDoubleNumbers(power, 1))
        sprintf(o_add, "+o(x-%lg)", point);
    else if (isEqualDoubleNumbers(point, 0))
        sprintf(o_add, "+o(x^{%d})", power);
    else 
        sprintf(o_add, "+o((x-%lg)^{%d})", point, power);
}

//----------------------------------------------------------------------------------------------------------------
