#include "value.h"

#ifndef _INTERPRETER
#define _INTERPRETER

void interpret(Value *tree);
Value *eval(Value *expr, Frame *env);
void printInterpTree(Value *tree);
void printValue(Value *value);

#endif

