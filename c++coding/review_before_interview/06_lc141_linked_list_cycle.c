// LC 141: Linked List Cycle
// Given a linked list, return true if it has a cycle.
// Constraint: O(1) space.
// Hint: fast and slow pointer (tortoise and hare).

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

struct ListNode {
    int val;
    struct ListNode *next;
};

bool hasCycle(struct ListNode *head) {
    struct ListNode* slow = head;
    struct ListNode* fast = head;

    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;         // 1 step
        fast = fast->next->next;   // 2 steps
        if (slow == fast)
            return true;           // they met — cycle exists
    }
    return false;  // fast hit NULL — no cycle
}

int main() {
    // Build: 1 -> 2 -> 3 -> 4 -> back to 2 (cycle)
    struct ListNode n1 = {1, NULL}, n2 = {2, NULL}, n3 = {3, NULL}, n4 = {4, NULL};
    n1.next = &n2; n2.next = &n3; n3.next = &n4; n4.next = &n2;
    printf("Has cycle: %d\n", hasCycle(&n1));  // expected: 1

    // Build: 1 -> 2 -> 3 (no cycle)
    struct ListNode m1 = {1, NULL}, m2 = {2, NULL}, m3 = {3, NULL};
    m1.next = &m2; m2.next = &m3;
    printf("Has cycle: %d\n", hasCycle(&m1));  // expected: 0
    return 0;
}
