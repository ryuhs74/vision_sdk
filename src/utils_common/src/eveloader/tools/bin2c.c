/*
 *******************************************************************************
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
  ******************************************************************************
 * \file bin2c.c
 *
 * \brief  This file implements logic to convert binariy file to hex so that it
 *         can be included in to another c file as header.
 *
 *
 * \version 0.0 (2001) : [KC] First version
 *
 *******************************************************************************
 */

#include <util.h>

#define MAX_BUFFER_SIZE (1*KB*KB)

Uint8 buffer[MAX_BUFFER_SIZE];

STATUS bin2c( Uint8 *inName) {
  STATUS status=E_PASS;
    Uint32 bytes=1, size, i, csize;
    FILE* fin;
  Uint32  chunkSize = MAX_BUFFER_SIZE;

  fprintf(stderr, "\r\n Converting binary file [%s] to C array ", inName );

    fin = fopen( inName, "rb");
    if(fin==NULL) {
    fprintf(stderr, "\r\n ERROR: Input file [%s] not found", inName);
    status = E_DEVICE;
    goto error_stop;
  }

    size=0;
        csize=0;
    while(bytes) {
        bytes = fread(buffer, 1, chunkSize, fin );
    fprintf(stderr, ".");
    for(i=0;i<bytes;i++) {
      if(i%(3*30)==0)
        printf("\n");
        printf("0x%02x,", buffer[i]);
        csize++;
    }
        size +=bytes;
    }
  printf("\n// %ld bytes \r\n", csize );
  fprintf(stderr, " Done. (%ld bytes)\r\n", size);

  if(csize!=size) {
    fprintf(stderr, "\n ERROR: Check output file (byte diff %ld)", size-csize);
  }
error_stop:
    fclose(fin);

    return status;
}

void main(int argc, char **argv) {

  if(argc!=2) {
    printf("\r\n USAGE: bin2c <binary file name> \n" );
    exit(0);
  }

  bin2c(argv[1]);
}
