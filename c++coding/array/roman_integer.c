int get_value(char c) {
    int return_val = 0;
    switch(c) {
        case 'I': return_val = 1;
        break;
        case 'V': return_val = 5;
        break;
        case 'X': return_val = 10;
        break;
        case 'L': return_val = 50;
        break;
        case 'C': return_val = 100;
        break;
        case 'D': return_val = 500;
        break;
        case 'M': return_val = 1000;
        break;
        default: return 0;
    }
    return return_val;
}

int romanToInt(char* c){
    // string pointer char* c

    int total_val = 0;
    int char_val, next_char_val;

    for (int i = 0; c[i] != '\0'; i++){
        char_val = get_value(c[i]);
        next_char_val = get_value(c[i+1]);
        if(char_val < next_char_val){
            total_val -= char_val;
        }
        else total_val += char_val;
    }

    return total_val;
}