#include "value.h"
#include "interpreter.h"
#include "talloc.h"
#include "parser.h"
#include "linkedlist.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


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
                printf("voidboi");
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
    printf("In evalIf. Args: ");
    printTree(args);
    printf("\n");
    if (length(args) != 3){
        error(3);
    }
    Value *condition = eval(car(args), frame);
    if (condition->type != BOOL_TYPE){
        error(4);
    }
    printf("returning... ");
    if (condition->i){
        //printTreeValue(car(cdr(cdr(args))));
        printf("\nhhhhh");
        return eval(car(cdr(args)), frame);
    } else {
        //printTreeValue(car(cdr(cdr(cdr(args)))));
        printf("\njjjjjj");
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
                printf("Found %s!\n", expr->s);
                return cdr(car(currentBinding));
            }
            currentBinding = cdr(currentBinding);
        }
        currentFrame = currentFrame->parent;
    }
    printTreeValue(expr);
    error(5);
    return expr;
}

Value *evalEach(Value *args, Frame *frame){
    printf("In evalEach\n");
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
    if (length(args) != 2){
        error(3);
    }
    printf("In evalDefine, car(args): ");
    printTreeValue(car(args));
    printf("\ncdr(args): ");
    printTree(cdr(args));
    printf("\n");
    Value *boop = cons(car(args), eval(cdr(args), frame));
    printf("cdr(boop)->type should be Closure (15): %i\n", cdr(boop)->type);
    frame->bindings = cons(boop, frame->bindings);
    printf("New bindings on the frame: ");
    printBindings(frame->bindings);
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

Value *apply(Value *function, Value *args){
    printf("function type: %i\n", function->type);
    assert(function->type == CLOSURE_TYPE);
    assert(length(args) == length(function->cl.paramNames));
    printf("I made it to apply()\n");
    printf("======== applying ========\n");
    Frame *frame = talloc(sizeof(Frame));
    frame->parent = function->cl.frame;
    frame->bindings = makeNull();
    Value *current = args;
    Value *currentParam = function->cl.paramNames;
    while (current->type != NULL_TYPE){
        Value *param = cons(car(currentParam), car(current));
        printf("current param: ");
        printTreeValue(car(currentParam));
        printf(", ");
        printTreeValue(car(current));
        printf("\n");
        frame->bindings = cons(param, frame->bindings);
        current = cdr(current);
        currentParam = cdr(currentParam);
    }
    printf("functionCode: ");
    if (function->cl.functionCode->type == CONS_TYPE){
        printTree(function->cl.functionCode);
    } else {
        printTreeValue(function->cl.functionCode);
    }
    printf("\n");
    Value *result = eval(function->cl.functionCode, frame);
    if (result->type == CONS_TYPE){
        printTree(result);
    } else {
        printTreeValue(result);
    }
    printf("\n");
    printf("======== done ========\n");

    return result;
}

Value *eval(Value *expr, Frame *frame){
    printf("In eval! expr: ");
    if (expr->type == CONS_TYPE){
        printTree(expr);
    } else {
        printTreeValue(expr);
    }
    printf("\n");
    switch (expr->type){
        case INT_TYPE: {
            printf("It's an int.\n");
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
            printf("Cons_type !\n");
            Value *temp = car(expr);
            if (temp->type != CONS_TYPE){
                if (length(expr) > 1){
                    temp = expr;
                }
                else{
                    printf("Temp isn't a cons type\n");
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
                printf("Found a define! this is what we will define: ");
                printTreeValue(car(args));
                printf("\n");
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
                printf("This function is not a special form. It's called:  ");
                printTreeValue(first);
                printf("\n");
                Value *evaledOperator = eval(first, frame);
                printf("Done with evaledOperator\n");
                Value *evaledArgs = evalEach(args, frame);
                return apply(evaledOperator, evaledArgs);
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

void interpret(Value *tree){
    Frame *frame = talloc(sizeof(Frame));
    frame->parent = NULL;
    frame->bindings = makeNull();
    Value *current = tree;
    while (current->type != NULL_TYPE){
        Value *answer = eval(current, frame);
        current = cdr(current);
        printf("FINAL ANSWER\n");
        if (answer->type == CONS_TYPE){
            printTree(answer);
        } else {
            printTreeValue(answer);
        }
        //frame->bindings = makeNull();
        printf("\n");
    }
}
