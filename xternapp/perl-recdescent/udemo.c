
#include <string.h>

#include "udemo_types.h"
/*
enum sel_e {
    selv1 = 0,
    selv2 = 1, 
    selv3 = 2,
};

struct sub_s {
  int   f1_i32;
  short f2_i16;
};

struct top_s {
  struct sub_s fa_sub_s;
  unsigned int fb_u32;
  enum sel_e   fc_enum;
};
*/

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

