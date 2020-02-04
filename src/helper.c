/*
  helper.c  helper functions for assembler and emulator
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

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

void
welcome(const char progname[64])
{

  printf("\n%s  Copyright (C) 2019 Patrick Günthard\n\
This program comes with ABSOLUTELY NO WARRANTY; This is free software, and you\n\
are welcome to redistribute it under certain conditions; \n\n\
\
You should have received a copy of the GNU General Public License along with\n\
this program.  If not, see <https://www.gnu.org/licenses/>.\n\n\
",
         progname);
}

size_t
readfile(char** string, char path[64], bool binary)
{
  FILE* f;

  if (binary) {
    f = fopen(path, "rb");
  } else {
    f = fopen(path, "r");
  }

  if (f == NULL) {
    printf("Error loading file\n");
    return 0;
  }

  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  *string = (char*)malloc(fsize + 1);
  fread(*string, 1, fsize, f);
  fclose(f);

  if (binary == false) {
    (*string)[fsize] = 0;
  }
  return fsize;
}

bool
contains_single(char* str, char s)
{
  for (int i = 0; i < strlen(str); i++) {
    if (str[i] == s) {
      return true;
    }
  }
  return false;
}

bool
contains(char* str, char* search)
{
  for (int i = 0; i < strlen(search); i++) {
    if (contains_single(str, search[i])) {
      return true;
    }
  }
  return false;
}

char*
trim(char* str)
{
  size_t len = 0;
  char* frontp = str;
  char* endp = NULL;

  if (str == NULL) {
    return NULL;
  }
  if (str[0] == '\0') {
    return str;
  }

  len = strlen(str);
  endp = str + len;

  /* Move the front and back pointers to address the first non-whitespace
   * characters from each end.
   */
  while (isspace((unsigned char)*frontp)) {
    ++frontp;
  }
  if (endp != frontp) {
    while (isspace((unsigned char)*(--endp)) && endp != frontp) {
    }
  }

  if (frontp != str && endp == frontp)
    *str = '\0';
  else if (str + len - 1 != endp)
    *(endp + 1) = '\0';

  /* Shift the string so that it starts at str so that if it's dynamically
   * allocated, we can still free it on the returned pointer.  Note the reuse
   * of endp to mean the front of the string buffer now.
   */
  endp = str;
  if (frontp != str) {
    while (*frontp) {
      *endp++ = *frontp++;
    }
    *endp = '\0';
  }

  return str;
}

int
strpos(char* str, char* target)
{
  char* res = strstr(str, target);
  if (res == NULL)
    return -1;
  else
    return res - str;
}

unsigned long
get_timestamp_ms()
{
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

char*
str_sep(char** src, char delim)
{
  if (*src == NULL || **src == '\0') {
    return NULL;
  }
  char* buffer = (char*)malloc(strlen(*src) * sizeof(char));
  int i = 0;
  while ((*src)[0] != delim && (*src)[0] != '\0') {
    buffer[i++] = (*src)[0];
    (*src)++;
  }
  buffer[i] = '\0';
  if (*src[0] != '\0') {
    (*src)++;
  }
  buffer = (char*) realloc(buffer, (strlen(buffer) + 1) * sizeof(char));
  return buffer;
}
