#include <stdbool.h>

bool isSubsequence(char* s, char* t) {
    int i = 0; // pointer to s
    int j = 0; // pointer to t

    while (s[i] != '\0'){

        if(t[j] == '\0') return false;

        if (t[j] == s[i]){
            i++; // inccrement j
        }
        j++;
    }

    return true;
}