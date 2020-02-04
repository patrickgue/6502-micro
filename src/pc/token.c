/*
  token.c  Parsing code to generate tokens
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

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../helper.h"

#include "token.h"

int 
prepare_token_scope_tree(token_scope_tree **token_tree, token* tokens, int tokens_start, int tokens_end, bool ignore_first)
{
  int i;
  (*token_tree) = (token_scope_tree*) malloc(sizeof(token_scope_tree));
  (*token_tree)->elements = malloc(0);
  (*token_tree)->elements_size = 0;
  for(i = tokens_start; i < tokens_end; i++) {
    token_scope_tree_element *new_element = (token_scope_tree_element *) malloc(sizeof(token_scope_tree_element));
    new_element->atoken = &tokens[i];
    if( (tokens[i].type == WHILE || tokens[i].type == FUNC || tokens[i].type == IF) && ignore_first == false) {
      int scope_open_count = 0, scope_index = i;
      bool first_scope_open_set = false;
      while ( (scope_open_count != 0 || first_scope_open_set == false) && scope_index < tokens_end) {
        if (tokens[scope_index].type == SCOPE_OPEN) {
          scope_open_count++;
          first_scope_open_set = true;
        } else if (tokens[scope_index].type == SCOPE_CLOSE) {
          scope_open_count--;
        }
        scope_index++;
      }
      new_element->type = TST_TREE;
      i = prepare_token_scope_tree(&new_element->tree, tokens, i+1, scope_index-1, true)-1;
    }
    else if(tokens[i].type == ARG_OPEN) {
      int j = i;
      for(; tokens[j].type != ARG_CLOSE;j++);
      
      new_element->type = TST_ARGS;
      i = prepare_token_scope_tree(&new_element->tree, tokens, i+1, j+1, false);
    }
    else {
      new_element->type = TST_TOKEN;
    }
    (*token_tree)->elements = (token_scope_tree_element*) realloc((*token_tree)->elements, sizeof(token_scope_tree_element) * ( (*token_tree)->elements_size + 1));
    (*token_tree)->elements[(*token_tree)->elements_size] = *new_element;
    (*token_tree)->elements_size++;
    ignore_first = false;
  }
  return i;
}

int
tokenize(char* code, token** target_tokens)
{
  regex_t* token_type_regex;
  regmatch_t matches[10];

  init_token_type_regex(&token_type_regex);

  *target_tokens = (token*)malloc(0);
  int target_tokens_size = 0;

  while (code != NULL && strcmp(code, "") != 0) {
    bool token_found = false;
    for (int i = 0; i < TOKEN_TYPE_LENGTH; i++) {
      int succ = regexec(&(token_type_regex[i]),
                         code,
                         /*sizeof(matches)/sizeof(matches[0])*/ 1,
                         (regmatch_t*)&matches,
                         0);
      if (succ == 0 && matches[0].rm_so == 0) {
        int token_text_size = matches[0].rm_eo - matches[0].rm_so;
        *target_tokens = (token*)realloc(
          *target_tokens, sizeof(token) * (target_tokens_size + 1));
        strncpy(
          (*target_tokens)[target_tokens_size].text, code, token_text_size);
        (*target_tokens)[target_tokens_size].text[token_text_size] = '\0';
        (*target_tokens)[target_tokens_size].type = (token_type)i;
        target_tokens_size++;
        code += matches[0].rm_eo - matches[0].rm_so;
        code = trim(code);
        token_found = true;
        break;
      }
    }

    if (token_found == false) {
      printf("Syntax error at: \"%s\"\n", code);
      break;
    }
  }

  for (int i = 0; i < TOKEN_TYPE_LENGTH; i++) {
    regfree(&token_type_regex[i]);
  }
  return target_tokens_size;
}

void
init_token_type_regex(regex_t** token_type_regex)
{
  *token_type_regex = (regex_t*)malloc(sizeof(regex_t) * TOKEN_TYPE_LENGTH);

  for (int i = 0; i < TOKEN_TYPE_LENGTH; i++) {
    int j = regcomp(&((*token_type_regex)[i]),
                    token_type_regex_str[i],
                    REG_EXTENDED | REG_NEWLINE);
    if (j != 0) {
      printf("Err at %d (%d)\n", i, j);
    }
  }
}
