#include "value.h"
#include "interpreter.h"
#include "talloc.h"
#include "parser.h"
#include "linkedlist.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Value *evalLambda(Value *args, Frame *frame);

void error(int status){
    switch (status){
        case 1: {
            printf("Error! :(\n" );
            break;
        }
        case 2: {
            printf("Evaluation error: not a recognized special form\n");
            break;
        }
        case 3: {
            printf("Error: incorrect number of arguments\n");
            break;
        }
        case 4: {
            printf("Error: first argument in IF should be a boolean\n");
            break;
        }
        case 5: {
            printf("Error: undefined symbol\n");
            break;
        }
        case 6: {
            printf("Error: non-number value passed to primitive function +\n");
            break;
        }
    }
    texit(1);
}

//Helper function for printTree
void printTreeValue(Value *value){
    switch (value->type){
            case INT_TYPE:
                printf("%i", value->i);
                break;
            case DOUBLE_TYPE:
                printf("%f", value->d);
                break;
            case STR_TYPE:
                printf("\"%s\"", value->s);
                break;
            case PTR_TYPE:
                printf("%p", value->p);
                break;
            case SYMBOL_TYPE:
                printf("%s", value->s);
                break;
            case OPEN_TYPE:
                printf("(");
                break;
            case CLOSE_TYPE:
                printf(")");
                break;
            case NULL_TYPE:
                printf("()");
                break;
            case BOOL_TYPE:
                if (value->i){
                    printf("#t");
                } else {
                    printf("#f");
                }
                break;
            case CLOSURE_TYPE:
                if (value->cl.functionCode->type == CONS_TYPE){
                    printTree(value->cl.functionCode);
                } else {
                    printTreeValue(value->cl.functionCode);
                }
                break;
            case VOID_TYPE:
                //printf("voidboi");
                break;
            case PRIMITIVE_TYPE:
                printf("Primitive function\n");
                break;
            default:
                break;
        }
}

void printBindings(Value *bindings){
    Value *cur = bindings;
    printf("Bindings\n");
    printf("-----------\n");
    while (cur->type != NULL_TYPE){
        printTreeValue(car(car(cur)));
        printf(" : ");
        printTreeValue(cdr(car(cur)));
        printf("\n");
        cur = cdr(cur);
    }
    printf("-----------\n");
}

Value *evalIf(Value *args, Frame *frame){
    if (length(args) != 3){
        error(3);
    }
    Value *condition = eval(car(args), frame);
    if (condition->type != BOOL_TYPE){
        error(4);
    }
    if (condition->i){
        return eval(car(cdr(args)), frame);
    } else {
        return eval(car(cdr(cdr(args))), frame);
    }
}

Value *evalLet(Value *args, Frame *frame){
    if (length(args) != 2){
        error(3);
    }
    Value *inner = car(car(args));

    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = frame;
    newFrame->bindings = makeNull();
    Value *currentVal = car(args);
    while (currentVal->type != NULL_TYPE){

        assert(car(car(currentVal))->type == SYMBOL_TYPE);
        Value *value = car(currentVal);
        Value *evalu = eval(cdr(value), frame);
        Value *cell = cons(car(value), evalu);
        newFrame->bindings = cons(cell, newFrame->bindings);
        currentVal = cdr(currentVal);
    }
    return eval(cdr(args), newFrame);
}

Value *lookUpSymbol(Value *expr, Frame *frame){
    Frame *currentFrame = frame;
    while (currentFrame != NULL){
        Value *currentBinding = currentFrame->bindings;
        while (currentBinding->type != NULL_TYPE){
            if (!strcmp(car(car(currentBinding))->s, expr->s)){
                return cdr(car(currentBinding));
            }
            currentBinding = cdr(currentBinding);
        }
        currentFrame = currentFrame->parent;
    }
    error(5);
    return expr;
}

Value *evalEach(Value *args, Frame *frame){
    Value *current = args;
    Value *evaledArgs = makeNull();
    while (current->type != NULL_TYPE){
        evaledArgs = cons(eval(car(current), frame), evaledArgs);
        current = cdr(current);
    }
    evaledArgs = reverse(evaledArgs);
    return evaledArgs;
}

Value *evalDefine(Value *args, Frame *frame){
    if (length(args) != 2 && length(args) != 3){
        printf("%i\n", length(args));
        error(3);
    }
    if (length(args) == 2){
        Value *boop = cons(car(args), eval(cdr(args), frame));
        frame->bindings = cons(boop, frame->bindings);
    } else {
        Value *boop = cons(car(args), evalLambda(cdr(args), frame));
        frame->bindings = cons(boop, frame->bindings);
    }
    Value *voidboi = talloc(sizeof(Value));
    voidboi->type = VOID_TYPE;
    return voidboi;
}

Value *evalLambda(Value *args, Frame *frame){
    if (length(args) != 2){
        error(3);
    }
    Value *closure = talloc(sizeof(Value));
    closure->type = CLOSURE_TYPE;
    closure->cl.frame = frame;
    closure->cl.paramNames = car(args);
    closure->cl.functionCode = cdr(args);
    return closure;
}

Value *applyPrimitive(Value *function, Value *args){
    assert(function->type == PRIMITIVE_TYPE);
    //printf("In applyPrimitive, about to return\n");
    Value *ret = (function->pf)(args);
    //printf("done\n");
    return ret;
}

Value *apply(Value *function, Value *args){
    assert(function->type == CLOSURE_TYPE);
    if (function->cl.paramNames->type == CONS_TYPE){
        assert(length(args) == length(function->cl.paramNames));
    } else {
        assert(function->cl.paramNames->type == SYMBOL_TYPE);
    }
    Frame *frame = talloc(sizeof(Frame));
    frame->parent = function->cl.frame;
    frame->bindings = makeNull();
    Value *current = args;
    Value *currentParam = function->cl.paramNames;
    if (currentParam->type == SYMBOL_TYPE) {
        Value *param = cons(currentParam, current);
        frame->bindings = cons(param, frame->bindings);
    }
    else {
        while (current->type != NULL_TYPE){
            Value *param = cons(car(currentParam), car(current));
            frame->bindings = cons(param, frame->bindings);
            current = cdr(current);
            currentParam = cdr(currentParam);
        }
    }
    Value *result = eval(function->cl.functionCode, frame);

    return result;
}

Value *eval(Value *expr, Frame *frame){
    switch (expr->type){
        case INT_TYPE: {
            return expr;
            break;
        }
        case SYMBOL_TYPE: {
            return lookUpSymbol(expr, frame);
            break;
        }
        case STR_TYPE: {
            return expr;
            break;
        }
        case CONS_TYPE: {
            Value *temp = car(expr);
            if (temp->type != CONS_TYPE){
                if (length(expr) > 1){
                    temp = expr;
                }
                else{
                    return eval(temp, frame);
                }
            }
            Value *first = car(temp);
            Value *args = cdr(temp);
            Value *result;
            assert(first->type == SYMBOL_TYPE);
            if (!strcmp(first->s, "if")){
                result = evalIf(args, frame);
            } else if (!strcmp(first->s, "let")){
                result = evalLet(args, frame);
            } else if (!strcmp(first->s, "quote")){
                if (length(args) != 1){
                    error(3);
                }
                result = args;
            } else if(!strcmp(first->s, "define")){
                result = evalDefine(args, frame);
                if (result->type != VOID_TYPE){
                    printf("Error within define: ");
                    if (result->type == CONS_TYPE){
                        printTree(result);
                    } else {
                        printTreeValue(result);
                    }
                    texit(1);
                }
            } else if (!strcmp(first->s, "lambda")){
                result = evalLambda(args, frame);
            } else {
                Value *evaledOperator = eval(first, frame);
                Value *evaledArgs = evalEach(args, frame);
                if (evaledOperator->type == CLOSURE_TYPE){
                    return apply(evaledOperator, evaledArgs);
                } else if (evaledOperator->type == PRIMITIVE_TYPE){
                    //printf("Hi this is right before applyPrimitive (in eval)\n");
                    return applyPrimitive(evaledOperator, evaledArgs);
                } else {
                    error(1);
                }
            }
            return result;
            break;
        }
        case DOUBLE_TYPE: {
            return expr;
            break;
        }
        case BOOL_TYPE: {
            return expr;
            break;
        }
        default: {
            error(1);
            break;
        }
    }
    return expr;
}

Value *primitiveAdd(Value *args){
    //printf("in primitiveAdd!\n" );
    Value *ret = talloc(sizeof(Value));
    ret->type = DOUBLE_TYPE;
    if (args->type == NULL_TYPE){
        ret->d = 0;
        return ret;
    }
    double total = 0;
    while (args->type != NULL_TYPE){
        if (car(args)->type == INT_TYPE){
            total += car(args)->i;
        } else if (car(args)->type == DOUBLE_TYPE) {
            total += car(args)->d;
        } else {
            error(6);
        }
        args = cdr(args);
    }
    ret->d = total;
    return ret;
}

Value *primitiveNull(Value *args){
    if (length(args) != 1){
        error(3);
    }
    Value *ret = talloc(sizeof(Value));
    ret->type = BOOL_TYPE;
    printf("%i\n", car(args)->type);
    if (car(args)->type == CONS_TYPE){
        if (car(car(args))->type == NULL_TYPE){
            ret->i = 1;
        }
    } else {
        ret->i = 0;
    }
    return ret;
}

Value *primitiveCar(Value *args){
    if (length(args) != 1){
        error(3);
    }
    return car(car(car(args)));
}

Value *primitiveCdr(Value *args){
    if (length(args) != 1){
        error(3);
    }
    Value *ret = talloc(sizeof(Value));
    ret->type = CONS_TYPE;
    ret->c.car = cdr(car(car(args)));
    ret->c.cdr = makeNull();
    return ret;
}

void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
    // Add primitive functions to top-level bindings list
    Value *value = talloc(sizeof(Value));
    value->type = PRIMITIVE_TYPE;
    value->pf = function;
    Value *nameVal = talloc(sizeof(Value));
    nameVal->type = STR_TYPE;
    nameVal->s = name;
    Value *funVal = talloc(sizeof(Value));
    funVal->type = PRIMITIVE_TYPE;
    funVal->pf = function;
    Value *cell = cons(nameVal, funVal);
    frame->bindings = cons(cell, frame->bindings);
    printBindings(frame->bindings);
}

void interpret(Value *tree){
    Frame *frame = talloc(sizeof(Frame));
    frame->parent = NULL;
    frame->bindings = makeNull();

    bind("+", primitiveAdd, frame);
    bind("null?", primitiveNull, frame);
    bind("car", primitiveCar, frame);
    bind("cdr", primitiveCdr, frame);

    Value *current = tree;
    while (current->type != NULL_TYPE){
        Value *answer = eval(current, frame);
        current = cdr(current);
        if (answer->type != VOID_TYPE) {
            if (answer->type == CONS_TYPE){
                printTree(answer);
            } else {
                printTreeValue(answer);
            }
            printf("\n");
        }
    }
}
