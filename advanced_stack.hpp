#ifndef ADVANCED_STACK_H
#define ADVANCED_STACK_H

#define STACK_ABORT_ON

#ifdef STACK_ABORT_ON
#define ABORT_ON(...) __VA_ARGS__
#else
#define ABORT_ON(...)
#endif

//----------------------------------------------------------------------------------------------------------------

#include <assert.h>
#include <cstdio>
#include <malloc.h>

#include "MyGeneralFunctions.hpp"
#include "Tree.hpp"

//----------------------------------------------------------------------------------------------------------------

enum STACK_ERRORS
{
    OK                      = 0,
    NON_EXISTENT_STACK      = 1 << 0,
    STACK_CANARY_SIGNAL     = 1 << 1,
    NEGATIVE_SIZE           = 1 << 2,
    CAPACITY_LESS_THAN_SIZE = 1 << 3,
    WRONG_DATA_PTR          = 1 << 4,
    WRONG_STACK_HASH        = 1 << 5,
    DATA_CANARY_SIGNAL      = 1 << 6,
    WRONG_CAPACITY          = 1 << 7,
    WRONG_DATA_HASH         = 1 << 8
};

//----------------------------------------------------------------------------------------------------------------

struct token
{
    Type type = NUM;
    Data data = {.value = 0};

    const char *init_symbol = nullptr;
};

//----------------------------------------------------------------------------------------------------------------

typedef token elem_t;
typedef void *stack_id;

//----------------------------------------------------------------------------------------------------------------

extern const size_t STD_CAPACITY;

//----------------------------------------------------------------------------------------------------------------

int  SetBirds  (stack_id  stk_id);
int  StackCtor (stack_id *stk_id_ptr, size_t capacity = STD_CAPACITY);
int  StackPush (stack_id  stk_id, elem_t item);
int  StackPop  (stack_id  stk_id, elem_t *target);
int  StackDtor (stack_id *stk_id_ptr); 
int  StackOk   (stack_id  stk_id);
void StackDump (stack_id  stk_id);
void getErrMsg (int err, const char **msg);

token GetItem      (stack_id stk_id, int pos);
size_t GetSize     (stack_id stk_id);
size_t GetCapacity (stack_id stk_id);

//----------------------------------------------------------------------------------------------------------------

#endif //ADVANCED_STACK_H