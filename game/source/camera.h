#ifndef CAMERA_H
#define CAMERA_H

#include <psxgte.h>

typedef struct Camera {
    VECTOR position;
    SVECTOR rotation;
    MATRIX look_at;
} Camera;

void camera_look_at(Camera *camera, const VECTOR *target, const SVECTOR *reference_up);

#endif
