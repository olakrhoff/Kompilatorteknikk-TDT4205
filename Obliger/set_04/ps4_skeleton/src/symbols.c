#include <vslc.h>

/* Global symbol table and string list */
symbol_table_t *global_symbols;
char **string_list;
size_t string_list_len;
size_t string_list_capacity;

static void find_globals(void);

static void bind_names(symbol_table_t *local_symbols, node_t *root);

static void print_symbol_table(symbol_table_t *table, int nesting);

static void destroy_symbol_tables(void);

static size_t add_string(char *string);

static void print_string_list(void);

static void destroy_string_list(void);

/* External interface */

/* Creates a global symbol table, and local symbol tables for each function.
 * While building the symbol tables:
 *  - All usages of symbols are bound to their symbol table entries.
 *  - All strings are entered into the string_list
 */
void create_tables(void)
{
    find_globals();
    
    //TODO:
    // First use find_globals() to create the global symbol table.
    // As global symbols are added, function symbols get their own local symbol tables as well.
    //
    // Once all global symbols are added, go through all functions bodies.
    // All references to variables and functions by name, should get pointers to the symbols in the table.
    // This should performed by bind_names( function symbol table, function body AST node )
    //
    // It also handles adding local variables to the local symbol table, and pushing and popping scopes.
    // A final task performed by bind_names(), is adding strings to the global string list
}

/* Prints the global symbol table, and the local symbol tables for each function.
 * Also prints the global string list.
 * Finally, prints out the AST again, with bound symbols.
 */
void print_tables(void)
{
    print_symbol_table(global_symbols, 0);
    printf("\n == STRING LIST == \n");
    print_string_list();
    printf("\n == BOUND SYNTAX TREE == \n");
    print_syntax_tree();
}

/* Destroys all symbol tables and the global string list */
void destroy_tables(void)
{
    destroy_symbol_tables();
    destroy_string_list();
}

/* Internal matters */

/* Goes through all global declarations in the syntax tree, adding them to the global symbol table.
 * When adding functions, local symbol tables are created, and symbols for the functions parameters are added.
 */
static void find_globals(void)
{
    global_symbols = symbol_table_init();
    // TODO: Create symbols for all global definitions (global variables, arrays and functions), and add them to the global symbol table
    // Functions can also get their local symbol tables created here, and symbols for all its parameters
    
    for (int i = 0; i < root->n_children; ++i)
    {
        node_t *child = root->children[i];
        switch (child->type)
        {
            case GLOBAL:
                break;
            case DECLARATION:
                for (int j = 0; j < child->n_children; ++j)
                {
                    symbol_t *symbol = (symbol_t *) malloc(sizeof(symbol_t));
                    symbol->name = child->children[j]->data;
                    symbol->type = SYMBOL_GLOBAL_VAR;
                    symbol->node = child->children[j];
                    symbol_table_insert(global_symbols, symbol);
                }
                break;
            case VARIABLE_LIST:
                break;
            case VARIABLE:
                break;
            case ARRAY_DECLARATION:
            {
                symbol_t *symbol = (symbol_t *) malloc(sizeof(symbol_t));
                symbol->name = child->children[0]->data;
                symbol->type = SYMBOL_GLOBAL_ARRAY;
                symbol->node = child;
                symbol_table_insert(global_symbols, symbol);
            }
                break;
            case ARRAY_INDEXING:
                break;
            case FUNCTION:
            {
                symbol_t *symbol = (symbol_t *) malloc(sizeof(symbol_t));
                symbol->name = child->children[0]->data;
                symbol->type = SYMBOL_FUNCTION;
                symbol->node = child;
                symbol->function_symtable = symbol_table_init();
                symbol->function_symtable->hashmap->backup = global_symbols->hashmap;
                symbol_table_insert(global_symbols, symbol);
            }
                break;
            case PARAMETER_LIST:
                break;
            case STATEMENT:
                break;
            case BLOCK:
                break;
            case DECLARATION_LIST:
                break;
            case STATEMENT_LIST:
                break;
            case ASSIGNMENT_STATEMENT:
                break;
            case RETURN_STATEMENT:
                break;
            case PRINT_STATEMENT:
                break;
            case PRINT_LIST:
                break;
            case PRINT_ITEM:
                break;
            case BREAK_STATEMENT:
                break;
            case IF_STATEMENT:
                break;
            case WHILE_STATEMENT:
                break;
            case RELATION:
                break;
            case FOR_STATEMENT:
                break;
            case ARGUMENT_LIST:
                break;
            case EXPRESSION_LIST:
                break;
            case EXPRESSION:
                break;
            case IDENTIFIER_DATA:
                break;
            case NUMBER_DATA:
                break;
            case STRING_DATA:
                break;
            case _NODE_COUNT:
                break;
            default:
                break;
        }
    }
    
    //Add local variables to function scopes
    for (int i = 0; i < global_symbols->n_symbols; ++i)
        if (global_symbols->symbols[i]->type == SYMBOL_FUNCTION)
            bind_names(global_symbols->symbols[i]->function_symtable, global_symbols->symbols[i]->node);
}

/* A recursive function that traverses the body of a function, and:
 *  - Adds variable declarations to the function's local symbol table.
 *  - Pushes and pops local variable scopes when entering blocks.
 *  - Binds identifiers to the symbol it references.
 *  - Inserts STRING_DATA nodes' data into the global string list, and replaces it with its list position.
 */
static void bind_names(symbol_table_t *local_symbols, node_t *node)
{
    // TODO: Implement bind_names, doing all the things described above
    // Tip: See symbol_hashmap_init () in symbol_table.h, to make new hashmaps for new scopes.
    // Remember the symbol_hashmap_t's backup pointer, allowing you to make linked lists.
    
    switch (node->type)
    {
        case FUNCTION:
            for (int i = 0; i < node->n_children; ++i)
                if (node->children[i]->type != IDENTIFIER_DATA)
                    bind_names(local_symbols, node->children[i]);
            break;
        case PARAMETER_LIST:
            for (int p = 0; p < node->n_children; ++p)
            {
                symbol_t *symbol = (symbol_t *) malloc(sizeof(symbol_t));
                symbol->name = node->children[p]->data;
                symbol->type = SYMBOL_PARAMETER;
                symbol->node = node->children[p];
                symbol_table_insert(local_symbols, symbol);
            }
            break;
        case BLOCK:
        {
            symbol_hashmap_t *local_hashmap = symbol_hashmap_init();
            local_hashmap->backup = local_symbols->hashmap;
            local_symbols->hashmap = local_hashmap;
            for (int p = 0; p < node->n_children; ++p)
                bind_names(local_symbols, node->children[p]);
            local_symbols->hashmap = local_hashmap->backup;
            symbol_hashmap_destroy(local_hashmap);
        }
            break;
        
        case DECLARATION_LIST:
            for (int i = 0; i < node->n_children; ++i)
                bind_names(local_symbols, node->children[i]);
            break;
        
        case DECLARATION:
            for (int d = 0; d < node->n_children; ++d)
            {
                symbol_t *symbol = (symbol_t *) malloc(sizeof(symbol_t));
                symbol->name = node->children[d]->data;
                symbol->type = SYMBOL_LOCAL_VAR;
                symbol->node = node->children[d];
                symbol_table_insert(local_symbols, symbol);
            }
            break;
        
        case STATEMENT_LIST:
            for (int s = 0; s < node->n_children; ++s)
                bind_names(local_symbols, node->children[s]);
            break;
        
        case PRINT_STATEMENT:
            for (int p = 0; p < node->n_children; ++p)
                bind_names(local_symbols, node->children[p]);
            break;
            
        case STRING_DATA:
        {
            size_t index = add_string(node->data);
            node->data = (int64_t *) malloc(sizeof(int64_t));
            *(int64_t *) node->data = (int64_t) index;
        }
            break;
        
        case IDENTIFIER_DATA:
            node->symbol = symbol_hashmap_lookup(local_symbols->hashmap, node->data);
            break;
        
        default:
            for (int c = 0; c < node->n_children; ++c)
                bind_names(local_symbols, node->children[c]);
            break;
    }
    
}

/* Prints the given symbol table, with sequence number, symbol names and types.
 * When printing function symbols, its local symbol table is recursively printed, with indentation.
 */
static void print_symbol_table(symbol_table_t *table, int nesting)
{
    // TODO: Output the given symbol table
    
    // TIP: Use SYMBOL_TYPE_NAMES[ my_sybmol->type ] to get a human readable string for each symbol type
    
    for (int i = 0; i < table->n_symbols; ++i)
    {
        symbol_t *symbol = table->symbols[i];
        printf("%*s%zu: %s(%s)\n", nesting, "", symbol->sequence_number, SYMBOL_TYPE_NAMES[symbol->type],
               symbol->name);
        if (symbol->function_symtable != NULL)
            print_symbol_table(symbol->function_symtable, nesting + 4);
    }
    
}

/* Frees up the memory used by the global symbol table, all local symbol tables, and their symbols */
static void destroy_symbol_tables(void)
{
    // TODO: Implement cleanup. All symbols in the program are owned by exactly one symbol table.
    for (int i = 0; i < global_symbols->n_symbols; ++i)
    {
        symbol_t *symbol = global_symbols->symbols[i];
        if (symbol->type == SYMBOL_FUNCTION)
            symbol_table_destroy(symbol->function_symtable);
    }
    symbol_table_destroy(global_symbols);
}

/* Adds the given string to the global string list, resizing if needed.
 * Takes ownership of the string, and returns its position in the string list.
 */
static size_t add_string(char *string)
{
    // TODO: Write a helper function you can use during bind_names(),
    // to easily add a string into the dynamically growing string_list
    
    // If the list is full, resize the list
    if (string_list_len + 1 >= string_list_capacity)
    {
        string_list_capacity = string_list_capacity * 2 + 8;
        string_list = realloc(string_list, string_list_capacity * sizeof(char *));
    }
    
    string_list[string_list_len] = string;
    string_list_len++;
    
    return string_list_len - 1;
}

/* Prints all strings added to the global string list */
static void print_string_list(void)
{
    // TODO: Implement printing of the string list like so:
    // 0: "string 1"
    // 1: "some other string"
    
    for (int i = 0; i < string_list_len; ++i)
        printf("%d: %s\n", i, string_list[i]);
}

/* Frees all strings in the global string list, and the string list itself */
static void destroy_string_list(void)
{
    // TODO: Called during cleanup, free strings, and the memory used by the string list itself
    for (int i = 0; i < string_list_len; ++i)
        free(string_list[i]);
    
    free(string_list);
}
