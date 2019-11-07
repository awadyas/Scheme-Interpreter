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
    error(5);
    return expr;
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
            } else {
                error(2);
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
