#include <stddef.h>
#include <stdlib.h>


// importnat to know how to define a queue.

struct TreeNode {
    int val;
    struct TreeNode *left;
    struct TreeNode *right;
};

double* averageOfLevels(struct TreeNode* root, int* returnSize) {
    if (root == NULL) {
        *returnSize = 0;
        return NULL;
    }

    int queueCap = 1024;
    struct TreeNode** queue = 
        (struct TreeNode**)malloc(queueCap * sizeof(struct TreeNode*));
    int front = 0;
    int back = 0;

    int resultCap = 128;
    double* result = (double*)malloc(resultCap * sizeof(double));
    int count = 0;

    queue[back++] = root;

    while (front < back) {
        int levelSize = back - front;
        double levelSum = 0.0;

        for (int i = 0; i < levelSize; i++) {
            struct TreeNode* node = queue[front++];
            levelSum += node->val;

            if (node->left != NULL) {
                if (back == queueCap) {
                    queueCap *= 2;
                    queue = (struct TreeNode**)realloc(queue, queueCap * sizeof(struct TreeNode*));
                }
                queue[back++] = node->left;
            }

            if (node->right != NULL) {
                if (back == queueCap) {
                    queueCap *= 2;
                    queue = (struct TreeNode**)realloc(queue, queueCap * sizeof(struct TreeNode*));
                }
                queue[back++] = node->right;
            }
        }

        if (count == resultCap) {
            resultCap *= 2;
            result = (double*)realloc(result, resultCap * sizeof(double));
        }

        result[count++] = levelSum / levelSize;
    }

    free(queue);
    *returnSize = count;
    return result;
}
