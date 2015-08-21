/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef STANDALONE
#include <linux/src/system/system_drm_egl.h>
#endif
#include "sgxRender3DSRV.h"


int load_texture(GLuint tex, int width, int height, int textureType, void* data)
{
    GLenum target = GL_TEXTURE_2D;
    GLint param = GL_NEAREST;

    if ((textureType == GL_RGB) || (textureType == GL_RGBA)) {
        target = GL_TEXTURE_2D;
        param = GL_NEAREST;

        glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            textureType,
            width,
            height,
            0,
            textureType,
            GL_UNSIGNED_BYTE,//textureFormat,
            data
            );
        GL_CHECK(glTexImage2D);
    } else {
        printf("Incorrect texture type %x\n", textureType);
        return -1;
    }

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, param); //GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, param); //GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    GL_CHECK(glTexParameteri);
    return 0;
}

int load_texture_from_raw_file(GLuint tex, int width, int height, int textureType, char* filename)
{
    int ret;
    void* data;
    FILE* fp;
    int dataread;
    int numbytes = 0;
    int bmp_offset= 0;
    //open and load raw file

    switch (textureType) {
        case GL_RGBA:
            numbytes = 4 * width * height;
            bmp_offset = 138;
            break;

        case GL_RGB:
            numbytes = 3 * width * height;
            bmp_offset = 0;
            break;


        default:
            printf("Invalid texture type %d\n", textureType);
            return -1;
    }

    fp = fopen(filename, "rb");
    if(!fp)
    {
        printf("Error opening file %s\n", filename);
        return -1;
    }

    data = malloc(numbytes);
    fseek(fp, bmp_offset, SEEK_CUR);
    dataread = fread(data, 1, numbytes, fp);
    fclose(fp);
    if(dataread != numbytes) {
        printf("Error in file size != width*height\n");
        return -1; //TODO: reinstate this
    }

    ret = load_texture(tex, width, height, textureType, data);

    free(data);

    return ret;
}




