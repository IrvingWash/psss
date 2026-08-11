#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <psxgte.h>
#include <sys/types.h>
#include <psxgpu.h>

typedef struct Display_Mesh {
    char name[16];

    short vertex_count;
    short normal_count;
    short primitive_count;

    short flags;

    VECTOR origin;

    SVECTOR *vertices;
    SVECTOR *normals;
} Display_Mesh;

void display_init(void);
void display_start_frame(const MATRIX *view_matrix);
void display_finish_frame(void);

Display_Mesh display_create_mesh_from_prm(const char *prm_data);
void display_destroy_mesh(Display_Mesh *mesh);

void display_draw_mesh_of_ft4(
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
