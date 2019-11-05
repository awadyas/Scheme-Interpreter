#include <stdio.h>
#include "tokenizer.h"
#include "value.h"
#include "linkedlist.h"
#include "parser.h"
#include "talloc.h"
#include "interpreter.h"

int main() {

    Value *list = tokenize(stdin);
    printf("Type of what should be let: %i\n", car(cdr(list))->type);
    //printf("tokenized\n");
    Value *tree = parse(list);
    printf("Parsing complete (in main.c)\n");
    displayTokens(list);

    //printTree(tree);
    interpret(tree);
    printf("Interpretation complete (in main.c)\n");
    tfree();
    return 0;
}
