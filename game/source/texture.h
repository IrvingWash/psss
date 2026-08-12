#ifndef TEXTURE_H
#define TEXTURE_H

#include <sys/types.h>
#include <psxgpu.h>

#define MAX_TEXTURES 800

#define	CLUT4         0x00
#define	CLUT8         0x01
#define	TRUE15        0x10

#define  CLUT_4BIT    0
#define  CLUT_8BIT    1
#define  CLUT_15BIT   2

#define  TRANSLUCENT  1

#define	 CLUT(x,y)      ((y << 6) | x)
#define	 TPAGE(c,a,x,y) ((c << 7) | (a << 5) | ((x >> 6) + ((y & 0x100) >> 4)))

#define TEXTURE_H_OFFSET(x) (x & 0xFFC0)
#define TEXTURE_V_OFFSET(y) (y & 0x100)

#define CLUT_TYPE(t) (t->flags & 7)

typedef struct Texture {
  short    type;
  short    tpage;
  short    clut;
  short    clut_x;
  short    clut_y;
  short    clut_w;
  short    clut_h;
  short    texture_x;
  short    texture_y;
  short    texture_w;
  short    texture_h;
  short    u0;
  short    v0;
  short    u1;
  short    v1;
  short    u2;
  short    v2;
  short    u3;
  short    v3;
} Texture;

typedef struct TIM_CLUT8 {
  long     id;
  long     flags;
  long     clut_bytes;
  short    clut_x;
  short    clut_y;
  short    clut_w;
  short    clut_h;
  short    clut[256];
  long     texture_bytes;
  short    texture_x;
  short    texture_y;
  short    texture_w;
  short    texture_h;
} TIM_CLUT8;

typedef struct TIM_CLUT4 {
  long     id;
  long     flags;
  long     clut_bytes;
  short    clut_x;
  short    clut_y;
  short    clut_w;
  short    clut_h;
  short    clut[16];
  long     texture_bytes;
  short    texture_x;
  short    texture_y;
  short    texture_w;
  short    texture_h;
} TIM_CLUT4;

typedef struct TIM {
  long id;
  long flags;
} TIM;

void texture_create_from_cmp(char *cmp_data);

Texture* get_texture(u_int index);

#endif
