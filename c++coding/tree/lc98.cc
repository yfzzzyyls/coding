class Solution {
public:
    bool isValidBST(TreeNode* root) {
        return check(root, LONG_MIN, LONG_MAX);
    }

private:
    bool check(TreeNode* node, long low, long high) {
        if (!node) {
            return true;
        }

        if (node->val <= low || node->val >= high) {
            return false;
        }

        return check(node->left, low, node->val) &&
               check(node->right, node->val, high);
    }
};
