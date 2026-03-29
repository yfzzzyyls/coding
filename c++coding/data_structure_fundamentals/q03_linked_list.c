// Implement a Singly Linked List
//
// Build a singly linked list from scratch.
//
// Required operations:
//   - insertHead(val)    — insert a new node at the front
//   - insertTail(val)    — insert a new node at the end
//   - deleteNode(val)    — delete the first node with the given value
//   - search(val)        — return 1 if val exists, 0 otherwise
//   - printList()        — print all elements in order
//   - freeList()         — free all nodes
//
// Each node holds an int value and a pointer to the next node.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct node{
    int val;
    struct node* next;
} node_t;

node_t* createNode(int val){
    node_t* l = (node_t*)malloc(1 * sizeof(node_t));
    l->val = val;
    l->next = NULL;
    return l;
}

node_t* insertHead(node_t* l, int val){
    node_t* head = createNode(val);
    head->next = l;
    return head;
}

void insertTail(node_t* self_node, int val){
    node_t* current = self_node;
    while (current->next != NULL){
        current = current->next;
    }
    node_t* tail = createNode(val);
    current->next = tail;
}

node_t* deleteNode(node_t* head, int val) {
    // Case 1: deleting the head itself
    if (head->val == val) {
        node_t* newHead = head->next;
        free(head);
        return newHead;
    }

    // Case 2: walk the list, track previous node
    node_t* prev = head;
    node_t* curr = head->next;
    while (curr != NULL) {
        if (curr->val == val) {
            prev->next = curr->next;  // unlink curr
            free(curr);               // free it
            return head;
        }
        prev = curr;
        curr = curr->next;
    }

    // val not found
    return head;
}

// void freeList(node_t* head) {
//     node_t* curr = head;
//     while (curr != NULL) {
//         node_t* next = curr->next;  // save next before freeing
//         free(curr);
//         curr = next;
//     }
// }

void printList(node_t* head) {
    node_t* curr = head;
    while (curr != NULL) {
        printf("%d -> ", curr->val);
        curr = curr->next;
    }
    printf("NULL\n");
}

void freeList(node_t* head) {
    if (head == NULL) return;
    freeList(head->next);
    free(head);
}


bool search(int val, node_t* start_node){
    if(start_node->val == val) return 1;
    else {
        node_t* next_node = start_node->next;
        while(next_node!=NULL){
            if(next_node->val == val) return 1;
            else next_node = next_node->next;
        }
    }
    return 0;
}

node_t* reverseList(node_t* head) {
    node_t* prev = NULL;
    node_t* current = head;
    while (current != NULL) {
        node_t* next_node = current->next;
        current->next = prev;
        prev = current;
        current = next_node;
    }
    return prev;
}

int main() {
    // Build list: 10 -> 20 -> 30 -> NULL
    node_t* head = createNode(10);
    insertTail(head, 20);
    insertTail(head, 30);
    printf("Initial:  ");
    printList(head);  // 10 -> 20 -> 30 -> NULL

    // Insert at head
    head = insertHead(head, 5);
    printf("InsHead:  ");
    printList(head);  // 5 -> 10 -> 20 -> 30 -> NULL

    // Insert at tail
    insertTail(head, 40);
    printf("InsTail:  ");
    printList(head);  // 5 -> 10 -> 20 -> 30 -> 40 -> NULL

    // Search
    printf("search 20: %d (expect 1)\n", search(20, head));
    printf("search 99: %d (expect 0)\n", search(99, head));

    // Delete middle
    head = deleteNode(head, 20);
    printf("Del 20:   ");
    printList(head);  // 5 -> 10 -> 30 -> 40 -> NULL

    // Delete head
    head = deleteNode(head, 5);
    printf("Del 5:    ");
    printList(head);  // 10 -> 30 -> 40 -> NULL

    // Delete tail
    head = deleteNode(head, 40);
    printf("Del 40:   ");
    printList(head);  // 10 -> 30 -> NULL

    // Reverse
    head = reverseList(head);
    printf("Reverse:  ");
    printList(head);  // 30 -> 10 -> NULL

    freeList(head);
    printf("PASS\n");
    return 0;
}