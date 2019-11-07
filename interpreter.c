#include "value.h"
#include "interpreter.h"
#include "talloc.h"
#include "parser.h"
#include "linkedlist.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//Value *eval(Value *expr, Frame *frame);

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
    printf("I made it to evalIf! :)\n");
    if (length(args) != 3){
        error(3);
    }
    Value *condition = eval(car(args), frame);
    if (condition->type != BOOL_TYPE){
        error(4);
    }
    if (condition->i){
        printf("Evaluating #t\n");
        return eval(car(cdr(args)), frame);
    } else {
        printf("Evaluating #f\n");
        return eval(car(cdr(cdr(args))), frame);
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
    //printf("\nhhhhhhh\n");

    printf("Cdr of args: ");
    printTree(cdr(args));
    printf("\n");

    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = frame;
    newFrame->bindings = makeNull();
    Value *currentVal = args;
    while (cdr(currentVal)->type != NULL_TYPE){
        printf("I made a new Frame. currentVal is: ");
        printTree(currentVal);
        printf("\n");
        assert(car(car(car(currentVal)))->type == SYMBOL_TYPE);
        Value *value = car(car(currentVal));
        printf("Value: ");
        printTree(value);
        printf("\n");
        printf("Car(value)->s: %s\n", car(value)->s);
        printf("Cdr(value)->i: %i\n", car(cdr(value))->i);
        printf("value->type: %i\n", value->type);
        printf("car(cdr(value)) type:  %i\n", car(cdr(value))->type);
        //Value *evalu = eval(car(cdr(car(args))), frame);
        Value *evalu = eval(car(cdr(value)), frame);
        printf("Evaluated : ");
        //printTree(evalu);
        printf("\n");
        Value *cell = cons(car(value), evalu);
        printf("These are the symbols that I am storing: ");
        printTreeValue(car(cell));
        printTreeValue(cdr(cell));
        //printTree(cell);
        newFrame->bindings = cons(cell, newFrame->bindings);
        currentVal = cdr(currentVal);
        printf("I made it to the end of the loop!\n");
    }
    return eval(cdr(args), newFrame);
}


Value *lookUpSymbol(Value *expr, Frame *frame){
    printf("I am in lookUpSymbol\n");
    Frame *currentFrame = frame;
    while (currentFrame->parent != NULL){
        Value *currentBinding = currentFrame->bindings;
        printTreeValue(currentBinding);
        while (currentBinding->type != NULL_TYPE){
            printf("currentBinding: \n");
            printf("current binding type : %i\n", currentBinding->type);
            printf("current car(binding) type : %i\n", car(currentBinding)->type);
            printf("current car(car(binding)) type : %i\n", car(car(currentBinding))->type);
            //printTreeValue(car(car(currentBinding)));
            printf("\n");
            printf("expr->s: %s\n", expr->s);
            if (!strcmp(car(car(currentBinding))->s, expr->s)){
                printf("I found my value!\n");
                printf("My value: %i\n", cdr(car(currentBinding))->i);
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
    if (expr->type == CONS_TYPE){
        printTree(expr);
        printf("\n");
    }
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
            Value *temp = car(expr);
            if (temp->type == CONS_TYPE){
                printTree(temp);
                printf("\n");
            }
            if (temp->type != CONS_TYPE){
                printf("Temp isn't CONS\n");
                return eval(temp, frame);
            }
            Value *first = car(temp);
            if (first->type == CONS_TYPE){
                printTree(first);
                printf("\n");
   
            }
            Value *args = cdr(temp);
            Value *result;
            //printf("cons\n");
            //printTree(expr);
            printf("First's type: %i\n", first->type);
            //printf("car()");
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
                printf("Hello I recognized the if!\n");
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
            printf("\nmade it to bool\n");
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
        //printTree(car(current));
        printf("\n");
        //eval(car(current), frame);
        Value *answer = eval(current, frame);
        printf("ANSWER!!! : \n");
        //need to print answer
        current = cdr(current);
        printf("ANSWER!!! : \n");
        printTreeValue(answer);
        printf("\n");
    }
}





//
