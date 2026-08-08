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
#define OT_LENGTH 16

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

// "Entities"
POLY_F3 *my_triangle;
TILE    *my_tile;
POLY_G4 *my_quad;

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
    // Clear OT, so we can reuse it
    ClearOTagR(ordering_table[screen.current_buffer], OT_LENGTH);
    next_primitive = primitive_buffer[screen.current_buffer];

    my_tile = (TILE*) next_primitive; // Point to the next slot in the primitive_buffer
    setTile(my_tile); // Initialize the primitive
    setXY0(my_tile, 82, 32);
    setWH(my_tile, 64, 64);
    setRGB0(my_tile, 0, 255, 0);
    addPrim(ordering_table[screen.current_buffer], my_tile); // Add the primitive to the ordering table, the function is from the SDK!
    next_primitive += sizeof(TILE);

    my_triangle = (POLY_F3*) next_primitive;
    setPolyF3(my_triangle);
    setXY3(my_triangle, 64, 100, 200, 150, 50, 220);
    setRGB0(my_triangle, 255, 0, 255);
    addPrim(ordering_table[screen.current_buffer], my_triangle);
    next_primitive += sizeof(POLY_F3);

    my_quad = (POLY_G4*) next_primitive;
    setPolyG4(my_quad);
    setXY4(my_quad, 140, 50, 200, 40, 170, 120, 220, 80);
    setRGB0(my_quad, 255, 0, 0);
    setRGB1(my_quad, 0, 255, 0);
    setRGB2(my_quad, 0, 0, 255);
    setRGB3(my_quad, 255, 255, 0);
    addPrim(ordering_table[screen.current_buffer], my_quad);
    next_primitive += sizeof(POLY_G4);
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
