#ifndef DISPLAY_H
#define DISPLAY_H

#include <psxgte.h>
#include <sys/types.h>

typedef struct Display_Mesh {
    short vertex_count;
    SVECTOR *vertices;

    short index_count;
    short *indices;

    short color_count;
    CVECTOR *colors;
} Display_Mesh;

void display_init(void);
void display_start_frame(const MATRIX *view_matrix);
void display_finish_frame(void);

Display_Mesh display_create_mesh(const char *mesh_data, u_long length);
void display_destroy_mesh(Display_Mesh *mesh);

void display_draw_mesh_of_f4(
    VECTOR position,
    VECTOR scale,
    SVECTOR rotation,
    const Display_Mesh *mesh
);

void display_draw_mesh_of_f3(
    VECTOR position,
    VECTOR scale,
    SVECTOR rotation,
    const Display_Mesh *mesh
);

#endif
