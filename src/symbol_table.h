#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#define MAX_SYMBOLS 100
#define MAX_NAME_LENGTH 100

typedef enum {
    SYMBOL_INT,
    SYMBOL_FLOAT
} SymbolType;

typedef struct {
    char name[MAX_NAME_LENGTH];
    SymbolType type;
    char scope[20];
    int initialized;
} Symbol;

typedef struct {
    Symbol symbols[MAX_SYMBOLS];
    int count;
} SymbolTable;

/* Initialize symbol table */
void initSymbolTable(SymbolTable *table);

/* Insert a new symbol */
int insertSymbol(SymbolTable *table, const char *name,
                 SymbolType type, const char *scope);

/* Find a symbol */
Symbol *lookupSymbol(SymbolTable *table, const char *name);

/* Mark a symbol as initialized */
int markInitialized(SymbolTable *table, const char *name);

/* Display the symbol table */
void printSymbolTable(const SymbolTable *table);

/* Convert symbol type to string */
const char *symbolTypeToString(SymbolType type);

#endif