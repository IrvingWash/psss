#include "texture.h"

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <psxgpu.h>
#include <sys/types.h>
#include "utils.h"
#include "../vendor/lzss/lzss.h"

static Texture *texture_storage[MAX_TEXTURES];
static u_long texture_count = 0;

Texture* get_texture(u_int index) {
    return texture_storage[index];
}

u_long get_texture_count() {
    return texture_count;
};

Texture* upload_texture_to_vram(long tim_ptr) {
    Texture *texture = malloc(sizeof(Texture));

    TIM *tim = (TIM*) tim_ptr;

    switch (CLUT_TYPE(tim)) {
        case CLUT_4BIT: {
            TIM_CLUT4 *tc4 = (TIM_CLUT4*) tim;

            // Populate texture
            texture->type = CLUT4;
            texture->texture_x = tc4->texture_x;
            texture->texture_y = tc4->texture_y;
            texture->texture_w = tc4->texture_w;
            texture->texture_h = tc4->texture_h;
            texture->clut_x = tc4->clut_x;
            texture->clut_y = tc4->clut_y;
            texture->clut_w = tc4->clut_w;
            texture->clut_h = tc4->clut_h;

            u_short x = tc4->texture_x - TEXTURE_H_OFFSET(tc4->texture_x);
            u_short y = tc4->texture_y - TEXTURE_V_OFFSET(tc4->texture_y);

            texture->u0 = (x << 2);
            texture->v0 = (y);
            texture->u1 = ((x + tc4->texture_w) << 2) - 1;
            texture->v1 = (y);
            texture->u2 = (x << 2);
            texture->v2 = (y + tc4->texture_h) - 1;
            texture->u3 = ((x + tc4->texture_w) << 2) - 1;
            texture->v3 = (y + tc4->texture_h) - 1;

            texture->tpage = TPAGE(CLUT_4BIT, TRANSLUCENT, texture->texture_x, texture->texture_y);
            texture->clut  = CLUT(texture->clut_x >> 4, texture->clut_y);

            // Upload CLUT
            RECT rect;
            rect.x = tc4->clut_x;
            rect.y = tc4->clut_y;
            rect.w = tc4->clut_w;
            rect.h = tc4->clut_h;
            LoadImage(&rect, (u_long*)(&tc4->clut));
            DrawSync(0);

            // Upload texture
            rect.x = tc4->texture_x;
            rect.y = tc4->texture_y;
            rect.w = tc4->texture_w;
            rect.h = tc4->texture_h;
            LoadImage(&rect, (u_long*)(tc4 + 1));
            DrawSync(0);

            break;
        }
        case CLUT_8BIT: {
            TIM_CLUT8 *tc8 = (TIM_CLUT8*) tim;

            texture->type = CLUT8;
            texture->texture_x = tc8->texture_x;
            texture->texture_y = tc8->texture_y;
            texture->texture_w = tc8->texture_w;
            texture->texture_h = tc8->texture_h;
            texture->clut_x    = tc8->clut_x;
            texture->clut_y    = tc8->clut_y;
            texture->clut_w    = tc8->clut_w;
            texture->clut_h    = tc8->clut_h;

            u_short x = tc8->texture_x - TEXTURE_H_OFFSET(tc8->texture_x);
            u_short y = tc8->texture_y - TEXTURE_V_OFFSET(tc8->texture_y);

            texture->u0 = (x << 1);
            texture->v0 = (y);
            texture->u1 = ((x + tc8->texture_w) << 1) - 1;
            texture->v1 = (y);
            texture->u2 = (x << 1);
            texture->v2 = (y + tc8->texture_h) - 1;
            texture->u3 = ((x + tc8->texture_w) << 1) - 1;
            texture->v3 = (y + tc8->texture_h) - 1;

            texture->tpage = TPAGE(CLUT_8BIT, TRANSLUCENT, texture->texture_x, texture->texture_y);
            texture->clut  = CLUT(texture->clut_x >> 4, texture->clut_y);

            RECT rect;
            rect.x = tc8->clut_x;
            rect.y = tc8->clut_y;
            rect.w = tc8->clut_w;
            rect.h = tc8->clut_h;
            LoadImage(&rect, (u_long*)(&tc8->clut));
            DrawSync(0);

            rect.x = tc8->texture_x;
            rect.y = tc8->texture_y;
            rect.w = tc8->texture_w;
            rect.h = tc8->texture_h;
            LoadImage(&rect, (u_long*)(tc8 + 1));
            DrawSync(0);

            break;
        }
    }

    return texture;
};

void texture_create_from_cmp(char *cmp_data) {
    u_long byte_counter = 0;

    u_short tim_count = get_long_le(cmp_data, &byte_counter);

    // Get sizes
    static long tim_offsets[400];
    u_long total_tim_size = 0;
    for (int i = 0; i < tim_count; i++) {
        tim_offsets[i] = total_tim_size;
        u_long tim_size = get_long_le(cmp_data, &byte_counter);
        total_tim_size += tim_size;
    }

    // Populate offsets
    static void *tims_base_addr;
    tims_base_addr = (char *) malloc(total_tim_size);
    for (int i = 0; i < tim_count; i++) {
        tim_offsets[i] += (long) tims_base_addr;
    }

    ExpandLZSSData(&cmp_data[byte_counter], tims_base_addr);

    // Upload to VRAM
    for (int i = 0; i < tim_count; i++) {
        Texture *texture = upload_texture_to_vram(tim_offsets[i]);
        if (texture != NULL) {
            texture_storage[texture_count++] = texture;
        }
        if (texture_count > MAX_TEXTURES) {
            printf("Too many textures: %d\n", texture_count);
            assert(false);
        }
    }

    free(tims_base_addr);
}
