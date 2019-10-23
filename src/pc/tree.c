#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>


#include "tree.h"
#include "token.h"


void 
build_tree(tree_scope **root_scope, token *tokens, int tokens_length)
{


    build_scope(root_scope, tokens, 0, tokens_length);

    printf("Scope {\n");
    for(int i = 0; i < (*root_scope)->statements_length; i++) {
        printf("  Statement #%i:\n",i);
        dbug_print_statement(&(*root_scope)->statements[i],1);
    }
    printf("}");
}

void 
build_scope(tree_scope **scope, token *tokens, int tokens_start, int tokens_end) 
{
    int start_index = 0;

    for(int i = tokens_start; i < tokens_end; i++) {
        if(tokens[i].type == STMT_SEP) {
            tree_statement* new_statement = build_statement(scope, tokens, start_index, i);
            (*scope)->statements = (tree_statement*) realloc((*scope)->statements, sizeof(tree_statement) * ((*scope)->statements_length + 1));
            (*scope)->statements[(*scope)->statements_length] = *new_statement;
            (*scope)->statements_length++;
            start_index = i+1;
        }
    }
}

tree_statement *
build_statement(tree_scope **current_scope, token *tokens, int tokens_start, int tokens_end)
{
    tree_statement *new_statement = (tree_statement*) malloc(sizeof(tree_statement));
    if(contains_statement_type(LET, tokens, tokens_start, tokens_end) == 0) {
        new_statement->statement_type = T_DEF;
        new_statement->left_type = T_NONE;
        tree_statement *right_statement = build_statement(current_scope, tokens, tokens_start+1, tokens_end);
        new_statement->right_type = right_statement->statement_type;
        new_statement->right = right_statement;
    }
    else if(contains_statement_type(ASSING, tokens, tokens_start, tokens_end) == 1) {
        new_statement->statement_type = T_ASSIGN;
        new_statement->left_type = T_ATOMIC;
        new_statement->left = build_atomic_statement(tokens, tokens_start);
        tree_statement *right_statement = build_statement(current_scope, tokens, tokens_start+2, tokens_end);
        new_statement->right_type = right_statement->statement_type;
        new_statement->right = right_statement;
    }
    else if(contains_statement_type(ARRITH, tokens, tokens_start, tokens_end) != -1) {
        int index = contains_statement_type(ARRITH, tokens, tokens_start, tokens_end);
        new_statement->statement_type = T_ARRITH;

        tree_statement *left_statement = build_statement(current_scope, tokens, tokens_start, tokens_start + index);
        new_statement->left_type = left_statement->statement_type;
        new_statement->left = left_statement;
        tree_statement *right_statement = build_statement(current_scope, tokens, tokens_start+index+1, tokens_end);
        new_statement->right_type = right_statement->statement_type;
        new_statement->right = right_statement;
    }
    else if(tokens_end - tokens_start == 1) {
        free(new_statement);
        new_statement = (tree_statement*) build_atomic_statement(tokens, tokens_start);
    }
    else {
        printf("ERROR\n");
    }
    return new_statement;
}

tree_statement *
build_atomic_statement(token *tokens, int tokens_index)
{
    tree_statement *new_statement = (tree_statement*) malloc(sizeof(tree_statement));
    new_statement->statement_type = T_ATOMIC;
    new_statement->atomic = (tree_atomic*) malloc(sizeof(tree_atomic));
    if(tokens[tokens_index].type == NUMBER || tokens[tokens_index].type == NUMBER_HEX) {
        new_statement->atomic->number = (int) strtol(tokens[tokens_index].text, NULL, tokens[tokens_index].type == NUMBER_HEX ? 16 : 10);
        new_statement->atomic->type = A_NUMBER;
    }
    else if(tokens[tokens_index].type == STRING) {
        strcpy(new_statement->atomic->string, tokens[tokens_index].text);
        new_statement->atomic->type = A_STRING;
    }
    else if(tokens[tokens_index].type == LABEL) {
        strcpy(new_statement->atomic->reference, tokens[tokens_index].text);
        new_statement->atomic->type = A_REFERENCE;
    }
    else {
        printf("Not atomic\n");
    }
    
    return new_statement;
}


int 
contains_statement_type(token_type type, token *tokens, int tokens_start, int tokens_end) {
    for(int i = tokens_start; i < tokens_end; i++) {
        if(tokens[i].type == type)
            return i - tokens_start;
    }
    return -1;
}

void
dbug_print_statement(tree_statement *statement, int level) 
{
    for(int i = 0; i < level; i++) {
        printf("  ");
    }
    if(statement->statement_type == T_ATOMIC) {
        printf("[ATOMIC]: ");
        if(((tree_atomic*)statement->atomic)->type == A_NUMBER) {
            printf("NUMBER (%d)\n",((tree_atomic*)statement->atomic)->number);
        }
        else if(((tree_atomic*)statement->atomic)->type == A_STRING) {
            printf("STRING: (\"%s\")\n",((tree_atomic*)statement->atomic)->string);
        }
        else if(((tree_atomic*)statement->atomic)->type == A_REFERENCE) {
            printf("REFERENCE: (\"%s\")\n",((tree_atomic*)statement->atomic)->reference);
        }
    }
    else {
        if(statement->statement_type == T_ASSIGN) { printf("[ASSIGN]:\n"); }
        else if(statement->statement_type == T_FUNCALL) {printf("[FUNCALL]:\n");}
        else if(statement->statement_type == T_CONTROL) {printf("[CONTROLL]:\n");}
        else if(statement->statement_type == T_ARRITH) {printf("[ARRITH]:\n");}
        else if(statement->statement_type == T_DEF) {printf("[DEF]:\n");}
        
        if(statement->left_type != T_NONE) {
            for(int i = 0; i < level; i++) {
                printf("  ");
            }
            printf("-> LEFT:\n");
            dbug_print_statement((tree_statement*)statement->left, level+1);
        }
        if(statement->right_type != T_NONE) {
            for(int i = 0; i < level; i++) {
                printf("  ");
            }
            printf("-> RIGHT:\n");
            dbug_print_statement((tree_statement*)statement->right, level+1);
        }
    }
}