#include "texture.h"

#include <psxgpu.h>
#include <sys/types.h>

#define CLUT_FLAG 0x8

Texture texture_create(u_long *data) {
    Texture texture = {0};

    {
        TIM_IMAGE info;
        GetTimInfo(data, &info);
        LoadImage(info.prect, info.paddr);
        DrawSync(0);

        if (info.mode & CLUT_FLAG) {
            LoadImage(info.crect, info.caddr);
            DrawSync(0);
        }

        texture.tim = info;
    }

    return texture;
}
