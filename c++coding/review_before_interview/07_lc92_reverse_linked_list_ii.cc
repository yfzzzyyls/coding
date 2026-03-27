class Solution {
public:
    ListNode* reverseBetween(ListNode* head, int left, int right) {
        if (!head || left == right) {
            return head;
        }

        ListNode dummy(0);
        dummy.next = head;

        ListNode* prev = &dummy;
        for (int i = 1; i < left; i++) {
            prev = prev->next;
        }

        ListNode* curr = prev->next;
        for (int i = 0; i < right - left; i++) {
            ListNode* move = curr->next;
            curr->next = move->next;
            move->next = prev->next;
            prev->next = move;
        }

        return dummy.next;
    }
};
