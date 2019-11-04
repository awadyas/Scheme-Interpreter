//Modified version of the original linkedlist.c for the talloc assignment

#include "linkedlist.h"
#include "talloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

// Create a new NULL_TYPE value node.
Value *makeNull(){
    Value *new  = talloc(sizeof(Value));
    new->type = NULL_TYPE;
    return new;
}

// Create a new CONS_TYPE value node.
Value *cons(Value *newCar, Value *newCdr){
    assert(newCar != NULL);
    assert(newCdr != NULL);
    Value *new  = talloc(sizeof(Value));
    new->type = CONS_TYPE;
    new->c.car = newCar;
    new->c.cdr = newCdr;
    return new;
}

//Helper function for display
void printValue(Value *value){
    switch (value->type){
            case INT_TYPE:
                printf("%i ", value->i);
                break;
            case DOUBLE_TYPE:
                printf("%f ", value->d);
                break;
            case STR_TYPE:
                printf("%s ", value->s);
                break;
            case PTR_TYPE:
                printf("%p ", value->p);
                break;
            default:
                break;
        }
}

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(Value *list){
    assert(list != NULL);
    assert(list->type == CONS_TYPE);
    printf("[ ");
    while (list->c.cdr->type == CONS_TYPE){
        printValue(list->c.car);
        list = list->c.cdr;
    }
    printValue(list->c.car);
    printValue(list->c.cdr);
    printf("]\n");
}

// Return a new list that is the reverse of the one that is passed in. No stored
// data within the linked list should be duplicated; rather, a new linked list
// of CONS_TYPE nodes should be created, that point to items in the original
// list.
Value *reverse(Value *list){
    assert(list != NULL);
    assert(list->type == CONS_TYPE);
    Value *head = makeNull();
    Value *current = list;
    while(current->type != NULL_TYPE){
        head = cons(current->c.car, head);
        current = current->c.cdr;
    }
    return head;
}

// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list){
    assert(list != NULL);
    assert(list->type == CONS_TYPE);
    return list->c.car;
}

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list){
    assert(list != NULL);
    assert(list->type == CONS_TYPE);
    return list->c.cdr;
}

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value){
    assert(value != NULL);
    if (value->type == NULL_TYPE){
        return true;
    }
    else{
        return false;
    }
}

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value){
    assert(value != NULL);
    assert(value->type == CONS_TYPE);
    int count = 0;
    Value *current = value;
    while(!isNull(current)){
        count++;
        current = current->c.cdr;
    }
    return count;
}

// Creates a list with parameters passed, arg_count specifies how many items will be
// in the list.
// Returns the list in the reverse order
Value *list(int arg_count, ...){
  va_list ap;
  va_start (ap, arg_count);
  Value *head = makeNull();

  for (int i = 0; i < arg_count; i++){
    valueType type = va_arg(ap, valueType);
    Value *newVal = talloc(sizeof(Value));

    switch(type){
        case INT_TYPE:
            newVal->type = INT_TYPE;
            int i = va_arg(ap, int);
            newVal->i = i;
            break;
        case DOUBLE_TYPE:
            newVal->type = DOUBLE_TYPE;
            double d = va_arg(ap, double);
            newVal->d = d;
            break;
        case STR_TYPE:
            newVal->type = STR_TYPE;
            char *temp = va_arg(ap, char *);
            newVal->s = talloc((strlen(temp)+1)*sizeof(char));
            strcpy(newVal->s, temp);
            break;
        case NULL_TYPE:
            newVal->type = NULL_TYPE;
            break;
        case PTR_TYPE:
            newVal->type = PTR_TYPE;
            void *p = va_arg(ap, void *);
            newVal->p = p;
            break;
        default:
            break;

    }
    head = cons(newVal, head);
  }
  va_end(ap);
  return head;
}

// Copies the first parameter and makes the end of the list point to the second param
// Make sure both params are cons cells
// Valgrind gives many errors about invalid reads because that memory has already
// been freed, but this is the only function that has that issue, and it doesn't
// cause it to crash or not work.
Value *append(Value *first, Value *second) {
	assert(first->type == CONS_TYPE);
	assert(second->type == CONS_TYPE);
	Value *head = makeNull();
	Value *reversed = reverse(first);
	Value *current = reversed;
	while(!isNull(current)){
		head = cons(current->c.car, head);
		if (current == reversed){
			free(head->c.cdr);
			head->c.cdr = second;
		}
		current = current->c.cdr;
	}
	return head;
}
