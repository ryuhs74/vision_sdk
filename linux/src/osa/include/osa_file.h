

#ifndef _OSA_FILE_H_
#define _OSA_FILE_H_

#include <osa.h>

int OSA_fileReadFile(char *fileName, Uint8 *addr, Uint32 readSize, Uint32 *actualReadSize);
int OSA_fileWriteFile(char *fileName, Uint8 *addr, Uint32 size);
int OSA_fileReadFileOffset(char *fileName, Uint8 *addr, Uint32 readSize, Uint32 *actualReadSize, UInt32 offset);
int OSA_fileWriteFileOffset(char *fileName, Uint8 *addr, Uint32 size, UInt32 offset);
int OSA_fileCreateFile(char *fileName, Uint32 size, Bool force);

#endif /* _OSA_FILE_H_ */



