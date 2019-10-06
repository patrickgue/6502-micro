
#include <assert.h>
#include <string.h>

#include "../helper.h"

void
test_str_sep()
{
  char* str = "Hello to my World";
  char* test = str_sep(&str, ' ');
  assert(strcmp(test, "Hello") == 0);
  assert(strcmp(str, "to my World") == 0);
  str_sep(&str, ' ');               // "to" / "my World"
  str_sep(&str, ' ');               // "my" / "World"
  char* test2 = str_sep(&str, ' '); // "World" / ""
  assert(strcmp(test2, "World") == 0);
  assert(str_sep(&str, ' ') == NULL);
}

int
main(int argc, char** argv)
{
  test_str_sep();
  return 0;
}