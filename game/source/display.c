#include "display.h"

#include <stdbool.h>
#include <assert.h>
#include <psxgpu.h>
#include <sys/types.h>
#include <psxgte.h>
#include <inline_c.h>

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
POLY_G4 *poly_g4;
POLY_F3 *poly_f3;
MATRIX world_transform = {0};

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

void display_start_frame(void) {
    // Clear OT, so we can reuse it
    ClearOTagR(ordering_table[screen.current_buffer], OT_LENGTH);
    next_primitive = primitive_buffer[screen.current_buffer];
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
) {
    // Prepare transforms
    RotMatrix(&rotation, &world_transform);
    TransMatrix(&world_transform, &position);
    ScaleMatrix(&world_transform, &scale);

    gte_SetRotMatrix(&world_transform);
    gte_SetTransMatrix(&world_transform);

    // Iterating faces
    for (int i = 0; i < face_count * vertices_per_face; i += vertices_per_face) {
        poly_g4 = (POLY_G4*) next_primitive;
        setPolyG4(poly_g4);
        setRGB0(poly_g4, color0[0], color0[1], color0[2]);
        setRGB1(poly_g4, color1[0], color1[1], color1[2]);
        setRGB2(poly_g4, color2[0], color2[1], color2[2]);
        setRGB3(poly_g4, color3[0], color3[1], color3[2]);

        // Load the vertices of this face
        gte_ldv3(
            &vertices[faces[i + 0]],
            &vertices[faces[i + 1]],
            &vertices[faces[i + 2]]
        );
        gte_rtpt(); // Rotation, translation, perspective on the loaded vertices


        int normal_clip;
        gte_nclip(); // Normal clip for backface culling
        gte_stopz(&normal_clip); // Get result
        if (normal_clip < 0) {
            continue; // Skipping this face because it is backfaced
        }

        int average_z;
        gte_avsz4(); // Calculate average z for depth sorting
        gte_stotz(&average_z); // Get result
        if (average_z >= OT_LENGTH || average_z < 0) {
            continue; // Didn't pass the depth test.
        }

        // Set transformed vertices back
        gte_stsxy0(&poly_g4->x0);
        gte_stsxy1(&poly_g4->x1);
        gte_stsxy2(&poly_g4->x2);

        // Compute the last vertex
        gte_ldv0(&vertices[faces[i + 3]]);
        gte_rtps();
        gte_stsxy(&poly_g4->x3);

        addPrim(ordering_table[screen.current_buffer][average_z], poly_g4);
        next_primitive += sizeof(POLY_G4);
    }
}

void display_draw_mesh_of_f3(
    VECTOR position,
    VECTOR scale,
    SVECTOR rotation,
    int face_count,
    int vertices_per_face,
    u_short color[3],
    SVECTOR *vertices,
    short *faces
) {
    RotMatrix(&rotation, &world_transform);
    TransMatrix(&world_transform, &position);
    ScaleMatrix(&world_transform, &scale);

    gte_SetRotMatrix(&world_transform);
    gte_SetTransMatrix(&world_transform);

    for (int i = 0; i < face_count * vertices_per_face; i += vertices_per_face) {
        poly_f3 = (POLY_F3*) next_primitive;
        setPolyF3(poly_f3);
        setRGB0(poly_f3, 255, 255, 255);

        gte_ldv3(
            &vertices[faces[i + 0]],
            &vertices[faces[i + 1]],
            &vertices[faces[i + 2]]
        );
        gte_rtpt();

        int normal_clip;
        gte_nclip();
        gte_stopz(&normal_clip);
        if (normal_clip < 0) {
            continue;
        }

        int average_z;
        gte_avsz3();
        gte_stotz(&average_z); // Get result
        if (average_z >= OT_LENGTH || average_z < 0) {
            continue;
        }

        gte_stsxy0(&poly_f3->x0);
        gte_stsxy1(&poly_f3->x1);
        gte_stsxy2(&poly_f3->x2);

        addPrim(ordering_table[screen.current_buffer][average_z], poly_f3);
        next_primitive += sizeof(POLY_F3);
    }
}
