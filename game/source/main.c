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

Entity sophia = {0};
Entity rescue = {0};

Entity create_entity(const char *prm_path, const char *cmp_path) {
    Entity entity = {0};

    entity.scale = (VECTOR) {ONE, ONE, ONE};

    u_long cmp_length;
    char *cmp = cd_load_file(cmp_path, &cmp_length);
    u_long texture_start = get_texture_count();
    texture_create_from_cmp(cmp);

    u_long prm_length;
    char *prm = cd_load_file(prm_path, &prm_length);
    entity.mesh = display_create_mesh_from_prm(prm, texture_start);
    entity.position = entity.mesh.origin;

    cd_unload_file(prm);
    cd_unload_file(cmp);

    return entity;
}

void setup(void) {
    display_init();
    joypad_init();
    cd_init();

    camera.position.vy = -600;
    camera.position.vz = -1500;

    sophia = create_entity("\\ASSETS\\ALLSH.PRM;1", "\\ASSETS\\ALLSH.CMP;1");
    rescue = create_entity("\\ASSETS\\RESCU.PRM;1", "\\ASSETS\\RESCU.CMP;1");
}

void deinit(void) {
    display_destroy_mesh(&sophia.mesh);
    display_destroy_mesh(&rescue.mesh);

    joypad_deinit();
}

void update(void) {
    camera_look_at(&camera, &sophia.position, &(SVECTOR) {0, -ONE, 0});

    if (joypad_is_pressed(PAD_LEFT)) {
        sophia.rotation.vy -= 15;
        rescue.rotation.vy -= 15;
    }
    if (joypad_is_pressed(PAD_RIGHT)) {
        sophia.rotation.vy += 15;
        rescue.rotation.vy += 15;
    }
}

void render(void) {
    display_start_frame(&camera.look_at);

    display_draw_mesh(
        &sophia.mesh,
        sophia.position,
        sophia.rotation,
        sophia.scale
    );

    display_draw_mesh(
        &rescue.mesh,
        rescue.position,
        rescue.rotation,
        rescue.scale
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
