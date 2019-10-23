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
        printf("%s / %d (%d)\n",
               (*target_tokens)[target_tokens_size - 1].text,
               (*target_tokens)[target_tokens_size - 1].type,
               token_text_size);
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
