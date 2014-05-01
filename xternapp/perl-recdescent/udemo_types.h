/* udemo_types.h */

enum sel_e {
    selv1 = 0,
    selv2 = 1, 
    selv3 = 2,
};

struct sub_s {
  int   f1_i4;
  short f2_i2;
};

struct top_s {
  struct sub_s fa_sub_s;
  struct sub_s f2_sub_s[2];
  unsigned int f3_u4[4];
  unsigned int fb_u4;
  enum sel_e   fc_em;
};


