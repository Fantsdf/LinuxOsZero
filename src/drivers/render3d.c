/*
 * LinuxOSZero - Software 3D Rasterizer Driver Implementation
 */

#include "render3d.h"
#include "fbdev.h"

#include <math.h>

void r3d_cube(mesh3d_t *m) {
    m->num_verts = 8;
    m->num_edges = 12;
    float v[8][3] = {
        {-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},
        {-1,-1, 1},{1,-1, 1},{1,1, 1},{-1,1, 1}
    };
    for (int i = 0; i < 8; i++) {
        m->verts[i].x = v[i][0];
        m->verts[i].y = v[i][1];
        m->verts[i].z = v[i][2];
    }
    uint8_t e[12][2] = {
        {0,1},{1,2},{2,3},{3,0},
        {4,5},{5,6},{6,7},{7,4},
        {0,4},{1,5},{2,6},{3,7}
    };
    for (int i = 0; i < 12; i++) {
        m->edges[i][0] = e[i][0];
        m->edges[i][1] = e[i][1];
    }
}

void r3d_render_mesh(const mesh3d_t *m, float rot_x, float rot_y,
                     int cx, int cy, int scale, uint32_t color) {
    float ca = (float)cos(rot_y), sa = (float)sin(rot_y);
    float cb = (float)cos(rot_x), sb = (float)sin(rot_x);

    /* Project each vertex through rotation then perspective. */
    int sx[16], sy[16];
    for (int i = 0; i < m->num_verts; i++) {
        float x = m->verts[i].x;
        float y = m->verts[i].y;
        float z = m->verts[i].z;

        /* Rotate around Y */
        float x1 = x * ca - z * sa;
        float z1 = x * sa + z * ca;
        /* Rotate around X */
        float y2 = y * cb - z1 * sb;
        float z2 = y * sb + z1 * cb;

        /* Perspective projection with camera at z = -4 */
        float cam = 4.0f;
        float p = cam / (cam + z2);
        sx[i] = cx + (int)(x1 * scale * p);
        sy[i] = cy + (int)(y2 * scale * p);
    }

    /* Draw edges */
    for (int e = 0; e < m->num_edges; e++) {
        int a = m->edges[e][0];
        int b = m->edges[e][1];
        fbdev_draw_line(sx[a], sy[a], sx[b], sy[b], color);
    }
}
