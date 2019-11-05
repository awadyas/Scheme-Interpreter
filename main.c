#include <stdio.h>
#include "tokenizer.h"
#include "value.h"
#include "linkedlist.h"
#include "parser.h"
#include "talloc.h"
#include "interpreter.h"

int main() {

    Value *list = tokenize(stdin);
    Value *tree = parse(list);
    printf("Parsing complete (in main.c)\n");
    //displayTokens(list);

    //printTree(tree);
    interpret(tree);
    printf("Interpretation complete (in main.c)\n");
    tfree();
    return 0;
}
