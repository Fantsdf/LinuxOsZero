/*
 * LinuxOSZero - Software 3D Rasterizer Driver (GPU-less 3D)
 *
 * Provides a lightweight 3D projection/render engine that draws 3D wireframe
 * meshes (e.g. a rotating cube) onto the 2D framebuffer. This gives the OS
 * basic 3D acceleration even without a physical GPU (VirtualBox std VGA /
 * QEMU std VGA / software framebuffer).
 */

#ifndef RENDER3D_H
#define RENDER3D_H

#include <stdint.h>

typedef struct {
    float x, y, z;
} vec3_t;

typedef struct {
    int num_verts;
    int num_edges;
    vec3_t  verts[16];
    uint8_t edges[16][2];   /* vertex index pairs */
} mesh3d_t;

/* Initialize a built-in cube mesh (edge length 2, centered at origin). */
void r3d_cube(mesh3d_t *m);

/* Render a wireframe mesh rotated by rot_x/rot_y (radians), projected with
 * perspective, into the framebuffer. Center at (cx, cy), scale s. */
void r3d_render_mesh(const mesh3d_t *m, float rot_x, float rot_y,
                     int cx, int cy, int scale, uint32_t color);

#endif /* RENDER3D_H */
