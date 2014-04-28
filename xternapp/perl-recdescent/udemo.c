
#include <string.h>

struct sub_s {
  int   f1_i32;
  short f2_i16;
};

struct top_s {
  struct sub_s f3_sub_s;
  unsigned int f4_u32;
};

struct top_s _instance;

int test_sub_function(void)
{
  return 1;
}

int main(int argc, char *argv[])
{
  memset(&_instance, 0, sizeof(_instance));
  return test_sub_function();
}

