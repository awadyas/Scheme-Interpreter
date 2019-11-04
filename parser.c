#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"
#include "assert.h"
#include "parser.h"

void parseError(int num){
    if (num == 0){
        printf("Syntax Error: Untokenizable\n");
    } else if (num == 1){
        printf("Parsing Error: Not enough close parentheses\n");
    } else if (num == 2){
        printf("Parsing Error: Too many close parentheses\n");
    }
    texit(1);
}

Value *makeSubTree(Value *tree){
    Value *subTree = makeNull();
    while (car(tree)->type != OPEN_TYPE && tree->type != NULL_TYPE){
        subTree = cons(tree->c.car, subTree);
        tree = tree->c.cdr;
    }
    if (tree->type == NULL_TYPE){
        parseError(2);
    }
    tree = tree->c.cdr;
    tree = cons(subTree, tree);
    return tree;
}

Value *addToTree(Value *tree, int *depth, Value *tokenToAdd){
    if (tokenToAdd->type == OPEN_TYPE){
        (*depth)++;
    }
    if (tokenToAdd->type == CLOSE_TYPE){
        (*depth)--;
        return makeSubTree(tree);
    }
    return cons(tokenToAdd, tree);
}

Value *parse(Value *tokens){
    Value *tree = makeNull();
    int depth = 0;
    Value *current = tokens;
    assert(current != NULL && "Error (parse): null pointer");
    while (current->type != NULL_TYPE){
        Value *token = car(current);
        tree = addToTree(tree, &depth, token);
        current = cdr(current);
    }
    if (depth != 0){
        parseError(1);
    }
    return reverse(tree);
}

void printTree(Value *tree){
    switch (tree->type){
        case INT_TYPE:
            printf("%i", tree->i);
            break;
        case DOUBLE_TYPE:
            printf("%f", tree->d);
            break;
        case STR_TYPE:
            printf("%s", tree->s);
            break;
        case PTR_TYPE:
            printf("%p", tree->p);
            break;
        case CONS_TYPE:
            if (car(tree)->type == CONS_TYPE){
                printf("(");
            }
            printTree(car(tree));

            if (car(tree)->type != CONS_TYPE && cdr(tree)->type != NULL_TYPE){
                printf(" ");
            }
            if (car(tree)->type == CONS_TYPE){
                printf(")");
            }
            printTree(cdr(tree));
            break;
        case BOOL_TYPE:
            if (tree->i == 0){
                printf("#t");
            } else {
                printf("#f");
            }
            break;
        case SYMBOL_TYPE:
            printf("%s", tree->s);
            break;
        case NULL_TYPE:
            break;
        case OPEN_TYPE:
            break;
        case CLOSE_TYPE:
            break;
        case OPENBRACKET_TYPE:
            break;
        case CLOSEBRACKET_TYPE:
            break;
    }
}
