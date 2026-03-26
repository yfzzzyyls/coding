// quite difficult as well

int lengthOfLongestSubstring(char* s) {
    int left = 0;
    int max_len = 1;

    if (s[0] == '\0') return 0; // empty string

    for (int right = 1; s[right] != '\0'; right++) {
        for (int j = left; j < right; j++) {
            if (s[right] == s[j]) {
                left = j + 1;
            }
        }

        int current_len = right - left + 1;
        if (current_len > max_len) {
            max_len = current_len;
        }
    }

    return max_len;
}
