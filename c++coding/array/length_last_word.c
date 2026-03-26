int lengthOfLastWord(char* s) {. // mine solution
    int word_length = 0;
    int hold = 0;
    for (int i = 0; s[i] != '\0'; i++){
        if(s[i] != ' '){
            if (hold) {
                word_length = 0;
                hold = 0; 
                // problem was the hold never gets reset back to 0;
            }
            word_length++;
        }
        else {hold = 1;}
    }
    return word_length;
}


// jump strait to the end and count backward is a better solution

int lengthOfLastWord(char* s) { // clean solution
    int i = 0;
    int length = 0;

    while (s[i] != '\0') {
        i++;
    }

    i--;

    while (i >= 0 && s[i] == ' ') {
        i--;
    }

    while (i >= 0 && s[i] != ' ') {
        length++;
        i--;
    }

    return length;
}
