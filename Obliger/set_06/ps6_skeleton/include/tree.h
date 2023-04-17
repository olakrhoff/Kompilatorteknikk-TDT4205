#ifndef TREE_H
#define TREE_H

#include <stdint.h>
#include "nodetypes.h"

/* This is the tree node structure, for the parse tree and abstract syntax tree */
typedef struct node
{
    node_type_t type;
    void* data; // Extra data describing the node. Only owned if node type ends in _DATA
    struct symbol *symbol; // Symbol table entry for nodes that declare symbols (not owned)
    uint64_t n_children;
    struct node **children;
} node_t;

/* Global root for parse tree and abstract syntax tree */
extern node_t *root;

// Export the node initializer function, needed by the parser
void node_init ( node_t * nd, node_type_t type, void *data, uint64_t n_children, ... );

void print_syntax_tree ( void );
void simplify_syntax_tree ( void );
void destroy_syntax_tree ( void );

// Special function used when syntax trees are output as graphviz graphs.
// Implemented in graphviz_output.c
void graphviz_node_print ( node_t *root );

#endif // TREE_H
