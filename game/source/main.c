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

Entity ship = {0};

Entity create_entity(void) {
    Entity entity = {0};

    entity.scale = (VECTOR) {ONE, ONE, ONE};

    u_long all_ships_cmp_length;
    char *all_ships_cmp = cd_load_file("\\ASSETS\\ALLSH.CMP;1", &all_ships_cmp_length);
    texture_create_from_cmp(all_ships_cmp);

    u_long all_ships_prm_length;
    char *all_ships_prm = cd_load_file("\\ASSETS\\ALLSH.PRM;1", &all_ships_prm_length);
    entity.mesh = display_create_mesh_from_prm(all_ships_prm);
    entity.position = entity.mesh.origin;

    cd_unload_file(all_ships_prm);
    cd_unload_file(all_ships_cmp);

    return entity;
}

void setup(void) {
    display_init();
    joypad_init();
    cd_init();

    camera.position.vy = -600;
    camera.position.vz = -900;

    ship = create_entity();
}

void deinit(void) {
    display_destroy_mesh(&ship.mesh);

    joypad_deinit();
}

void update(void) {
    camera_look_at(&camera, &ship.position, &(SVECTOR) {0, -ONE, 0});

    if (joypad_is_pressed(PAD_LEFT)) {
        ship.rotation.vy -= 15;
    }
    if (joypad_is_pressed(PAD_RIGHT)) {
        ship.rotation.vy += 15;
    }
}

void render(void) {
    display_start_frame(&camera.look_at);

    display_draw_mesh(
        &ship.mesh,
        ship.position,
        ship.rotation,
        ship.scale
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
