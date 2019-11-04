#include <stdio.h>
#include "tokenizer.h"
#include "value.h"
#include "linkedlist.h"
#include "parser.h"
#include "talloc.h"
#include "interpreter.h"

int main() {

    Value *list = tokenize(stdin);
    //printf("tokenized\n");
    Value *tree = parse(list);
    printf("parsed\n");
    //displayTokens(list);
    //printTree(tree);
    interpret(tree);

    tfree();
    return 0;
}
