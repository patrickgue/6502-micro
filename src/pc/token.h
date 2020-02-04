/*
  token.h  Headerfile for token.c 
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


#ifndef TOKEN_H
#define TOKEN_H

#include <regex.h>
#include <stdbool.h>


enum e_token_type {PREPR, ARRITH, LOGIC, ASSING, FUNC, IF, ELSE, WHILE, LET, NUMBER_HEX, NUMBER, LABEL, STRING, SCOPE_OPEN, SCOPE_CLOSE, ARG_OPEN, ARG_CLOSE, STMT_SEP, TOKEN_TYPE_LENGTH};

typedef enum e_token_type token_type;

static char token_type_regex_str[18][32] = {
    /* PREPR       */ "(#[a-z]*)",
    /* ARRITH      */ "([-+\\*\\/%])",
    /* LOGIC       */ "([&|\\^])",
    /* ASSING      */ "(=)",
    /* FUNC        */ "(func)",
    /* IF          */ "(if)",
    /* ELSE        */ "(else)",
    /* WHILE       */ "(while)",
    /* LET         */ "(let)",
    /* NUMBER_HEX  */ "(0x[0-9A-z]+)",
    /* NUMBER      */ "([0-9]+)",
    /* LABEL       */ "([A-z][A-z0-9_]*)",
    /* STRING      */ "(\"[A-z0-9.,!?:… ]+\")",
    /* SCOPE_OPEN  */ "({)",
    /* SCOPE_CLOSE */ "(})",
    /* ARG_OPEN    */ "(\\()",
    /* ARG_CLOSE   */ "(\\))",
    /* STMT_SEP    */ "([\n;,])",
  };

struct s_token 
{
    token_type type;
    char text[32];
};

typedef struct s_token token;



struct s_token_scope_tree;
typedef struct s_token_scope_tree token_scope_tree;
struct s_token_scope_tree_element;
typedef struct s_token_scope_tree_element token_scope_tree_element;

enum e_token_scope_tree_element_type {TST_TOKEN, TST_TREE, TST_ARGS};
typedef enum e_token_scope_tree_element_type token_scope_tree_element_type;

struct s_token_scope_tree {
    token_scope_tree_element *elements;
    signed int elements_size;
};

struct s_token_scope_tree_element {
    token_scope_tree_element_type type;
    token *atoken;
    token_scope_tree *tree;
};


int 
tokenize(char *, token**);

void 
init_token_type_regex(regex_t **);

int 
prepare_token_scope_tree(token_scope_tree **, token*, int, int, bool);

#endif
