# C/C++ Interview Review Cheatsheet

10 representative problems from the most relevant categories for embedded/firmware interviews.

---

## Category 1: Bit Manipulation (Most Important for Embedded!)

### 01 — LC 136: Single Number
**Pattern**: XOR cancels duplicates (`a ^ a = 0`, `a ^ 0 = a`)

```c
int singleNumber(int* nums, int n) {
    int result = 0;
    for (int i = 0; i < n; i++)
        result ^= nums[i];
    return result;
}
```

**Trace**: `[4, 1, 2, 1, 2]` -> `0^4=4`, `4^1=5`, `5^2=7`, `7^1=6`, `6^2=4` -> answer is 4

**Complexity**: O(n) time, O(1) space

---

### 02 — LC 191: Number of 1 Bits
**Pattern**: `n & (n-1)` clears the lowest set bit

```c
int hammingWeight(unsigned int n) {
    int count = 0;
    while (n != 0) {
        n &= (n - 1);   // clear lowest set bit
        count++;
    }
    return count;
}
```

**Trace**: `n = 6 (110)`:
- `110 & 101 = 100`, count=1
- `100 & 011 = 000`, count=2
- n=0, done -> answer is 2

**Why it works**: `n-1` flips the lowest set bit and all bits below it. AND with `n` clears that bit.

**Complexity**: O(k) time where k = number of 1 bits, O(1) space

---

### 03 — LC 190: Reverse Bits
**Pattern**: Shift result left, extract lowest bit with `n & 1`, OR into result, shift source right

```c
uint32_t reverseBits(uint32_t n) {
    uint32_t result = 0;
    for (int i = 0; i < 32; i++) {
        result <<= 1;       // make room
        result |= (n & 1);  // grab lowest bit of n
        n >>= 1;            // move to next bit
    }
    return result;
}
```

**Trace** (4-bit example, `n = 1011`):
- i=0: result=`0001`, n=`0101`
- i=1: result=`0011`, n=`0010`
- i=2: result=`0110`, n=`0001`
- i=3: result=`1101`, n=`0000`
- Result: `1101` (reversed `1011`)

**Complexity**: O(32) = O(1) time, O(1) space

---

## Category 2: Arrays & Strings

### 04 — LC 88: Merge Sorted Array
**Pattern**: Merge from the END to avoid overwriting elements

```c
void merge(int* nums1, int nums1Size, int m, int* nums2, int nums2Size, int n) {
    int i = m - 1;          // last valid element in nums1
    int j = n - 1;          // last valid element in nums2
    int k = nums1Size - 1;  // write position (from end)

    while (j >= 0) {
        if (i >= 0 && nums1[i] > nums2[j]) {
            nums1[k--] = nums1[i--];
        } else {
            nums1[k--] = nums2[j--];
        }
    }
}
```

**Key insight**: Start from the back. Once nums2 is exhausted (`j < 0`), remaining nums1 elements are already in place.

**Complexity**: O(m+n) time, O(1) space

---

### 05 — LC 238: Product of Array Except Self
**Pattern**: Two-pass — prefix products (left to right), then suffix products (right to left)

```cpp
vector<int> productExceptSelf(vector<int>& nums) {
    int n = nums.size();
    vector<int> result(n, 1);

    // Pass 1: prefix products
    int prefix = 1;
    for (int i = 0; i < n; i++) {
        result[i] = prefix;      // product of everything LEFT of i
        prefix *= nums[i];
    }

    // Pass 2: multiply by suffix products
    int suffix = 1;
    for (int i = n - 1; i >= 0; i--) {
        result[i] *= suffix;     // multiply by product of everything RIGHT of i
        suffix *= nums[i];
    }

    return result;
}
```

**Trace**: `nums = [1, 2, 3, 4]`
- After prefix pass: `result = [1, 1, 2, 6]` (prefix products: 1, 1, 1*2, 1*2*3)
- After suffix pass: `result = [24, 12, 8, 6]` (multiply by suffix: 4*3*2, 4*3, 4, 1)

**Complexity**: O(n) time, O(1) extra space

---

## Category 3: Linked Lists

### 06 — LC 141: Linked List Cycle
**Pattern**: Floyd's tortoise and hare — slow moves 1 step, fast moves 2 steps

```c
bool hasCycle(struct ListNode *head) {
    struct ListNode *slow = head;
    struct ListNode *fast = head;

    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;         // 1 step
        fast = fast->next->next;   // 2 steps
        if (slow == fast) return true;
    }
    return false;
}
```

**Why it works**: If there's a cycle, fast will eventually "lap" slow inside the cycle. If no cycle, fast hits NULL.

**Key check**: `fast != NULL && fast->next != NULL` — must check both before advancing.

**Complexity**: O(n) time, O(1) space

---

### 07 — LC 92: Reverse Linked List II
**Pattern**: Dummy node + repeated pull-and-insert

```cpp
ListNode* reverseBetween(ListNode* head, int left, int right) {
    ListNode dummy(0);
    dummy.next = head;

    // Walk prev to the node BEFORE the reversal section
    ListNode* prev = &dummy;
    for (int i = 1; i < left; i++)
        prev = prev->next;

    // Repeatedly pull next node and insert after prev
    ListNode* curr = prev->next;
    for (int i = 0; i < right - left; i++) {
        ListNode* move = curr->next;
        curr->next = move->next;    // unlink move
        move->next = prev->next;    // move points to front of reversed section
        prev->next = move;          // prev points to move
    }

    return dummy.next;
}
```

**Trace**: `1 -> 2 -> 3 -> 4 -> 5`, left=2, right=4:
- prev=1, curr=2
- i=0: pull 3 -> `1 -> 3 -> 2 -> 4 -> 5`
- i=1: pull 4 -> `1 -> 4 -> 3 -> 2 -> 5`
- Done! Section [2,4] is reversed.

**Key trick**: Dummy node avoids special-casing when left=1 (reversing from head).

**Complexity**: O(n) time, O(1) space

---

### Bonus — Reverse Entire Linked List (LC 206)
**Pattern**: Three pointers — prev, curr, next

```c
node_t* reverseList(node_t* head) {
    node_t* prev = NULL;
    node_t* current = head;
    while (current != NULL) {
        node_t* next_node = current->next;  // save next
        current->next = prev;               // reverse the link
        prev = current;                     // advance prev
        current = next_node;                // advance current
    }
    return prev;  // prev is the new head
}
```

**Complexity**: O(n) time, O(1) space

---

## Category 4: Stack & Queue

### 08 — LC 20: Valid Parentheses
**Pattern**: Stack — push opening brackets, match closing brackets with top

```c
bool isValid(char* s) {
    int len = strlen(s);
    char* stack = (char*)malloc(len * sizeof(char));
    int top = -1;

    for (int i = 0; s[i] != '\0'; i++) {
        if (s[i] == '(' || s[i] == '{' || s[i] == '[') {
            stack[++top] = s[i];           // push opening bracket
        } else {
            if (top == -1) return false;   // nothing to match
            char open = stack[top--];      // pop
            if ((s[i] == ')' && open != '(') ||
                (s[i] == '}' && open != '{') ||
                (s[i] == ']' && open != '['))
                return false;
        }
    }

    free(stack);
    return top == -1;  // stack must be empty
}
```

**Key checks**:
1. Closing bracket with empty stack -> false
2. Closing bracket doesn't match top -> false
3. End of string but stack not empty -> false

**Complexity**: O(n) time, O(n) space

---

## Category 5: Two Pointers

### 09 — LC 15: 3Sum
**Pattern**: Sort + fix one element + two pointers + skip duplicates

```cpp
vector<vector<int>> threeSum(vector<int>& nums) {
    sort(nums.begin(), nums.end());
    vector<vector<int>> result;
    int n = nums.size();

    for (int i = 0; i < n; i++) {
        if (i > 0 && nums[i] == nums[i-1]) continue;  // skip duplicate i

        int left = i + 1, right = n - 1;
        while (left < right) {
            long sum = (long)nums[i] + nums[left] + nums[right];
            if (sum < 0)       left++;
            else if (sum > 0)  right--;
            else {
                result.push_back({nums[i], nums[left], nums[right]});
                left++;  right--;
                while (left < right && nums[left] == nums[left-1])   left++;   // skip dup
                while (left < right && nums[right] == nums[right+1]) right--;  // skip dup
            }
        }
    }
    return result;
}
```

**Key insight**: Sort first so two pointers work. Duplicate skipping at TWO levels:
1. Outer loop: skip if `nums[i] == nums[i-1]`
2. After finding match: skip identical left/right values

**Complexity**: O(n^2) time, O(1) extra space

---

## Category 6: Binary Search

### 10 — LC 33: Search in Rotated Sorted Array
**Pattern**: Modified binary search — one half is always sorted

```cpp
int search(vector<int>& nums, int target) {
    int left = 0, right = nums.size() - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (nums[mid] == target) return mid;

        if (nums[left] <= nums[mid]) {
            // Left half is sorted
            if (nums[left] <= target && target < nums[mid])
                right = mid - 1;   // target in sorted left half
            else
                left = mid + 1;    // target in right half
        } else {
            // Right half is sorted
            if (nums[mid] < target && target <= nums[right])
                left = mid + 1;    // target in sorted right half
            else
                right = mid - 1;   // target in left half
        }
    }
    return -1;
}
```

**Key insight**: After rotation, at any mid point, ONE half is always sorted. Check `nums[left] <= nums[mid]` to determine which. Then check if target falls within the sorted range.

**Trace**: `[4,5,6,7,0,1,2]`, target=0:
- mid=7, left half [4,5,6] sorted, 0 not in [4,7) -> search right
- mid=1, right half [1,2] sorted, 0 not in (1,2] -> search left
- mid=0, found!

**Complexity**: O(log n) time, O(1) space

---

## Quick Reference Table

| # | Problem | Key Trick | Time |
|---|---------|-----------|------|
| 1 | Single Number | `XOR` cancels duplicates | O(n) |
| 2 | Number of 1 Bits | `n & (n-1)` clears lowest set bit | O(k) |
| 3 | Reverse Bits | Shift left, OR `n&1`, shift right | O(1) |
| 4 | Merge Sorted Array | Merge from the END | O(m+n) |
| 5 | Product Except Self | Prefix then suffix pass | O(n) |
| 6 | Linked List Cycle | Floyd's slow/fast pointers | O(n) |
| 7 | Reverse LL II | Dummy node + pull-and-insert | O(n) |
| 8 | Valid Parentheses | Stack: push open, match close | O(n) |
| 9 | 3Sum | Sort + fix one + two pointers | O(n^2) |
| 10 | Rotated Array Search | Binary search: find sorted half | O(log n) |

---

## Data Structures Built Today (in 00_data_structure_fundamentals/)

| Structure | Key Concepts |
|-----------|-------------|
| **Stack** | Array + top pointer, push/pop from same end (LIFO) |
| **Queue** | Circular array + head/tail + count, modulo wrap (FIFO) |
| **Linked List** | Node struct with next pointer, malloc per node, walk to traverse |
| **Hash Table** | Array of linked lists, `key % capacity` hash, separate chaining |

### Memory Pattern (applies to all):
```c
// Create: malloc struct, malloc internal array, init fields
thing_t* t = malloc(sizeof(thing_t));
t->data = malloc(capacity * sizeof(int));
t->size = capacity;

// Free: free internal first, then struct
free(t->data);
free(t);
```
