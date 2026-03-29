// Implement a Queue (Array-Based, Circular Buffer)
//
// Build a queue from scratch using a dynamically allocated circular array.
//
// Required operations:
//   - createQueue(capacity)  — allocate and return a new queue
//   - enqueue(val)           — add val to the back of the queue
//   - dequeue()              — remove and return the front element
//   - peek()                 — return the front element without removing
//   - isEmpty()              — return 1 if empty, 0 otherwise
//   - freeQueue()            — free all allocated memory
//
// Constraints:
//   - Use malloc for the internal array
//   - Use a circular buffer so that space is reused after dequeue
//   - Handle enqueue on a full queue by printing an error
//   - Handle dequeue/peek on an empty queue by printing an error and returning -1
//
// Hint: think about how you used the extra-bit trick in your sync FIFO design.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// your code here
typedef struct queue{
    int head;
    int tail;
    int* data;
    int size;
    int count;
} queue_t;

queue_t* createQueue(int size) {
    queue_t* q = (queue_t*)malloc(1 * sizeof(queue_t));
    q->data = (int*)malloc(size * sizeof(int));
    q->head = 0;
    q->tail = 0;
    q->size = size;
    q->count = 0;
    return q;
}

bool isEmpty(queue_t* q){
    if(q->count == 0) return true;
    else return false;
}

bool isFull(queue_t* q){
    return (q->count == q->size) ? true : false;
}

void enqueue(queue_t* q, int val){
    if(!isFull(q)){
        q->data[q->tail] = val;
        q->tail++;
        q->count++;
        q->tail %= q->size;
    }
    else printf("FULL!\n");
}

int dequeue(queue_t* q) {
    if(!isEmpty(q)){
        int entry = q->data[q->head];
        q->head = (q->head + 1) % q->size;
        q->count--;
        return entry;
    }
    else return -1;
}

void freeQueue(queue_t* q){
    free(q->data);
    free(q);
}

int main() {
    queue_t* q = createQueue(4);

    enqueue(q, 10);
    enqueue(q, 20);
    enqueue(q, 30);

    printf("dequeue: %d (expect 10)\n", dequeue(q));
    printf("dequeue: %d (expect 20)\n", dequeue(q));

    enqueue(q, 40);
    enqueue(q, 50);

    printf("dequeue: %d (expect 30)\n", dequeue(q));
    printf("dequeue: %d (expect 40)\n", dequeue(q));
    printf("dequeue: %d (expect 50)\n", dequeue(q));
    printf("isEmpty: %d (expect 1)\n", isEmpty(q));

    // edge case: dequeue on empty
    printf("dequeue empty: %d (expect -1)\n", dequeue(q));

    // edge case: enqueue to full
    enqueue(q, 1);
    enqueue(q, 2);
    enqueue(q, 3);
    enqueue(q, 4);
    enqueue(q, 5); // should print FULL!

    freeQueue(q);
    printf("PASS\n");
    return 0;
}