#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "standard_types.c"
#include "lexer/lexer.c"
#include "parser/parser.c"

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        printf("No input file given\n");
        return 0;
    }
    FILE* file = fopen(argv[1], "r");

    Tokens tokens;
    tokenise(&tokens, file);
    
    parse(tokens);
}
