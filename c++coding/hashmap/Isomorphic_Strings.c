#include <stdbool.h>

bool isIsomorphic(char* s, char* t) {
    int map_st[256] = {0};
    int map_ts[256] = {0};

    int i = 0;
    for (; s[i] != '\0' && t[i] != '\0'; i++) {
        char cs = s[i];
        char ct = t[i];

        if (map_st[cs] == 0 && map_ts[ct] == 0) {
            map_st[cs] = ct;
            map_ts[ct] = cs;
        } else if (map_st[cs] != ct || map_ts[ct] != cs) {
            return false;
        }
    }

    return s[i] == '\0' && t[i] == '\0';
}
