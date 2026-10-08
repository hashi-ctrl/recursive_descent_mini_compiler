#include <stdio.h>
#include <string.h>

#include "symbol_table.h"

void initSymbolTable(SymbolTable *table)
{
    table->count = 0;
}

int insertSymbol(SymbolTable *table, const char *name,
                 SymbolType type, const char *scope)
{
    /* Check for duplicate declaration */
    if (lookupSymbol(table, name) != NULL)
    {
        return 0;
    }

    /* Check table capacity */
    if (table->count >= MAX_SYMBOLS)
    {
        return 0;
    }

    strcpy(table->symbols[table->count].name, name);
    table->symbols[table->count].type = type;
    strcpy(table->symbols[table->count].scope, scope);

    /* Newly declared variables are not initialized */
    table->symbols[table->count].initialized = 0;

    table->count++;

    return 1;
}

Symbol *lookupSymbol(SymbolTable *table, const char *name)
{
    for (int i = 0; i < table->count; i++)
    {
        if (strcmp(table->symbols[i].name, name) == 0)
        {
            return &table->symbols[i];
        }
    }

    return NULL;
}

int markInitialized(SymbolTable *table, const char *name)
{
    Symbol *symbol = lookupSymbol(table, name);

    if (symbol == NULL)
    {
        return 0;
    }

    symbol->initialized = 1;

    return 1;
}

const char *symbolTypeToString(SymbolType type)
{
    switch (type)
    {
        case SYMBOL_INT:
            return "int";

        case SYMBOL_FLOAT:
            return "float";

        default:
            return "unknown";
    }
}

void printSymbolTable(const SymbolTable *table)
{
    printf("\n");
    printf("SYMBOL TABLE\n");
    printf("---------------------------------------------\n");
    printf("%-15s %-10s %-10s %-12s\n",
           "Name", "Type", "Scope", "Initialized");
    printf("---------------------------------------------\n");

    for (int i = 0; i < table->count; i++)
    {
        printf("%-15s %-10s %-10s %-12s\n",
               table->symbols[i].name,
               symbolTypeToString(table->symbols[i].type),
               table->symbols[i].scope,
               table->symbols[i].initialized ? "Yes" : "No");
    }

    printf("---------------------------------------------\n");
}