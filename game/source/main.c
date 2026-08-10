#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
// PSn00bSDK
#include <sys/types.h> // u_char, etc
#include <psxgte.h> // For geometry

#include "joypad.h"
#include "display.h"
#include "cd.h"
#include "texture.h"
#include "camera.h"

typedef struct Cube {
    SVECTOR rotation;
    VECTOR  position;
    VECTOR  scale;
    Display_Mesh mesh;
} Cube;

Camera camera;

Cube  cube  = {
    { 0, 0, 0 }, // Rotation
    { 0, -400, 1800 }, // Position
    { ONE, ONE, ONE }, // Scale
    {0}, // Mesh
};

void setup(void) {
    display_init();
    joypad_init();
    cd_init();

    camera.position.vx = 500;
    camera.position.vy = -1000;
    camera.position.vz = -1500;
    camera.look_at = (MATRIX){0};

    { // Load cube mesh
        u_long mesh_data_length;
        char *mesh_data = cd_load_file("\\ASSETS\\MODEL.BIN;1", &mesh_data_length);

        u_long bricks_texture_data_length;
        u_long *bricks_texture_data = (u_long*) cd_load_file("\\ASSETS\\BRICKS.TIM;1", &bricks_texture_data_length);

        cube.mesh = display_create_mesh(mesh_data);
        cube.mesh.texture = texture_create(bricks_texture_data).tim;

        cd_unload_file(mesh_data);
        cd_unload_file((char*) bricks_texture_data);
    }
}

void deinit(void) {
    display_destroy_mesh(&cube.mesh);

    joypad_deinit();
}

void update(void) {
    { // Camera
        int speed = 50;

        if (joypad_is_pressed(PAD_LEFT)) {
             camera.position.vx -= speed;
        }
        if (joypad_is_pressed(PAD_RIGHT)) {
            camera.position.vx += speed;
        }
        if (joypad_is_pressed(PAD_UP)) {
            camera.position.vy -= speed;
        }
        if (joypad_is_pressed(PAD_DOWN)) {
            camera.position.vy += speed;
        }
        if (joypad_is_pressed(PAD_CROSS)) {
            camera.position.vz += speed;
        }
        if (joypad_is_pressed(PAD_CIRCLE)) {
            camera.position.vz -= speed;
        }
    }
}

void render(void) {
    camera_look_at(&camera, &cube.position, &(SVECTOR){0, -ONE, 0});
    display_start_frame(&camera.look_at);

    display_draw_mesh_of_ft4(
        cube.position,
        cube.scale,
        cube.rotation,
        &cube.mesh
    );

    display_finish_frame();
}

int main(void) {
    setup();

    while (true) {
        update();
        render();
    }

    deinit();

    return 0;
}
