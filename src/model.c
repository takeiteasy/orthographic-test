//
//  model.c
//  tbce
//
//  Created by George Watson on 10/01/2024.
//

#include "model.h"
#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

void LoadModelObj(const char *path, Model *out) {
    Mesh *mesh = out->meshes = malloc(sizeof(Mesh));
    out->sizeOfMeshes = 1;
    mesh->texture = NULL;
    out->position = Vec3Zero();
    out->scale = Vec3New(1.f, 1.f, 1.f);
    out->rotation = Vec3Zero();
    fastObjMesh* obj = fast_obj_read(path);
    assert(obj);
    
    mesh->sizeOfVertices = obj->face_count * 3;
    mesh->vertices = malloc(obj->face_count * 3 * 8 * sizeof(float));
    for (int i = 0; i < mesh->sizeOfVertices; i++) {
        fastObjIndex vertex = obj->indices[i];
        unsigned int pos = i * 8;
        unsigned int v_pos = vertex.p * 3;
        unsigned int n_pos = vertex.n * 3;
        unsigned int t_pos = vertex.t * 2;
        memcpy(mesh->vertices + pos, obj->positions + v_pos, 3 * sizeof(float));
        memcpy(mesh->vertices + pos + 3, obj->normals + n_pos, 3 * sizeof(float));
        memcpy(mesh->vertices + pos + 6, obj->texcoords + t_pos, 2 * sizeof(float));
    }
}

static void RenderMesh(Mesh *mesh, int tx, int ty, Camera *camera) {
    if (mesh->texture) {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, mesh->texture->id);
    } else
        glDisable(GL_TEXTURE_2D);
    
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    Vec3f scale = Vec3New(.002f, .002f, .002f);
    glScalef(scale.x * camera->zoom, scale.y * camera->zoom, scale.z * camera->zoom);
    
    Vec2f rotation = Vec2New(45.f, 0.f);
    Vec2f adjustment = Vec2New(REMAP(-camera->pitch, PI + HALF_PI, TWO_PI, 0.f, 360.f / 8.f),
                               TO_DEGREES(camera->angle)) - rotation;
    glRotatef(adjustment.x, 1.0, 0.0, 0.0); // Rotate around the x-axis
    glRotatef(adjustment.y, 0.0, 1.0, 0.0); // Rotate around the y-axis
    
    Vec3f translate = camera->position + Vec3New(.75f, 1.f, .75f) * Vec3New(tx + 1, 1.f, ty + 1);
    glTranslatef(translate.x, translate.y, translate.z);
 
    glBegin(GL_TRIANGLES);
    size_t sz = mesh->sizeOfVertices;
    Vec3f positions[sz], normals[sz];
    Vec2f texcoords[sz];
    for (int i = 0; i < mesh->sizeOfVertices; i += 3) {
        for (int j = 0; j < 3; ++j) {
            float *vertex = mesh->vertices + ((i + j) * 8);
            memcpy(positions + i + j, vertex, sizeof(float) * 3);
            memcpy(normals + i + j, vertex + 3, sizeof(float) * 3);
            memcpy(texcoords + i + j, vertex + 6, sizeof(float) * 2);
            Vec3f position = Vec3New(vertex[0], vertex[1], vertex[2]);
            Vec3f normal   = Vec3New(vertex[3], vertex[4], vertex[5]);
            Vec2f texcoord = Vec2New(vertex[6], vertex[7]);

            if (mesh->texture)
                glTexCoord2f(texcoord.x, texcoord.y);
            else
                glColor4f(1.f, 0.f, 1.f, 1.f);
            glNormal3f(normal.x, normal.y, normal.z);
            glVertex3f(position.x, position.y, position.z);
        }
    }
    glEnd();
    glPopMatrix();
    glDisable(GL_DEPTH_TEST);
    
    if (mesh->texture) {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }
}

void RenderModel(Model *model, int tx, int ty, Camera *camera) {
    for (int i = 0; i < model->sizeOfMeshes; i++)
        RenderMesh(&model->meshes[0], tx, ty, camera);
}
