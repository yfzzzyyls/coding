// Implement a Stack (Array-Based)
//
// Build a stack from scratch using a dynamically allocated array.
//
// Required operations:
//   - createStack(capacity)  — allocate and return a new stack
//   - push(val)              — push val onto the top
//   - pop()                  — remove and return the top element
//   - peek()                 — return the top element without removing
//   - isEmpty()              — return 1 if empty, 0 otherwise
//   - freeStack()            — free all allocated memory
//
// Constraints:
//   - Use malloc for the internal array
//   - Handle push on a full stack by printing an error (or ignoring)
//   - Handle pop/peek on an empty stack by printing an error and returning -1

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct stack{
    int* data; 
    int size;
    int ptr;
} stack_t;

bool isEmpty(stack_t* s){
    return s->ptr == 0;
}

bool isFull(stack_t* s){
    return s->ptr == s->size;
}

stack_t* createStack(int capacity){
    stack_t* s= malloc(1 * sizeof(stack_t));
    s->data = malloc(capacity * sizeof(int));
    s->ptr = 0;
    s->size = capacity;
    return s;
}

int peek(stack_t* s){
    if(!isEmpty(s)) return s->data[s->ptr - 1];
    else return -1;
}

void push(stack_t* s, int val)
{
    if(!isFull(s)){
        s->data[s->ptr] = val;
        s->ptr++;
    }
    else printf("NO Push when full\n");
}

int pop(stack_t* s)
{
    if(!isEmpty(s)){
        s->ptr--;
        return s->data[s->ptr];
    }
    else return -1;
}

void free_stack(stack_t* s){
    free(s->data);
    free(s);
}

int main() {
    stack_t* s = createStack(4);

    push(s, 10);
    push(s, 20);
    push(s, 30);

    printf("peek: %d (expect 30)\n", peek(s));
    printf("pop:  %d (expect 30)\n", pop(s));
    printf("pop:  %d (expect 20)\n", pop(s));
    printf("isEmpty: %d (expect 0)\n", isEmpty(s));
    printf("pop:  %d (expect 10)\n", pop(s));
    printf("isEmpty: %d (expect 1)\n", isEmpty(s));

    // edge case: pop on empty
    printf("pop empty: %d (expect -1)\n", pop(s));

    // edge case: push to full
    push(s, 1);
    push(s, 2);
    push(s, 3);
    push(s, 4);
    push(s, 5); // should print error — stack is full

    free_stack(s);
    printf("PASS\n");
    return 0;
}
