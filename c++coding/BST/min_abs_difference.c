#include <limits.h>

struct TreeNode {
    int val;
    struct TreeNode *left;
    struct TreeNode *right;
};

void inorder(struct TreeNode* root, int* prev, int* hasPrev, int* minDiff) {
    if (root == NULL) {
        return;
    }

    inorder(root->left, prev, hasPrev, minDiff);

    if (*hasPrev) {
        int diff = root->val - *prev;
        if (diff < *minDiff) {
            *minDiff = diff;
        }
    }

    *prev = root->val;
    *hasPrev = 1;

    inorder(root->right, prev, hasPrev, minDiff);
}

int getMinimumDifference(struct TreeNode* root) {
    int prev = 0;
    int hasPrev = 0;
    int minDiff = INT_MAX;

    inorder(root, &prev, &hasPrev, &minDiff);

    return minDiff;
}
