// Implement a Hash Table (Separate Chaining)
//
// Build a hash table from scratch using an array of linked lists.
//
// Required operations:
//   - createTable(capacity)  — allocate and return a new hash table
//   - put(key, value)        — insert a key-value pair (update if key exists)
//   - get(key)               — return the value for the key, or -1 if not found
//   - delete(key)            — remove the key-value pair
//   - freeTable()            — free all allocated memory
//
// Use a simple hash function: key % capacity
// Handle collisions with separate chaining (linked list at each bucket).

#include <stdio.h>
#include <stdlib.h>


typedef struct node{
    int key;
    int val;
    struct node* next;
} node_t;

typedef struct hashtable{
    int capacity;
    node_t** bucket;
} hashtable_t;

hashtable_t* createTable(int capacity){
    hashtable_t* t = (hashtable_t*)malloc(1 * sizeof(hashtable_t));
    t->bucket = (node_t**)malloc(capacity * sizeof(node_t*));
    t->capacity = capacity;
    for (int i = 0; i < capacity; i++){
        t->bucket[i] = NULL;
    }
    return t;
}

void put(hashtable_t* t, int key, int val) {
    int i = key % t->capacity;

    // Walk the bucket — if key exists, update value
    node_t* curr = t->bucket[i];
    while (curr != NULL) {
        if (curr->key == key) {
            curr->val = val;
            return;
        }
        curr = curr->next;
    }

    // Key not found — create new node and insert at head of bucket
    node_t* newNode = (node_t*)malloc(sizeof(node_t));
    newNode->key = key;
    newNode->val = val;
    newNode->next = t->bucket[i];
    t->bucket[i] = newNode;
}

int get(hashtable_t* t, int key) {
    int i = key % t->capacity;
    node_t* curr = t->bucket[i];
    while (curr != NULL) {
        if (curr->key == key) return curr->val;
        curr = curr->next;
    }
    return -1;
}

void removeKey(hashtable_t* t, int key) {
    int i = key % t->capacity;
    node_t* curr = t->bucket[i];
    node_t* prev = NULL;
    while (curr != NULL) {
        if (curr->key == key) {
            if (prev == NULL)
                t->bucket[i] = curr->next;
            else
                prev->next = curr->next;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void freeTable(hashtable_t* t) {
    for (int i = 0; i < t->capacity; i++) {
        node_t* curr = t->bucket[i];
        while (curr != NULL) {
            node_t* next = curr->next;
            free(curr);
            curr = next;
        }
    }
    free(t->bucket);
    free(t);
}

int main() {
    hashtable_t* t = createTable(4);

    put(t, 5, 100);
    put(t, 10, 200);
    put(t, 9, 300);   // collides with key 5 (both % 4 = 1)

    printf("get 5:  %d (expect 100)\n", get(t, 5));
    printf("get 10: %d (expect 200)\n", get(t, 10));
    printf("get 9:  %d (expect 300)\n", get(t, 9));
    printf("get 99: %d (expect -1)\n", get(t, 99));

    // Update existing key
    put(t, 5, 999);
    printf("get 5:  %d (expect 999)\n", get(t, 5));

    // Delete
    removeKey(t, 5);
    printf("get 5:  %d (expect -1)\n", get(t, 5));

    freeTable(t);
    printf("PASS\n");
    return 0;
}