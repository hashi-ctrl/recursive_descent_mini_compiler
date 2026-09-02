#include <stdio.h>

#include "parser.h"

int main(void)
{
    const char *source =
        "int a;\n"
        "int b;\n"
        "int c;\n"
        "a = b + c * 5;\n";

    Parser parser;

    printf("========================================\n");
    printf("   RECURSIVE DESCENT MINI COMPILER\n");
    printf("========================================\n\n");

    printf("SOURCE PROGRAM\n");
    printf("----------------------------------------\n");
    printf("%s\n", source);

    printf("THREE ADDRESS CODE\n");
    printf("----------------------------------------\n");

    parser_init(&parser, source);
    parse_program(&parser);

    printf("----------------------------------------\n");

    if (parser_has_errors(&parser))
    {
        printf("Compilation finished with %d error(s).\n",
               parser.error_count);
    }
    else
    {
        printf("Syntax: VALID\n");
        printf("Compilation successful.\n");
    }

    return 0;
}
