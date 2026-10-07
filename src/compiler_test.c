#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

#define MAX_SOURCE_SIZE 10000

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");

    if (file == NULL)
    {
        printf("Error: Could not open file '%s'\n", argv[1]);
        return 1;
    }

    char source[MAX_SOURCE_SIZE];

    size_t bytes_read = fread(
        source,
        sizeof(char),
        MAX_SOURCE_SIZE - 1,
        file
    );

    fclose(file);

    source[bytes_read] = '\0';

    printf("========================================\n");
    printf("   RECURSIVE DESCENT MINI COMPILER\n");
    printf("========================================\n\n");

    printf("SOURCE FILE\n");
    printf("----------------------------------------\n");
    printf("%s\n", source);

    printf("THREE ADDRESS CODE\n");
    printf("----------------------------------------\n");

    Parser parser;

    parser_init(&parser, source);
    parse_program(&parser);

    printf("----------------------------------------\n");

    if (parser_has_errors(&parser))
    {
        printf("Syntax: INVALID\n");
        printf("Errors detected: %d\n", parser.error_count);
    }
    else
    {
        printf("Syntax: VALID\n");
        printf("Compilation successful.\n");
    }

    return 0;
}
