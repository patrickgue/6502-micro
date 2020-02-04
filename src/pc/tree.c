#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"
#include "dbg.h"

#include "tree.h"


void
build_tree(tree_scope** root_scope, token* tokens, int tokens_length)
{
  token_scope_tree *tree;
  prepare_token_scope_tree(&tree, tokens, 0, tokens_length, false);

  //dbg_print_token_scope_tree(tree,0);
  build_scope(root_scope, tree, 0, tree->elements_size);

  dbg_print_scope(*root_scope, 0);
  
}



void
build_scope(tree_scope** scope, token_scope_tree* tokens, int tokens_start, int tokens_end)
{
  int start_index = 0;

  for (int i = tokens_start; i < tokens_end; i++) {
    //bool add_statement = false;
    tree_statement* new_statement = NULL;
    if (tokens->elements[i].type == TST_TOKEN) {
      if (tokens->elements[i].atoken->type == STMT_SEP || tokens->elements[i].atoken->type == ARG_CLOSE || tokens->elements[i+1].type == TST_ARGS) {
        new_statement =
          build_statement(scope, tokens, start_index, i +1);
        add_statement(scope, new_statement);
              
        start_index = i+1;

      }
    }
    
    if(tokens->elements[i].type == TST_TREE){
      new_statement = create_empty_statement(T_CONTROL);
      switch(tokens->elements[i].atoken->type) {
      case WHILE:
        new_statement->control_type = TC_WHILE;
        break;
      case IF:
        new_statement->control_type = TC_IF;
        break;
      case FUNC:
        new_statement->control_type = TC_FUNC;
        break;
      default:
        break;
      }

      new_statement->sub_scope = (tree_scope*) malloc(sizeof(tree_scope));
      new_statement->sub_scope->statements = (tree_statement*) malloc(0);
      new_statement->sub_scope->statements_length = 0;
      build_scope(&(new_statement->sub_scope), tokens->elements[i].tree, 0, tokens->elements[i].tree->elements_size);
      add_statement(scope, new_statement);
            
      start_index = i+1;
    }

    if(tokens->elements[i].type == TST_ARGS) {
      new_statement = create_empty_statement(T_ARGS);
      new_statement->args_scope = (tree_scope*) malloc(sizeof(tree_scope));
      new_statement->args_scope->statements = (tree_statement*) malloc(0);
      new_statement->args_scope->statements_length = 0;
      build_scope(&(new_statement->args_scope), tokens->elements[i].tree, 0, tokens->elements[i].tree->elements_size);
      add_statement(scope, new_statement);
            
      start_index = i+1;

    }
  }
}

void 
add_statement(tree_scope **scope, tree_statement* new_statement) 
{
  (*scope)->statements = (tree_statement*)realloc(
    (*scope)->statements,
    sizeof(tree_statement) * ((*scope)->statements_length + 1));
  (*scope)->statements[(*scope)->statements_length] = *new_statement;
  (*scope)->statements_length++;
}

tree_statement*
build_statement(tree_scope** current_scope,
                token_scope_tree* tokens,
                int tokens_start,
                int tokens_end)
{
  tree_statement* new_statement;
  if (contains_token_type(LET, tokens, tokens_start, tokens_end) == 0) {
    new_statement = create_empty_statement(T_DEF);
    tree_statement* right_statement =
      build_statement(current_scope, tokens, tokens_start + 1, tokens_end);
    new_statement->right_type = right_statement->statement_type;
    new_statement->right = right_statement;
  } else if (contains_token_type(ASSING, tokens, tokens_start, tokens_end) ==
             1) {
    new_statement = create_empty_statement(T_ASSIGN);
    new_statement->left_type = T_ATOMIC;
    new_statement->left = build_atomic_statement(tokens, tokens_start);
    tree_statement* right_statement =
      build_statement(current_scope, tokens, tokens_start + 2, tokens_end);
    new_statement->right_type = right_statement->statement_type;
    new_statement->right = right_statement;
  } else if (contains_token_type(ARRITH, tokens, tokens_start, tokens_end) !=
             -1) {
    int index = contains_token_type(ARRITH, tokens, tokens_start, tokens_end);
    new_statement = create_empty_statement(T_ARRITH);

    tree_statement* left_statement = build_statement(
      current_scope, tokens, tokens_start, tokens_start + index);
    new_statement->left_type = left_statement->statement_type;
    new_statement->left = left_statement;
    tree_statement* right_statement = build_statement(
      current_scope, tokens, tokens_start + index + 1, tokens_end);
    new_statement->right_type = right_statement->statement_type;
    new_statement->right = right_statement;
  } 
  else if (contains_token_type(LABEL, tokens, tokens_start, tokens_end) == 0 ||
             contains_token_type(STRING, tokens, tokens_start, tokens_end) == 0 ||
             contains_token_type(NUMBER, tokens, tokens_start, tokens_end) == 0 ||
             contains_token_type(NUMBER_HEX, tokens, tokens_start, tokens_end) == 0) {
    new_statement =
      (tree_statement*)build_atomic_statement(tokens, tokens_start);
  } else {
    printf("ERROR (%d not implemented, with index %d, size %d)\n", tokens->elements[tokens_start].atoken->type, tokens_start, tokens_end);
    new_statement = create_empty_statement(T_NONE);
  }
  return new_statement;
}

tree_statement*
build_atomic_statement(token_scope_tree* tokens, int tokens_index)
{
  tree_statement* new_statement = create_empty_statement(T_ATOMIC);
  new_statement->atomic = (tree_atomic*)malloc(sizeof(tree_atomic));
  if (tokens->elements[tokens_index].atoken->type == NUMBER ||
      tokens->elements[tokens_index].atoken->type == NUMBER_HEX) {
    new_statement->atomic->number =
      (int)strtol(tokens->elements[tokens_index].atoken->text,
                  NULL,
                  tokens->elements[tokens_index].atoken->type == NUMBER_HEX ? 16 : 10);
    new_statement->atomic->type = A_NUMBER;
  } else if (tokens->elements[tokens_index].atoken->type == STRING) {
    strcpy(new_statement->atomic->string, tokens->elements[tokens_index].atoken->text);
    new_statement->atomic->type = A_STRING;
  } else if (tokens->elements[tokens_index].atoken->type == LABEL) {
    strcpy(new_statement->atomic->reference, tokens->elements[tokens_index].atoken->text);
    new_statement->atomic->type = A_REFERENCE;
  } else {
    printf("Not atomic\n");
  }

  return new_statement;
}

tree_statement * 
create_empty_statement(tree_statement_type type) {
  tree_statement *statement = (tree_statement*) malloc(sizeof(tree_statement));
  statement->statement_type = type;
  statement->right_type = T_NONE;
  statement->left_type = T_NONE;
  return statement;
}

int
contains_token_type(token_type type,
                    token_scope_tree* tokens,
                    int tokens_start,
                    int tokens_end)
{
  for (int i = tokens_start; i < tokens_end; i++) {
    if (tokens->elements[i].atoken->type == type)
      return i - tokens_start;
  }
  return -1;
}
