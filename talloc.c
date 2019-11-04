#include <stdlib.h>
#include <assert.h>
#include "talloc.h"
#include "value.h"
Value *head = NULL;
int memCount = 0;

Value *create(){
	Value *head = malloc(sizeof(Value));
	head->type = NULL_TYPE;
	return head;
}

Value *tallocCons(Value *newCar, Value *newCdr){
    assert(newCar != NULL);
    assert(newCdr != NULL);
    Value *new  = malloc(sizeof(Value));
    new->type = CONS_TYPE;
    new->c.car = newCar; 
    new->c.cdr = newCdr;
    return new;
}

void *talloc(size_t size){
	memCount += size;
	if (head == NULL){
		head = create();
		memCount += sizeof(Value);
	}
	void *new = malloc(size);
	head = tallocCons(new, head);
	memCount += sizeof(Value);
	return new;
}

void tfree(){
	assert(head != NULL);
	Value *prev = head;
    Value *current = head->c.cdr;
    while (current->type != NULL_TYPE){
    	free(current->c.car);
        prev = current;
        current = current->c.cdr;
        free(prev);
    }
    free(current);
    free(head->c.car);
    free(head);
    head = NULL;
    memCount = 0;
}

void texit(int status){
	tfree();
	exit(status);
}

int tallocMemoryCount(){
	return memCount;
}