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

typedef struct Entity {
    Display_Mesh mesh;
} Entity;

Camera camera = {0};

Entity entity = {0};

void setup(void) {
    display_init();
    joypad_init();
    cd_init();

    {
        u_long all_ships_prm_length;
        char *all_ships_prm = cd_load_file("\\ASSETS\\ALLSH.PRM;1", &all_ships_prm_length);

        entity.mesh = display_create_mesh_from_prm(all_ships_prm);

        cd_unload_file(all_ships_prm);
    }
}

void deinit(void) {
    display_destroy_mesh(&entity.mesh);

    joypad_deinit();
}

void update(void) {}

void render(void) {
    display_start_frame(&camera.look_at);

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
