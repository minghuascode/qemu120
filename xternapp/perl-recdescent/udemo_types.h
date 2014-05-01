/* udemo_types.h */

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


