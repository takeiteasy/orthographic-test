//
//  model.h
//  tbce
//
//  Created by George Watson on 10/01/2024.
//

#ifndef model_h
#define model_h
#include "common.h"

typedef struct {
    float *vertices;
    int sizeOfVertices;
    Texture *texture;
} Mesh;

typedef struct {
    Mesh *meshes;
    int sizeOfMeshes;
    Vec3f position;
    Vec3f scale;
    Vec3f rotation;
} Model;

void LoadModelObj(const char *path, Model *out);
void RenderModel(Model *model, int tx, int ty, Camera *camera);

#endif /* model_h */
