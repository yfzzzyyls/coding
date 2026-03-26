#include <ctype.h>
#include <stdbool.h>


//this problem is pretty complicated

bool isPalindrome(char* s) {

    int i = 0; // pointer to the begining
    int j = 0; // pointer to the end

    while(s[j] != '\0'){
        s[j] = tolower(s[j]);
        j++;
    }
    j--; // set to the last char before '\0'
    // set all to lowercase from upper case.

    while (i < j){
        while(!isalnum(s[i])){
            i++;
        }

        while(!isalnum(s[j])){
            j--;
        }

        if(s[i] != s[j]){
            return false;
        }

        i++;
        j--;
    }

    return true;
}
