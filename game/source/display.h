#ifndef DISPLAY_H
#define DISPLAY_H

#include <psxgte.h>
#include <sys/types.h>

void display_init(void);
void display_start_frame(void);
void display_finish_frame(void);

void display_draw_mesh_of_g4(
    VECTOR position,
    VECTOR scale,
    SVECTOR rotation,
    int face_count,
    int vertices_per_face,
    u_short color0[3],
    u_short color1[3],
    u_short color2[3],
    u_short color3[3],
    SVECTOR *vertices,
    short *faces
);

void display_draw_mesh_of_f3(
    VECTOR position,
    VECTOR scale,
    SVECTOR rotation,
    int face_count,
    int vertices_per_face,
    u_short color[3],
    SVECTOR *vertices,
    short *faces
);

#endif
