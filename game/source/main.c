#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
// PSn00bSDK
#include <sys/types.h> // u_char, etc
#include <psxgte.h> // For geometry

#include "joypad.h"
#include "display.h"
#include "camera.h"

#define CUBE_FACE_COUNT 6
#define CUBE_VERTICES_PER_FACE 4
typedef struct Cube {
    SVECTOR rotation;
    VECTOR  position;
    VECTOR  scale;
    VECTOR  velocity;
    VECTOR  acceleration;
    SVECTOR vertices[8];
    short   faces[CUBE_FACE_COUNT * CUBE_VERTICES_PER_FACE];
} Cube;

#define FLOOR_FACE_COUNT 2
#define FLOOR_VERTICES_PER_FACE 3
typedef struct Floor {
    SVECTOR rotation;
    VECTOR  position;
    VECTOR  scale;
    SVECTOR vertices[4];
    short   faces[FLOOR_FACE_COUNT * FLOOR_VERTICES_PER_FACE];
} Floor;

Camera camera;

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

void setup(void) {
    display_init();
    joypad_init();

    camera.position.vx = 500;
    camera.position.vy = -1000;
    camera.position.vz = -1500;
    camera.look_at = (MATRIX){0};
}

void deinit(void) {
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

    { // Cube
        // Update the position based on the acceleration and velocity
        cube.velocity.vx += cube.acceleration.vx;
        cube.velocity.vy += cube.acceleration.vy;
        cube.velocity.vz += cube.acceleration.vz;

        cube.position.vx += cube.velocity.vx;
        cube.position.vy += cube.velocity.vy;
        cube.position.vz += cube.velocity.vz;

        if (cube.position.vy + 200 > floor.position.vy) {
            cube.velocity.vy *= -1;
        }
    }
}

void render(void) {
    camera_look_at(&camera, &cube.position, &(SVECTOR){0, -ONE, 0});
    display_start_frame(&camera.look_at);

    display_draw_mesh_of_g4(
        cube.position,
        cube.scale,
        cube.rotation,
        CUBE_FACE_COUNT,
        CUBE_VERTICES_PER_FACE,
        (u_short[]){255, 0, 0},
        (u_short[]){0, 255, 0},
        (u_short[]){0, 0, 255},
        (u_short[]){255, 255, 0},
        cube.vertices,
        cube.faces
    );

    display_draw_mesh_of_f3(
        floor.position,
        floor.scale,
        floor.rotation,
        FLOOR_FACE_COUNT,
        FLOOR_VERTICES_PER_FACE,
        (u_short[]){255, 255, 255},
        floor.vertices,
        floor.faces
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
