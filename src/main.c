/* main.c -- https://github.com/takeiteasy/tbce
 
 “Commons Clause” License Condition v1.0

 The Software is provided to you by the Licensor under the License, as defined
 below, subject to the following condition.

 Without limiting other conditions in the License, the grant of rights under the
 License will not include, and the License does not grant to you, the right to
 Sell the Software.

 For purposes of the foregoing, “Sell” means practicing any or all of the rights
 granted to you under the License to provide to third parties, for a fee or
 other consideration (including without limitation fees for hosting or
 consulting/ support services related to the Software), a product or service
 whose value derives, entirely or substantially, from the functionality of the
 Software. Any license notice or attribution required by the License must also
 include this Commons Clause License Condition notice.

 Software: "tbce"

 License: GNU General Public License v3 (GPL-3) */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "common.h"
#include "map.h"
#include "debug.h"
#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

typedef struct {
    float *vertices;
    int sizeOfVertices;
} Mesh;

static struct {
    GLFWwindow *mainWindow;
    Map map;
    Camera camera;
    Camera cameraTarget;
    Texture tileTexture;
    Vec2i cursor;
    Vec2f mousePosition;
    Vec2f lastMousePosition;
    int m1Down;
    int ctrlDown;
    double lastTime;
    double deltaTime;
    Vec2f scrollDelta;
    
    Mesh testMesh;
    Texture testTexture;
} state;

static void LoadMesh(Mesh *mesh, const char *path) {
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

void MulMatrix(Matrix mat) {
   float buffer[16];
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
           buffer[col * 4 + row] = mat[row][col];
   glMultMatrixf(buffer);
}

static void RenderMesh(Mesh *mesh, Texture *texture) {
    if (texture) {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindTexture(GL_TEXTURE_2D, texture->id);
    } else
        glDisable(GL_TEXTURE_2D);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    Vec3f scale = Vec3New(.002f, .002f, .002f);
    glScalef(scale.x * state.camera.zoom, scale.y * state.camera.zoom, scale.z * state.camera.zoom);
    
    Vec2f rotation = Vec2New(45.f, 0.f);
    Vec2f adjustment = Vec2New(REMAP(-state.camera.pitch, PI + HALF_PI, TWO_PI, 0.f, 360.f / 8.f),
                               TO_DEGREES(state.camera.angle)) - rotation;
    glRotatef(adjustment.x, 1.0, 0.0, 0.0); // Rotate around the x-axis
    glRotatef(adjustment.y, 0.0, 1.0, 0.0); // Rotate around the y-axis
    
    Vec3f translate = state.camera.position + Vec3New(.75f, 1.f, .75f);
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

//            glTexCoord2f(texcoord.x, texcoord.y);
            glColor4f(1.f, 0.f, 0.f, 1.f);
//            glNormal3f(normal.x, normal.y, normal.z);
            glVertex3f(position.x, position.y, position.z);
        }
    }
    glEnd();
    glPopMatrix();
    
    if (texture) {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
       glfwSetWindowShouldClose(window, GLFW_TRUE);
    
    if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
        state.ctrlDown = action == GLFW_PRESS;
    
    switch (key) {
        case GLFW_KEY_KP_0: // Top down
            state.cameraTarget.angle = PI;
            state.cameraTarget.pitch = PI + HALF_PI;
            break;
        case GLFW_KEY_KP_2:
            state.cameraTarget.angle = PI * 2.f;
            break;
        case GLFW_KEY_KP_1:
            state.cameraTarget.angle = PI * .25f;
            break;
        case GLFW_KEY_KP_4:
            state.cameraTarget.angle = PI * .5f;
            break;
        case GLFW_KEY_KP_7:
            state.cameraTarget.angle = PI * .75f;
            break;
        case GLFW_KEY_KP_8:
            state.cameraTarget.angle = PI * 1.f;
            break;
        case GLFW_KEY_KP_9:
            state.cameraTarget.angle = PI * 1.25f;
            break;
        case GLFW_KEY_KP_6:
            state.cameraTarget.angle = PI * 1.5f;
            break;
        case GLFW_KEY_KP_3:
            state.cameraTarget.angle = PI * 1.75f;
            break;
        case GLFW_KEY_KP_SUBTRACT:
            state.cameraTarget.zoom = CLAMP(state.cameraTarget.zoom - 10.f, .1, MAX_ZOOM);
            break;
        case GLFW_KEY_KP_ADD:
            state.cameraTarget.zoom = CLAMP(state.cameraTarget.zoom + 10.f, .1, MAX_ZOOM);
            break;
        case GLFW_KEY_KP_DIVIDE:
            state.cameraTarget.pitch = CLAMP(state.cameraTarget.pitch - .5f / PI, PI + HALF_PI, TWO_PI);
            break;
        case GLFW_KEY_KP_MULTIPLY:
            state.cameraTarget.pitch = CLAMP(state.cameraTarget.pitch + .5f / PI, PI + HALF_PI, TWO_PI);
            break;
    }
}

static void ButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_1) {
        state.m1Down = action == GLFW_PRESS;
        state.ctrlDown = mods & GLFW_MOD_CONTROL;
    }
}

static void MouseCallback(GLFWwindow *window, double x, double y) {
    state.lastMousePosition = state.mousePosition;
    state.mousePosition = Vec2New(x, y);
}

static void ScrollCallback(GLFWwindow *window, double xoff, double yoff) {
    state.scrollDelta = Vec2New(xoff, yoff);
}

int main(int argc, const char* argv[]) {
    if (!glfwInit())
        return 0;
    if (!(state.mainWindow = glfwCreateWindow(640, 480, "tbce", NULL, NULL)))
        return 0;
    glfwMakeContextCurrent(state.mainWindow);
    if (InitOpenGL())
        return 0;
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glfwSetKeyCallback(state.mainWindow, KeyCallback);
    glfwSetMouseButtonCallback(state.mainWindow, ButtonCallback);
    glfwSetCursorPosCallback(state.mainWindow, MouseCallback);
    glfwSetScrollCallback(state.mainWindow, ScrollCallback);
    
    state.camera = (Camera) {
        .position = Vec3Zero(),
        .angle = 0.f,
        .pitch = PI + HALF_PI,
        .zoom = 64.f
    };
    memcpy(&state.cameraTarget, &state.camera, sizeof(Camera));
    state.tileTexture = LoadTexture("assets/5z1KX.png");
    InitMap(&state.map, &state.tileTexture, 64, 64);
    InitDebug();
    double mouseX, mouseY;
    glfwGetCursorPos(state.mainWindow, &mouseX, &mouseY);
    state.mousePosition = state.lastMousePosition = Vec2New(mouseX, mouseY);
    state.lastTime = glfwGetTime();
    
//    state.testTexture = LoadTexture("assets/test.png");
    LoadMesh(&state.testMesh, "assets/suzanne.obj");
    
    while (!glfwWindowShouldClose(state.mainWindow)) {
        double now = glfwGetTime();
        state.deltaTime = now - state.lastTime;
        state.lastTime = now;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        int windowWidth, windowHeight;
        glfwGetWindowSize(state.mainWindow, &windowWidth, &windowHeight);
#if defined(PLATFORM_MAC)
        int framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize(state.mainWindow, &framebufferWidth, &framebufferHeight);
        glViewport(0, 0, framebufferWidth, framebufferHeight);
#else
        glViewport(0, 0, windowWidth, windowHeight);
#endif
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
        
        if (state.m1Down) {
            Vec2f delta = state.mousePosition - state.lastMousePosition;
            if (state.ctrlDown) {
                float angle = state.cameraTarget.angle +  delta.x * 1.f * state.deltaTime;
                if (angle > TWO_PI)
                    state.cameraTarget.angle = angle - TWO_PI;
                else if (angle < 0)
                    state.cameraTarget.angle = TWO_PI + angle;
                else
                    state.cameraTarget.angle = angle;
                state.cameraTarget.pitch = CLAMP(state.cameraTarget.pitch + -delta.y * 1.f * state.deltaTime, PI + HALF_PI, TWO_PI);
            } else {
                Vec3f right = {
                    sinf(state.camera.angle + HALF_PI),
                    0.0f,
                    cosf(state.camera.angle + HALF_PI)
                };
                static const Vec3f up = Vec3New(0.f, 1.f, 0.f);
                float cameraSpeed = EaseExpoOut(state.cameraTarget.zoom / MAX_ZOOM, 0.f, 1.f, 1.f);
                cameraSpeed = REMAP(cameraSpeed, 0.1, 1.f, 100.f, .1f);
                Vec3f move = Vec3New(delta.x * right.x,
                                     delta.y * up.y,
                                     delta.x * right.z) * cameraSpeed * state.deltaTime;
                state.cameraTarget.position += move;
            }
        }
        
        if (!Vec2Equals(state.scrollDelta, Vec2Zero()))
            state.cameraTarget.zoom  = CLAMP(state.cameraTarget.zoom + state.scrollDelta.y * 20.f * state.deltaTime, .1, MAX_ZOOM);
        
        state.camera.position += (state.cameraTarget.position - state.camera.position) * 10.f * state.deltaTime;
        float diff = state.cameraTarget.angle - state.camera.angle;
        if (fabs(diff) > M_PI)
           diff += (diff > 0 ? -2 : 2) * M_PI;
        float angle = state.camera.angle + diff * 10.f * state.deltaTime;
        if (angle > TWO_PI)
            state.camera.angle = angle - TWO_PI;
        else if (angle < 0)
            state.camera.angle = TWO_PI + angle;
        else
            state.camera.angle = angle;
        state.camera.pitch += (state.cameraTarget.pitch - state.camera.pitch) * 10.f * state.deltaTime;
        state.camera.zoom += (state.cameraTarget.zoom - state.camera.zoom) * 10.f * state.deltaTime;
        
        RenderMap(&state.map, windowWidth, windowHeight, &state.camera, state.cursor);
        
        glEnable(GL_DEPTH_TEST);
        RenderMesh(&state.testMesh, NULL);
        glDisable(GL_DEPTH_TEST);
        
        DebugFormat(8, 8, windowWidth, windowHeight, HEX(0xFFFF0000), "CAMERA: %f, %f\n", state.camera.position.x, state.camera.position.y);
        DebugFormat(8, 16, windowWidth, windowHeight, HEX(0xFFFF0000), "        %f, %f %f\n", state.camera.angle, state.camera.pitch, state.camera.zoom);
        
        glfwSwapBuffers(state.mainWindow);
        state.lastMousePosition = state.mousePosition;
        state.scrollDelta = Vec2Zero();
        glfwPollEvents();
    }
    return 0;
}
