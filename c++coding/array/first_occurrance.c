int strStr(char* haystack, char* needle) {
    int needle_length = 0;

    int l = 0;
    while(needle[l] != '\0'){
        needle_length++;
        l++;
    }

    if (needle_length == 0) return -1;

    for (int i = 0; haystack[i] != '\0'; i++){
        int j = 0;
        while (needle[j] != '\0' &&
               haystack[i + j] != '\0' &&
               needle[j] == haystack[i + j]){
                // the key is this line of code.
                j++;
            }
        if (j == needle_length) return i;
    }

    return -1;
}
