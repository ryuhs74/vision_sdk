

#ifndef _OSA_MEM_H_
#define _OSA_MEM_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


#include <osa_mutex.h>
#include <osa_mem_map.h>


typedef enum
{
  OSA_MEM_REGION_TYPE_SR0,
  OSA_MEM_REGION_TYPE_SR1,
  OSA_MEM_REGION_TYPE_REMOTE_LOG,
  OSA_MEM_REGION_TYPE_SYSTEM_IPC,
  OSA_MEM_REGION_TYPE_AUTO,
  OSA_MEM_REGION_TYPE_INVALID,
  OSA_MEM_REGION_TYPE_MAX

} OSA_MemRegionType;

typedef enum
{
    OSA_HEAPID_DDR_NON_CACHED_SR0 = SYSTEM_HEAPID_DDR_NON_CACHED_SR0,
    OSA_HEAPID_DDR_CACHED_SR1     = SYSTEM_HEAPID_DDR_CACHED_SR1,
    OSA_HEAPID_MAXNUMHEAPS
} Osa_HeapId;

typedef struct
{
  unsigned int physAddr;
  unsigned int virtAddr;
  unsigned int size;
} OSA_MemRegion;

int OSA_memInit();
int OSA_memDeInit();

unsigned int OSA_memMap(unsigned int physAddr, unsigned int size);
int          OSA_memUnMap(unsigned int virtAddr, unsigned int size);

unsigned int OSA_memPhys2Virt(unsigned int physAddr, OSA_MemRegionType type);
unsigned int OSA_memVirt2Phys(unsigned int virtAddr, OSA_MemRegionType type);

unsigned int OSA_memOffset2Virt(unsigned int offset, OSA_MemRegionType type);
unsigned int OSA_memVirt2Offset(unsigned int virtAddr, OSA_MemRegionType type);

unsigned int OSA_memPhys2Offset(unsigned int physAddr, OSA_MemRegionType type);
unsigned int OSA_memOffset2Phys(unsigned int offset, OSA_MemRegionType type);

unsigned int OSA_memMapHeapIdToRegionId(Osa_HeapId heapId);

int OSA_memCacheInv(unsigned int virtAddr, unsigned int length);

#endif /* _OSA_MEM_H_ */




