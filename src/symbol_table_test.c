#include <stdio.h>

#include "symbol_table.h"

int main()
{
    SymbolTable table;

    initSymbolTable(&table);

    printf("Inserting symbols...\n");

    if (insertSymbol(&table, "a", SYMBOL_INT, "global"))
        printf("Inserted: a\n");

    if (insertSymbol(&table, "value", SYMBOL_FLOAT, "global"))
        printf("Inserted: value\n");

    if (insertSymbol(&table, "count", SYMBOL_INT, "global"))
        printf("Inserted: count\n");

    /* Test duplicate declaration */
    if (!insertSymbol(&table, "a", SYMBOL_INT, "global"))
        printf("Duplicate declaration detected: a\n");

    /* Mark a variable as initialized */
    markInitialized(&table, "a");

    /* Test lookup */
    Symbol *symbol = lookupSymbol(&table, "value");

    if (symbol != NULL)
    {
        printf("\nLookup successful:\n");
        printf("Name: %s\n", symbol->name);
        printf("Type: %s\n", symbolTypeToString(symbol->type));
    }

    /* Test undeclared variable */
    symbol = lookupSymbol(&table, "xyz");

    if (symbol == NULL)
    {
        printf("Undeclared identifier detected: xyz\n");
    }

    printSymbolTable(&table);

    return 0;
}