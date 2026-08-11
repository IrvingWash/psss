#include "display.h"

#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <psxgpu.h>
#include <sys/types.h>
#include <psxgte.h>
#include <inline_c.h>
#include "utils.h"
#include "display_primitives.h"

#define SCREEN_RES_X 320
#define SCREEN_RES_Y 240
#define SCREEN_CENTER_X (SCREEN_RES_X >> 1)
#define SCREEN_CENTER_Y (SCREEN_RES_Y >> 1)
#define SCREEN_Z 400

#define BUFFER_COUNT 2
#define OT_LENGTH 2048
#define PRIMITIVE_BUFFER_SIZE 2048

typedef struct {
    DRAWENV draw[BUFFER_COUNT];
    DISPENV disp[BUFFER_COUNT];

    short current_buffer; // 0 or 1
} Double_Buffer;

Double_Buffer screen;
// Contains pointers to geometry which will be drawn, passed as is to the SDK
u_long ordering_table[BUFFER_COUNT][OT_LENGTH] = {0};
// Contains geometry data, of different types
char   primitive_buffer[BUFFER_COUNT][PRIMITIVE_BUFFER_SIZE] = {0};
// Pointes to the end of the last primitive in the primitive buffer
char   *next_primitive;

// These are reused per mesh
POLY_FT4 *poly_ft4;
MATRIX world_transform = {0};
MATRIX view_transform  = {0};

void display_init(void) {
    assert(BUFFER_COUNT == 2); // If this fails, many things should be changed

    ResetGraph(0); // Reset the GPU

    // Display area of first buffer
    SetDefDispEnv(&screen.disp[0], 0, 0, SCREEN_RES_X, SCREEN_RES_Y);
    SetDefDrawEnv(&screen.draw[0], 0, SCREEN_RES_Y, SCREEN_RES_X, SCREEN_RES_Y);

    // Display area of second buffer
    SetDefDispEnv(&screen.disp[1], 0, SCREEN_RES_Y, SCREEN_RES_X, SCREEN_RES_Y);
    SetDefDrawEnv(&screen.draw[1], 0, 0, SCREEN_RES_X, SCREEN_RES_Y);

    // Background buffers
    screen.draw[0].isbg = true;
    screen.draw[1].isbg = true;

    // Clear color
    setRGB0(&screen.draw[0], 63, 0, 127);
    setRGB0(&screen.draw[1], 63, 0, 127);

    // Initial buffer
    screen.current_buffer = 0;
    PutDispEnv(&screen.disp[screen.current_buffer]);
    PutDrawEnv(&screen.draw[screen.current_buffer]);

    // Initialize GTE and set geometry offsets
    InitGeom();
    gte_SetGeomOffset(SCREEN_CENTER_X, SCREEN_CENTER_Y);
    gte_SetGeomScreen(SCREEN_Z);

    // Enable display
    SetDispMask(1);

    next_primitive = primitive_buffer[screen.current_buffer];
}

void display_start_frame(const MATRIX *new_view_transform) {
    // Clear OT, so we can reuse it
    ClearOTagR(ordering_table[screen.current_buffer], OT_LENGTH);
    next_primitive = primitive_buffer[screen.current_buffer];
    view_transform = *new_view_transform;
}

void display_finish_frame(void) {
    DrawSync(0);
    VSync(0);

    PutDispEnv(&screen.disp[screen.current_buffer]);
    PutDrawEnv(&screen.draw[screen.current_buffer]);

    DrawOTag(ordering_table[screen.current_buffer] + OT_LENGTH - 1); // Pointer to the end, because we need to go from back to front

    // Swap buffers
    screen.current_buffer = !screen.current_buffer;
}

// void display_draw_mesh_of_ft4(
//     VECTOR position,
//     VECTOR scale,
//     SVECTOR rotation,
//     const Display_Mesh *mesh
// ) {
//     // Prepare transforms
//     RotMatrix(&rotation, &world_transform);
//     TransMatrix(&world_transform, &position);
//     ScaleMatrix(&world_transform, &scale);
//
//     MATRIX transform = {0};
//     CompMatrixLV(&view_transform, &world_transform, &transform);
//
//     gte_SetRotMatrix(&transform);
//     gte_SetTransMatrix(&transform);
//
//     // Iterating faces
//     // int q = 0;
//     for (int i = 0, q = 0; i < mesh->index_count; i += 4, q++) { // 4, because f4
//         poly_ft4 = (POLY_FT4*) next_primitive;
//         setPolyFT4(poly_ft4);
//
//         setRGB0(poly_ft4, 128, 128, 128);
//
//         poly_ft4->u0 = 0;  poly_ft4->v0 = 0;
//         poly_ft4->u1 = 63; poly_ft4->v1 = 0;
//         poly_ft4->u2 = 0;  poly_ft4->v2 = 63;
//         poly_ft4->u3 = 63; poly_ft4->v3 = 63;
//
//         poly_ft4->tpage = getTPage(mesh->texture.mode & 0x3, 0, mesh->texture.prect->x, mesh->texture.prect->y);
//         poly_ft4->clut = getClut(mesh->texture.crect->x, mesh->texture.crect->y);
//
//         // Load the vertices of this face
//         gte_ldv3(
//             &mesh->vertices[mesh->indices[i + 0]],
//             &mesh->vertices[mesh->indices[i + 1]],
//             &mesh->vertices[mesh->indices[i + 2]]
//         );
//         gte_rtpt(); // Rotation, translation, perspective on the loaded vertices
//
//
//         int normal_clip;
//         gte_nclip(); // Normal clip for backface culling
//         gte_stopz(&normal_clip); // Get result
//         if (normal_clip < 0) {
//             continue; // Skipping this face because it is backfaced
//         }
//
//         int average_z;
//         gte_avsz4(); // Calculate average z for depth sorting
//         gte_stotz(&average_z); // Get result
//         if (average_z >= OT_LENGTH || average_z < 0) {
//             continue; // Didn't pass the depth test.
//         }
//
//         // Set transformed vertices back
//         gte_stsxy0(&poly_ft4->x0);
//         gte_stsxy1(&poly_ft4->x1);
//         gte_stsxy2(&poly_ft4->x2);
//
//         // Compute the last vertex
//         gte_ldv0(&mesh->vertices[mesh->indices[i + 3]]);
//         gte_rtps();
//         gte_stsxy(&poly_ft4->x3);
//
//         addPrim(ordering_table[screen.current_buffer][average_z], poly_ft4);
//         next_primitive += sizeof(POLY_FT4);
//     }
// }

// byte - name[16]
// short - vertex_count
// byte - unused[6]
// short - normal count
// byte - unused[6]
// short - primitive count
// byte - unused[22]
// short - flags
// byte - unused[26]
// long - origin.x
// long - origin.y
// long - origin.z
// byte - unused[48]
// short - vertices[0].x
// ...
Display_Mesh display_create_mesh_from_prm(const char *prm_data) {
    Display_Mesh mesh = {0};

    u_long byte_counter = 0;

    for (int i = 0; i < 16; i++) {
        mesh.name[i] = get_char(prm_data, &byte_counter);
    }

    mesh.vertex_count = get_short_be(prm_data, &byte_counter);
    byte_counter += 6;

    mesh.normal_count = get_short_be(prm_data, &byte_counter);
    byte_counter += 6;

    mesh.primitive_count = get_short_be(prm_data, &byte_counter);
    byte_counter += 22;

    mesh.flags = get_short_be(prm_data, &byte_counter);
    byte_counter += 26;

    mesh.origin.vx = get_long_be(prm_data, &byte_counter);
    mesh.origin.vy = get_long_be(prm_data, &byte_counter);
    mesh.origin.vz = get_long_be(prm_data, &byte_counter);
    byte_counter += 48;

    mesh.vertices = malloc(mesh.vertex_count * sizeof(SVECTOR));
    for (int i = 0; i < mesh.vertex_count; i++) {
        mesh.vertices[i].vx = get_short_be(prm_data, &byte_counter);
        mesh.vertices[i].vy = get_short_be(prm_data, &byte_counter);
        mesh.vertices[i].vz = get_short_be(prm_data, &byte_counter);
        byte_counter += 2;
    }

    mesh.normals = malloc(mesh.normal_count * sizeof(SVECTOR));
    for (int i = 0; i < mesh.normal_count; i++) {
        mesh.normals[i].vx = get_short_be(prm_data, &byte_counter);
        mesh.normals[i].vy = get_short_be(prm_data, &byte_counter);
        mesh.normals[i].vz = get_short_be(prm_data, &byte_counter);
        byte_counter += 2;
    }

    return mesh;
}

void display_destroy_mesh(Display_Mesh *mesh) {
    free(mesh->vertices);
    free(mesh->normals);
}
