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
#define OT_LENGTH 2048

typedef struct {
    DRAWENV draw[BUFFER_COUNT];
    DISPENV disp[BUFFER_COUNT];

    short current_buffer; // 0 or 1
} Double_Buffer;

#define CUBE_FACE_COUNT 6
#define CUBE_VERTICES_PER_FACE 4
typedef struct Cube {
    SVECTOR rotation;
    VECTOR  position;
    VECTOR  scale;
    VECTOR  velocity;
    VECTOR  acceleration;
    SVECTOR vertices[8];
    short   faces[24];
} Cube;

typedef struct Floor {
    SVECTOR rotation;
    VECTOR  position;
    VECTOR  scale;
    SVECTOR vertices[4];
    short   faces[6];
} Floor;

Double_Buffer screen;

// Contains pointers to geometry which will be drawn, passed as is to the SDK
u_long ordering_table[BUFFER_COUNT][OT_LENGTH] = {0};
// Contains geometry data, of different types
char   primitive_buffer[BUFFER_COUNT][2048] = {0};
// Pointes to the end of the last primitive in the primitive buffer
char   *next_primitive;

POLY_G4 *my_poly;

MATRIX world_transform = {0};

Cube  cube  = {
    { 0, 0, 0 }, // Rotation
    { 0, -400, 1800 }, // Position
    { ONE, ONE, ONE }, // Scale
    { 0, 0, 0 },       // Velocity
    { 0, 1, 0 },       // Acceleration
    { // Vertices
        { -128, -128, -128 },
        { 128, -128, -128 },
        { 128, -128, 128 },
        { -128, -128, 128 },
        { -128, 128, -128 },
        { 128, 128, -128 },
        { 128, 128, 128 },
        { -128, 128, 128 },
    },
    { // Faces
        3, 2, 0, 1,
        0, 1, 4, 5,
        4, 5, 7, 6,
        1, 2, 5, 6,
        2, 3, 6, 7,
        3, 0, 7, 4,
    },
};

Floor floor = {
    { 0, 0, 0 }, // Rotation
    { 0, 450, 1800 }, // Position
    { ONE, ONE, ONE }, // Scale
    { // Vertices
        { -900, 0, -900, },
        { -900, 0, 900, },
        { 900, 0, -900, },
        { 900, 0, 900, },
    },
    { // Faces
        0, 1, 2,
        1, 3, 2,
    },
};

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
    // Update the position based on the acceleration and velocity
    cube.velocity.vx += cube.acceleration.vx;
    cube.velocity.vy += cube.acceleration.vy;
    cube.velocity.vz += cube.acceleration.vz;

    cube.position.vx += cube.velocity.vx;
    cube.position.vy += cube.velocity.vy;
    cube.position.vz += cube.velocity.vz;

    if (cube.position.vy > 400) {
        cube.velocity.vy *= -1;
    }

    cube.rotation.vx += 6;
    cube.rotation.vy += 8;
    cube.rotation.vz += 12;

    // Clear OT, so we can reuse it
    ClearOTagR(ordering_table[screen.current_buffer], OT_LENGTH);
    next_primitive = primitive_buffer[screen.current_buffer];

    // Prepare transforms
    RotMatrix(&cube.rotation, &world_transform);
    TransMatrix(&world_transform, &cube.position);
    ScaleMatrix(&world_transform, &cube.scale);

    gte_SetRotMatrix(&world_transform);
    gte_SetTransMatrix(&world_transform);

    // Iterating faces
    for (int i = 0; i < CUBE_FACE_COUNT * CUBE_VERTICES_PER_FACE; i += CUBE_VERTICES_PER_FACE) {
        my_poly = (POLY_G4*) next_primitive;
        setPolyG4(my_poly);
        setRGB0(my_poly, 255, 0, 255);
        setRGB1(my_poly, 255, 255, 0);
        setRGB2(my_poly, 0, 255, 255);
        setRGB3(my_poly, 255, 255, 255);

        // Load the vertices of this face
        gte_ldv3(
            &cube.vertices[cube.faces[i + 0]],
            &cube.vertices[cube.faces[i + 1]],
            &cube.vertices[cube.faces[i + 2]]
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
        gte_stsxy0(&my_poly->x0);
        gte_stsxy1(&my_poly->x1);
        gte_stsxy2(&my_poly->x2);

        // Compute the last vertex
        gte_ldv0(&cube.vertices[cube.faces[i + 3]]);
        gte_rtps();
        gte_stsxy(&my_poly->x3);

        addPrim(ordering_table[screen.current_buffer][average_z], my_poly);
        next_primitive += sizeof(POLY_G4);
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
