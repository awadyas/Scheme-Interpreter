#include "value.h"
#include "interpreter.h"
#include "talloc.h"
#include "parser.h"
#include "linkedlist.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


Value *eval(Value *expr, Frame *frame);

void error(int status){
    switch (status){
        case 1: {
            printf("Error! :(\n" );
        }
        case 2: {
            printf("Evaluation error: not a recognized special form.\n");
        }
        case 3: {
            printf("Error: incorrect number of arguments.\n");
        }
        case 4: {
            printf("Error: first argument in IF should be a boolean\n");
        }
        case 5: {
            printf("Error: undefined symbol\n");
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
        return eval(cdr(args), frame);
    } else {
        return eval(cdr(cdr(args)), frame);
    }
}

Value *evalLet(Value *args, Frame *frame){
    printf("In evalLet!\n");
    if (length(args) != 2){
        error(3);
    }
    printf("Car of args: ");
    printTree(car(args));
    Value *inner = car(car(args));
    printf("\nCar of car of args (aka \"inner\"): ");
    printTree(car(car(args)));
    printf("\nType of \"inner\": %i\n", car(car(args))->type);
    printf("I can take the cdr of inner: ");
    printTree(cdr(inner));
    printf("\n");
    printf("I should be able to take car(inner). Here is it's type: ");
    printf("%i", inner->c.car->type);
    //printTree(inner->c.car);
    printf("hhhhhhh\n");

    printf("Cdr of args: ");
    printTree(cdr(args));
    printf("\n");

    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = frame;
    newFrame->bindings = makeNull();
    while (args->type != NULL_TYPE){
        assert(car(car(car(args)))->type == SYMBOL_TYPE);
        Value *value = car(car(args));
        printf("Value: ");
        printTree(value);
        printf("\n");
        printf("Car(value)->s: %s\n", car(value)->s);
        printf("value->type: %i\n", value->type);
        Value *evalu = eval(car(cdr(car(args))), frame);
        printf("Evaluated :");
        printTree(evalu);
        printf("\n");
        Value *cell = cons(value, evalu);
        newFrame->bindings = cons(cell, newFrame->bindings);
    }
    return eval(cdr(args), newFrame);
}


Value *lookUpSymbol(Value *expr, Frame *frame){
    Frame *currentFrame = frame;
    while (currentFrame->parent != NULL){
        Value *currentBinding = currentFrame->bindings;
        while (currentBinding->type != NULL_TYPE){
            if (strcmp(car(car(currentBinding))->s, expr->s)){
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
    printf("We are now in eval(), evaluating this: ");
    printTree(expr);
    printf("\n");
    switch (expr->type){
        case INT_TYPE: {
            return expr;
            break;
        }
        case SYMBOL_TYPE: {
            printf("switch: SYMBOL_TYPE\n");
            return lookUpSymbol(expr, frame);
            break;
        }
        case STR_TYPE: {
            return expr;
            break;
        }
        case CONS_TYPE: {
            printf("switch: CONS_TYPE\n");
            Value *first = car(expr);
            Value *args = cdr(expr);
            Value *result;
            //printf("cons\n");
            //printTree(expr);
            //printf("First's type: %i\n", first->type);
            //display(expr);
            //printf("\n");
            //display(car(cdr(expr)));
            //printf("First->s: ");
            //printf("%s\n", first->s);

            assert(first->type == SYMBOL_TYPE);
            //printf("Args: ");
            //printTree(args);
            //printf("\n");

            if (!strcmp(first->s, "if")){
                result = evalIf(args, frame);
            } else if (!strcmp(first->s, "let")){
                printf("First->s is let, let's do evalLet\n");
                result = evalLet(args, frame);
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
                printf("%s", value->s);
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
        printf("Evaluating... ");
        printTree(car(current));
        printf("\n");
        eval(car(current), frame);
        current = cdr(current);
    }
}





//
