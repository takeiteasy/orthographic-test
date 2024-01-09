//
//  map.h
//  tbce
//
//  Created by George Watson on 24/12/2023.
//

#ifndef map_h
#define map_h
#include "ez/ezmath.h"
#include "common.h"

typedef struct {
    int x, y, solid;
    Vec2i faces[6];
} Tile;

typedef struct {
    Vec3f points[4];
} Quad;

typedef struct {
    Vec3f points[8];
} Cube;

typedef struct {
    Vec3f position;
    float angle;
    float pitch;
    float zoom;
} Camera;

#define MAX_ZOOM 256.f

typedef enum {
    FLOOR_FACE   = 0,
    NORTH_FACE   = 1,
    EAST_FACE    = 2,
    SOUTH_FACE   = 3,
    WEST_FACE    = 4,
    CEILING_FACE = 5
} TileFace;

typedef struct Face {
    Quad quad;
    Tile *tile;
    TileFace face;
} Face;

typedef struct {
    Tile *tiles;
    Texture spritesheet;
} Map;

void InitMap(Map *map, Texture *spritesheet, int w, int h);
void DestroyMap(Map *map);
void RenderMap(Map *map,  int vw, int vh, Camera *camera, Vec2i cursor);

#endif /* map_h */
