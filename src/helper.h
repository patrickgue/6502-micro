/*
  helper.h  header file for helper.c
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

#ifndef HELPER_H
#define HELPER_H
#include <Z/hardware/CPU/architecture/6502.h>
#include <stdbool.h>

void welcome(const char[]);

zusize readfile(char **string, char path[64], bool binary);

bool contains_single(char *str, char s);
bool contains(char *str, char *search);
char *trim(char*);
int strpos(char *str, char *target);
unsigned long get_timestamp_ms();
char *str_sep(char **, char);
#endif /* HELPER_H */
