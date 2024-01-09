//
//  debug.c
//  tbce
//
//  Created by George Watson on 27/12/2023.
//

#include "debug.h"

static struct {
    Texture font;
} debug;

void InitDebug(void) {
    static const int width = 128 * 8;
    ezImage *tmp = ezImageNew(width, 8);
    for (int x = 0; x < 128; x++)
        ezImageDrawCharacter(tmp, (char)x, x * 8, 0, 0xFFFFFFFF);
    debug.font = LoadTextureFromMemory(tmp);
    ezImageFree(tmp);
}

void DrawCharacter(int x, int y, int vw, int vh, Color color, unsigned char c) {
    const float scale = 1.f;
    float tx = (c / 128.0f);
    float tw = 1.0f / 128.0f;
    float w = 8.0f * scale;
    float h = 8.0f * scale;
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glOrtho(0, (float)vw, (float)vh, 0, -1, 1);
    
    glBindTexture(GL_TEXTURE_2D, debug.font.id);
    glBegin(GL_QUADS);
    
    PushColor(color);
    glTexCoord2f(tx, 1.0f);
    glVertex2f(x, y + h);
    glTexCoord2f(tx, 0.0f);
    glVertex2f(x, y);
    glTexCoord2f(tx + tw, 0.0f);
    glVertex2f(x + w, y);
    glTexCoord2f(tx + tw, 1.0f);
    glVertex2f(x + w, y + h);
    
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();
}

void DebugPrint(int _x, int _y, int vw, int vh, Color color, const char *string) {
    int x = _x, y = _y;
    for (int i = 0; i < strlen(string); i++) {
        switch (string[i]) {
            case '\n':
                x = _x;
                y += 8;
                break;
            default:
                DrawCharacter(x, y, vw, vh, color, string[i]);
            case ' ':
                x += 8;
                break;
        }
    }
}

#if !defined(_WIN32) && !defined(_WIN64)
// Taken from: https://stackoverflow.com/a/4785411
static int _vscprintf(const char *format, va_list pargs) {
    va_list argcopy;
    va_copy(argcopy, pargs);
    int retval = vsnprintf(NULL, 0, format, argcopy);
    va_end(argcopy);
    return retval;
}
#endif

void DebugFormat(int x, int y, int vw, int vh, Color color, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t size = _vscprintf(fmt, args) + 1;
    char *str = malloc(sizeof(char) * size);
    vsnprintf(str, size, fmt, args);
    va_end(args);
    DebugPrint(x, y, vw, vh, color, str);
    free(str);
}
