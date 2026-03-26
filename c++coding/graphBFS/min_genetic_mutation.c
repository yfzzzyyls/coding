#include <stdbool.h>
#include <string.h>

static bool oneMutationAway(const char* a, const char* b) {
    int diff = 0;

    for (int i = 0; i < 8; i++) {
        if (a[i] != b[i]) {
            diff++;
            if (diff > 1) {
                return false;
            }
        }
    }

    return diff == 1;
}

int minMutation(char* startGene, char* endGene, char** bank, int bankSize) {
    int endIndex = -1;
    for (int i = 0; i < bankSize; i++) {
        if (strcmp(bank[i], endGene) == 0) {
            endIndex = i;
            break;
        }
    }

    if (endIndex == -1) {
        return -1;
    }

    int queue[bankSize + 1];
    int steps[bankSize + 1];
    bool visited[bankSize];
    for (int i = 0; i < bankSize; i++) {
        visited[i] = false;
    }

    int front = 0;
    int back = 0;

    for (int i = 0; i < bankSize; i++) {
        if (oneMutationAway(startGene, bank[i])) {
            queue[back] = i;
            steps[back] = 1;
            back++;
            visited[i] = true;
        }
    }

    while (front < back) {
        int current = queue[front];
        int step = steps[front];
        front++;

        if (current == endIndex) {
            return step;
        }

        for (int i = 0; i < bankSize; i++) {
            if (!visited[i] && oneMutationAway(bank[current], bank[i])) {
                queue[back] = i;
                steps[back] = step + 1;
                back++;
                visited[i] = true;
            }
        }
    }

    return -1;
}
