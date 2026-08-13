#include "display.h"

#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <psxgpu.h>
#include <sys/types.h>
#include <psxgte.h>
#include <inline_c.h>
#include "utils.h"
#include "display_primitives.h"
#include "texture.h"

#define SCREEN_RES_X 320
#define SCREEN_RES_Y 240
#define SCREEN_CENTER_X (SCREEN_RES_X >> 1)
#define SCREEN_CENTER_Y (SCREEN_RES_Y >> 1)
#define SCREEN_Z 400

#define BUFFER_COUNT 2
#define OT_LENGTH 8192
#define PRIMITIVE_BUFFER_SIZE 131072

typedef struct {
    DRAWENV draw[BUFFER_COUNT];
    DISPENV disp[BUFFER_COUNT];

    short current_buffer; // 0 or 1
} Double_Buffer;

Double_Buffer screen;
// Contains pointers to geometry which will be drawn, passed as is to the SDK
u_long ordering_table[BUFFER_COUNT][OT_LENGTH] = {0};
// Contains geometry data, of different types
char   primitive_buffer[BUFFER_COUNT][PRIMITIVE_BUFFER_SIZE] = {0};
// Pointes to the end of the last primitive in the primitive buffer
char   *next_primitive;

MATRIX view_transform  = {0};

void display_init(void) {
    assert(BUFFER_COUNT == 2); // If this fails, many things should be changed

    ResetGraph(0); // Reset the GPU

    // Display area of first buffer
    SetDefDispEnv(&screen.disp[0], 0, 0, SCREEN_RES_X, SCREEN_RES_Y);
    SetDefDrawEnv(&screen.draw[0], 0, SCREEN_RES_Y, SCREEN_RES_X, SCREEN_RES_Y);

    // Display area of second buffer
    SetDefDispEnv(&screen.disp[1], 0, SCREEN_RES_Y, SCREEN_RES_X, SCREEN_RES_Y);
    SetDefDrawEnv(&screen.draw[1], 0, 0, SCREEN_RES_X, SCREEN_RES_Y);

    // Background buffers
    screen.draw[0].isbg = true;
    screen.draw[1].isbg = true;

    // Clear color
    setRGB0(&screen.draw[0], 0, 0, 127);
    setRGB0(&screen.draw[1], 0, 0, 127);

    // Initial buffer
    screen.current_buffer = 0;
    PutDispEnv(&screen.disp[screen.current_buffer]);
    PutDrawEnv(&screen.draw[screen.current_buffer]);

    // Initialize GTE and set geometry offsets
    InitGeom();
    gte_SetGeomOffset(SCREEN_CENTER_X, SCREEN_CENTER_Y);
    gte_SetGeomScreen(SCREEN_Z);

    // Enable display
    SetDispMask(1);

    next_primitive = primitive_buffer[screen.current_buffer];
}

void display_start_frame(const MATRIX *new_view_transform) {
    // Clear OT, so we can reuse it
    ClearOTagR(ordering_table[screen.current_buffer], OT_LENGTH);
    next_primitive = primitive_buffer[screen.current_buffer];
    view_transform = *new_view_transform;
}

void display_finish_frame(void) {
    DrawSync(0);
    VSync(0);

    PutDispEnv(&screen.disp[screen.current_buffer]);
    PutDrawEnv(&screen.draw[screen.current_buffer]);

    DrawOTag(ordering_table[screen.current_buffer] + OT_LENGTH - 1); // Pointer to the end, because we need to go from back to front

    // Swap buffers
    screen.current_buffer = !screen.current_buffer;
}

void display_draw_mesh(
    const Display_Mesh *mesh,
    VECTOR position,
    SVECTOR rotation,
    VECTOR scale
) {
    { // Prepare transforms
        MATRIX world_transform = {0};

        RotMatrix(&rotation, &world_transform);
        TransMatrix(&world_transform, &position);
        ScaleMatrix(&world_transform, &scale);

        MATRIX transform = {0};
        CompMatrixLV(&view_transform, &world_transform, &transform);

        gte_SetRotMatrix(&transform);
        gte_SetTransMatrix(&transform);
    }

    for (int i = 0; i < mesh->primitive_count; i++) {
        switch (mesh->primitives[i].type) {
            case TYPE_F3: {
                F3 *prm = (F3*) mesh->primitives[i].primitive;
                POLY_F3 *poly = (POLY_F3*) next_primitive;

                // Load vertices into GTE
                gte_ldv0(&mesh->vertices[prm->coords[0]]);
                gte_ldv1(&mesh->vertices[prm->coords[1]]);
                gte_ldv2(&mesh->vertices[prm->coords[2]]);

                // Transform
                gte_rtpt();

                { // Backface culling
                    int nclip = 0;

                    gte_nclip();
                    gte_stopz(&nclip);

                    if (nclip < 0) {
                        continue;
                    }
                }

                // Load vertices back
                gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);

                int otz = 0;
                gte_avsz3();
                gte_stotz(&otz);
                if (otz >= OT_LENGTH || otz < 0) { // Depth test
                    continue;
                }

                setPolyF3(poly);
                setRGB0(poly, prm->color.r, prm->color.g, prm->color.b);

                addPrim(&ordering_table[screen.current_buffer][otz], poly);
                next_primitive += sizeof(POLY_F3);

                break;
            }
            case TYPE_G3: {
                G3 *prm = (G3*) mesh->primitives[i].primitive;
                POLY_G3 *poly = (POLY_G3*) next_primitive;

                // Load vertices into GTE
                gte_ldv0(&mesh->vertices[prm->coords[0]]);
                gte_ldv1(&mesh->vertices[prm->coords[1]]);
                gte_ldv2(&mesh->vertices[prm->coords[2]]);

                // Transform
                gte_rtpt();

                { // Backface culling
                    int nclip = 0;

                    gte_nclip();
                    gte_stopz(&nclip);

                    if (nclip < 0) {
                        continue;
                    }
                }

                // Load vertices back
                gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);

                int otz = 0;
                gte_avsz3();
                gte_stotz(&otz);
                if (otz >= OT_LENGTH || otz < 0) { // Depth test
                    continue;
                }

                setPolyG3(poly);
                setRGB0(poly, prm->color[0].r, prm->color[0].g, prm->color[0].b);
                setRGB1(poly, prm->color[1].r, prm->color[1].g, prm->color[1].b);
                setRGB2(poly, prm->color[2].r, prm->color[2].g, prm->color[2].b);

                addPrim(&ordering_table[screen.current_buffer][otz], poly);
                next_primitive += sizeof(POLY_G3);

                break;
            }
            case TYPE_FT3: {
                FT3 *prm = (FT3*) mesh->primitives[i].primitive;
                POLY_FT3 *poly = (POLY_FT3*) next_primitive;

                // Load vertices into GTE
                gte_ldv0(&mesh->vertices[prm->coords[0]]);
                gte_ldv1(&mesh->vertices[prm->coords[1]]);
                gte_ldv2(&mesh->vertices[prm->coords[2]]);

                // Transform
                gte_rtpt();

                { // Backface culling
                    int nclip = 0;

                    gte_nclip();
                    gte_stopz(&nclip);

                    if (nclip < 0) {
                        continue;
                    }
                }

                // Load vertices back
                gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);

                int otz = 0;
                gte_avsz3();
                gte_stotz(&otz);
                if (otz >= OT_LENGTH || otz < 0) { // Depth test
                    continue;
                }

                setPolyFT3(poly);
                setRGB0(poly, prm->color.r, prm->color.g, prm->color.b);

                poly->tpage = prm->tpage;
                poly->clut = prm->clut;
                setUV3(poly, prm->u0, prm->v0, prm->u1, prm->v1, prm->u2, prm->v2);

                addPrim(&ordering_table[screen.current_buffer][otz], poly);
                next_primitive += sizeof(POLY_FT3);

                break;
            }
            case TYPE_GT3: {
                GT3 *prm = (GT3*) mesh->primitives[i].primitive;
                POLY_GT3 *poly = (POLY_GT3*) next_primitive;

                // Load vertices into GTE
                gte_ldv0(&mesh->vertices[prm->coords[0]]);
                gte_ldv1(&mesh->vertices[prm->coords[1]]);
                gte_ldv2(&mesh->vertices[prm->coords[2]]);

                // Transform
                gte_rtpt();

                { // Backface culling
                    int nclip = 0;

                    gte_nclip();
                    gte_stopz(&nclip);

                    if (nclip < 0) {
                        continue;
                    }
                }

                // Load vertices back
                gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);

                int otz = 0;
                gte_avsz3();
                gte_stotz(&otz);
                if (otz >= OT_LENGTH || otz < 0) { // Depth test
                    continue;
                }

                setPolyGT3(poly);
                setRGB0(poly, prm->color[0].r, prm->color[0].g, prm->color[0].b);
                setRGB1(poly, prm->color[1].r, prm->color[1].g, prm->color[1].b);
                setRGB2(poly, prm->color[2].r, prm->color[2].g, prm->color[2].b);

                poly->tpage = prm->tpage;
                poly->clut = prm->clut;
                setUV3(poly, prm->u0, prm->v0, prm->u1, prm->v1, prm->u2, prm->v2);

                addPrim(&ordering_table[screen.current_buffer][otz], poly);
                next_primitive += sizeof(POLY_GT3);

                break;
            }
            case TYPE_F4: {
                F4 *prm = (F4*) mesh->primitives[i].primitive;
                POLY_F4 *poly = (POLY_F4*) next_primitive;

                // Load vertices into GTE
                gte_ldv0(&mesh->vertices[prm->coords[0]]);
                gte_ldv1(&mesh->vertices[prm->coords[1]]);
                gte_ldv2(&mesh->vertices[prm->coords[2]]);

                // Transform
                gte_rtpt();

                { // Backface culling
                    int nclip = 0;

                    gte_nclip();
                    gte_stopz(&nclip);

                    if (nclip < 0) {
                        continue;
                    }
                }

                // Transform fourth vertex
                gte_stsxy0(&poly->x0);
                gte_ldv0(&mesh->vertices[prm->coords[3]]);
                gte_rtps();
                gte_stsxy3(&poly->x1, &poly->x2, &poly->x3);

                int otz = 0;
                gte_avsz4();
                gte_stotz(&otz);
                if (otz >= OT_LENGTH || otz <= 0) { // Depth test
                    continue;
                }

                setPolyF4(poly);
                setRGB0(poly, prm->color.r, prm->color.g, prm->color.b);

                addPrim(&ordering_table[screen.current_buffer][otz], poly);
                next_primitive += sizeof(POLY_F4);

                break;
            }
            case TYPE_FT4: {
                FT4 *prm = (FT4*) mesh->primitives[i].primitive;
                POLY_FT4 *poly = (POLY_FT4*) next_primitive;

                // Load vertices into GTE
                gte_ldv0(&mesh->vertices[prm->coords[0]]);
                gte_ldv1(&mesh->vertices[prm->coords[1]]);
                gte_ldv2(&mesh->vertices[prm->coords[2]]);

                // Transform
                gte_rtpt();

                { // Backface culling
                    int nclip = 0;

                    gte_nclip();
                    gte_stopz(&nclip);

                    if (nclip < 0) {
                        continue;
                    }
                }

                // Transform fourth vertex
                gte_stsxy0(&poly->x0);
                gte_ldv0(&mesh->vertices[prm->coords[3]]);
                gte_rtps();
                gte_stsxy3(&poly->x1, &poly->x2, &poly->x3);

                int otz = 0;
                gte_avsz4();
                gte_stotz(&otz);
                if (otz >= OT_LENGTH || otz <= 0) { // Depth test
                    continue;
                }

                setPolyFT4(poly);
                setRGB0(poly, prm->color.r, prm->color.g, prm->color.b);
                poly->tpage = prm->tpage;
                poly->clut = prm->clut;
                setUV4(poly, prm->u0, prm->v0, prm->u1, prm->v1, prm->u2, prm->v2, prm->u3, prm->v3);

                addPrim(&ordering_table[screen.current_buffer][otz], poly);
                next_primitive += sizeof(POLY_FT4);

                break;
            }
            case TYPE_G4: {
                G4 *prm = (G4*) mesh->primitives[i].primitive;
                POLY_G4 *poly = (POLY_G4*) next_primitive;

                // Load vertices into GTE
                gte_ldv0(&mesh->vertices[prm->coords[0]]);
                gte_ldv1(&mesh->vertices[prm->coords[1]]);
                gte_ldv2(&mesh->vertices[prm->coords[2]]);

                // Transform
                gte_rtpt();

                { // Backface culling
                    int nclip = 0;

                    gte_nclip();
                    gte_stopz(&nclip);

                    if (nclip < 0) {
                        continue;
                    }
                }

                // Transform fourth vertex
                gte_stsxy0(&poly->x0);
                gte_ldv0(&mesh->vertices[prm->coords[3]]);
                gte_rtps();
                gte_stsxy3(&poly->x1, &poly->x2, &poly->x3);

                int otz = 0;
                gte_avsz4();
                gte_stotz(&otz);
                if (otz >= OT_LENGTH || otz <= 0) { // Depth test
                    continue;
                }

                setPolyG4(poly);
                setRGB0(poly, prm->color[0].r, prm->color[0].g, prm->color[0].b);
                setRGB1(poly, prm->color[1].r, prm->color[1].g, prm->color[1].b);
                setRGB2(poly, prm->color[2].r, prm->color[2].g, prm->color[2].b);
                setRGB3(poly, prm->color[3].r, prm->color[3].g, prm->color[3].b);

                addPrim(&ordering_table[screen.current_buffer][otz], poly);
                next_primitive += sizeof(POLY_G4);

                break;
            }
            case TYPE_GT4: {
                GT4 *prm = (GT4*) mesh->primitives[i].primitive;
                POLY_GT4 *poly = (POLY_GT4*) next_primitive;

                // Load vertices into GTE
                gte_ldv0(&mesh->vertices[prm->coords[0]]);
                gte_ldv1(&mesh->vertices[prm->coords[1]]);
                gte_ldv2(&mesh->vertices[prm->coords[2]]);

                // Transform
                gte_rtpt();

                { // Backface culling
                    int nclip = 0;

                    gte_nclip();
                    gte_stopz(&nclip);

                    if (nclip < 0) {
                        continue;
                    }
                }

                // Transform fourth vertex
                gte_stsxy0(&poly->x0);
                gte_ldv0(&mesh->vertices[prm->coords[3]]);
                gte_rtps();
                gte_stsxy3(&poly->x1, &poly->x2, &poly->x3);

                int otz = 0;
                gte_avsz4();
                gte_stotz(&otz);
                if (otz >= OT_LENGTH || otz <= 0) { // Depth test
                    continue;
                }

                setPolyGT4(poly);
                setRGB0(poly, prm->color[0].r, prm->color[0].g, prm->color[0].b);
                setRGB1(poly, prm->color[1].r, prm->color[1].g, prm->color[1].b);
                setRGB2(poly, prm->color[2].r, prm->color[2].g, prm->color[2].b);
                setRGB3(poly, prm->color[3].r, prm->color[3].g, prm->color[3].b);
                poly->tpage = prm->tpage;
                poly->clut = prm->clut;
                setUV4(poly, prm->u0, prm->v0, prm->u1, prm->v1, prm->u2, prm->v2, prm->u3, prm->v3);

                addPrim(&ordering_table[screen.current_buffer][otz], poly);
                next_primitive += sizeof(POLY_GT4);

                break;
            }
        }
    }
}

Display_Mesh_Bundle display_create_meshes_from_prm(const char *prm_data, u_long texture_start, int mesh_count) {
    Display_Mesh_Bundle bundle = {0};

    u_long byte_counter = 0;
    for (int i = 0; i < 8; i++) {
        bundle.meshes[i] = display_create_mesh_from_prm(prm_data + byte_counter, texture_start, &byte_counter);
        bundle.actual_count++;
    }

    return bundle;
}

Display_Mesh display_create_mesh_from_prm(const char *prm_data, u_long texture_start, u_long* byte_counter_return) {
    Display_Mesh mesh = {0};

    u_long byte_counter = 0;

    for (int i = 0; i < 16; i++) {
        mesh.name[i] = get_char(prm_data, &byte_counter);
    }

    mesh.vertex_count = get_short_be(prm_data, &byte_counter);
    byte_counter += 6;

    mesh.normal_count = get_short_be(prm_data, &byte_counter);
    byte_counter += 6;

    mesh.primitive_count = get_short_be(prm_data, &byte_counter);
    byte_counter += 22;

    mesh.flags = get_short_be(prm_data, &byte_counter);
    byte_counter += 26;

    mesh.origin.vx = get_long_be(prm_data, &byte_counter);
    mesh.origin.vy = get_long_be(prm_data, &byte_counter);
    mesh.origin.vz = get_long_be(prm_data, &byte_counter);
    byte_counter += 48;

    mesh.vertices = malloc(mesh.vertex_count * sizeof(SVECTOR));
    for (int i = 0; i < mesh.vertex_count; i++) {
        mesh.vertices[i].vx = get_short_be(prm_data, &byte_counter);
        mesh.vertices[i].vy = get_short_be(prm_data, &byte_counter);
        mesh.vertices[i].vz = get_short_be(prm_data, &byte_counter);
        byte_counter += 2;
    }

    mesh.normals = malloc(mesh.normal_count * sizeof(SVECTOR));
    for (int i = 0; i < mesh.normal_count; i++) {
        mesh.normals[i].vx = get_short_be(prm_data, &byte_counter);
        mesh.normals[i].vy = get_short_be(prm_data, &byte_counter);
        mesh.normals[i].vz = get_short_be(prm_data, &byte_counter);
        byte_counter += 2;
    }

    mesh.primitives = malloc(mesh.primitive_count * sizeof(Primitive_Node));
    for (int i = 0; i < mesh.primitive_count; i++) {
        mesh.primitives[i].type = get_short_be(prm_data, &byte_counter);
        mesh.primitives[i].flag = get_short_be(prm_data, &byte_counter);

        switch (mesh.primitives[i].type) {
            case TYPE_F3: {
                F3 *prm;
                mesh.primitives[i].primitive = (Prm*) malloc(sizeof(F3));
                prm            = (F3*) mesh.primitives[i].primitive;
                prm->type      = TYPE_F3;
                prm->coords[0] = get_short_be(prm_data, &byte_counter);
                prm->coords[1] = get_short_be(prm_data, &byte_counter);
                prm->coords[2] = get_short_be(prm_data, &byte_counter);
                prm->pad1      = get_short_be(prm_data, &byte_counter);
                prm->color.r   = get_char(prm_data, &byte_counter);
                prm->color.g   = get_char(prm_data, &byte_counter);
                prm->color.b   = get_char(prm_data, &byte_counter);
                prm->color.cd  = get_char(prm_data, &byte_counter);
                break;
            }
            case TYPE_FT3: {
                FT3 *prm;
                mesh.primitives[i].primitive = (Prm*) malloc(sizeof(FT3));
                prm            = (FT3*) mesh.primitives[i].primitive;
                prm->type      = TYPE_FT3;
                prm->coords[0] = get_short_be(prm_data, &byte_counter);
                prm->coords[1] = get_short_be(prm_data, &byte_counter);
                prm->coords[2] = get_short_be(prm_data, &byte_counter);
                prm->texture   = get_short_be(prm_data, &byte_counter);
                prm->clut      = get_short_be(prm_data, &byte_counter);
                prm->tpage     = get_short_be(prm_data, &byte_counter);
                prm->u0        = get_char(prm_data, &byte_counter);
                prm->v0        = get_char(prm_data, &byte_counter);
                prm->u1        = get_char(prm_data, &byte_counter);
                prm->v1        = get_char(prm_data, &byte_counter);
                prm->u2        = get_char(prm_data, &byte_counter);
                prm->v2        = get_char(prm_data, &byte_counter);
                prm->pad1      = get_short_be(prm_data, &byte_counter);
                prm->color     = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };

                prm->texture += texture_start;

                Texture *texture = get_texture(prm->texture);
                u_short u_offset = texture->u0;
                u_short v_offset = texture->v0;

                prm->u0 += u_offset;
                prm->v0 += v_offset;
                prm->u1 += u_offset;
                prm->v1 += v_offset;
                prm->u2 += u_offset;
                prm->v2 += v_offset;
                prm->tpage = texture->tpage;
                prm->clut = texture->clut;

                break;
            }
          case TYPE_F4: {
            F4 *prm;
            mesh.primitives[i].primitive = (Prm*) malloc(sizeof(F4));
            prm            = (F4*) mesh.primitives[i].primitive;
            prm->type      = TYPE_F4;
            prm->coords[0] = get_short_be(prm_data, &byte_counter);
            prm->coords[1] = get_short_be(prm_data, &byte_counter);
            prm->coords[2] = get_short_be(prm_data, &byte_counter);
            prm->coords[3] = get_short_be(prm_data, &byte_counter);
            prm->color     = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };
            break;
          }
          case TYPE_FT4: {
            FT4 *prm;
            mesh.primitives[i].primitive = (Prm*) malloc(sizeof(FT4));
            prm            = (FT4*) mesh.primitives[i].primitive;
            prm->type      = TYPE_FT4;
            prm->coords[0] = get_short_be(prm_data, &byte_counter);
            prm->coords[1] = get_short_be(prm_data, &byte_counter);
            prm->coords[2] = get_short_be(prm_data, &byte_counter);
            prm->coords[3] = get_short_be(prm_data, &byte_counter);
            prm->texture   = get_short_be(prm_data, &byte_counter);
            prm->clut      = get_short_be(prm_data, &byte_counter);
            prm->tpage     = get_short_be(prm_data, &byte_counter);
            prm->u0        = get_char(prm_data, &byte_counter);
            prm->v0        = get_char(prm_data, &byte_counter);
            prm->u1        = get_char(prm_data, &byte_counter);
            prm->v1        = get_char(prm_data, &byte_counter);
            prm->u2        = get_char(prm_data, &byte_counter);
            prm->v2        = get_char(prm_data, &byte_counter);
            prm->u3        = get_char(prm_data, &byte_counter);
            prm->v3        = get_char(prm_data, &byte_counter);
            prm->pad1      = get_short_be(prm_data, &byte_counter);
            prm->color     = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };

            prm->texture += texture_start;

            Texture *texture = get_texture(prm->texture);
            u_short u_offset = texture->u0;
            u_short v_offset = texture->v0;

            prm->u0 += u_offset;
            prm->v0 += v_offset;
            prm->u1 += u_offset;
            prm->v1 += v_offset;
            prm->u2 += u_offset;
            prm->v2 += v_offset;
            prm->u3 += u_offset;
            prm->v3 += v_offset;
            prm->tpage = texture->tpage;
            prm->clut = texture->clut;

            break;
          }
          case TYPE_G3: {
            G3 *prm;
            mesh.primitives[i].primitive = (Prm*) malloc(sizeof(G3));
            prm            = (G3*) mesh.primitives[i].primitive;
            prm->type      = TYPE_G3;
            prm->coords[0] = get_short_be(prm_data, &byte_counter);
            prm->coords[1] = get_short_be(prm_data, &byte_counter);
            prm->coords[2] = get_short_be(prm_data, &byte_counter);
            prm->pad1      = get_short_be(prm_data, &byte_counter);
            prm->color[0]  = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };
            prm->color[1]  = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };
            prm->color[2]  = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };
            break;
          }
          case TYPE_GT3: {
            GT3 *prm;
            mesh.primitives[i].primitive = (Prm*) malloc(sizeof(GT3));
            prm            = (GT3*) mesh.primitives[i].primitive;
            prm->type      = TYPE_GT3;
            prm->coords[0] = get_short_be(prm_data, &byte_counter);
            prm->coords[1] = get_short_be(prm_data, &byte_counter);
            prm->coords[2] = get_short_be(prm_data, &byte_counter);
            prm->texture   = get_short_be(prm_data, &byte_counter);
            prm->clut      = get_short_be(prm_data, &byte_counter);
            prm->tpage     = get_short_be(prm_data, &byte_counter);
            prm->u0        = get_char(prm_data, &byte_counter);
            prm->v0        = get_char(prm_data, &byte_counter);
            prm->u1        = get_char(prm_data, &byte_counter);
            prm->v1        = get_char(prm_data, &byte_counter);
            prm->u2        = get_char(prm_data, &byte_counter);
            prm->v2        = get_char(prm_data, &byte_counter);
            prm->pad1      = get_short_be(prm_data, &byte_counter);
            prm->color[0]  = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };
            prm->color[1]  = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };
            prm->color[2]  = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };

            prm->texture += texture_start;

            Texture *texture = get_texture(prm->texture);
            u_short u_offset = texture->u0;
            u_short v_offset = texture->v0;

            prm->u0 += u_offset;
            prm->v0 += v_offset;
            prm->u1 += u_offset;
            prm->v1 += v_offset;
            prm->u2 += u_offset;
            prm->v2 += v_offset;
            prm->tpage = texture->tpage;
            prm->clut = texture->clut;

            break;
          }
          case TYPE_G4: {
            G4 *prm;
            mesh.primitives[i].primitive = (Prm*) malloc(sizeof(G4));
            prm            = (G4*) mesh.primitives[i].primitive;
            prm->type      = TYPE_G4;
            prm->coords[0] = get_short_be(prm_data, &byte_counter);
            prm->coords[1] = get_short_be(prm_data, &byte_counter);
            prm->coords[2] = get_short_be(prm_data, &byte_counter);
            prm->coords[3] = get_short_be(prm_data, &byte_counter);
            prm->color[0]  = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };
            prm->color[1]  = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };
            prm->color[2]  = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };
            prm->color[3]  = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };
            break;
          }
          case TYPE_GT4: {
            GT4 *prm;
            mesh.primitives[i].primitive = (Prm*) malloc(sizeof(GT4));
            prm            = (GT4*) mesh.primitives[i].primitive;
            prm->type      = TYPE_GT4;
            prm->coords[0] = get_short_be(prm_data, &byte_counter);
            prm->coords[1] = get_short_be(prm_data, &byte_counter);
            prm->coords[2] = get_short_be(prm_data, &byte_counter);
            prm->coords[3] = get_short_be(prm_data, &byte_counter);
            prm->texture   = get_short_be(prm_data, &byte_counter);
            prm->clut      = get_short_be(prm_data, &byte_counter);
            prm->tpage     = get_short_be(prm_data, &byte_counter);
            prm->u0        = get_char(prm_data, &byte_counter);
            prm->v0        = get_char(prm_data, &byte_counter);
            prm->u1        = get_char(prm_data, &byte_counter);
            prm->v1        = get_char(prm_data, &byte_counter);
            prm->u2        = get_char(prm_data, &byte_counter);
            prm->v2        = get_char(prm_data, &byte_counter);
            prm->u3        = get_char(prm_data, &byte_counter);
            prm->v3        = get_char(prm_data, &byte_counter);
            prm->pad1      = get_short_be(prm_data, &byte_counter);
            prm->color[0]  = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };
            prm->color[1]  = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };
            prm->color[2]  = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };
            prm->color[3]  = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };

            prm->texture += texture_start;

            Texture *texture = get_texture(prm->texture);
            u_short u_offset = texture->u0;
            u_short v_offset = texture->v0;

            prm->u0 += u_offset;
            prm->v0 += v_offset;
            prm->u1 += u_offset;
            prm->v1 += v_offset;
            prm->u2 += u_offset;
            prm->v2 += v_offset;
            prm->u3 += u_offset;
            prm->v3 += v_offset;
            prm->tpage = texture->tpage;
            prm->clut = texture->clut;

            break;
          }
          case TYPE_TSPR:
          case TYPE_BSPR: {
            SPR *prm;
            mesh.primitives[i].primitive = (Prm*) malloc(sizeof(SPR));
            prm            = (SPR*) mesh.primitives[i].primitive;
            prm->type      = TYPE_TSPR;
            prm->coord     = get_short_be(prm_data, &byte_counter);
            prm->width     = get_short_be(prm_data, &byte_counter);
            prm->height    = get_short_be(prm_data, &byte_counter);
            prm->texture   = get_short_be(prm_data, &byte_counter);
            prm->color     = (CVECTOR) { get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter), get_char(prm_data, &byte_counter) };
            break;
          }
          case TYPE_SPLINE: {
            byte_counter += 52;
            break;
          }
          case TYPE_POINT_LIGHT: {
            byte_counter += 24;
            break;
          }
          case TYPE_SPOT_LIGHT: {
            byte_counter += 36;
            break;
          }
          case TYPE_INFINITE_LIGHT: {
            byte_counter += 12;
            break;
          }
          case TYPE_LSF3: {
            byte_counter += 12;
            break;
          }
          case TYPE_LSFT3: {
            byte_counter += 24;
            break;
          }
          case TYPE_LSF4: {
            byte_counter += 16;
            break;
          }
          case TYPE_LSFT4: {
            byte_counter += 28;
            break;
          }
          case TYPE_LSG3: {
            byte_counter += 24;
            break;
          }
          case TYPE_LSGT3: {
            byte_counter += 36;
            break;
          }
          case TYPE_LSG4: {
            byte_counter += 32;
            break;
          }
          case TYPE_LSGT4: {
            byte_counter += 42;
            break;
          }
        }
    }

    if (byte_counter_return != NULL) {
        *byte_counter_return += byte_counter;
    }

    return mesh;
}

void display_destroy_mesh(Display_Mesh *mesh) {
    free(mesh->vertices);
    free(mesh->normals);
}
