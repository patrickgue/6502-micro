/*
  tree.h  Headerfile for tree.c 
  Copyright (C) 2019 Patrick Günthard

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef TREE_H
#define TREE_H

#include <stdbool.h>
#include "token.h"

struct s_tree_scope {
    tree_statement *statements;
    unsigned int statements_length;
};

typedef struct s_tree_scope tree_scope;


enum e_tree_statement_type {T_DEF, T_ASSIGN, T_FUNCALL, T_CONTROL, T_ARRITH, T_ATOMIC, T_NONE};
typedef enum e_tree_statement_type tree_statement_type;


enum e_tree_atomic_type {A_NUMBER,A_STRING,A_REFERENCE};
typedef enum e_tree_atomic_type tree_atomic_type;

struct s_tree_atomic {
    tree_atomic_type type;
    char reference[32];
    int number;
    char string[128];
};

typedef struct s_tree_atomic tree_atomic;

struct s_tree_statement {
    tree_statement_type statement_type;
    tree_atomic *atomic;
    void *left;
    tree_statement_type left_type;
    void *right;
    tree_statement_type right_type;
    tree_scope *sub_scope;
};

typedef struct s_tree_statement tree_statement;


void
build_tree(tree_scope **, token *, int);

void 
build_scope(tree_scope **scope, token *tokens, int tokens_start, int tokens_end);

tree_statement *
build_statement(tree_scope **, token *, int, int);

tree_statement *
build_atomic_statement(token *, int);


int 
contains_statement_type(token_type, token *, int, int);

void 
dbug_print_statement(tree_statement*,int);

#endif
