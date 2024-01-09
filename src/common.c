//
//  common.c
//  tbce
//
//  Created by George Watson on 24/12/2023.
//

#include "common.h"

Texture LoadTexture(const char *path) {
    ezImage *image = ezImageLoadFromPath(path);
    Texture result = LoadTextureFromMemory(image);
    ezImageFree(image);
    return result;
}

Texture LoadTextureFromMemory(ezImage *image) {
    GLuint id = -1;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->w, image->h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image->buf);
    glBindTexture(GL_TEXTURE_2D, 0);
    return (Texture) {
        .id = id,
        .width = image->w,
        .height = image->h
    };
}

void PushColor(Color color) {
    glColor4f(TO_FLOAT(color.r),
              TO_FLOAT(color.g),
              TO_FLOAT(color.b),
              TO_FLOAT(color.a));
}
