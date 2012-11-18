
#include <clcpp/clcpp.h>

clcpp_reflect(outer)
namespace outer {

    typedef struct inner1_s { 
        int v1;
        int v2;
        enum v1_enum {
            V1_START, 
            V1_START2,
            V1_START3,
            V1_START4,
        };
        static const int v2_ini1 = 11;
        static const int v2_ini2 = 12;
    } inner1_t;
    typedef struct inner2_s { 
        int v3;
        inner1_t v4;
    } inner2_t;
}

