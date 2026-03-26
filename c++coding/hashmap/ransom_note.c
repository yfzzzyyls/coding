#include <stdbool.h>

bool canConstruct(char* ransomNote, char* magazine) {
    // scan all letters and map to array
    int letters[26] = {0};

    for (int i = 0; magazine[i]!= '\0'; i++){
        letters[ magazine[i] - 'a' ] +=1;
    }

    for (int j = 0; ransomNote[j]!= '\0'; j++){
        if (letters[ ransomNote[j] - 'a' ] == 0){
            return false;
        }
        letters[ ransomNote[j] - 'a' ]--;
    }
    return true;
}