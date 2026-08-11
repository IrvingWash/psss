#ifndef DISPLAY_PRIMITIVES_H
#define DISPLAY_PRIMITIVES_H

/* Primitive Flags */
#define SINGLE_SIDED 0x0001
#define SHIP_ENGINE  0x0002
#define TRANSLUCENT 0x0004

/* Primitive Types */
#define TYPE_F3            1
#define TYPE_FT3           2
#define TYPE_F4            3
#define TYPE_FT4           4
#define TYPE_G3            5
#define TYPE_GT3           6
#define TYPE_G4            7
#define TYPE_GT4           8

#define TYPE_LF2           9
#define TYPE_TSPR          10
#define TYPE_BSPR          11

#define TYPE_LSF3          12
#define TYPE_LSFT3         13
#define TYPE_LSF4          14
#define TYPE_LSFT4         15
#define TYPE_LSG3          16
#define TYPE_LSGT3         17
#define TYPE_LSG4          18
#define TYPE_LSGT4         19

#define TYPE_SPLINE        20

#define TYPE_INFINITE_LIGHT 21
#define TYPE_POINT_LIGHT    22
#define TYPE_SPOT_LIGHT     23

typedef struct F3 {
   short   type;      /* Type    of primitive */
   short   flag;
   short   coords[3]; /* Indices of the       coords */
   short   pad1;
   CVECTOR color;
} F3;

typedef struct FT3 {
   short   type;      /* Type    of primitive */
   short   flag;
   short   coords[3]; /* Indices of the       coords */
   short   texture;
   short   clut;
   short   tpage;
   char    u0;
   char    v0;
   char    u1;
   char    v1;
   char    u2;
   char    v2;
   short   pad1;
   CVECTOR color;
} FT3;

typedef struct F4 {
   short   type;      /* Type    of primitive */
   short   flag;
   short   coords[4]; /* Indices of the       coords */
   CVECTOR color;
} F4;

typedef struct FT4 {
   short   type;      /* Type    of primitive */
   short   flag;
   short   coords[4]; /* Indices of the       coords */
   short   texture;
   short   clut;
   short   tpage;
   char    u0;
   char    v0;
   char    u1;
   char    v1;
   char    u2;
   char    v2;
   char    u3;
   char    v3;
   short   pad1;
   CVECTOR color;
} FT4;

typedef struct G3 {
   short   type;      /* Type    of primitive */
   short   flag;
   short   coords[3]; /* Indices of the       coords */
   short   pad1;
   CVECTOR color[3];
} G3;

typedef struct GT3 {
   short   type;      /* Type    of primitive */
   short   flag;
   short   coords[3]; /* Indices of the       coords */
   short   texture;
   short   clut;
   short   tpage;
   char    u0;
   char    v0;
   char    u1;
   char    v1;
   char    u2;
   char    v2;
   short   pad1;
   CVECTOR color[3];
} GT3;

typedef struct G4 {
   short   type;      /* Type    of primitive */
   short   flag;
   short   coords[4]; /* Indices of the       coords */
   CVECTOR color[4];
} G4;

typedef struct GT4 {
   short   type;      /* Type    of primitive */
   short   flag;
   short   coords[4]; /* Indices of the       coords */
   short   texture;
   short   clut;
   short   tpage;
   char    u0;
   char    v0;
   char    u1;
   char    v1;
   char    u2;
   char    v2;
   char    u3;
   char    v3;
   short   pad1;
   CVECTOR color[4];
} GT4;

typedef struct LSF3 {
   short   type;      /* Type    of primitive */
   short   flag;
   short   coords[3]; /* Indices of the       coords  */
   short   normal;    /* Indices of the       normals */
   CVECTOR color;
} LSF3;

typedef struct LSFT3 {
   short   type;      /* Type    of primitive */
   short   flag;
   short   coords[3]; /* Indices of the       coords  */
   short   normal;    /* Indices of the       normals */
   short   texture;
   short   clut;
   short   tpage;
   char    u0;
   char    v0;
   char    u1;
   char    v1;
   char    u2;
   char    v2;
   CVECTOR color;
} LSFT3;

typedef struct LSF4 {
   short   type;      /* Type    of primitive */
   short   flag;
   short   coords[4]; /* Indices of the       coords  */
   short   normal;    /* Indices of the       normals */
   short   pad1;
   CVECTOR color;
} LSF4;

typedef struct LSFT4 {
   short   type;      /* Type    of primitive */
   short   flag;
   short   coords[4]; /* Indices of the       coords  */
   short   normal;    /* Indices of the       normals */
   short   texture;
   short   clut;
   short   tpage;
   char    u0;
   char    v0;
   char    u1;
   char    v1;
   char    u2;
   char    v2;
   char    u3;
   char    v3;
   CVECTOR color;
} LSFT4;

typedef struct LSG3 {
   short   type;       /* Type    of primitive */
   short   flag;
   short   coords[3];  /* Indices of the       coords  */
   short   normals[3]; /* Indices of the       normals */
   CVECTOR color[3];
} LSG3;

typedef struct LSGT3 {
   short   type;       /* Type    of primitive */
   short   flag;
   short   coords[3];  /* Indices of the       coords  */
   short   normals[3]; /* Indices of the       normals */
   short   texture;
   short   clut;
   short   tpage;
   char    u0;
   char    v0;
   char    u1;
   char    v1;
   char    u2;
   char    v2;
   CVECTOR color[3];
} LSGT3;

typedef struct LSG4 {
   short   type;       /* Type    of primitive */
   short   flag;
   short   coords[4];  /* Indices of the       coords  */
   short   normals[4]; /* Indices of the       normals */
   CVECTOR color[4];
} LSG4;

typedef struct LSGT4 {
   short   type;       /* Type    of primitive */
   short   flag;
   short   coords[4];  /* Indices of the       coords  */
   short   normals[4]; /* Indices of the       normals */
   short   texture;
   short   clut;
   short   tpage;
   char    u0;
   char    v0;
   char    u1;
   char    v1;
   char    u2;
   char    v2;
   char    u3;
   char    v3;
   short   pad1;
   CVECTOR color[4];
} LSGT4;

typedef struct Line_F2 {
   u_long* tag;
   CVECTOR color0;
   DVECTOR xy0;
   DVECTOR xy1;
} Line_F2;

typedef struct LF2 {
   short   type;    /* Type   of  primitive */
   short   flag;
   short   lines;   /* Number of  lines     */
   Line_F2* line[2]; /* 2      GPU Primitive arrays for double buffer */
} LF2;

typedef struct SPR {
   short   type;
   short   flag;
   short   coord;
   short   width;
   short   height;
   short   texture;
   CVECTOR color;
} SPR;

typedef struct Spline {
   short   type; /* Type of primitive */
   short   flag;
   SVECTOR control1;
   SVECTOR position;
   SVECTOR control2;
   CVECTOR color;
} Spline;

typedef struct Point_Light {
   short   type;
   short   flag;
   VECTOR  position;
   CVECTOR color;
   short   start_falloff;
   short   end_falloff;
} Point_Light;

typedef struct Spot_Light {
   short   type;
   short   flag;
   VECTOR  position;
   SVECTOR direction;
   CVECTOR color;
   short   start_falloff;
   short   end_alloff;
   short   cone_angle;
   short   spread_angle;
} Spot_Light;

typedef struct Infinite_Light {
   short   type;
   short   flag;
   SVECTOR direction;
   CVECTOR color;
} Infinite_Light;

typedef union Prm {
  F3            *f3;
  FT3           *ft3;
  F4            *f4;
  FT4           *ft4;
  G3            *g3;
  GT3           *gt3;
  G4            *g4;
  GT4           *gt4;
  SPR           *spr;
  Spline        *spline;
  Point_Light    *point_light;
  Spot_Light     *spot_light;
  Infinite_Light *infinite_light;
  LSF3          *lsf3;
  LSFT3         *lsft3;
  LSF4          *lsf4;
  LSFT4         *lsft4;
  LSG3          *lsg3;
  LSGT3         *lsgt3;
  LSG4          *lsg4;
  LSGT4         *lsgt4;
} Prm;

#endif
