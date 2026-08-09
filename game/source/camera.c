#include "camera.h"
#include <psxgte.h>

static inline void cross_s(const SVECTOR *a, const SVECTOR *b, VECTOR *out) {
    out->vx = ((int)a->vy * b->vz - (int)a->vz * b->vy) >> 12;
    out->vy = ((int)a->vz * b->vx - (int)a->vx * b->vz) >> 12;
    out->vz = ((int)a->vx * b->vy - (int)a->vy * b->vx) >> 12;
}

void camera_look_at(Camera *camera, const VECTOR *target, const SVECTOR *reference_up) {
    VECTOR x_right, y_up, z_forward;
    SVECTOR x, y, z;
    VECTOR position, t;

    z_forward.vx = target->vx - camera->position.vx;
    z_forward.vy = target->vy - camera->position.vy;
    z_forward.vz = target->vz - camera->position.vz;
    VectorNormalS(&z_forward, &z);

    cross_s(&z, reference_up, &x_right);
    VectorNormalS(&x_right, &x);

    cross_s(&z, &x, &y_up);
    VectorNormalS(&y_up, &y);

    camera->look_at.m[0][0] = x.vx; camera->look_at.m[0][1] = x.vy; camera->look_at.m[0][2] = x.vz;
    camera->look_at.m[1][0] = y.vx; camera->look_at.m[1][1] = y.vy; camera->look_at.m[1][2] = y.vz;
    camera->look_at.m[2][0] = z.vx; camera->look_at.m[2][1] = z.vy; camera->look_at.m[2][2] = z.vz;

    position.vx = -camera->position.vx;
    position.vy = -camera->position.vy;
    position.vz = -camera->position.vz;

    ApplyMatrixLV(&camera->look_at, &position, &t);
    TransMatrix(&camera->look_at, &t);
}
