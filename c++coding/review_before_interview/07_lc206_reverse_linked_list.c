// LC 206: Reverse Linked List
// Given a singly linked list, reverse it and return the new head.
// Hint: 3 pointers — prev, curr, next.

#include <stdio.h>
#include <stddef.h>

struct ListNode {
    int val;
    struct ListNode *next;
};

struct ListNode* reverseList(struct ListNode *head) {
    struct ListNode* prev = NULL;
    struct ListNode* curr = head;

    while (curr != NULL) {
        struct ListNode* next = curr->next;  // save next
        curr->next = prev;                   // reverse pointer
        prev = curr;                         // advance prev
        curr = next;                         // advance curr
    }
    return prev;  // prev is the new head
}

int main() {
    // Build: 1 -> 2 -> 3 -> 4 -> 5
    struct ListNode n5 = {5, NULL}, n4 = {4, &n5}, n3 = {3, &n4}, n2 = {2, &n3}, n1 = {1, &n2};
    struct ListNode* result = reverseList(&n1);
    printf("Reversed: ");
    while (result) { printf("%d ", result->val); result = result->next; }
    printf("\n");  // expected: 5 4 3 2 1
    return 0;
}
