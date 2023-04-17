#define NODETYPES_IMPLEMENTATION
#include <vslc.h>

/* Global root for parse tree and abstract syntax tree */
node_t *root;

// Tasks
static void node_print ( node_t *node, int nesting );
static void node_finalize ( node_t *discard );
static void destroy_subtree ( node_t *discard );
static node_t* simplify_tree ( node_t *node );
static node_t* constant_fold_expression( node_t *node );
static node_t* replace_for_statement ( node_t* for_node );

/* External interface */
void print_syntax_tree ()
{
    if ( getenv("GRAPHVIZ_OUTPUT") != NULL )
        graphviz_node_print( root );
    else
        node_print ( root, 0 );
}

void simplify_syntax_tree ( void )
{
    root = simplify_tree ( root );
}

void destroy_syntax_tree ( void )
{
    destroy_subtree ( root );
    root = NULL;
}

/* Initialize a node with type, data, and children */
void node_init ( node_t *nd, node_type_t type, void *data, uint64_t n_children, ... )
{
    va_list child_list;
    *nd = (node_t) {
        .type = type,
        .data = data,
        .symbol = NULL,
        .n_children = n_children,
        .children = (node_t **) malloc ( n_children * sizeof(node_t *) )
    };
    va_start ( child_list, n_children );
    for ( uint64_t i=0; i<n_children; i++ )
        nd->children[i] = va_arg ( child_list, node_t * );
    va_end ( child_list );
}

/* Inner workings */
/* Prints out the given node and all its children recursively */
static void node_print ( node_t *node, int nesting )
{
    if ( node != NULL )
    {
        printf ( "%*s%s", nesting, "", node_strings[node->type] );
        if ( node->type == IDENTIFIER_DATA ||
             node->type == EXPRESSION ||
             node->type == RELATION)
            printf ( "(%s)", (char *) node->data );
        else if ( node->type == NUMBER_DATA )
            printf ( "(%ld)", *(int64_t *)node->data );
        else if ( node->type == STRING_DATA ) {
            if ( node->data && *(char *)node->data != '"')
                printf ( "(#%ld)", *(int64_t *)node->data );
            else
                printf ( "(%s)", (char *) node->data );
        }

        // If the node has a symbol, print that as well
        if ( node->symbol )
            printf( " %s(%ld)", SYMBOL_TYPE_NAMES[node->symbol->type], node->symbol->sequence_number );

        putchar ( '\n' );
        for ( int64_t i=0; i<node->n_children; i++ )
            node_print ( node->children[i], nesting+1 );
    }
    else
        printf ( "%*s(NULL)\n", nesting, "" );
}

/* Frees the memory owned by the given node, but does not touch its children */
static void node_finalize ( node_t *discard )
{
    if ( discard == NULL )
        return;

    // Only free data if the data field is owned by the node
    switch ( discard->type )
    {
        case IDENTIFIER_DATA:
        case NUMBER_DATA:
        case STRING_DATA:
            free ( discard->data );
        default:
            break;
    }
    free ( discard->children );
    free ( discard );
}

/* Recursively frees the memory owned by the given node, and all its children */
static void destroy_subtree ( node_t *discard )
{
    if ( discard != NULL )
    {
        for ( uint64_t i=0; i < discard->n_children; i++ )
            destroy_subtree ( discard->children[i] );
        node_finalize ( discard );
    }
}

/* Recursive function to convert a parse tree into an abstract syntax tree */
static node_t* simplify_tree ( node_t *node )
{
    if ( node == NULL )
        return NULL;

    // Simplify everything is the node's subtree before proceeding
    for ( uint64_t i = 0; i < node->n_children; i++ )
        node->children[i] = simplify_tree ( node->children[i] );

    switch ( node->type )
    {
        // Eliminate nodes of purely syntactic value.
        // These nodes only have one child, and carry no semantic value
        case PROGRAM:
        case GLOBAL:
        case STATEMENT:
        case PRINT_ITEM:
            assert ( node->n_children == 1 );
            node_t *result = node->children[0];
            node_finalize ( node );
            return result;

        // Nodes that only serve as a wrapper for a (optional) node below.
        // Removes the child and takes over its children as its own.
        case PARAMETER_LIST:
        case ARGUMENT_LIST:
        case PRINT_STATEMENT:
        case DECLARATION:
        case ARRAY_DECLARATION:
            if ( node->n_children == 1 ) {
                node_t *result = node->children[0];
                result->type = node->type;
                node_finalize ( node );
                return result;
            }
            break;

        // Flatten linked list structures.
        // Any list node with two children, has a list node to the left, and an element to the right.
        // We return the left list node, but with the right node appended to its list
        case GLOBAL_LIST:
        case VARIABLE_LIST:
        case DECLARATION_LIST:
        case STATEMENT_LIST:
        case PRINT_LIST:
        case EXPRESSION_LIST:
            // If we have 0 or 1 children, we are already done
            if ( node->n_children == 2 ) {
                node_t *result = node->children[0];
                result->n_children++;
                result->children = realloc ( result->children, result->n_children*sizeof(node_t) );
                result->children[result->n_children-1] = node->children[1];
                node_finalize ( node );
                return result;
            }
            break;

        // Do contstant folding, if possible
        // Also prunes expressions that are just wrapping atomic expressions
        case EXPRESSION:
            return constant_fold_expression ( node );

        case FOR_STATEMENT:
            return replace_for_statement ( node );

        default: break;
    }

    return node;
}

// Helper macros for manually building an AST
#define NODE(variable_name, ...)                        \
    node_t *variable_name = malloc(sizeof(node_t));     \
    node_init(variable_name, __VA_ARGS__)
// After an IDENTIFIER_NODE has been added to the tree, it can't be added again
// This macro replaces the given variable with a new node, containting a copy of the data
#define DUPLICATE_VARIABLE(variable) do {                    \
        char *identifier = strdup(variable->data);           \
        variable = malloc(sizeof(node_t));                   \
        node_init(variable, IDENTIFIER_DATA, identifier, 0); \
    } while (false)
#define FOR_END_VARIABLE "__FOR_END__"

static node_t* constant_fold_expression( node_t *node )
{
    assert ( node->type == EXPRESSION );

    char* operator = node->data;
    if ( operator == NULL ) // No operation means we are just a wrapper for some value node
    {
        assert ( node->n_children == 1 );
        node_t *result = node->children[0];
        node_finalize ( node );
        return result;
    }

    // We now know that we are some operation,
    // but can only do constant folding if all operands are NUMBER_DATA
    for ( int i = 0; i < node->n_children; i++ )
    {
        if (node->children[i]->type != NUMBER_DATA)
        {
            return node;
        }
    }

    int64_t result;
    if ( node->n_children == 2 )
    {
        assert ( operator != NULL );
        int64_t lhs = *(int64_t*)node->children[0]->data;
        int64_t rhs = *(int64_t*)node->children[1]->data;
        if ( strcmp(operator, "+" ) == 0 )
        {
            result = lhs + rhs;
        }
        else if ( strcmp(operator, "-" ) == 0 )
        {
            result = lhs - rhs;
        }
        else if ( strcmp(operator, "*" ) == 0 )
        {
            result = lhs * rhs;
        }
        else if ( strcmp(operator, "/" ) == 0 )
        {
            result = lhs / rhs;
        }
        else
        {
            return node;
        }
    }
    else if ( node->n_children == 1 )
    {
        int64_t operand = *(int64_t*)node->children[0]->data;
        if ( strcmp ( operator, "-" ) == 0 ) {
            result = -operand;
        } else {
            return node;
        }
    }

    // Destroy both the original node, and its children
    destroy_subtree ( node );

    int64_t *result_data = malloc ( sizeof(int64_t) );
    *result_data = result;
    NODE ( result_node, NUMBER_DATA, result_data, 0 );
    return result_node;
}

// Replaces the FOR_STATEMENT with a BLOCK.
// The block contains varables, setup, and a while loop
static node_t* replace_for_statement ( node_t* for_node )
{
    assert ( for_node->type == FOR_STATEMENT );

    node_t *variable = for_node->children[0];
    node_t *start_value = for_node->children[1];
    node_t *end_value = for_node->children[2];
    node_t *body = for_node->children[3];

    node_finalize ( for_node );

    // Make the declaration for both variables
    // var <variable>, __FOR_END__
    NODE ( end_variable, IDENTIFIER_DATA, strdup(FOR_END_VARIABLE), 0 );
    NODE ( declaration, DECLARATION, NULL, 2, variable, end_variable );
    NODE ( declaration_list, DECLARATION_LIST, NULL, 1, declaration );

    // make the assignments
    // <variable> := <start_value>
    // __FOR_END__ := <end_value>
    DUPLICATE_VARIABLE ( variable );
    NODE ( init_assignment, ASSIGNMENT_STATEMENT, NULL, 2, variable, start_value );
    DUPLICATE_VARIABLE ( end_variable );
    NODE ( end_assignment, ASSIGNMENT_STATEMENT, NULL, 2, end_variable, end_value );

    // make the relation
    // <variable> < __FOR_END__
    DUPLICATE_VARIABLE ( variable );
    DUPLICATE_VARIABLE ( end_variable );
    NODE ( relation, RELATION, strdup("<"), 2, variable, end_variable );

    // make the increment statement
    // <variable> := <variable> + 1
    DUPLICATE_VARIABLE ( variable );
    int64_t *one = malloc ( sizeof(int64_t) );
    *one = 1;
    NODE ( one_node, NUMBER_DATA, one, 0 );
    NODE ( variable_plus_one, EXPRESSION, strdup("+"), 2, variable, one_node );
    DUPLICATE_VARIABLE ( variable );
    NODE ( increment, ASSIGNMENT_STATEMENT, NULL, 2, variable, variable_plus_one );

    // make a block statement containing both the original for-loop body, and the increment
    // begin
    //     <body>
    //     <variable> := <variable> + 1
    // end
    NODE ( inner_statement_list, STATEMENT_LIST, NULL, 2, body, increment );
    NODE ( inner_block, BLOCK, NULL, 1, inner_statement_list );

    // Make the while loop like so:
    // while <variable> < __FOR_END__ begin
    //     <body>
    //     <variable> := <variable> + 1
    // end
    NODE ( while_node, WHILE_STATEMENT, NULL, 2, relation, inner_block );

    // Put it all together into a statement list
    // <variable> := <start_value>
    // __FOR_END__ := <end_value>
    // while <variable> < __FOR_END__ begin
    //     <body>
    //     <variable> := <variable> + 1
    // end
    NODE ( result_statement_list, STATEMENT_LIST, NULL, 3, init_assignment, end_assignment, while_node );

    // Include the declaration of the two local variables
    NODE ( result, BLOCK, NULL, 2, declaration_list, result_statement_list );

    return result;
}
