#include <vslc.h>

// This header defines a bunch of macros we can use to emit assembly to stdout
#include "emit.h"

// In the System V calling convention, the first 6 integer parameters are passed in registers
#define NUM_REGISTER_PARAMS 6
static const char *REGISTER_PARAMS[6] = {RDI, RSI, RDX, RCX, R8, R9};

// Takes in a symbol of type SYMBOL_FUNCTION, and returns how many parameters the function takes
#define FUNC_PARAM_COUNT(func) ((func)->node->children[1]->n_children)

static void generate_stringtable(void);

static void generate_global_variables(void);

static void generate_function(symbol_t *function);

static void generate_statement(node_t *node);

static void generate_main(symbol_t *first);

/* Entry point for code generation */
void generate_program(void)
{
    generate_stringtable();
    generate_global_variables();
    
    // TODO: (Part of 2.3)
    // For each function in global_symbols, generate it using generate_function ()
    DIRECTIVE(".section __TEXT, __text");
    for (int i = 0; i < global_symbols->n_symbols; ++i)
        if (global_symbols->symbols[i]->type == SYMBOL_FUNCTION)
            generate_function(global_symbols->symbols[i]);
    // TODO: (Also part of 2.3)
    // In VSL, the topmost function in a program is its entry point.
    // We want to be able to take parameters from the command line,
    // and have them be sent into the entry point function.
    //
    // Due to the fact that parameters are all passed as strings,
    // and passed as the (argc, argv)-pair, we need to make a wrapper for our entry function.
    // This wrapper handles string -> int64_t conversion, and is already implemented.
    // call generate_main ( <entry point function symbol> );
    for (int i = 0; i < global_symbols->n_symbols; ++i)
        if (global_symbols->symbols[i]->type == SYMBOL_FUNCTION)
        {
            generate_main(global_symbols->symbols[i]);
            break;
        }
}

/* Prints one .asciz entry for each string in the global string_list */
static void generate_stringtable(void)
{
    //DIRECTIVE ( ".section .rodata" );
    DIRECTIVE (".section __TEXT, __cstring"); //mach-o variant
    // These strings are used by printf
    DIRECTIVE ("intout: .asciz \"%s\"", "%ld ");
    DIRECTIVE ("strout: .asciz \"%s\"", "%s ");
    // This string is used by the entry point-wrapper
    DIRECTIVE ("errout: .asciz \"%s\"", "Wrong number of arguments");
    
    // TODO 2.1: Print all strings in the program here, with labels you can refer to later
    
    for (int i = 0; i < string_list_len; ++i)
    DIRECTIVE("string%d: .asciz %s", i, string_list[i]);
}

/* Prints .zero entries in the .bss section to allocate room for global variables and arrays */
static void generate_global_variables(void)
{
    // TODO 2.2: Generate a section where global variables and global arrays can live
    // Give each a label you can find later, and the appropriate size.
    // Remember to mangle the name in some way, to avoid collisions if a variable is called e.g. "main"
    
    DIRECTIVE(".section __DATA, __bss");
    DIRECTIVE(".align 8");
    for (int i = 0; i < global_symbols->n_symbols; ++i)
    {
        if (global_symbols->symbols[i]->type != SYMBOL_GLOBAL_ARRAY &&
            global_symbols->symbols[i]->type != SYMBOL_GLOBAL_VAR)
            continue; // Only handle global variables (and arrays) here.
        int32_t array_length = 1;
        if (global_symbols->symbols[i]->type == SYMBOL_GLOBAL_ARRAY)
            array_length = *(int32_t *) global_symbols->symbols[i]->node->children[1]->data; //Get array length
        DIRECTIVE(".%s:\t.zero %d", global_symbols->symbols[i]->name, 8 * array_length);
    }
}

/* Prints the entry point. preable, statements and epilouge of the given function */
static void generate_function(symbol_t *function)
{
    // TODO: 2.3
    // TODO: 2.3.1 Do the prologue, including call frame building and parameter pushing
    if (strcmp(function->name, "main") == 0)
    {
        function->name = realloc(function->name, 6);
        memcpy(function->name, "start", 6);
    }
    LABEL("_%s", function->name);
    
    PUSHQ(RBP); //Push the base pointer
    MOVQ(RSP, RBP); //Set the stack pointer to be the new base pointer
    int32_t num_params = function->node->children[1]->n_children;
    if (num_params > 0)
    {
        PUSHQ(RDI);
        num_params--;
    }
    if (num_params > 0)
    {
        PUSHQ(RSI);
        num_params--;
    }
    if (num_params > 0)
    {
        PUSHQ(RDX);
        num_params--;
    }
    if (num_params > 0)
    {
        PUSHQ(RCX);
        num_params--;
    }
    if (num_params > 0)
    {
        PUSHQ(R8);
        num_params--;
    }
    if (num_params > 0)
    {
        PUSHQ(R9);
        num_params--;
    }
    
    //For each local variable, element in the declaration list, push a space on the stack
    for (int i = 0; i < function->node->children[1]->n_children; ++i)
    PUSHQ("$0");
    
    
    
    // TODO: 2.4 the function body can be sent to generate_statement()
    generate_statement(function->node->children[2]); // generate the function body from the outer block
    
    node_t *block = function->node->children[2];
    node_t *statement_list = block->children[block->n_children - 1];
    node_t *would_be_return = statement_list->children[statement_list->n_children - 1];
    //If we do not have a return statement in the function, set RAX to zero.
    if (would_be_return->type != RETURN_STATEMENT)
        MOVQ("$0", RAX);
    
    // TODO: 2.3.2
    MOVQ(RBP, RSP); //Reset the stack pointer to the callers
    POPQ(RBP); //Reset the callers base pointer
    RET;
}

static void generate_function_call(node_t *call)
{
    // TODO 2.4.3
    //Here we need to set up the function call
    //First we must load the parameters into it's given registers
    node_t *parameters = call->children[1];
    uint64_t num_params = parameters->n_children;
    if (num_params > 6)
    {
        for (uint64_t i = parameters->n_children - 1; i >= 6; --i)
        {
            //TODO: Push the excess parameters to the stack
        }
    }
    
    //Add the first (max 6) parameters to their designated registers.
    for (int i = 0; i < num_params && i < 6; ++i)
    {
        //The parameters to the function can be of three types:
        //LOCAL VAR, GLOBAL VAR, and PARAMETER
        if (parameters->children[i]->type == NUMBER_DATA)
        {
        
        }
        else
        {
            switch (parameters->children[i]->symbol->type)
            {
                case SYMBOL_LOCAL_VAR:
                    break;
                case SYMBOL_PARAMETER:
                {
                    //Extract the stack place of the first argument
                    uint64_t seq_num = parameters->children[0]->symbol->sequence_number;
                    int offset = (1 + seq_num) * 8;
                    char *source = malloc(9 * sizeof(char)); //TODO: Ensure this is cleaned up
                    memcpy(source, "-8(%rbp)", 9); //TODO: This is gonna fail for other arguments
                    source[1] = (char) (offset + '0');
                    MOVQ(source, RAX);
                }
                break;
                case SYMBOL_GLOBAL_VAR:
                    break;
                case SYMBOL_GLOBAL_ARRAY:
                    exit(10); //TODO: Might be a GLOBAL ARRAY as well, figure this out
                    break;
                    default:
                        printf("Parameter (symbol) type not recognised");
                        exit(9);
            }
        }
        //Then we move the value into it's respective register
        switch (i)
        {
            case 0:
            MOVQ(RAX, RDI);
                break;
            case 1:
            MOVQ(RAX, RSI);
                break;
            case 2:
            MOVQ(RAX, RDX);
                break;
            case 3:
            MOVQ(RAX, RCX);
                break;
            case 4:
            MOVQ(RAX, R8);
                break;
            case 5:
            MOVQ(RAX, R9);
                break;
            default:
                printf("This is bad, should not loop more than the 6 first params");
                exit(11);
        }
    }
    //Now the function frame has been sat up for the call, we then call the function
    EMIT("call _%s", (char *) call->children[0]->data);
    
}

static void generate_expression(node_t *expression)
{
    // TODO: 2.4.1
    switch (expression->type)
    {
        case NUMBER_DATA:
            EMIT("movq $%d, %s", *(int32_t *) expression->data, RAX);
            break;
        case IDENTIFIER_DATA:
        {
            //TODO: Handle non-globals
            EMIT("movq .%s(%%rip), %s", expression->symbol->name, RAX);
        }
            break;
        case ARRAY_INDEXING:
        {
            node_t *identifier = expression->children[0];
            node_t *index = expression->children[1];
            //Evaluate the index
            generate_expression(index);
            MOVQ(RAX, R10); //Index
            //Load the base address of the array into R11
            EMIT("leaq .%s(%%rip), %s", identifier->symbol->name, R11);
            
            EMIT("movq (%s, %s, 8), %s", R11, R10, RAX);
            return;
        }
        case EXPRESSION:
            /*
                expression -> expression ’+’ expression
                | expression ’-’ expression
                | expression ’*’ expression
                | expression ’/’ expression
                | ’-’ expression
                | ’(’ expression ’)’
                | number
                | identifier
                | array_indexing
                | identifier ’(’ argument_list ’)’
                
                argument_list -> expression_list
                | ε
                
                expression_list -> expression
                | expression_list ’,’ expression
             */
            if (expression->n_children == 1)
            {
                if (expression->children[0]->type == EXPRESSION)
                {
                    /*
                        | ’-’ expression
                        | ’(’ expression ’)’
                     */
                    generate_expression(expression->children[0]);
                    if (*(char *) expression->data == '-') //Multiply the evaluated expression with minus one
                    {
                        MOVQ("$-1", R10);
                        IMULQ(R10, RAX);
                    }
                }
                else
                {
                    /*
                        | number
                        | identifier
                        | array_indexing
                     */
                    generate_expression(expression->children[0]);
                }
            }
            else //Two arguments
            {
                /*
                    expression -> expression ’+’ expression
                    | expression ’-’ expression
                    | expression ’*’ expression
                    | expression ’/’ expression
                 */
                if (expression->children[0]->type != IDENTIFIER_DATA && expression->children[1]->type != ARGUMENT_LIST)
                {
                    generate_expression(expression->children[0]); // Left side
                    PUSHQ(RAX);
                    generate_expression(expression->children[1]); // Right side
                    MOVQ(RAX, R10);
                    POPQ(RAX);
                    switch (*(char *) expression->data)
                    {
                        //TODO: Fill this out
                        case '+':
                        ADDQ(R10, RAX);
                            break;
                        case '-':
                        SUBQ(R10, RAX);
                            break;
                        case '*':
                        IMULQ(R10, RAX);
                            break;
                        case '/':
                        IDIVQ(R10); //TODO: Check if this is correct, weird that it does not take two args
                            break;
                        default:
                            printf("Not recognised operator for expression");
                            exit(7);
                    }
                }
                else
                {
                    //I Think this is a function call
                    //identifier: function name
                    //argument_list: parameters
                    /*
                        | identifier ’(’ argument_list ’)’
                     */
                    generate_function_call(expression);
                }
            }
            break;
        default:
            printf("generate_expression failed\n");
            exit(4);
    }
    
}

static void generate_assignment_statement(node_t *statement)
{
    // TODO: 2.4.2
    // Right-hand side is always an expression
    generate_expression(statement->children[1]); //Should leave the return value on top of the stack
    PUSHQ(RAX); //Save result
    if (statement->children[0]->type == ARRAY_INDEXING)
    {
        //Evaluate the index
        generate_expression(statement->children[0]->children[1]);
        MOVQ(RAX, R10); //Index
        POPQ(RAX); //Right-hand value
        //Load the base address of the array into R11
        EMIT("leaq .%s(%%rip), %s", statement->children[0]->children[0]->symbol->name, R11);
    
        EMIT("movq %s, (%s, %s, 8)", RAX, R11, R10);
        return;
    }
    else if (statement->children[0]->type == IDENTIFIER_DATA)
    {
        if (statement->children[0]->symbol->type == SYMBOL_GLOBAL_VAR)
        {
            POPQ(RAX);
            EMIT("movq %s, .%s(%%rip)", RAX, statement->children[0]->symbol->name);
        }
        else if (statement->children[0]->symbol->type == SYMBOL_LOCAL_VAR)
        {
            exit(123);
        }
        else if (statement->children[0]->symbol->type == SYMBOL_PARAMETER)
        {
            exit(124);
        }
        
    }
}

static void generate_print_statement(node_t *statement)
{
    // TODO: 2.4.4
    for (int i = 0; i < statement->n_children; ++i)
    {
        switch (statement->children[i]->type)
        {
            case STRING_DATA:
            {
                EMIT("leaq string%d(%%rip), %s", (int) *(uint64_t *) statement->children[i]->data, RDI);
                MOVQ("$0", RSI);
            }
                break;
            case ARRAY_INDEXING:
            {
                //array_indexing -> identifier ’[’ expression ’]’
                generate_expression(statement->children[i]);
                //Value of expression in RAX
                
                LEAQ("intout", RDI);
                MOVQ(RAX, RSI);
            }
                break;
            case IDENTIFIER_DATA:
            {
                generate_expression(statement->children[i]);
                //Value of expression in RAX
                
                LEAQ("intout", RDI);
                MOVQ(RAX, RSI);
            }
            break;
            case EXPRESSION:
            {
                generate_expression(statement->children[i]);
                //Value of expression in RAX
                
                LEAQ("intout", RDI);
                MOVQ(RAX, RSI);
            }
                break;
            default:
                printf("Invalid print statement item: (%d) %s", statement->children[i]->type, SYMBOL_TYPE_NAMES[statement->children[i]->type]);
                exit(15);
        }
        
        
        MOVQ("$0", RAX);
        EMIT("call _printf");
    }
}

static void generate_return_statement(node_t *statement)
{
    // TODO: 2.4.5 Store the value in %rax and jump to the function epilogue
    
}

/* Recursively generate the given statement node, and all sub-statements. */
static void generate_statement(node_t *node)
{
    // TODO: 2.4
    switch (node->type)
    {
        //block -> BEGIN declaration_list statement_list END | BEGIN statement_list END
        case BLOCK:
        {
            int index = 0;
            if (node->n_children == 1)
                goto no_declaration_list;
            
            //There could be new local variables, push space to the stack
            if (node->children[0]->type != DECLARATION_LIST)
                exit(3); // Should be
            
            //Add all new local variables in the scope of the block
            for (int declaration = 0; declaration < node->children[0]->n_children; ++declaration)
            {
                for (int d = 0; d < node->children[0]->children[declaration]->n_children; ++d)
                {
                    PUSHQ("$0"); //Push space for each local variable on the stack
                }
            }
            index = 1; // If we had a declaration list then the statement list is on index 1 not 0
            no_declaration_list:
            //There should be a statement list
            if (node->children[index]->type != STATEMENT_LIST)
                exit(4); // Should be
            
            //statement_list -> statement | statement_list statement, after folding we have a statement_list node with only statements as children
            for (int i = 0; i < node->children[index]->n_children; ++i)
                generate_statement(node->children[index]->children[i]);
            
            if (node->n_children == 1) //If we did not have a declaration list we are done
                return;
            
            //After all recursive calls have been made, we leave the block scope, hence we pop the local variables
            for (int declaration = 0; declaration < node->children[0]->n_children; ++declaration)
            {
                for (int d = 0; d < node->children[0]->children[declaration]->n_children; ++d)
                {
                    POPQ("%R10"); //Push space for each local variable on the stack
                }
            }
        }
            break;
        case ASSIGNMENT_STATEMENT:
            generate_assignment_statement(node);
            break;
        case PRINT_STATEMENT:
            generate_print_statement(node);
            break;
        case RETURN_STATEMENT:
            break;
        default:
            printf("There is a case not handled in: generate_statement\n");
            exit(2); // Not handled, panic
    }
}

// Generates a wrapper, to be able to use a vsl function as our entrypoint
static void generate_main(symbol_t *first)
{
    // Make the globally available main function
    DIRECTIVE (".globl _main");
    LABEL ("_main");
    
    // Save old base pointer, and set new base pointer
    PUSHQ(RBP);
    MOVQ(RSP, RBP);
    
    // Which registers argc and argv are passed in
    const char *argc = RDI;
    const char *argv = RSI;
    
    const size_t expected_args = FUNC_PARAM_COUNT(first);
    
    
    SUBQ("$1", argc); // argc counts the name of the binary, so subtract that
    EMIT("cmpq $%ld, %s", expected_args, argc);
    JNE("ABORT"); // If the provdied number of arguments is not equal, go to the abort label
    
    if (expected_args == 0)
        goto skip_args; // No need to parse argv
    
    // Now we emit a loop to parse all parameters, and push them to the stack,
    // in right-to-left order
    
    // First move the argv pointer to the vert rightmost parameter
    EMIT("addq $%ld, %s", expected_args * 8, argv);
    
    // We use rcx as a counter, starting at the number of arguments
    MOVQ (argc, RCX);
    LABEL ("PARSE_ARGV"); // A loop to parse all parameters
    PUSHQ (argv); // push registers to caller save them
    PUSHQ (RCX);
    
    // Now call strtol to parse the argument
    EMIT ("movq (%s), %s", argv, RDI); // 1st argument, the char *
    MOVQ ("$0", RSI); // 2nd argument, a null pointer
    MOVQ ("$10", RDX); //3rd argument, we want base 10
    EMIT ("call strtol");
    
    // Restore caller saved registers
    POPQ (RCX);
    POPQ (argv);
    PUSHQ (RAX); // Store the parsed argument on the stack
    
    SUBQ ("$8", argv); // Point to the previous char*
    EMIT ("loop PARSE_ARGV"); // Loop uses RCX as a counter automatically
    
    // Now, pop up to 6 arguments into registers instead of stack
    for (size_t i = 0; i < expected_args && i < NUM_REGISTER_PARAMS; i++)
    POPQ (REGISTER_PARAMS[i]);
    
    skip_args:

EMIT ("call _%s", first->name);
    MOVQ (RAX, RDI); // Move the return value of the function into RDI
    EMIT ("call _exit"); // Exit with the return value as exit code
    
    LABEL ("ABORT"); // In case of incorrect number of arguments
    LEAQ ("errout", RDI);
    EMIT ("call _puts"); // print the errout string
    MOVQ ("$1", RDI);
    EMIT ("call _exit"); // Exit with return code 1
    
}
