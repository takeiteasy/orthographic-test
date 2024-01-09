//
//  map.c
//  tbce
//
//  Created by George Watson on 24/12/2023.
//

#include "map.h"

static const int faces[6][4] = {
    [FLOOR_FACE]   = { 4, 0, 1, 5 },
    [NORTH_FACE]   = { 3, 0, 1, 2 },
    [EAST_FACE]    = { 6, 5, 4, 7 },
    [SOUTH_FACE]   = { 7, 4, 0, 3 },
    [WEST_FACE]    = { 2, 1, 5, 6 },
    [CEILING_FACE] = { 7, 3, 2, 6 }
};

static void CreateCube(Tile *tile, int vw, int vh, Cube *out, Camera *camera) {
    Cube unit = (Cube) {
        .points = {
            [0] = Vec3New(0.f, 0.f, 0.f),
            [1] = Vec3New(1.f, 0.f, 0.f),
            [2] = Vec3New(1.f, -1.f, 0.f),
            [3] = Vec3New(0.f, -1.f, 0.f),
            [4] = Vec3New(0.f, 0.f, 1.f),
            [5] = Vec3New(1.f, 0.f, 1.f),
            [6] = Vec3New(1.f, -1.f, 1.f),
            [7] = Vec3New(0.f, -1.f, 1.f)
        }
    };
    
    for (int i = 0; i < 8; i++)
        unit.points[i] += Vec3New(tile->x - camera->position.x,
                                  0.f,
                                  tile->y - camera->position.y);
    for (int i = 0; i < 8; i++)
        unit.points[i] *= camera->zoom;
    
    Cube rotation;
    float sa = sinf(camera->angle);
    float ca = cosf(camera->angle);
    for (int i = 0; i < 8; i++)
        rotation.points[i] = Vec3New(unit.points[i].x * ca + unit.points[i].z * sa,
                                     unit.points[i].y,
                                     unit.points[i].x * -sa + unit.points[i].z * ca);
    
    Cube world;
    float sp = sinf(camera->pitch);
    float cp = cosf(camera->pitch);
    for (int i = 0; i < 8; i++)
        world.points[i] = Vec3New(rotation.points[i].x,
                                  rotation.points[i].y * cp - rotation.points[i].z * sp,
                                  rotation.points[i].y * sp + rotation.points[i].z * cp);
    
    for (int i = 0; i < 8; i++)
        out->points[i] = Vec3New(world.points[i].x + (float)vw * .5f,
                                 world.points[i].y + (float)vh * .5f,
                                 world.points[i].z);
}

static Quad MakeQuad(Cube *cube, int a, int b, int c, int d) {
    return (Quad) {
        .points = {
            [0] = cube->points[a],
            [1] = cube->points[b],
            [2] = cube->points[c],
            [3] = cube->points[d]
        }
    };
}

static Face MakeFace(Cube *cube, Tile *tile, TileFace _face, int a, int b, int c, int d) {
    return (Face) {
        .quad = MakeQuad(cube, a, b, c, d),
        .tile = tile,
        .face = _face
    };
}

#define MAKE_FACE(I)                                                                                                  \
do {                                                                                                                  \
    if (visible[(I)] == 1) {                                                                                          \
        Face face = MakeFace(&cube, tile, (TileFace)(I), faces[(I)][0], faces[(I)][1], faces[(I)][2], faces[(I)][3]); \
        memcpy(&out[*n], &face, sizeof(Face));                                                                        \
        (*n)++;                                                                                                       \
    }                                                                                                                 \
} while(0)

static void GetCubeFaces(Tile *tile, int vw, int vh, Camera *camera, int visible[6], Face *out, int *n) {
    Cube cube;
    CreateCube(tile, vw, vh, &cube, camera);
    if (tile->solid) {
        for (int i = 1; i < 6; i++)
            MAKE_FACE(i);
    } else
        MAKE_FACE(0);
}

static int SortFaces(const void *faceA, const void *faceB) {
    Face *fa = (Face*)faceA;
    Face *fb = (Face*)faceB;
    return (fa->quad.points[0].z + fa->quad.points[1].z + fa->quad.points[2].z + fa->quad.points[3].z) * .25f -
           (fb->quad.points[0].z + fb->quad.points[1].z + fb->quad.points[2].z + fb->quad.points[3].z) * .25f;
}

static int CheckNormal(Cube *cube, int a, int b, int c) {
    Vec2f va = (Vec2f){ cube->points[a].x, cube->points[a].y };
    Vec2f vb = (Vec2f){ cube->points[b].x, cube->points[b].y };
    Vec2f vc = (Vec2f){ cube->points[c].x, cube->points[c].y };
    return Vec2Cross(vb - va, vc - va) > 0;
}

static Tile DefaultTile(int x, int y, int solid) {
    return (Tile) {
        .x = x,
        .y = y,
        .solid = solid,
        .faces = {
            [FLOOR_FACE]   = (Vec2i){ 0,  0 },
            [NORTH_FACE]   = (Vec2i){ 9,  0 },
            [EAST_FACE]    = (Vec2i){ 9,  0 },
            [SOUTH_FACE]   = (Vec2i){ 9,  0 },
            [WEST_FACE]    = (Vec2i){ 9,  0 },
            [CEILING_FACE] = (Vec2i){ 11, 1 }
        }
    };
}

void InitMap(Map *map, Texture *spritesheet, int w, int h) {
    memcpy(&map->spritesheet, spritesheet, sizeof(Texture));
    map->tiles = malloc(sizeof(Tile) * w * h);
    for (int x = 0; x < w; x++)
        for (int y = 0; y < h; y++)
            map->tiles[y * w + x] = DefaultTile(x, y, 1);
}

void DestroyMap(Map *map) {
    if (map && map->tiles)
        free(map->tiles);
}

void RenderMap(Map *map, int vw, int vh, Camera *camera, Vec2i cursor) {
    int visible[6];
    memset(visible, 0, sizeof(int) * 6);
    Cube cull;
    CreateCube(&map->tiles[0], vw, vh, &cull, camera);
    for (int i = 0; i < 6; i++)
        visible[i] = CheckNormal(&cull, faces[i][0], faces[i][1], faces[i][2]);
    int inc = 0;
    int count = 0;
    for (int i = 1; i < 6; i++)
        inc += visible[i];
    for (int x = 0; x < 64; x++)
        for (int y = 0; y < 64; y++)
            count += map->tiles[y * 64 + x].solid ? inc : 1;
    
    Face *faces = malloc(sizeof(Face) * count);
    memset(faces, 0, sizeof(Face) * count);
    int n = 0;
    for (int x = 0; x < 64; x++)
        for (int y = 0; y < 64; y++)
            GetCubeFaces(&map->tiles[y * 64 + x], vw, vh, camera, visible, faces, &n);
    qsort(faces, count, sizeof(Face), SortFaces);
    
    for (int i = 0; i < count; i++) {
        Face *currentFace = &faces[i];
        Vec2f pos[4] = {
            { currentFace->quad.points[0].x, currentFace->quad.points[0].y },
            { currentFace->quad.points[1].x, currentFace->quad.points[1].y },
            { currentFace->quad.points[2].x, currentFace->quad.points[2].y },
            { currentFace->quad.points[3].x, currentFace->quad.points[3].y }
        };
        
        Vec4f w = Vec4New(1.f, 1.f, 1.f, 1.f);
        Vec2f center = Vec2Zero();
        float rd = ((pos[2].x - pos[0].x) * (pos[3].y - pos[1].y) - (pos[3].x - pos[1].x) * (pos[2].y - pos[0].y));
        if (!rd)
            continue;
        
        Vec2f scale = Vec2New(1.f / (float)map->spritesheet.width,
                              1.f / (float)map->spritesheet.height);
        Vec2i offset = currentFace->tile->faces[currentFace->face] * 32;
        Vec2f offsetf = Vec2New(offset.x, offset.y);
        Vec2f uvtl = offsetf * scale;
        Vec2f uvbr = uvtl + (Vec2New(32.f, 32.f) * scale);
        Vec2f uvs[4] = {
            { uvtl.x, uvtl.y },
            { uvtl.x, uvbr.y },
            { uvbr.x, uvbr.y },
            { uvbr.x, uvtl.y }
        };
        Vec2f vInvScreenSize = Vec2New(1.f / (float)vw, 1.f / (float)vh);
        
        rd = 1.0f / rd;
        float rn = ((pos[3].x - pos[1].x) * (pos[0].y - pos[1].y) - (pos[3].y - pos[1].y) * (pos[0].x - pos[1].x)) * rd;
        float sn = ((pos[2].x - pos[0].x) * (pos[0].y - pos[1].y) - (pos[2].y - pos[0].y) * (pos[0].x - pos[1].x)) * rd;
        if (!(rn < 0.f || rn > 1.f || sn < 0.f || sn > 1.f))
            center = pos[0] + rn * (pos[2] - pos[0]);
        
        float d[4];
        for (int j = 0; j < 4; j++)
            d[j] = Vec2Length(pos[j] - center);
        
        for (int j = 0; j < 4; j++) {
            float q = d[j] == 0.f ? 1.f : (d[j] + d[(j + 2) & 3]) / d[(j + 2) & 3];
            uvs[j] *= q;
            w[j] *= q;
            pos[j] = Vec2New((pos[j].x * vInvScreenSize.x) * 2.f - 1.f,
                             ((pos[j].y * vInvScreenSize.y) * 2.f - 1.f) * -1.f);
        }
        
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glBindTexture(GL_TEXTURE_2D, map->spritesheet.id);
        glBegin(GL_TRIANGLE_FAN);
        for (uint32_t n = 0; n < 4; n++) {
            glColor4f(1.f, 1.f, 1.f, 1.f);
            glTexCoord4f(uvs[n].x, uvs[n].y, 0.f, w[n]);
            glVertex2f(pos[n].x, pos[n].y);
            
        }
        glEnd();
        
        if (currentFace->tile->x == cursor.x && currentFace->tile->y == cursor.y) {
            glLineWidth(4.f);
            glDisable(GL_TEXTURE_2D);
            glBegin(GL_LINES);
            for (uint32_t n = 0; n < 4; n++) {
                glColor4f(1.f, 0.f, 0.f, 1.f);
                glVertex2f(pos[n].x, pos[n].y);
                int j = n + 1;
                if (j == 4)
                    j = 0;
                glVertex2f(pos[j].x, pos[j].y);
            }
            glEnd();
        }
    }
    free(faces);
}
