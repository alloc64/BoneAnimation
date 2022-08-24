/***********************************************************************
 * Copyright (c) 2009 Milan Jaitner                                   *
 * Distributed under the MIT software license, see the accompanying    *
 * file COPYING or https://www.opensource.org/licenses/mit-license.php.*
 ***********************************************************************/

#ifndef _TEXTURES_H
#define _TEXTURES_H

#include <gl/gl.h>
#include <gl/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <jpeglib.h>


struct xmTexData {
    GLsizei width;
    GLsizei height;

    GLenum format;
    GLint internalFormat;
    GLuint id;
    GLint depth;
    GLint totalComponenets;
    GLint totalMipMaps;
    GLint mipFactor;
    GLubyte *texels;
};

struct xmLoadedTexture {
    char name[255];
    int usedIndex;
};

class xmTexture {
public:
    bool xmLoadTextureExtensions();

    int xmLoadTexture(const char *xmFileName, GLuint xmTexIndex, GLuint xmFiltering);

    xmTexData *xmLoadJPG(const char *xmFileName);

    xmTexData *xmLoadXMT(const char *xmFileName);

    xmTexData *xmLoadDDS(const char *xmFileName);

    void SaveTGAScreenShot();

    xmLoadedTexture usedTexture[3001];

private:
    GLuint xmMinFilter;
    GLuint xmMaxFilter;
    GLint TEXTURE_TYPE;

};


#endif
