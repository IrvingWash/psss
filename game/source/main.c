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
Entity solaar = {0};
Entity jacko = {0};
Entity chang = {0};
Entity arian = {0};
Entity arial = {0};
Entity anasta = {0};
Entity dekka = {0};

#define ENTITY_COUNT 8
Entity* entities[ENTITY_COUNT] = {
    &sophia,
    &solaar,
    &jacko,
    &chang,
    &arian,
    &arial,
    &anasta,
    &dekka,
};

bool is_right_pressed = false;
bool is_left_pressed = false;

typedef enum Entity_Type {
    SOPHIA,
    SOLAAR,
    JACKO,
    CHANG,
    ARIAN,
    ARIAL,
    ANASTA,
    DEKKA,

    ENTITY_TYPE_COUNT,
} Entity_Type;

enum Entity_Type current_entity = SOPHIA;

void setup(void) {
    display_init();
    joypad_init();
    cd_init();

    camera.position.vy = -600;
    camera.position.vz = -1500;

    { // "All ships"
        u_long cmp_length;
        char *cmp = cd_load_file( "\\ASSETS\\ALLSH.CMP;1", &cmp_length);
        u_long texture_start = get_texture_count();
        texture_create_from_cmp(cmp);

        u_long prm_length;
        char *prm = cd_load_file("\\ASSETS\\ALLSH.PRM;1", &prm_length);
        Display_Mesh_Bundle mesh_bundle = display_create_meshes_from_prm(prm, texture_start, 8);

        sophia.mesh = mesh_bundle.meshes[0];
        solaar.mesh = mesh_bundle.meshes[1];
        jacko.mesh = mesh_bundle.meshes[2];
        chang.mesh = mesh_bundle.meshes[3];
        arian.mesh = mesh_bundle.meshes[4];
        arial.mesh = mesh_bundle.meshes[5];
        anasta.mesh = mesh_bundle.meshes[6];
        dekka.mesh = mesh_bundle.meshes[7];
        sophia.position = sophia.mesh.origin;
        solaar.position = solaar.mesh.origin;
        jacko.position = jacko.mesh.origin;
        chang.position = chang.mesh.origin;
        arian.position = arian.mesh.origin;
        arial.position = arial.mesh.origin;
        anasta.position = anasta.mesh.origin;
        dekka.position = dekka.mesh.origin;
        sophia.scale = (VECTOR) {ONE, ONE, ONE};
        solaar.scale = (VECTOR) {ONE, ONE, ONE};
        jacko.scale = (VECTOR) {ONE, ONE, ONE};
        chang.scale = (VECTOR) {ONE, ONE, ONE};
        arian.scale = (VECTOR) {ONE, ONE, ONE};
        arial.scale = (VECTOR) {ONE, ONE, ONE};
        anasta.scale = (VECTOR) {ONE, ONE, ONE};
        dekka.scale = (VECTOR) {ONE, ONE, ONE};

        cd_unload_file(prm);
        cd_unload_file(cmp);
    }
}

void deinit(void) {
    display_destroy_mesh(&sophia.mesh);
    display_destroy_mesh(&solaar.mesh);
    display_destroy_mesh(&jacko.mesh);
    display_destroy_mesh(&chang.mesh);
    display_destroy_mesh(&arian.mesh);
    display_destroy_mesh(&arial.mesh);
    display_destroy_mesh(&anasta.mesh);
    display_destroy_mesh(&dekka.mesh);

    joypad_deinit();
}

void update(void) {
    camera_look_at(&camera, &sophia.position, &(SVECTOR) {0, -ONE, 0});

    if (joypad_is_pressed(PAD_LEFT) && !is_left_pressed) {
        current_entity = (current_entity - 1 + ENTITY_TYPE_COUNT) % ENTITY_TYPE_COUNT;
        is_left_pressed = true;
    }
    if (joypad_is_pressed(PAD_RIGHT) && !is_right_pressed) {
        current_entity = (current_entity + 1) % ENTITY_TYPE_COUNT;
        is_right_pressed = true;
    }

    if (joypad_is_released(PAD_RIGHT) && is_right_pressed) {
        is_right_pressed = false;
    }
    if (joypad_is_released(PAD_LEFT) && is_left_pressed) {
        is_left_pressed = false;
    }

    for (int i = 0; i < ENTITY_COUNT; i++) {
        entities[i]->rotation.vy += 10;
    }
}

void render(void) {
    display_start_frame(&camera.look_at);

    switch (current_entity) {
        case SOPHIA:
            display_draw_mesh(
                &sophia.mesh,
                sophia.position,
                sophia.rotation,
                sophia.scale
            );
            break;
        case SOLAAR:
            display_draw_mesh(
                &solaar.mesh,
                solaar.position,
                solaar.rotation,
                solaar.scale
            );
            break;
        case JACKO:
            display_draw_mesh(
                &jacko.mesh,
                jacko.position,
                jacko.rotation,
                jacko.scale
            );
            break;
        case CHANG:
            display_draw_mesh(
                &chang.mesh,
                chang.position,
                chang.rotation,
                chang.scale
            );
            break;
        case ARIAN:
            display_draw_mesh(
                &arian.mesh,
                arian.position,
                arian.rotation,
                arian.scale
            );
            break;
        case ARIAL:
            display_draw_mesh(
                &arial.mesh,
                arial.position,
                arial.rotation,
                arial.scale
            );
            break;
        case ANASTA:
            display_draw_mesh(
                &anasta.mesh,
                anasta.position,
                anasta.rotation,
                anasta.scale
            );
            break;
        case DEKKA:
            display_draw_mesh(
                &dekka.mesh,
                dekka.position,
                dekka.rotation,
                dekka.scale
            );
            break;
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
