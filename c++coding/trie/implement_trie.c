#include <stdbool.h>
#include <stdlib.h>

typedef struct TrieNode {
    struct TrieNode* children[26];
    bool isEnd;
} Trie;

Trie* trieCreate(void) {
    Trie* node = (Trie*)calloc(1, sizeof(Trie));
    return node;
}

void trieInsert(Trie* obj, char* word) {
    Trie* current = obj;

    for (int i = 0; word[i] != '\0'; i++) {
        int idx = word[i] - 'a';
        if (current->children[idx] == NULL) {
            current->children[idx] = trieCreate();
        }
        current = current->children[idx];
    }

    current->isEnd = true;
}

bool trieSearch(Trie* obj, char* word) {
    Trie* current = obj;

    for (int i = 0; word[i] != '\0'; i++) {
        int idx = word[i] - 'a';
        if (current->children[idx] == NULL) {
            return false;
        }
        current = current->children[idx];
    }

    return current->isEnd;
}

bool trieStartsWith(Trie* obj, char* prefix) {
    Trie* current = obj;

    for (int i = 0; prefix[i] != '\0'; i++) {
        int idx = prefix[i] - 'a';
        if (current->children[idx] == NULL) {
            return false;
        }
        current = current->children[idx];
    }

    return true;
}

void trieFree(Trie* obj) {
    if (obj == NULL) {
        return;
    }

    for (int i = 0; i < 26; i++) {
        trieFree(obj->children[i]);
    }

    free(obj);
}

/**
 * Your Trie struct will be instantiated and called as such:
 * Trie* obj = trieCreate();
 * trieInsert(obj, word);
 
 * bool param_2 = trieSearch(obj, word);
 
 * bool param_3 = trieStartsWith(obj, prefix);
 
 * trieFree(obj);
*/
