/***********************************************************************
 * Copyright (c) 2009 Milan Jaitner                                   *
 * Distributed under the MIT software license, see the accompanying    *
 * file COPYING or https://www.opensource.org/licenses/mit-license.php.*
 ***********************************************************************/

#ifndef _XMANIMATION_H
#define _XMANIMATION_H

#include "math/vector.h"
#include "math/mat4.h"
#include <gl/gl.h>

struct BoundingBox {
    vec3 min;
    vec3 max;
    short int bid;
};


struct xmaAnimation {
    int id;
    char animName[256];
    int animNumBoneAffectNames;
    char boneAffectName[256][256];
    int animType;

    int animStartFrame;
    int animEndFrame;
    int animFramerate;
};

struct xmaRotationKeyFrame {
    xmaRotationKeyFrame() {
        time = 0.0f;
        rotation = vec3(0, 0, 0);
    }

    float time;
    vec3 rotation;
};

struct xmaPositionKeyFrame {
    xmaPositionKeyFrame() {
        time = 0.0f;
        position = vec3(0, 0, 0);
    }

    float time;
    vec3 position;
};

struct xmaBone {
    short int parent;
    char boneName[256];
    char boneParent[256];

    vec3 min;
    vec3 max;

    vec3 rotation;
    vec3 position;
    mat4 abs;
    mat4 rel;
    mat4 final;

    unsigned short int iNumPositionKeys;
    unsigned short int iNumRotationKeys;
    xmaPositionKeyFrame *positionKey;
    xmaRotationKeyFrame *rotationKey;
};

struct xmaObject {
    vec3 *vertex;
    vec3 *tangent;
    vec3 *bitangent;
    vec3 *normal;
    vec2 *texcoord;
    int *boneid;

    vec3 *animverts;
    vec3 *animnormals;
    vec3 *animtangents;
    vec3 *animbitangents;

    vec3 min;
    vec3 max;
    vec3 center;
    float radius;
    int iNumVertices;
    char name[256];
    char texturePath[1024];
    short int textureFilter;
    float opacity;
    GLuint uDiffuseMap;
    GLuint uNormalMap;
    GLuint uSpecularMap;

    GLuint uVertexVBO;
    GLuint uTexCoordVBO;
    GLuint uNormalVBO;
};

class xmAnimation {

public:
    xmAnimation();

    ~xmAnimation();

    bool xmLoadBinaryXMAnimation(const char *xmaModelPath);

    bool setAnimation(int FirstFrame, int LastFrame, int AnimFPS, bool playAnimation);

    bool setAnimationByID(int id, bool bPlay);

    bool setBlendedAnimation(int animID1, int animID2, int AnimFPS, bool playAnimation);

    void Rotate(vec3 angle);

    void FPSPerson();

    void draw();

    xmaObject *object;
    xmaBone *bone;
    xmaAnimation *anim;
    BoundingBox *boundingBox;
    mat4 finalMatrix;
    int bBones[20];
    int iNumObjects;
    int iNumAnimations;
    int iNumBones;
    int iLastFrame;
    int iNumIndexedBBs;
    int iFirstFrame;
    float iFramerate;
    float fAnimationTime;
    float BlendedAnimationTime[2];
    bool bFirstTime;
    bool bPlay;
    int iPrevFrame;
    int iCurrFrame;
    float fInterpolation;

    int iSpineIndex;
    int iPelvisIndex;

    vec3 SpineRotation;
    vec3 PelvisRotation;

private:
    char headerTitle[255];
    char header[7];

};

#endif
