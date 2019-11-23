#include "value.h"
#include "interpreter.h"
#include "talloc.h"
#include "parser.h"
#include "linkedlist.h"
#include "tokenizer.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Value *evalLambda(Value *args, Frame *frame);
void printTree2(Value *tree);
Value *apply(Value *function, Value * args);

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
        case 6: {
            printf("Error: non-number value passed to arithmetic function\n");
            break;
        }
        case 7: {
            printf("Error: equal? cannot compare this kind of value\n");
            break;
        }
        case 8: {
            printf("Error: append's arguments must be lists\n");
            break;
        }
        case 9: {
            printf("Error: parameter must be a boolean\n");
            break;
        }
        case 10: {
            printf("Error: bad syntax - 'else' clause must be last\n");
            break;
        }
        case 11: {
            printf("Error: filename must be a string\n");
            break;
        }
        case 12: {
            printf("Error: file does not exist\n");
            break;
        }
        case 13: {
            printf("Error: cannot define variable twice\n");
            break;
        }
        case 14: {
            printf("Error: Parameters must be symbols\n");
            break;
        }
        case 15: {
            printf("Error: Cannot use variable before initialization\n");
            break;
        }
    }
    texit(1);
}

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
                printf("#<procedure>");
                break;
            case VOID_TYPE:
                break;
            case PRIMITIVE_TYPE:
                printf("Primitive function\n");
                break;
            default:
                break;
        }
}

void printTree2(Value *tree){
    if (tree->type != CONS_TYPE){
        printTreeValue(tree);
    } else {
        Value *current = tree;
        while (current->type != NULL_TYPE){
            if (car(current)->type == CONS_TYPE){
                printf("(");
                printTree2(car(current));
                printf(")");
            } else {
                printTreeValue(car(current));
                if (cdr(current)->type == CONS_TYPE){
                    printf(" ");
                } else if (cdr(current)->type != NULL_TYPE){
                    printf(" . ");
                    printTreeValue(cdr(current));
                    break;
                }
            }
            current = cdr(current);
        }
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

Value *evalCond(Value *args, Frame *frame){
    if (length(args) == 0){
        error(3);
    }
    Value *current = args;
    Value *boolean;
    while (current->type != NULL_TYPE){
        if (car(car(current))->type == SYMBOL_TYPE && !strcmp(car(car(current))->s, "else")){
            if (cdr(current)->type != NULL_TYPE){
                error(10);
            }
            return eval(cdr(car(current)), frame);
        } else {

            boolean = eval(car(car(current)), frame);
            if (boolean->type != BOOL_TYPE){
                error(9);
            } else if (boolean->i){
                return eval(cdr(car(current)), frame);
            }
        }
        current = cdr(current);
    }
    Value *voidVal = talloc(sizeof(Value));
    voidVal->type = VOID_TYPE;
    return voidVal;
}

Value *evalLet(Value *args, Frame *frame){
    if (length(args) != 2){
        error(3);
    }
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = frame;
    newFrame->bindings = makeNull();
    Value *currentVal = car(args);
    while (currentVal->type != NULL_TYPE){
        assert(car(car(currentVal))->type == SYMBOL_TYPE);
        Value *currentBinding = newFrame->bindings;
        while (currentBinding->type != NULL_TYPE){
            if (!strcmp(car(car(currentBinding))->s, car(car(currentVal))->s)){
                error(13);
            }
            currentBinding = cdr(currentBinding);
        }
        Value *value = car(currentVal);
        Value *evalu = eval(cdr(value), frame);
        Value *cell = cons(car(value), evalu);
        newFrame->bindings = cons(cell, newFrame->bindings);
        currentVal = cdr(currentVal);
    }
    return eval(cdr(args), newFrame);
}

Value *evalLetStar(Value *args, Frame *frame){
    if (length(args) != 2){
        error(3);
    }
    Frame *oldFrame = frame;
    Frame *newFrame;
    Value *current = car(args);
    while (current->type != NULL_TYPE){
        newFrame = talloc(sizeof(Frame));
        newFrame->parent = oldFrame;
        newFrame->bindings = makeNull();

        assert(car(car(current))->type == SYMBOL_TYPE);
        Value *value = car(current);
        Value *evalu = eval(cdr(value), newFrame);
        Value *cell = cons(car(value), evalu);
        newFrame->bindings = cons(cell, newFrame->bindings);
        current = cdr(current);
        oldFrame = newFrame;
    }
    return eval(cdr(args), newFrame);
}

Value *evalLetRec(Value *args, Frame *frame){
    if (length(args) != 2){
        error(3);
    }
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = frame;
    newFrame->bindings = makeNull();
    Value *currentVal = car(args);
    while (currentVal->type != NULL_TYPE){
        Value *voidVal = talloc(sizeof(Value));
        voidVal->type = VOID_TYPE;
        assert(car(car(currentVal))->type == SYMBOL_TYPE);
        Value *value = car(currentVal);
        Value *cell = cons(car(value), voidVal);
        newFrame->bindings = cons(cell, newFrame->bindings);
        currentVal = cdr(currentVal);
    }
    currentVal = car(args);
    while (currentVal->type != NULL_TYPE){
        Value *value = car(currentVal);
        Value *evalu = eval(cdr(value), newFrame);
        if (evalu->type == VOID_TYPE){
            error(15);
        }
        Value *currentBinding = newFrame->bindings;
        while (currentBinding->type != NULL_TYPE){
            evalu = eval(cdr(value), newFrame);
            if (!strcmp(car(car(currentBinding))->s, car(value)->s)){
                car(currentBinding)->c.cdr = evalu;
                break;
            }
            currentBinding = cdr(currentBinding);
        }
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
                return cdr(car(currentBinding));
            }
            currentBinding = cdr(currentBinding);
        }
        currentFrame = currentFrame->parent;
    }
    error(5);
    return expr;
}

Value *evalEach(Value *args, Frame *frame){
    Value *current = args;
    Value *evaledArgs = makeNull();
    while (current->type != NULL_TYPE){
        Value *firstEval = eval(car(current), frame);
        if (firstEval->type == CLOSURE_TYPE && car(current)->type == CONS_TYPE && length(car(current))==1){
            firstEval = apply(firstEval, cdr(car(current)));
        }
        evaledArgs = cons(firstEval, evaledArgs);
        current = cdr(current);
    }
    evaledArgs = reverse(evaledArgs);
    return evaledArgs;
}

Value *evalDefine(Value *args, Frame *frame){
    if (length(args) != 2 && length(args) != 3){
        error(3);
    }
    if (length(args) == 2){
        Value *boop = cons(car(args), eval(cdr(args), frame));
        frame->bindings = cons(boop, frame->bindings);
    } else {
        Value *boop = cons(car(args), evalLambda(cdr(args), frame));
        frame->bindings = cons(boop, frame->bindings);
    }
    Value *voidboi = talloc(sizeof(Value));
    voidboi->type = VOID_TYPE;
    return voidboi;
}

Value *evalSet(Value *args, Frame *frame){
    if (length(args) != 2){
        error(3);
    }
    Value *variable = car(args);
    Value *newVal = eval(car(cdr(args)) ,frame);
    Value *voidboi = talloc(sizeof(Value));
    voidboi->type = VOID_TYPE;
    Frame *currentFrame = frame;
    while (currentFrame != NULL){
        Value *currentBinding = currentFrame->bindings;
        while (currentBinding->type != NULL_TYPE){
            if (!strcmp(car(car(currentBinding))->s, variable->s)){
                car(currentBinding)->c.cdr = newVal;
                return voidboi;
            }
            currentBinding = cdr(currentBinding);
        }
        currentFrame = currentFrame->parent;
    }
    error(5);
    return voidboi;
}

Value *evalBegin(Value *args, Frame *frame){
    if (length(args) < 1){
        error(3);
    }
    Value *currentVal = args;
    Value *answer;
    while (currentVal->type != NULL_TYPE){
        answer = eval(currentVal, frame);
        currentVal = cdr(currentVal);
    }
    return answer;
}

Value *evalLambda(Value *args, Frame *frame){
    if (length(args) != 2){
        error(3);
    }
    Value *currentParamName = car(args);
    while (currentParamName->type != NULL_TYPE){
        if (car(currentParamName)->type != SYMBOL_TYPE){
            error(14);
        }
        currentParamName = cdr(currentParamName);
    }
    Value *closure = talloc(sizeof(Value));
    closure->type = CLOSURE_TYPE;
    closure->cl.frame = frame;
    closure->cl.paramNames = car(args);
    closure->cl.functionCode = cdr(args);
    return closure;
}

Value *evalAnd(Value *args, Frame *frame){
    if (length(args) != 2){
        error(3);
    }
    Value *first = eval(car(args), frame);
    Value *second = eval(car(cdr(args)), frame);
    if (first->type != BOOL_TYPE || second->type != BOOL_TYPE){
        error(9);
    }
    Value *ret = talloc(sizeof(Value));
    ret->type = BOOL_TYPE;
    if (first->i && second->i){
        ret->i = 1;
    } else {
        ret->i = 0;
    }
    return ret;
}

Value *evalOr(Value *args, Frame *frame){
    if (length(args) != 2){
        error(3);
    }
    Value *first = eval(car(args), frame);
    Value *second = eval(car(cdr(args)), frame);
    if (first->type != BOOL_TYPE || second->type != BOOL_TYPE){
        error(9);
    }
    Value *ret = talloc(sizeof(Value));
    ret->type = BOOL_TYPE;
    if (first->i || second->i){
        ret->i = 1;
    } else {
        ret->i = 0;
    }
    return ret;
}

Value *applyPrimitive(Value *function, Value *args){
    assert(function->type == PRIMITIVE_TYPE);
    Value *ret = (function->pf)(args);
    return ret;
}

Value *apply(Value *function, Value *args){
    assert(function->type == CLOSURE_TYPE);
    if (function->cl.paramNames->type == CONS_TYPE){
        assert(length(args) == length(function->cl.paramNames));
    }
    Frame *frame = talloc(sizeof(Frame));
    frame->parent = function->cl.frame;
    frame->bindings = makeNull();
    Value *current = args;
    Value *currentParam = function->cl.paramNames;
    if (currentParam->type == SYMBOL_TYPE) {
        Value *param = cons(currentParam, current);
        frame->bindings = cons(param, frame->bindings);
    }
    else {
        while (current->type != NULL_TYPE && currentParam->type != NULL_TYPE){
            Value *param = cons(car(currentParam), car(current));
            frame->bindings = cons(param, frame->bindings);
            current = cdr(current);
            currentParam = cdr(currentParam);
        }
    }
    Value *result = eval(function->cl.functionCode, frame);

    return result;
}

void loadFile(Value *args){
    if (args->type != STR_TYPE){
        error(11);
    }
    FILE *file = fopen(args->s, "r");
    if (file == NULL){
        error(12);
    }
    fclose(file);
    char *script = talloc(strlen("./interpreter < ") + strlen(args->s) + 1);
    strcpy(script, "./interpreter < ");
    strcat(script, args->s);
    if (system(script)){
        error(1);
    }
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
                if (length(expr) > 1){
                    temp = expr;
                } else{
                    Value *tempEval = eval(temp,frame);
                    if (tempEval->type == CLOSURE_TYPE){
                        Value *nullArgs = talloc(sizeof(Value));
                        nullArgs->type = CONS_TYPE;
                        tempEval = apply(tempEval, nullArgs);
                    }
                    return tempEval;
                }
            }
            Value *first = car(temp);
            Value *args = cdr(temp);
            Value *result;
            if (!strcmp(first->s, "if")){
                result = evalIf(args, frame);
            } else if (!strcmp(first->s, "let")){
                result = evalLet(args, frame);
            } else if (!strcmp(first->s, "quote")){
                if (length(args) != 1){
                    error(3);
                }
                result = car(args);
            } else if(!strcmp(first->s, "define")){
                result = evalDefine(args, frame);
                if (result->type != VOID_TYPE){
                    printf("Error within define: ");
                    if (result->type == CONS_TYPE){
                        printTree(result);
                    } else {
                        printTreeValue(result);
                    }
                    printf("\n");
                    texit(1);
                }
            } else if (!strcmp(first->s, "lambda")){
                result = evalLambda(args, frame);
            } else if (!strcmp(first->s, "let*")){
                result = evalLetStar(args, frame);
            } else if (!strcmp(first->s, "letrec")){
                result = evalLetRec(args, frame);
            } else if (!strcmp(first->s, "set!")){
                result = evalSet(args, frame);
            } else if (!strcmp(first->s, "begin")){
                result = evalBegin(args, frame);
            } else if (!strcmp(first->s, "cond")){
                result = evalCond(args, frame);
            } else if (!strcmp(first->s, "and")){
                result = evalAnd(args, frame);
            } else if (!strcmp(first->s, "or")){
                result = evalOr(args, frame);
            } else if (!strcmp(first->s, "load")){
                Value *voidVal = talloc(sizeof(Value));
                voidVal->type = VOID_TYPE;
                result = voidVal;
                if (length(args) != 1){
                    error(3);
                }
                loadFile(car(args));
            } else {
                Value *evaledOperator = eval(first, frame);
                Value *evaledArgs = evalEach(args, frame);
                if (evaledOperator->type == CLOSURE_TYPE){
                    return apply(evaledOperator, evaledArgs);
                } else if (evaledOperator->type == PRIMITIVE_TYPE){
                    return applyPrimitive(evaledOperator, evaledArgs);
                } else {
                    error(1);
                }
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

Value *primitiveAdd(Value *args){
    Value *ret = talloc(sizeof(Value));
    ret->type = DOUBLE_TYPE;
    if (args->type == NULL_TYPE){
        ret->d = 0;
        return ret;
    }
    double total = 0;
    while (args->type != NULL_TYPE){
        if (car(args)->type == INT_TYPE){
            total += car(args)->i;
        } else if (car(args)->type == DOUBLE_TYPE){
            total += car(args)->d;
        } else {
            error(6);
        }
        args = cdr(args);
    }
    ret->d = total;
    return ret;
}

Value *primitiveMultiply(Value *args){
    if (length(args) < 2){
        error(3);
    }
    Value *ret = talloc(sizeof(Value));
    ret->type = DOUBLE_TYPE;
    double total = 1;
    while (args->type != NULL_TYPE){
        if (car(args)->type == INT_TYPE){
            total *= car(args)->i;
        } else if (car(args)->type == DOUBLE_TYPE){
            total *= car(args)->d;
        } else {
            error(6);
        }
        args = cdr(args);
    }
    ret->d = total;
    return ret;
}

Value *primitiveSubtract(Value *args){
    if (length(args) != 2){
        error(3);
    }
    Value *ret = talloc(sizeof(Value));
    ret->type = DOUBLE_TYPE;
    double total;
    Value *first = car(args);
    if (first->type == INT_TYPE){
        total = first->i;
    } else if (first->type == DOUBLE_TYPE){
        total = first->d;
    } else {
        error(6);
    }
    Value *second = car(cdr(args));
    if (second->type == INT_TYPE){
        total -= second->i;
    } else if (second->type == DOUBLE_TYPE){
        total -= second->d;
    } else {
        error(6);
    }
    ret->d = total;
    return ret;
}

Value *primitiveDivide(Value *args){
    if (length(args) != 2){
        error(3);
    }
    Value *first = car(args);
    Value *second = car(cdr(args));
    Value *ret = talloc(sizeof(Value));
    int itotal;
    double dtotal;
    if (first->type == INT_TYPE && second->type == INT_TYPE){
        itotal = (first->i / second->i);
        dtotal = ((double) first->i / (double) second->i);
        if (itotal == dtotal){
            ret->type = INT_TYPE;
            ret->i = itotal;
        } else {
            ret->type = DOUBLE_TYPE;
            ret->d = dtotal;
        }
    } else if (first->type == INT_TYPE){
        dtotal = (first->i / second->d);
        ret->type = DOUBLE_TYPE;
        ret->d = dtotal;
    } else if (first->type == DOUBLE_TYPE){
        dtotal = first->d;
        if (second->type == INT_TYPE){
            dtotal = dtotal / second->i;
        } else if (second->type == DOUBLE_TYPE){
            dtotal = dtotal / second->d;
        } else {
            error(6);
        }
        ret->type = DOUBLE_TYPE;
        ret->d = dtotal;
    } else {
        error(6);
    }
    return ret;
}

Value *primitiveLessThan(Value *args){
    if (length(args) != 2){
        error(3);
    }
    Value *ret = talloc(sizeof(Value));
    ret->type = BOOL_TYPE;
    Value *first = car(args);
    Value *second = car(cdr(args));
    double comp;
    if (first->type == INT_TYPE){
        comp = first->i;
    } else if (first->type == DOUBLE_TYPE){
        comp = first->d;
    } else {
        error(6);
    }
    if (second->type == INT_TYPE){
        if (comp < second->i){
            ret->i = 1;
        } else {
            ret->i = 0;
        }
    } else if (first->type == DOUBLE_TYPE){
        if (comp < second->d){
            ret->i = 1;
        } else {
            ret->i = 0;
        }
    } else {
        error(6);
    }
    return ret;
}

Value *primitiveGreaterThan(Value *args){
    if (length(args) != 2){
        error(3);
    }
    Value *ret = talloc(sizeof(Value));
    ret->type = BOOL_TYPE;
    Value *first = car(args);
    Value *second = car(cdr(args));
    double comp;
    if (first->type == INT_TYPE){
        comp = first->i;
    } else if (first->type == DOUBLE_TYPE){
        comp = first->d;
    } else {
        error(6);
    }
    if (second->type == INT_TYPE){
        if (comp > second->i){
            ret->i = 1;
        } else {
            ret->i = 0;
        }
    } else if (first->type == DOUBLE_TYPE){
        if (comp > second->d){
            ret->i = 1;
        } else {
            ret->i = 0;
        }
    } else {
        error(6);
    }
    return ret;
}

// Function for the literal '=' NOT 'equals?' - checks only numbers
Value *primitiveEquals(Value *args){
    if (length(args) != 2){
        error(3);
    }
    Value *first = car(args);
    Value *second = car(cdr(args));
    Value *ret = talloc(sizeof(Value));
    ret->type = BOOL_TYPE;
    double comp;
    if (first->type == INT_TYPE){
        comp = (double) first->i;
    } else if (first->type == DOUBLE_TYPE){
        comp = first->d;
    } else {
        error(6);
    }
    if (second->type == INT_TYPE){
        if (comp == (double) second->i){
            ret->i = 1;
        } else {
            ret->i = 0;
        }
    } else if (second->type == DOUBLE_TYPE){
        if (comp == second->d){
            ret->i = 1;
        } else {
            ret->i = 0;
        }
    } else {
        error(6);
    }
    return ret;
}

Value *primitiveModulo(Value *args){
    if (length(args) != 2){
        error(3);
    }
    Value *first = car(args);
    Value *second = car(cdr(args));
    if (first->type != INT_TYPE || second->type != INT_TYPE){
        error(6);
    }
    Value *ret = talloc(sizeof(Value));
    ret->type = INT_TYPE;
    ret->i = (first->i % second->i);
    return ret;
}

Value *primitiveNull(Value *args){
    if (length(args) != 1){
        error(3);
    }
    Value *ret = talloc(sizeof(Value));
    ret->type = BOOL_TYPE;
    if (car(args)->type == NULL_TYPE){
        ret->i = 1;
    } else {
        ret->i = 0;
    }
    return ret;
}

Value *primitiveCar(Value *args){
    if (length(args) != 1){
        error(3);
    }
    return car(car(args));
}

Value *primitiveCdr(Value *args){
    if (length(args) != 1){
        error(3);
    }
    return cdr(car(args));
}

Value *primitiveCons(Value *args){
    if (length(args) != 2){
        error(3);
    }
    return cons(car(args), car(cdr(args)));
}

Value *primitiveEqual(Value *args){
    if (length(args) != 2){
        error(3);
    }
    Value *first = car(args);
    Value *second = car(cdr(args));
    Value *answer = talloc(sizeof(Value));
    answer->type = BOOL_TYPE;
    if (first->type == INT_TYPE){
        if (second->type != INT_TYPE){
           answer->i = 0;
        } else {
            if (first->i == second->i){
                answer->i = 1;
            } else {
                answer->i = 0;
            }
        }
    } else if (first->type == STR_TYPE){
        if (second->type != STR_TYPE){
           answer->i = 0;
        }
        else{
            if (!strcmp(first->s,second->s)){
                answer->i = 1;
            } else {
                answer->i = 0;
            }
        }
    } else if (first->type == DOUBLE_TYPE){
        if (second->type != DOUBLE_TYPE){
           answer->i = 0;
        } else {
            if (first->d == second->d){
                answer->i = 1;
            } else {
                answer->i = 0;
            }
        }
    } else {
        error(7);
    }
    return answer;
}

Value *primitiveList(Value *args){
    assert(args->type == CONS_TYPE);
    Value *ans = makeNull();
    while (args->type == CONS_TYPE){
        ans = cons(car(args), ans);
        args = cdr(args);
    }
    ans = reverse(ans);
    return ans;
}

Value *primitiveAppend(Value *args) {
    assert(args->type == CONS_TYPE);
    Value *ans = makeNull();
    while (args->type == CONS_TYPE){
        if (car(args)->type != CONS_TYPE){
            error(8);
        } else {
            Value *inner = car(args);
            while (inner->type != NULL_TYPE){
                ans = cons(car(inner), ans);
                inner = cdr(inner);
            }
        }
        args = cdr(args);
    }
    ans = reverse(ans);
    return ans;
}

void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
    Value *value = talloc(sizeof(Value));
    value->type = PRIMITIVE_TYPE;
    value->pf = function;
    Value *nameVal = talloc(sizeof(Value));
    nameVal->type = STR_TYPE;
    nameVal->s = name;
    Value *funVal = talloc(sizeof(Value));
    funVal->type = PRIMITIVE_TYPE;
    funVal->pf = function;
    Value *cell = cons(nameVal, funVal);
    frame->bindings = cons(cell, frame->bindings);
}

void interpret(Value *tree){
    Frame *frame = talloc(sizeof(Frame));
    frame->parent = NULL;
    frame->bindings = makeNull();

    bind("+", primitiveAdd, frame);
    bind("*", primitiveMultiply, frame);
    bind("/", primitiveDivide, frame);
    bind("-", primitiveSubtract, frame);
    bind("<", primitiveLessThan, frame);
    bind(">", primitiveGreaterThan, frame);
    bind("=", primitiveEquals, frame);
    bind("modulo", primitiveModulo, frame);
    bind("null?", primitiveNull, frame);
    bind("car", primitiveCar, frame);
    bind("cdr", primitiveCdr, frame);
    bind("cons", primitiveCons, frame);
    bind("equal?", primitiveEqual, frame);
    bind("list", primitiveList, frame);
    bind("append", primitiveAppend, frame);

    Value *current = tree;
    while (current->type != NULL_TYPE){
        Value *answer = eval(current, frame);
        current = cdr(current);
        if (answer->type != VOID_TYPE) {
            if (answer->type == CONS_TYPE){
                printf("(");
                printTree2(answer);
                printf(")");
            } else {
                printTree2(answer);
            }
            printf("\n");
        }
    }
}
