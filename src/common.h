//
//  common.h
//  tbce
//
//  Created by George Watson on 24/12/2023.
//

#ifndef common_h
#define common_h
#include "cwcgl.h"
#include <GLFW/glfw3.h>
#include "ez/ezimage.h"
#include <assert.h>
#include "ez/ezmath.h"

#define PLATFORM_POSIX
#if defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
#define PLATFORM_MAC
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
#define PLATFORM_WINDOWS
#if !defined(PLATFORM_FORCE_POSIX)
#undef PLATFORM_POSIX
#endif
#elif defined(__gnu_linux__) || defined(__linux__) || defined(__unix__)
#define PLATFORM_LINUX
#else
#error "Unsupported operating system"
#endif

#define GL_CHECK_ERRORS()                                      \
do {                                                           \
    GLenum err = 0;                                            \
    int errorCaught = 0;                                       \
    while ((err = glGetError())) {                             \
        printf("OpenGL ERROR #%d @ line %d\n", err, __LINE__); \
        errorCaught = 1;                                       \
    }                                                          \
    if (errorCaught)                                           \
        abort();                                               \
} while (0)

typedef union {
    struct {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    };
    uint32_t value;
} Color;

#define RGBA(R, G, B, A) (Color){.r = (R), .g = (G), .b = (B), .a = (A)}
#define HEX(H) (Color){.value = (H)}
#define RGB(R, G, B) RGBA((R), (G), (B), 255)

typedef struct {
    GLuint id;
    int width, height;
} Texture;

#define TO_FLOAT(V) ((float)(V) / 255.f)

Texture LoadTexture(const char *path);
Texture LoadTextureFromMemory(ezImage *image);

void PushColor(Color color);

typedef struct {
    Vec3f position;
    float angle;
    float pitch;
    float zoom;
} Camera;

#define MAX_ZOOM 256.f

#endif /* common_h */

