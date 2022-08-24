/***********************************************************************
 * Copyright (c) 2009 Milan Jaitner                                   *
 * Distributed under the MIT software license, see the accompanying    *
 * file COPYING or https://www.opensource.org/licenses/mit-license.php.*
 ***********************************************************************/

#include <gl/glew.h>
#include "xmanimation.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "timer.h"
#include "textures.h"
#include "math/mat4.h"
#include <gl/glut.h>

//#define _info

GLfloat light_pos[] = {10.0f, 50.0f, 10.0f, 1.0f};
GLfloat light_Ka[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat light_Kd[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat light_Ks[] = {1.0f, 1.0f, 1.0f, 1.0f};
extern xmTimer timer;
xmTexture texture;


xmAnimation::xmAnimation() {
    bFirstTime = true;
    bPlay = false;
    fAnimationTime = 0.0f;
    iPrevFrame = 0;
    iCurrFrame = 0;
    fInterpolation = 0;
    iNumIndexedBBs = 0;
    iSpineIndex = 0;

}

xmAnimation::~xmAnimation() {

}


bool xmAnimation::xmLoadBinaryXMAnimation(const char *xmaModelPath) {
    FILE *pFile = fopen(xmaModelPath, "rb");
    if (!pFile) {
        printf("error: could not open file %s!", xmaModelPath);
        return false;
    }

    fread(headerTitle, 1, 74, pFile);
    fread(header, 1, 7, pFile);

    if (strcmp(header, "LXMA19")) {
        printf("error: file %s is not valid xmAnimation file!\n");
        return false;
    }
    iFirstFrame = 0;
    texture.xmLoadTextureExtensions();
    fread(&iNumObjects, 1, sizeof(int), pFile);
    fread(&iFramerate, 1, sizeof(float), pFile);
    fread(&iLastFrame, 1, sizeof(int), pFile);
    fread(&iNumBones, 1, sizeof(int), pFile);
    fread(&iNumAnimations, 1, sizeof(int), pFile);

    anim = new xmaAnimation[iNumAnimations];
    if (!anim) return false;

    for (int a = 0; a < iNumAnimations; a++) {
        short int size = 0;
        fread(&anim[a].id, sizeof(int), 1, pFile);
        fread(&size, sizeof(short int), 1, pFile);
        fread(&anim[a].animName, sizeof(char), size, pFile);
        fread(&anim[a].animNumBoneAffectNames, sizeof(int), 1, pFile);
        for (int b = 0; b < anim[a].animNumBoneAffectNames; b++) {
            fread(&size, sizeof(short int), 1, pFile);
            fread(&anim[a].boneAffectName[b], sizeof(char), size, pFile);
        }
        fread(&anim[a].animType, sizeof(int), 1, pFile);
        fread(&anim[a].animStartFrame, sizeof(int), 1, pFile);
        fread(&anim[a].animEndFrame, sizeof(int), 1, pFile);
        fread(&anim[a].animFramerate, sizeof(int), 1, pFile);
        printf("%d %s %d %d %d %d\n", anim[a].id, anim[a].animName, anim[a].animType, anim[a].animStartFrame,
               anim[a].animEndFrame, anim[a].animFramerate);
        printf("%d\n", anim[a].animNumBoneAffectNames);
        for (int b = 0; b < anim[a].animNumBoneAffectNames; b++) {
            printf("%s\n", anim[a].boneAffectName[b]);
        }

    }

    if (!glGenBuffersARB || !glBindBufferARB || !glBufferDataARB || !glDeleteBuffersARB ||
        !glBufferDataARB || !glBufferSubDataARB || !glMapBufferARB || !glUnmapBufferARB
            )//&& framework.xmCheckEXTension("GL_ARB_vertex_buffer_object"))
    {
        glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) wglGetProcAddress("glGenBuffersARB");
        glBindBufferARB = (PFNGLBINDBUFFERARBPROC) wglGetProcAddress("glBindBufferARB");
        glBufferDataARB = (PFNGLBUFFERDATAARBPROC) wglGetProcAddress("glBufferDataARB");
        glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) wglGetProcAddress("glDeleteBuffersARB");
        glBufferDataARB = (PFNGLBUFFERDATAARBPROC) wglGetProcAddress("glBufferDataARB");
        glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC) wglGetProcAddress("glBufferSubDataARB");
        glMapBufferARB = (PFNGLMAPBUFFERARBPROC) wglGetProcAddress("glMapBufferARB");
        glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC) wglGetProcAddress("glUnmapBufferARB");

        if (!glGenBuffersARB || !glBindBufferARB || !glBufferDataARB || !glDeleteBuffersARB ||
            !glBufferDataARB || !glBufferSubDataARB || !glMapBufferARB || !glUnmapBufferARB) {
            printf("error: LOL cannot load GL_ARB_vertex_buffer_object functions, its awesome ! :(\n");
            return false;
        }
    }


    bone = new xmaBone[iNumBones];
    if (!bone) return false;

#ifdef _info
    printf("iNumObjects: %d\n", iNumObjects);
#endif
    if (iNumObjects <= 0) return false;
    object = new xmaObject[iNumObjects];
    if (!object) return false;

    for (int o = 0; o < iNumObjects; o++) {
        short int size = 0;
        fread(&size, 1, sizeof(short int), pFile);
        fread(object[o].name, 1, size, pFile);
#ifdef _info
        printf("%s %d %d\n",object[o].name, strlen(object[o].name), o);
#endif

        fread(&object[o].iNumVertices, 1, sizeof(int), pFile);
#ifdef _info
        printf("%d %d %d %d %.1f\n",object[o].iNumVertices,
                                    iNumBones,
                                    iFirstFrame,
                                    iLastFrame,
                                    iFramerate);
#endif

        object[o].texcoord = new vec2[object[o].iNumVertices];
        object[o].vertex = new vec3[object[o].iNumVertices];
        object[o].animverts = new vec3[object[o].iNumVertices];
        object[o].boneid = new int[object[o].iNumVertices];
        object[o].normal = new vec3[object[o].iNumVertices];
        object[o].tangent = new vec3[object[o].iNumVertices];
        object[o].bitangent = new vec3[object[o].iNumVertices];

        if (!object[o].vertex || !object[o].texcoord || !object[o].normal || !object[o].tangent ||
            !object[o].bitangent) {
            printf("error: cannot alloc. memory for vertex, texcoords and things about it for xmAnimation!\n");
            return false;
        }

        for (int i = 0; i < object[o].iNumVertices; i++) {
            fread(&object[o].vertex[i], 1, sizeof(vec3), pFile);
            fread(&object[o].normal[i], 1, sizeof(vec3), pFile);
            fread(&object[o].texcoord[i], 1, sizeof(vec2), pFile);
            fread(&object[o].boneid[i], 1, sizeof(int), pFile);
        }

        for (int i = 0; i < object[o].iNumVertices; i += 3) {
            if (object[o].min.x == 0) object[o].min.x = 999999.9f;
            if (object[o].max.x == 0) object[o].max.x = -999999.9f;
            if (object[o].min.z == 0) object[o].min.z = 999999.9f;
            if (object[o].max.z == 0) object[o].max.z = -999999.9f;
            if (object[o].min.y == 0) object[o].min.y = 999999.9f;
            if (object[o].max.y == 0) object[o].max.y = -999999.9f;

            int ind[3];
            ind[0] = i;
            ind[1] = i + 1;
            ind[2] = i + 2;

            for (int v = 0; v < 3; v++) {
                if (object[o].vertex[ind[v]].x < object[o].min.x) object[o].min.x = object[o].vertex[ind[v]].x - 2.0f;
                if (object[o].vertex[ind[v]].y < object[o].min.y) object[o].min.y = object[o].vertex[ind[v]].y - 2.0f;
                if (object[o].vertex[ind[v]].z < object[o].min.z) object[o].min.z = object[o].vertex[ind[v]].z - 2.0f;

                if (object[o].vertex[ind[v]].x > object[o].max.x) object[o].max.x = object[o].vertex[ind[v]].x + 2.0f;
                if (object[o].vertex[ind[v]].y > object[o].max.y) object[o].max.y = object[o].vertex[ind[v]].y + 2.0f;
                if (object[o].vertex[ind[v]].z > object[o].max.z) object[o].max.z = object[o].vertex[ind[v]].z + 2.0f;
            }

        }

#ifdef _info
        for(int v=0; v < object[o].iNumVertices; v++)
        {
            printf("vertex %f %f %f %d %f %f\n", object[o].vertex[v].x,
                                                 object[o].vertex[v].y,
                                                 object[o].vertex[v].z,
                                                 object[o].boneid[v],
                                                 object[o].texcoord[v].x,
                                                 object[o].texcoord[v].y);
        }
#endif

        fread(&size, 1, sizeof(short int), pFile);
        fread(object[o].texturePath, 1, size, pFile);
        fread(&object[o].textureFilter, 1, sizeof(short int), pFile);

#ifdef _info
        printf("size %d texture path %s  filter %f\n", size, object[o].texturePath, object[o].textureFilter);
#endif

        char tempTexName[1024];
        sprintf((char *) &tempTexName, "%s", (char *) &(object[o].texturePath));
        object[o].uDiffuseMap = texture.xmLoadTexture((char *) &tempTexName, o, (int) object[o].textureFilter);

        object[o].center = ((object[o].max + object[o].min) / 2);
        object[o].radius = Distance(object[o].max, object[o].center);

        glGenBuffersARB(1, &object[o].uVertexVBO);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, object[o].uVertexVBO);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, object[o].iNumVertices * 3 * sizeof(float), object[o].vertex,
                        GL_STREAM_DRAW_ARB);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        glGenBuffersARB(1, &object[o].uTexCoordVBO);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, object[o].uTexCoordVBO);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, object[o].iNumVertices * 2 * sizeof(float), object[o].texcoord,
                        GL_STATIC_DRAW_ARB);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        glGenBuffersARB(1, &object[o].uNormalVBO);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, object[o].uNormalVBO);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, object[o].iNumVertices * 3 * sizeof(float), object[o].normal,
                        GL_STATIC_DRAW_ARB);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        if (object[o].texcoord) delete[] object[o].texcoord;
        if (object[o].normal) delete[] object[o].normal;
        if (object[o].texcoord) object[o].texcoord = NULL;
        if (object[o].normal) object[o].normal = NULL;

    }


    for (int i = 0; i < iNumBones; i++) {
        short int size = 0;
        fread(&size, 1, sizeof(short int), pFile);
        fread(&bone[i].boneName, 1, size, pFile);
        //printf("%d %s\n",size,bone[i].boneName);

        fread(&size, 1, sizeof(short int), pFile);
        fread(&bone[i].boneParent, 1, size, pFile);
        //printf("%d %s\n",size,bone[i].boneParent);

        fread(&bone[i].rotation, 1, sizeof(vec3), pFile);
        fread(&bone[i].position, 1, sizeof(vec3), pFile);


        fread(&bone[i].iNumRotationKeys, sizeof(unsigned short), 1, pFile);
        fread(&bone[i].iNumPositionKeys, sizeof(unsigned short), 1, pFile);

        bone[i].rotationKey = new xmaRotationKeyFrame[bone[i].iNumRotationKeys > 0 ? bone[i].iNumRotationKeys : 100];
        bone[i].positionKey = new xmaPositionKeyFrame[bone[i].iNumPositionKeys > 0 ? bone[i].iNumPositionKeys : 100];

        fread(bone[i].rotationKey, sizeof(xmaRotationKeyFrame), bone[i].iNumRotationKeys, pFile);
        fread(bone[i].positionKey, sizeof(xmaPositionKeyFrame), bone[i].iNumPositionKeys, pFile);


        //printf("%s %s\n", bone[i].boneName, bone[i].boneParent);
#ifdef _info
        for(int r=0; r < bone[i].iNumRotationKeys; r++)
        {
            printf("rotation key bid %d rkid %d %f %f %f %f\n", i, r, bone[i].rotationKey[r].time,
                                bone[i].rotationKey[r].rotation.x,
                                bone[i].rotationKey[r].rotation.y,
                                bone[i].rotationKey[r].rotation.z);
        }

        for(int p=0; p < bone[i].iNumPositionKeys; p++)
        {
            printf("position key bid %d pkid %d %f %f %f %f\n", i, p, bone[i].positionKey[p].time,
                                bone[i].positionKey[p].position.x,
                                bone[i].positionKey[p].position.y,
                                bone[i].positionKey[p].position.z);
        }
        printf("%f %f %f    %f %f %f\n",
            bone[i].position.x,
            bone[i].position.y,
            bone[i].position.z,
            bone[i].rotation.x,
            bone[i].rotation.y,
            bone[i].rotation.z);
#endif
    }
    for (int i = 0; i < iNumBones; i++) {
        bone[i].parent = -1;
        if (bone[i].boneParent[0] == '\0') continue;

        for (int n = i - 1; n >= 0; n--) {
            if (!strcmp(bone[n].boneName, bone[i].boneParent)) {
                bone[i].parent = n;
                break;
            }
        }
        if (strstr(bone[i].boneName, "Forearm") ||
            strstr(bone[i].boneName, "UpperArm") ||
            strstr(bone[i].boneName, "Clavicle") ||
            strstr(bone[i].boneName, "Spine") ||
            strstr(bone[i].boneName, "Head") ||
            strstr(bone[i].boneName, "Thigh") ||
            strstr(bone[i].boneName, "Calf") ||
            strstr(bone[i].boneName, "Hand") ||
            strstr(bone[i].boneName, "Foot")) {
            bBones[iNumIndexedBBs] = i;
            iNumIndexedBBs++;
        }

        if (!strcmp(bone[i].boneName, "Bip01 Spine")) iSpineIndex = i;
        if (!strcmp(bone[i].boneName, "Bip01 Pelvis")) iPelvisIndex = i;

    }
    printf("%d %d\n", iSpineIndex, iPelvisIndex);

    mat4 mat;
    for (int i = 0; i < iNumBones; i++) {
        mat.Translate(bone[i].position);
        mat.Rotate(bone[i].rotation);

        bone[i].rel = mat;

        if (bone[i].parent != -1) mat = mat * bone[bone[i].parent].abs;
        bone[i].abs = mat;
        mat.Identity();
    }

    for (int k = 0; k < iNumBones; k++) {
        mat = bone[k].abs;
        mat.Inverse();
        for (int o = 0; o < iNumObjects; o++) {
            for (int j = 0; j < object[o].iNumVertices; j++) {
                if (object[o].boneid[j] == k) {
                    object[o].vertex[j] = mat * object[o].vertex[j];
                }
            }
        }
    }

    setAnimation(0, 0, 0, true);

    fclose(pFile);
    return true;
}

bool xmAnimation::setAnimationByID(int id, bool bPlay) {
    if (id > iNumAnimations) return false;
    return setAnimation(anim[id].animStartFrame, anim[id].animEndFrame, anim[id].animFramerate, bPlay);
}


bool xmAnimation::setAnimation(int FirstFrame, int LastFrame, int AnimFPS, bool playAnimation) {
    glPushMatrix();
    {
        if (playAnimation) bPlay = true;
        if (bPlay) {
            if (AnimFPS < 0) AnimFPS = (int) iFramerate;

            if (bFirstTime) fAnimationTime = iFirstFrame;

            //printf("%f\n", fAnimationTime);
            fAnimationTime += (float) AnimFPS * timer.fFrameInterval;

            if (fAnimationTime >= LastFrame) {
                bPlay = false;
                fAnimationTime = FirstFrame;
            }

            for (int o = 0; o < iNumObjects; o++) {
                for (int i = 0; i < iNumBones; i++) {
                    iPrevFrame = 0;
                    iCurrFrame = 0;
                    fInterpolation = 0;

                    while (bone[i].positionKey[iCurrFrame].time < fAnimationTime &&
                           iCurrFrame < bone[i].iNumPositionKeys)
                        iCurrFrame++;
                    iPrevFrame = iCurrFrame;
                    if (iCurrFrame > 0) iPrevFrame--;
                    fInterpolation = (fAnimationTime - bone[i].positionKey[iPrevFrame].time);
                    if (iCurrFrame != iPrevFrame) fInterpolation /= bone[i].positionKey[iCurrFrame].time -
                                                                    bone[i].positionKey[iPrevFrame].time;
                    vec3 vTransformation =
                            bone[i].positionKey[iCurrFrame].position - bone[i].positionKey[iPrevFrame].position;
                    if (iCurrFrame == bone[i].iNumPositionKeys) fInterpolation = 0;
                    vTransformation *= fInterpolation;
                    vTransformation += bone[i].positionKey[iPrevFrame].position;
                    finalMatrix.Translate(vTransformation);

                    iPrevFrame = 0;
                    iCurrFrame = 0;
                    while (bone[i].rotationKey[iCurrFrame].time < fAnimationTime &&
                           iCurrFrame < bone[i].iNumRotationKeys)
                        iCurrFrame++;
                    iPrevFrame = iCurrFrame;
                    if (iCurrFrame > 0)iPrevFrame--;
                    fInterpolation = (fAnimationTime - bone[i].rotationKey[iPrevFrame].time);
                    if (iCurrFrame != iPrevFrame) fInterpolation /= bone[i].rotationKey[iCurrFrame].time -
                                                                    bone[i].rotationKey[iPrevFrame].time;
                    vec3 vRotation =
                            bone[i].rotationKey[iCurrFrame].rotation - bone[i].rotationKey[iPrevFrame].rotation;
                    if (iCurrFrame == bone[i].iNumRotationKeys) fInterpolation = 0;
                    vRotation *= fInterpolation;
                    vRotation += bone[i].rotationKey[iPrevFrame].rotation;
                    if (i == iSpineIndex) vRotation += SpineRotation;
                    if (i == iPelvisIndex) vRotation += PelvisRotation;

                    finalMatrix.Rotate(vRotation);

                    bone[i].abs = finalMatrix;
                    finalMatrix = finalMatrix * bone[i].rel;

                    if (bone[i].parent != -1) finalMatrix = finalMatrix * bone[bone[i].parent].final;
                    bone[i].final = finalMatrix;
                    finalMatrix.Identity();
                }
            }
        }
    }
    if (playAnimation) bFirstTime = false;
    glPopMatrix();

    return bPlay;
}


bool xmAnimation::setBlendedAnimation(int animID1, int animID2, int AnimFPS, bool playAnimation) {
    if (animID1 > iNumAnimations || animID2 > iNumAnimations) return false;
    mat4 boneMatrix[128][2];
    glPushMatrix();
    {
        if (playAnimation) bPlay = true;
        if (bPlay) {
            int FirstFrame = anim[animID1].animStartFrame;
            int LastFrame = anim[animID1].animEndFrame;

            int FirstFrame1 = anim[animID2].animStartFrame;
            int LastFrame1 = anim[animID2].animEndFrame;

            //printf("%d %d %d %d\n", FirstFrame, LastFrame, FirstFrame1, LastFrame1);

            if (AnimFPS < 0) AnimFPS = (int) iFramerate;

            if (bFirstTime) {
                BlendedAnimationTime[0] = FirstFrame;
                BlendedAnimationTime[1] = FirstFrame1;
            }

            BlendedAnimationTime[0] += (float) anim[animID1].animFramerate * timer.fFrameInterval;
            BlendedAnimationTime[1] += (float) anim[animID2].animFramerate * timer.fFrameInterval;

            if (BlendedAnimationTime[0] >= LastFrame) {
                BlendedAnimationTime[0] = FirstFrame;
            }

            if (BlendedAnimationTime[1] >= LastFrame1) {
                BlendedAnimationTime[1] = FirstFrame1;
            }


            for (int i = 0; i < iNumBones; i++) {

                iPrevFrame = 0;
                iCurrFrame = 0;
                fInterpolation = 0;

                while (bone[i].positionKey[iCurrFrame].time < BlendedAnimationTime[0] &&
                       iCurrFrame < bone[i].iNumPositionKeys)
                    iCurrFrame++;
                iPrevFrame = iCurrFrame;
                if (iCurrFrame > 0) iPrevFrame--;
                fInterpolation = (BlendedAnimationTime[0] - bone[i].positionKey[iPrevFrame].time);
                if (iCurrFrame != iPrevFrame) fInterpolation /= bone[i].positionKey[iCurrFrame].time -
                                                                bone[i].positionKey[iPrevFrame].time;
                vec3 vTransformation =
                        bone[i].positionKey[iCurrFrame].position - bone[i].positionKey[iPrevFrame].position;
                if (iCurrFrame == bone[i].iNumPositionKeys) fInterpolation = 0;
                vTransformation *= fInterpolation;
                vTransformation += bone[i].positionKey[iPrevFrame].position;
                boneMatrix[i][0].Translate(vTransformation);

                iPrevFrame = 0;
                iCurrFrame = 0;
                while (bone[i].rotationKey[iCurrFrame].time < BlendedAnimationTime[0] &&
                       iCurrFrame < bone[i].iNumRotationKeys)
                    iCurrFrame++;
                iPrevFrame = iCurrFrame;
                if (iCurrFrame > 0)iPrevFrame--;
                fInterpolation = (BlendedAnimationTime[0] - bone[i].rotationKey[iPrevFrame].time);
                if (iCurrFrame != iPrevFrame) fInterpolation /= bone[i].rotationKey[iCurrFrame].time -
                                                                bone[i].rotationKey[iPrevFrame].time;
                vec3 vRotation = bone[i].rotationKey[iCurrFrame].rotation - bone[i].rotationKey[iPrevFrame].rotation;
                if (iCurrFrame == bone[i].iNumRotationKeys) fInterpolation = 0;
                vRotation *= fInterpolation;
                vRotation += bone[i].rotationKey[iPrevFrame].rotation;
                boneMatrix[i][0].Rotate(vRotation);

                bone[i].abs = boneMatrix[i][0];
                boneMatrix[i][0] = boneMatrix[i][0] * bone[i].rel;
                if (bone[i].parent != -1) boneMatrix[i][0] = boneMatrix[i][0] * bone[bone[i].parent].final;
                bone[i].final = boneMatrix[i][0];
            }

            for (int i = 0; i < iNumBones; i++) {
                bool blend = false;
                for (int b = 0; b < anim[animID2].animNumBoneAffectNames; b++) {
                    //printf("%s %s\n", bone[i].boneName, anim[animID2].boneAffectName[b]);
                    if (!strcmp(bone[i].boneName, anim[animID2].boneAffectName[b])) blend = true;
                }

                if (blend) {
                    iPrevFrame = 0;
                    iCurrFrame = 0;
                    fInterpolation = 0;

                    while (bone[i].positionKey[iCurrFrame].time < BlendedAnimationTime[1] &&
                           iCurrFrame < bone[i].iNumPositionKeys)
                        iCurrFrame++;
                    iPrevFrame = iCurrFrame;
                    if (iCurrFrame > 0) iPrevFrame--;
                    fInterpolation = (BlendedAnimationTime[1] - bone[i].positionKey[iPrevFrame].time);
                    if (iCurrFrame != iPrevFrame) fInterpolation /= bone[i].positionKey[iCurrFrame].time -
                                                                    bone[i].positionKey[iPrevFrame].time;
                    vec3 vTransformation =
                            bone[i].positionKey[iCurrFrame].position - bone[i].positionKey[iPrevFrame].position;
                    if (iCurrFrame == bone[i].iNumPositionKeys) fInterpolation = 0;
                    vTransformation *= fInterpolation;
                    vTransformation += bone[i].positionKey[iPrevFrame].position;
                    boneMatrix[i][1].Translate(vTransformation);

                    iPrevFrame = 0;
                    iCurrFrame = 0;
                    while (bone[i].rotationKey[iCurrFrame].time < BlendedAnimationTime[1] &&
                           iCurrFrame < bone[i].iNumRotationKeys)
                        iCurrFrame++;
                    iPrevFrame = iCurrFrame;
                    if (iCurrFrame > 0)iPrevFrame--;
                    fInterpolation = (BlendedAnimationTime[1] - bone[i].rotationKey[iPrevFrame].time);
                    if (iCurrFrame != iPrevFrame) fInterpolation /= bone[i].rotationKey[iCurrFrame].time -
                                                                    bone[i].rotationKey[iPrevFrame].time;
                    vec3 vRotation =
                            bone[i].rotationKey[iCurrFrame].rotation - bone[i].rotationKey[iPrevFrame].rotation;
                    if (iCurrFrame == bone[i].iNumRotationKeys) fInterpolation = 0;
                    vRotation *= fInterpolation;
                    vRotation += bone[i].rotationKey[iPrevFrame].rotation;
                    boneMatrix[i][1].Rotate(vRotation);

                    bone[i].abs = boneMatrix[i][1];
                    boneMatrix[i][1] = boneMatrix[i][1] * bone[i].rel;
                    if (bone[i].parent != -1) boneMatrix[i][1] = boneMatrix[i][1] * bone[bone[i].parent].final;

                    float delta = 1.0f;
                    bone[i].final = boneMatrix[i][1] * delta + boneMatrix[i][1] * (1 - delta);
                }
            }

        }
    }
    if (playAnimation) bFirstTime = false;
    glPopMatrix();

    return bPlay;
}

void drawbox(vec3 min, vec3 max) {

    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    glVertex3f(max.x, max.y, min.z); // 0
    glVertex3f(min.x, max.y, min.z); // 1
    glVertex3f(min.x, min.y, min.z); // 2
    glVertex3f(max.x, min.y, min.z); // 3
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex3f(max.x, min.y, max.z); // 4
    glVertex3f(max.x, max.y, max.z); // 5
    glVertex3f(min.x, max.y, max.z); // 6
    glVertex3f(min.x, min.y, max.z); // 7
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex3f(max.x, max.y, min.z); // 0
    glVertex3f(max.x, max.y, max.z); // 5
    glVertex3f(min.x, max.y, max.z); // 6
    glVertex3f(min.x, max.y, min.z); // 1
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex3f(max.x, min.y, max.z); // 4
    glVertex3f(min.x, min.y, max.z); // 7
    glVertex3f(min.x, min.y, min.z); // 2
    glVertex3f(max.x, min.y, min.z); // 3

    glEnd();
}

void xmAnimation::Rotate(vec3 angle) {
    SpineRotation = angle;
    PelvisRotation.x = angle.x * 2.0;
}

void xmAnimation::draw() {
    //glEnable(GL_CULL_FACE);

    //glDisable(GL_DEPTH_TEST);
    glPointSize(10.0);
    glColor3f(1, 0, 0);
    for (int i = 0; i < iNumBones; i++) {
        vec3 bonePos = bone[i].final * vec3(0.0, 0.0, 0.0);
        vec3 parent = bone[bone[i].parent].final * vec3(0.0, 0.0, 0.0);

        glBegin(GL_POINTS);
        glVertex3f(bonePos.x, bonePos.y, bonePos.z);
        glEnd();
    }

    //for(int i=0; i < iNumIndexedBBs; i++)
    //{
    //    vec3 bonePos = bone[bBones[i]].final * vec3(0.0, 0.0, 0.0);
    //    glPushMatrix();
    //        glTranslatef(bonePos.x, bonePos.y, bonePos.z);
    //        glutWireSphere(8, 12, 12);
    //    glPopMatrix();          
    //}


    glColor3f(1, 1, 1);
    //glEnable(GL_DEPTH_TEST);

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_Ka);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_Kd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_Ks);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);


    for (int o = 0; o < iNumObjects; o++) {
        //450FPS
        for (int i = 0; i < object[o].iNumVertices; i++) {
            object[o].animverts[i] = bone[object[o].boneid[i]].final * object[o].vertex[i];
        }

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, object[o].uVertexVBO);
        float *ptr = (float *) glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB);
        if (ptr) {
            memcpy(ptr, (float *) object[o].animverts, object[o].iNumVertices * sizeof(vec3));
            glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
        }

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        //glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture(GL_TEXTURE_2D, object[o].uDiffuseMap);
        //glUniform1iARB(shader.iUniformDiffuseMap, 0);  
        //glActiveTextureARB(GL_TEXTURE0_ARB);

        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, object[o].uTexCoordVBO);
        glTexCoordPointer(2, GL_FLOAT, 0, 0);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        glEnableClientState(GL_VERTEX_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, object[o].uVertexVBO);
        glVertexPointer(3, GL_FLOAT, 0, 0);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        glEnableClientState(GL_NORMAL_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, object[o].uNormalVBO);
        glNormalPointer(GL_FLOAT, 0, 0);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        glDrawArrays(GL_TRIANGLES, 0, object[o].iNumVertices);
        glDisable(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_NORMAL_ARRAY);
        glDisable(GL_VERTEX_ARRAY);

    }
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);
}


void xmAnimation::FPSPerson() {
    glPushMatrix();
    glDepthRange(0, 0.1);
    //vec3 cpos = camera.GetPosition();
    //vec2 angle = camera.GetAngle();
    //glTranslatef(-cpos.x, -cpos.y, -cpos.z);
    //glRotatef(-angle.y+180,0,1,0);
    //glRotatef(angle.x+180,1,0,0);
    draw();
    glDepthRange(0.0, 1);
    glPopMatrix();
}

