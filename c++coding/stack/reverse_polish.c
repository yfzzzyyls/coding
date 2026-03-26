#include <stdlib.h>

//int* stack = (int*)malloc(tokensize * sizeof(int))

int evalRPN(char** tokens, int tokensSize) {
    int* stack = (int*)malloc(tokensSize * sizeof(int));
    // important to rememebr how to allocate a stack
    int top = -1;

    for (int i = 0; i < tokensSize; i++) {
        char* token = tokens[i];

        if (token[1] == '\0' &&
            (token[0] == '+' || token[0] == '-' || token[0] == '*' || token[0] == '/')) {
            int b = stack[top--];
            int a = stack[top--];

            switch (token[0]) {
                case '+':
                    stack[++top] = a + b;
                    break;
                case '-':
                    stack[++top] = a - b;
                    break;
                case '*':
                    stack[++top] = a * b;
                    break;
                case '/':
                    stack[++top] = a / b;
                    break;
            }
        } else {
            stack[++top] = atoi(token);
        }
    }

    int result = stack[top];
    free(stack);
    return result;
}


// int* stack = malloc(stack);

// for all elemnts in char** tokens{
//     if(is_number){
//         push to the stack;
//         top++;
//     }
//     else if(+, -, x, /){

//         assert(if(top < 2) return error;)

//         op1 = stack(top -1);

//         op2 = stack(top -2);
//         top-=2;
//         op = token[i];
//         swtich (op){
//             case: (+) result =  op1 + op2
//             case: (-) result -(op1-op2)
//             ...
//             default result error;
//         }
//         stack.push(result);
//         top++;
//     }
// }

// return (top==0) ? result : 0;

