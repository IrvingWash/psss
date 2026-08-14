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

typedef struct Entity {
    VECTOR position;
    SVECTOR rotation;
    VECTOR scale;

    Display_Mesh mesh;
} Entity;

Camera camera = {0};

#define SHIP_COUNT 8
Entity ships[SHIP_COUNT];

#define SCENERY_COUNT 264
Entity sceneries[SCENERY_COUNT];

int current_ship = 0;

void setup(void) {
    display_init();
    joypad_init();
    cd_init();

    { // "All ships"
        u_long cmp_length;
        char *cmp = cd_load_file( "\\ASSETS\\ALLSH.CMP;1", &cmp_length);
        u_long texture_start = get_texture_count();
        texture_create_from_cmp(cmp);

        u_long prm_length;
        char *prm = cd_load_file("\\ASSETS\\ALLSH.PRM;1", &prm_length);
        Display_Mesh_Bundle mesh_bundle = display_create_meshes_from_prm(prm, texture_start, SHIP_COUNT);

        for (int i = 0; i < SHIP_COUNT; i++) {
            ships[i].mesh = mesh_bundle.meshes[i];
            ships[i].position = ships[i].mesh.origin;
            ships[i].scale = (VECTOR) {ONE, ONE, ONE};
        }

        cd_unload_file(prm);
        cd_unload_file(cmp);
    }

    { // Scenery
        u_long cmp_length;
        char *cmp = cd_load_file( "\\ASSETS\\SCENE.CMP;1", &cmp_length);
        u_long texture_start = get_texture_count();
        texture_create_from_cmp(cmp);

        u_long prm_length;
        char *prm = cd_load_file("\\ASSETS\\SCENE.PRM;1", &prm_length);
        Display_Mesh_Bundle mesh_bundle = display_create_meshes_from_prm(prm, texture_start, SCENERY_COUNT);

        for (int i = 0; i < SCENERY_COUNT; i++) {
            sceneries[i].mesh = mesh_bundle.meshes[i];
            sceneries[i].scale = (VECTOR) {ONE, ONE, ONE};
            sceneries[i].position = sceneries[i].mesh.origin;
        }

        cd_unload_file(prm);
        cd_unload_file(cmp);
    }

    ships[current_ship].position.vx = 32599;
    ships[current_ship].position.vy = -347;
    ships[current_ship].position.vz = -45310;

    camera.position.vx = ships[current_ship].position.vx;
    camera.position.vy = ships[current_ship].position.vy - 100;
    camera.position.vz = ships[current_ship].position.vz - 1000;
}

void deinit(void) {
    for (int i = 0; i < SHIP_COUNT; i++) {
        display_destroy_mesh(&ships[i].mesh);
    }

    for (int i = 0; i < SCENERY_COUNT; i++) {
        display_destroy_mesh(&sceneries[i].mesh);
    }

    joypad_deinit();
}

void update(void) {
    camera_look_at(&camera, &ships[current_ship].position, &(SVECTOR) {0, -ONE, 0});

    if (joypad_is_pressed(PAD_LEFT)) {
        camera.position.vx -= 100;
    }
    if (joypad_is_pressed(PAD_RIGHT)) {
        camera.position.vx += 100;
    }
    if (joypad_is_pressed(PAD_UP)) {
        camera.position.vz += 100;
    }
    if (joypad_is_pressed(PAD_DOWN)) {
        camera.position.vz -= 100;
    }

    if (joypad_is_released(PAD_RIGHT)) {
    }
    if (joypad_is_released(PAD_LEFT)) {
    }
}

void render(void) {
    display_start_frame(&camera.look_at);

    display_draw_mesh(
        &ships[0].mesh,
        ships[0].position,
        ships[0].rotation,
        ships[0].scale
    );

    for (int i = 0; i < SCENERY_COUNT; i++) {
        Entity *scenery_object = &sceneries[i];

        { // Draw scenery which is close enough to the camera
            VECTOR delta = {0};
            delta.vx = scenery_object->position.vx - camera.position.vx;
            delta.vy = scenery_object->position.vy - camera.position.vy;
            delta.vz = scenery_object->position.vz - camera.position.vz;

            u_long delta_magnitude_squared = delta.vx * delta.vx + delta.vy * delta.vy + delta.vz * delta.vz;

            if (delta_magnitude_squared < 700000000) {
                display_draw_mesh(
                    &scenery_object->mesh,
                    scenery_object->position,
                    scenery_object->rotation,
                    scenery_object->scale
                );
            }
        }
    }

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
