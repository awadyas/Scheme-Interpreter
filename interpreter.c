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
            default:
                break;
        }
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
    while (currentFrame->parent != NULL){
        Value *currentBinding = currentFrame->bindings;
        while (currentBinding->type != NULL_TYPE){
            if (!strcmp(car(car(currentBinding))->s, expr->s)){
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
    Value *evaledArgs = makeNull();
    Value *current = args;
    while (current->type != NULL_TYPE){
        evaledArgs = cons(eval(car(current), frame), evaledArgs);
        current = cdr(args);
    }
    reverse(evaledArgs);
    return evaledArgs;
}

Value *evalDefine(Value *args, Frame *frame){
    if (length(args) != 2){
        error(3);
    }
    printTreeValue(car(args));
    printf("\n");
    Value *boop = cons(car(args), eval(cdr(args), frame));
    frame->bindings = cons(boop, frame->bindings);
    printf("%i\n", frame->bindings->type);
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
    assert(function->type == CLOSURE_TYPE);
    assert(length(args) == length(function->cl.paramNames));
    printf("I made it to apply()\n");
    Frame *frame = talloc(sizeof(Frame));
    frame->parent = function->cl.frame;
    frame->bindings = makeNull();
    
    Value *current = args;
    Value *currentParam = function->cl.paramNames;
    while (current->type != NULL_TYPE){
        Value *param = cons(car(currentParam), car(current)); 
        frame->bindings = cons(param, frame->bindings);
        current = cdr(current);
        currentParam = cdr(currentParam);
    }
    return eval(function->cl.functionCode, frame);
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
                return eval(temp, frame);
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
                result = args;
            } else if(!strcmp(first->s, "define")){
                result = evalDefine(args, frame);
            } else if (!strcmp(first->s, "lambda")){
                result = evalLambda(args, frame);
            } else {
                printf("Not a defined function - we make one!\n");
                Value *evaledOperator = eval(first, frame);
                Value *evaledArgs = evalEach(args, frame);
                return apply(evaledOperator,evaledArgs);
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
        if (answer->type == CONS_TYPE){
            printTree(answer);
        } else {
            printTreeValue(answer);
        }
        printf("\n");
    }
}
