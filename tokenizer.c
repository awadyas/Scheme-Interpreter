#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "value.h"
#include "linkedlist.h"
#include "talloc.h"

void error(){
    printf("Syntax error: untokenizeable\n");
    texit(1);
}

int isSymbol(char charRead){
    char *symbols = "%%!$&*/:<=>?~_^.+-";
    if (isalpha(charRead) || isdigit(charRead)){
        return 1;
    } else {
        for (int i = 0; i < strlen(symbols); i++){
            if (charRead == symbols[i]){
                return 1;
            }
        }
    }
    return 0;
}

char *doSymbol(char charRead){
    char *sym = talloc(sizeof(char)*300);
    int length = 0;
    while (charRead != ' ' && charRead != EOF){
        if (isSymbol(charRead)){
            sym[length] = charRead;
            length++;
        } else {
            error();
        }
        charRead = (char)fgetc(stdin);
    }
    sym[length] = '\0';
    return sym;
}

Value *doFloatOrInt(char charRead, Value *value){
    char *num = talloc(sizeof(char)*300);
    int length = 0;
    int isFloat = 0;
    while (isdigit(charRead) || charRead == '.'){
        if (charRead == '.'){
            if (isFloat == 1){
                error();
            }
            isFloat = 1;
            num[length] = charRead;
            length++;
        } else if (isdigit(charRead)){
            num[length] = charRead;
            length++;
        } else {
            error();
        }
        charRead = (char)fgetc(stdin);
    }
    num[length] = '\0';
    ungetc(charRead, stdin);

    if (isFloat == 1){
        value->type = DOUBLE_TYPE;
        value->d = strtod(num, NULL);
    } else if (isFloat == 0){
        value->type = INT_TYPE;
        value->i = strtol(num, NULL, 0);
    } else {
        error();
    }
    return value;
}

char *doString(char charRead){
    char *str = talloc(sizeof(char)*300);
    str[0] = charRead;
    int length = 1;
    charRead = (char)fgetc(stdin);
    while (charRead != '"'){
      if (charRead == EOF){
          error();
      }
      str[length] = charRead;
      length++;
      charRead = (char)fgetc(stdin);
    }
    str[length] = charRead;
    str[length+1] = '\0';
    return str;
}

Value *tokenize() {
    char charRead;
    Value *list = makeNull();
    charRead = (char)fgetc(stdin);

    while (charRead != EOF) {
        Value *value = talloc(sizeof(Value));
        if (charRead == '(') {
            value->type = OPEN_TYPE;
            list = cons(value, list);
        } else if (charRead == ')') {
            value->type = CLOSE_TYPE;
            list = cons(value, list);
        } else if (charRead == '#'){
            value->type = BOOL_TYPE;
            char boo = (char)fgetc(stdin);
            if (boo == 'f'){
                value->i = 0;
            } else if (boo == 't') {
                value->i = 1;
            } else {
                error();
            }
            list = cons(value, list);
        } else if (isdigit(charRead) || charRead == '+' || charRead == '-'){
            char next = (char)fgetc(stdin);
            ungetc(next, stdin);
            if (next == ' '){
                value->type = SYMBOL_TYPE;
                char *str = talloc(sizeof(char)+1);
                str[0] = charRead;
                str[1] = '\0';
                value->s = str;
                list = cons(value, list);
            } else {
                value = doFloatOrInt(charRead, value);
                list = cons(value, list);
            }
        } else if (isSymbol(charRead)){
            char *sym = doSymbol(charRead);
            value->type = SYMBOL_TYPE;
            value->s = sym;
            list = cons(value, list);
        } else if (charRead == '"'){
            char *str = doString(charRead);
            value->type = STR_TYPE;
            value->s = str;
            list = cons(value, list);
        } else if (charRead == ';'){
            while (charRead != '\n'){
                charRead = (char)fgetc(stdin);
            }
        } else if (charRead == '[') {
            value->type = OPENBRACKET_TYPE;
            list = cons(value, list);
        } else if (charRead == ']') {
            value->type = CLOSEBRACKET_TYPE;
            list = cons(value, list);
        } else if (charRead == '\''){
            value->type = SINGLEQUOTE_TYPE;
            list = cons(value, list);

        } else if (charRead == ' '){
            char next = (char)fgetc(stdin);
            char second = (char)fgetc(stdin);
            if (next == '.' && second == ' '){
                value->type = DOT_TYPE;
                list = cons(value, list);
            }
            else{
                ungetc(second,stdin);
                ungetc(next, stdin);
            }
        } else if (charRead != ' ' && charRead != '\n'){
            error();
        }
        charRead = (char)fgetc(stdin);
    }

    Value *revList = reverse(list);
    return revList;
}

void printToken(Value *val){
    switch (val->type){
            case INT_TYPE:
                printf("%i :int\n", val->i);
                break;
            case DOUBLE_TYPE:
                printf("%f :double\n", val->d);
                break;
            case STR_TYPE:
                printf("%s :string\n", val->s);
                break;
            case BOOL_TYPE:
                if (val->i == 0){
                    printf("#f :boolean\n");
                }
                printf("#t :boolean\n");
                break;
            case SYMBOL_TYPE:
                printf("%s :symbol\n", val->s);
                break;
            case OPEN_TYPE:
                printf("( :open\n");
                break;
            case CLOSE_TYPE:
                printf(") :close\n");
                break;
            case OPENBRACKET_TYPE:
                printf("[ :open bracket\n");
                break;
            case CLOSEBRACKET_TYPE:
                printf("] :close bracket\n");
                break;
            case SINGLEQUOTE_TYPE:
                printf("' :single quote\n");
                break;
            case DOT_TYPE:
                printf(". :dot\n");
                break;
            default:
                break;
        }
}

void displayTokens(Value *list){
    while (list->c.cdr->type == CONS_TYPE){
        printToken(list->c.car);
        list = list->c.cdr;
    }
    printToken(list->c.car);
}
