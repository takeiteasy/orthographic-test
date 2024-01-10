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

#define EZ_IMPLEMENTATION
#include "common.h"
#include "map.h"
#include "debug.h"
#include "model.h"

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
    
    Model suzanne;
} state;

static void ClampCursor(int dx, int dy) {
    Vec2i old = state.cursor;
    Vec2i delta = (Vec2i){ dx, dy };
    Vec2i new = old + delta;
    state.cursor = (Vec2i) {
        CLAMP(new.x, 0, state.map.w),
        CLAMP(new.y, 0, state.map.h)
    };
    if (old.x != new.x || old.y != new.y)
        state.cameraTarget.position = Vec3New(state.cursor.x + .5f,
                                              state.cursor.y + .5f,
                                              0.f);
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
       glfwSetWindowShouldClose(window, GLFW_TRUE);
    
    if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
        state.ctrlDown = action == GLFW_PRESS;
    
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_KP_0:
                state.cameraTarget.angle = TWO_PI;
                state.cameraTarget.pitch = PI + HALF_PI;
                break;
            case GLFW_KEY_KP_5:
                state.cameraTarget.angle = TWO_PI;
                state.cameraTarget.pitch = TWO_PI;
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
                state.cameraTarget.zoom = CLAMP(state.cameraTarget.zoom - 5.f, .1, MAX_ZOOM);
                break;
            case GLFW_KEY_KP_ADD:
                state.cameraTarget.zoom = CLAMP(state.cameraTarget.zoom + 5.f, .1, MAX_ZOOM);
                break;
            case GLFW_KEY_KP_MULTIPLY:
                state.cameraTarget.pitch = CLAMP(state.cameraTarget.pitch - .5f / PI, PI + HALF_PI, TWO_PI);
                break;
            case GLFW_KEY_KP_DIVIDE:
                state.cameraTarget.pitch = CLAMP(state.cameraTarget.pitch + .5f / PI, PI + HALF_PI, TWO_PI);
                break;
                
            case GLFW_KEY_LEFT:
            case GLFW_KEY_A:
                ClampCursor(-1, 0);
                break;
            case GLFW_KEY_RIGHT:
            case GLFW_KEY_D:
                ClampCursor(1, 0);
                break;
            case GLFW_KEY_UP:
            case GLFW_KEY_W:
                ClampCursor(0, -1);
                break;
            case GLFW_KEY_DOWN:
            case GLFW_KEY_S:
                ClampCursor(0, 1);
                break;
        }
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
    
    LoadModelObj("assets/suzanne.obj", &state.suzanne);
    
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
        
        RenderModel(&state.suzanne, 0, 0, &state.camera);
        
        DebugFormat(8, 8, windowWidth, windowHeight, HEX(0xFFFF0000), "CAMERA: %f, %f\n", state.camera.position.x, state.camera.position.y);
        DebugFormat(8, 16, windowWidth, windowHeight, HEX(0xFFFF0000), "        %f, %f %f\n", state.camera.angle, state.camera.pitch, state.camera.zoom);
        
        glfwSwapBuffers(state.mainWindow);
        state.lastMousePosition = state.mousePosition;
        state.scrollDelta = Vec2Zero();
        glfwPollEvents();
    }
    return 0;
}
