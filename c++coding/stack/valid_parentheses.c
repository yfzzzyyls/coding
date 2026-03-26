#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

char return_pair(char c){
    switch (c){
        case ('('): return (')');
        case (')'): return ('(');
        case ('{'): return ('}');
        case ('}'): return ('{');
        case ('['): return (']');
        case (']'): return ('[');
        default: return '\0';
    }
}

bool isValid(char* s) {
    int len = strlen(s);

    char* stack = (char*)malloc(len * sizeof(char));
    int top = -1;

    for (int i = 0; s[i] != '\0'; i++) {
        if (s[i] == '(' || s[i] == '{' || s[i] == '[') {
            top++;
            stack[top] = s[i];
        } else {
            if (top == -1 || return_pair(stack[top]) != s[i]) {
                free(stack);
                return false;
            }
            top--;
        }
    }

    bool valid = (top == -1);
    free(stack);
    return valid;
}
