/* udemo_dec.c */

#include "udemo_types.h"

struct top_s top__instance;

/* decoding table */

  typedef unsigned int u32;
  #define NM_LEN 40
  struct dectbl_entry_s {
    char name[NM_LEN+2]; char type[NM_LEN+2]; u32 size, offset;
  };
  #define def(x) {#x, "unknown-type", \
                  sizeof(x), ((u32)&x-((u32)&top__instance))}, 
  /* sample data from udemo-result */
  struct dectbl_entry_s dectbl[] = {
#include "udemo_def_gen.h"
//    def(                 top__instance.fa_sub_s.f1_i32 ) 
//    def(                 top__instance.fa_sub_s.f2_i16 ) 
//    def(                 top__instance.fb_u32 ) 
//    def(                 top__instance.fc_enum)  
    {{(char)0}, {(char)0}, 0,0}
  };
  #undef def

/* decoding */
#include <cxxabi.h>
#include <typeinfo>
#include <string.h>
#include <stdio.h>
int main(int argc, char *argv[])
{
  int i=0;
  int status=0;
  memset(&top__instance, 0, sizeof(top__instance));

  #define def(x) \
      strncpy(dectbl[i].type, \
          abi::__cxa_demangle( typeid(x).name(), 0, 0, &status), \
          NM_LEN); i++; \
      dectbl[i].type[NM_LEN]='\0';\
      if (dectbl[i].name[0] == (char)0) break;
  do{ 

#include "udemo_def_gen.h"

//    def(                 top__instance.fa_sub_s.f1_i32 ) 
//    def(                 top__instance.fa_sub_s.f2_i16 ) 
//    def(                 top__instance.fb_u32 ) 
//    def(                 top__instance.fc_enum)  
  }while(0);

  #undef def

  printf("\n");
  for (i=0; i<100; i++) {
    if ( dectbl[i].name[0] == (char)0 ) {
        printf("    %4i: %s\n", i, "null-name-done");
        break;
    }
    printf("    %4i:  %30s  %20s  %2u  %3u\n", 
           i, dectbl[i].name, dectbl[i].type, 
              dectbl[i].size, dectbl[i].offset );
  }
  printf("\n");
  return 0;
}

