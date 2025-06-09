#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

#define MAX_INPUT_LENGTH 256

int main(int argc, char* argv[]) {
    printf("Enter the expression:\n");
    char *input = (char*)malloc(sizeof(char) * MAX_INPUT_LENGTH);

    fgets(input, MAX_INPUT_LENGTH, stdin);

    int res = parse_expression(input);
    printf("\n");
    printf("Result: %i\n", res);

    // expression for testing, should be -44
    // printf("%i\n", -5 + 3 * -(2 ^ 4) / (7 % 3) - -(-8 * 2) + 10 / -2);

    free(input);
    return 0;
}