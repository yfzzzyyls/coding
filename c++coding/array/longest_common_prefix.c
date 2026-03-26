#include <stdlib.h>

char* longestCommonPrefix(char** strs, int strsSize) {
    if (strsSize == 0){
        char* empty = (char*)malloc(1 * sizeof(char));
        empty[0] = '\0';
        return empty;
    }

    int result_str_length = 0;
    int l = 0;
    while (strs[0][l] != '\0'){
        result_str_length++;
        l++;
    }

    for (int i = 1; i<strsSize; i++){
        int j = 0;
        while(strs[i][j] != '\0' && (j < result_str_length)){
            if(strs[i][j] == strs[0][j]){
                j++;
                continue;
            }
            break; 
            // remember: while needs to break
        }
        result_str_length = j;
    }

    char* result_str = (char*)malloc((result_str_length + 1)* sizeof(char));
    for (int k = 0; k<result_str_length; k++){
        result_str[k] = strs[0][k];
    }
    result_str[result_str_length] = '\0';

    return result_str;
}
