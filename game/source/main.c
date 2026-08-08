#include <stdlib.h>
#include <stdbool.h>
// PSn00bSDK
#include <psxgte.h>    // The "Geometry Transformation Engine" co-processor stuff
#include <psxetc.h>    // Input and stuff
#include <psxgpu.h>    // GPU stuff
#include <inline_c.h>  // IDK why it's here, in original PsyQ it seems to be in libgte.h

#define VIDEO_MODE 0
#define SCREEN_RES_X 320
#define SCREEN_RES_Y 240
#define SCREEN_CENTER_X (SCREEN_RES_X >> 1)
#define SCREEN_CENTER_Y (SCREEN_RES_Y >> 1)

typedef struct {
    DRAWENV draw[2];
    DISPENV disp[2];

    short current_buffer; // 0 or 1
} Double_Buffer;

Double_Buffer screen;

void init_screen(void) {
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
    gte_SetGeomScreen(SCREEN_CENTER_X);

    // Enable display
    SetDispMask(1);
}

void display_frame(void) {
    DrawSync(0);
    VSync(0);

    PutDispEnv(&screen.disp[screen.current_buffer]);
    PutDrawEnv(&screen.draw[screen.current_buffer]);

    // Swap buffers
    screen.current_buffer = !screen.current_buffer;
}

void setup(void) {
    init_screen();
}

void update(void) {}

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
