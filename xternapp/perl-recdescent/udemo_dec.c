/* udemo_dec.c */

#include "udemo_types.h"

struct top_s top__instance;

/* decoding table */
  #define def(x) #x,
  /* sample data from udemo-result */
  char *dectbl[] = {
    def(                 top__instance.fa_sub_s.f1_i32 ) 
    def(                 top__instance.fa_sub_s.f2_i16 ) 
    def(                 top__instance.fb_u32 ) 
    def(                 top__instance.fc_enum)  
    (char*)0
  };

/* decoding */
#include <string.h>
#include <stdio.h>
int main(int argc, char *argv[])
{
  int i=0;
  memset(&top__instance, 0, sizeof(top__instance));
  printf("\n");
  for (i=0; i<100; i++) {
    if ( dectbl[i] == NULL ) {
        printf("    %4i: %s\n", i, "null-char-done");
        break;
    }
    printf("    %4i: %s\n", i, dectbl[i]);
  }
  printf("\n");
  return 0;
}

