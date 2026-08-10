#ifndef TEXTURE_H
#define TEXTURE_H

#include <sys/types.h>
#include <psxgpu.h>

typedef struct Texture {
    TIM_IMAGE tim;
} Texture;

Texture texture_create(u_long *data);

#endif
