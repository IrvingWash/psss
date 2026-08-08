#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
// PSn00bSDK
#include <psxgte.h>    // The "Geometry Transformation Engine" co-processor stuff
#include <psxetc.h>    // Input and stuff
#include <psxgpu.h>    // GPU stuff
#include <inline_c.h>  // IDK why it's here, in original PsyQ it seems to be in libgte.h
#include <sys/types.h> // u_char, etc

#define SCREEN_RES_X 320
#define SCREEN_RES_Y 240
#define SCREEN_CENTER_X (SCREEN_RES_X >> 1)
#define SCREEN_CENTER_Y (SCREEN_RES_Y >> 1)
#define SCREEN_Z 400

#define BUFFER_COUNT 2
#define OT_LENGTH 256

#define VERTEX_COUNT 8
const SVECTOR const VERTICES[] = {
    { -128, -128, -128 },
    { 128, -128, -128 },
    { 128, -128, 128 },
    { -128, -128, 128 },
    { -128, 128, -128 },
    { 128, 128, -128 },
    { 128, 128, 128 },
    { -128, 128, 128 },
};

#define FACE_COUNT 12
#define VERTICES_PER_FACE 3
short const FACES[] = {
    0, 3, 2,
    0, 2, 1,
    4, 0, 1,
    4, 1, 5,
    7, 4, 5,
    7, 5, 6,
    5, 1, 2,
    5, 2, 6,
    2, 3, 7,
    2, 7, 6,
    0, 4, 7,
    0, 7, 3,
};

typedef struct {
    DRAWENV draw[BUFFER_COUNT];
    DISPENV disp[BUFFER_COUNT];

    short current_buffer; // 0 or 1
} Double_Buffer;

Double_Buffer screen;

// Contains pointers to geometry which will be drawn, passed as is to the SDK
u_long ordering_table[BUFFER_COUNT][OT_LENGTH] = {0};
// Contains geometry data, of different types
char   primitive_buffer[BUFFER_COUNT][2048] = {0};
// Pointes to the end of the last primitive in the primitive buffer
char   *next_primitive;

POLY_G3 *my_poly;

SVECTOR rotation    = { 0, 0, 0 };
VECTOR  translation = { 0, 0, 900 };
VECTOR  scale       = { ONE, ONE, ONE };

MATRIX world_transform = {0};

void init_screen(void) {
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
}

void display_frame(void) {
    DrawSync(0);
    VSync(0);

    PutDispEnv(&screen.disp[screen.current_buffer]);
    PutDrawEnv(&screen.draw[screen.current_buffer]);

    DrawOTag(ordering_table[screen.current_buffer] + OT_LENGTH - 1); // Pointer to the end, because we need to go from back to front

    // Swap buffers
    screen.current_buffer = !screen.current_buffer;
}

void setup(void) {
    init_screen();

    next_primitive = primitive_buffer[screen.current_buffer];
}

void update(void) {
    rotation.vx += 6;
    rotation.vy += 8;
    rotation.vz += 12;

    // Clear OT, so we can reuse it
    ClearOTagR(ordering_table[screen.current_buffer], OT_LENGTH);
    next_primitive = primitive_buffer[screen.current_buffer];

    // Prepare transforms
    RotMatrix(&rotation, &world_transform);
    TransMatrix(&world_transform, &translation);
    ScaleMatrix(&world_transform, &scale);

    gte_SetRotMatrix(&world_transform);
    gte_SetTransMatrix(&world_transform);

    // Iterating faces
    for (int i = 0; i < FACE_COUNT * VERTICES_PER_FACE; i += VERTICES_PER_FACE) {
        my_poly = (POLY_G3*) next_primitive;
        setPolyG3(my_poly);
        setRGB0(my_poly, 255, 0, 255);
        setRGB1(my_poly, 255, 255, 0);
        setRGB2(my_poly, 0, 255, 255);

        // Load the vertices of this face
        gte_ldv3(
            &VERTICES[FACES[i + 0]],
            &VERTICES[FACES[i + 1]],
            &VERTICES[FACES[i + 2]]
        );
        gte_rtpt(); // Rotation, translation, perspective on the loaded vertices


        int normal_clip;
        gte_nclip(); // Normal clip for backface culling
        gte_stopz(&normal_clip); // Get result
        if (normal_clip < 0) {
            continue; // Skipping this face because it is backfaced
        }

        int average_z;
        gte_avsz3(); // Calculate average z for depth sorting
        gte_stotz(&average_z); // Get result
        if (average_z >= OT_LENGTH || average_z < 0) {
            continue; // Didn't pass the depth test.
        }

        // Set transformed vertices back
        gte_stsxy0(&my_poly->x0);
        gte_stsxy1(&my_poly->x1);
        gte_stsxy2(&my_poly->x2);

        addPrim(ordering_table[screen.current_buffer][average_z], my_poly);
        next_primitive += sizeof(POLY_G3);
    }
}

void render(void) {
    display_frame();
}

int main(void) {
    setup();

    while (true) {
        update();
        render();
    }

    return 0;
}
