
#include <osa_mem.h>
#include <dev_memcache.h>
#include <sys/ioctl.h>


//#define OSA_DEBUG_MEM

typedef struct
{
  OSA_MemRegion memRegion[OSA_MEM_REGION_TYPE_MAX];
  unsigned int  non_cache_fd;
  unsigned int  cache_fd;
} OSA_Mem;

OSA_Mem gOsaMem;

unsigned int OSA_memMapFd(int fd, unsigned int physAddr, unsigned int size)
{
    unsigned int pageSize = getpagesize ();
    unsigned int taddr;
    unsigned int tsize;
    unsigned int virtAddr;

    /* Mapping this physical address to linux user space */
    taddr = physAddr;
    tsize = size;

    /* Align the physical address to page boundary */
    tsize = OSA_align(tsize + (taddr % pageSize), pageSize);
    taddr = OSA_floor(taddr, pageSize);

    virtAddr              = (unsigned int)mmap(0, tsize,
                          (PROT_READ | PROT_WRITE),
                          (MAP_SHARED), fd, taddr);

    if(virtAddr==0xFFFFFFFF)
        return 0;

    #ifdef OSA_DEBUG_MEM
    printf(" OSA: MEM: Mapped 0x%08x to 0x%08x of size 0x%08x (pagesize = 0x%04x)\n",
        taddr, virtAddr, tsize, pageSize
        );
    #endif

    return virtAddr + (physAddr % pageSize);
}

unsigned int OSA_memMap(unsigned int physAddr, unsigned int size)
{
    return OSA_memMapFd(gOsaMem.non_cache_fd, physAddr, size);
}

int OSA_memUnMap(unsigned int virtAddr, unsigned int size)
{
    int status=OSA_SOK;

    unsigned int pageSize = getpagesize ();
    unsigned int taddr;
    unsigned int tsize;

    taddr = virtAddr;
    tsize = size;

    tsize = OSA_align(tsize + (taddr % pageSize), pageSize);
    taddr = OSA_floor(taddr, pageSize);

    munmap((void *)taddr, tsize);

    #ifdef OSA_DEBUG_MEM
    printf(" OSA: MEM: Unmapped 0x%08x of size 0x%08x (pagesize = 0x%08x)\n",
        taddr, tsize, pageSize
        );
    #endif

    return status;
}

unsigned int OSA_memPhys2RegionType(unsigned int physAddr)
{
    OSA_MemRegion *pMemRegion;
    int i;

    for(i=0; i<OSA_MEM_REGION_TYPE_AUTO; i++)
    {
        pMemRegion = &gOsaMem.memRegion[i];

        if(physAddr >= pMemRegion->physAddr
                &&
            physAddr < (pMemRegion->physAddr + pMemRegion->size)
        )
        {
            return i;
        }
    }

    return OSA_MEM_REGION_TYPE_MAX;
}

unsigned int OSA_memVirt2RegionType(unsigned int virtAddr)
{
    OSA_MemRegion *pMemRegion;
    int i;

    for(i=0; i<OSA_MEM_REGION_TYPE_AUTO; i++)
    {
        pMemRegion = &gOsaMem.memRegion[i];

        if(virtAddr >= pMemRegion->virtAddr
                &&
            virtAddr < (pMemRegion->virtAddr + pMemRegion->size)
        )
        {
            return i;
        }
    }

    return OSA_MEM_REGION_TYPE_MAX;
}

unsigned int OSA_memPhys2Virt(unsigned int physAddr, OSA_MemRegionType type)
{
    OSA_MemRegion *pMemRegion;

    if(type==OSA_MEM_REGION_TYPE_AUTO)
    {
        type = OSA_memPhys2RegionType(physAddr);
    }

    if(type>=OSA_MEM_REGION_TYPE_MAX)
    {
        return 0;
    }

    pMemRegion = &gOsaMem.memRegion[type];

    if(physAddr < pMemRegion->physAddr
        ||
       physAddr >= (pMemRegion->physAddr + pMemRegion->size)
    )
    {
        return 0;
    }

    return pMemRegion->virtAddr + (physAddr - pMemRegion->physAddr);
}

unsigned int OSA_memVirt2Phys(unsigned int virtAddr, OSA_MemRegionType type)
{
    OSA_MemRegion *pMemRegion;

    if(type==OSA_MEM_REGION_TYPE_AUTO)
    {
        type = OSA_memVirt2RegionType(virtAddr);
    }

    if(type>=OSA_MEM_REGION_TYPE_MAX)
        return 0;

    pMemRegion = &gOsaMem.memRegion[type];

    if(virtAddr < pMemRegion->virtAddr
        ||
       virtAddr >= (pMemRegion->virtAddr + pMemRegion->size)
    )
    {
        return 0;
    }

    return pMemRegion->physAddr + (virtAddr - pMemRegion->virtAddr);
}

unsigned int OSA_memOffset2Virt(unsigned int offset, OSA_MemRegionType type)
{
    OSA_MemRegion *pMemRegion;

    if(type==OSA_MEM_REGION_TYPE_AUTO)
    {
        /* type cannot be AUTO for this API */
        OSA_assert(0);
    }

    if(type>=OSA_MEM_REGION_TYPE_MAX)
    {
        return 0;
    }

    pMemRegion = &gOsaMem.memRegion[type];

    if(offset >= pMemRegion->size)
    {
        return 0;
    }

    return pMemRegion->virtAddr + offset;
}

unsigned int OSA_memOffset2Phys(unsigned int offset, OSA_MemRegionType type)
{
    OSA_MemRegion *pMemRegion;

    if(type==OSA_MEM_REGION_TYPE_AUTO)
    {
        /* type cannot be AUTO for this API */
        OSA_assert(0);
    }

    if(type>=OSA_MEM_REGION_TYPE_MAX)
    {
        return 0;
    }

    pMemRegion = &gOsaMem.memRegion[type];

    if(offset >= pMemRegion->size)
    {
        return 0;
    }

    return pMemRegion->physAddr + offset;
}

unsigned int OSA_memVirt2Offset(unsigned int virtAddr, OSA_MemRegionType type)
{
    OSA_MemRegion *pMemRegion;

    if(type==OSA_MEM_REGION_TYPE_AUTO)
    {
        type = OSA_memVirt2RegionType(virtAddr);
    }

    if(type>=OSA_MEM_REGION_TYPE_MAX)
        return 0;

    pMemRegion = &gOsaMem.memRegion[type];

    if(virtAddr < pMemRegion->virtAddr
        ||
       virtAddr >= (pMemRegion->virtAddr + pMemRegion->size)
    )
    {
        return 0;
    }

    return (virtAddr - pMemRegion->virtAddr);
}

unsigned int OSA_memPhys2Offset(unsigned int physAddr, OSA_MemRegionType type)
{
    OSA_MemRegion *pMemRegion;

    if(type==OSA_MEM_REGION_TYPE_AUTO)
    {
        type = OSA_memPhys2RegionType(physAddr);
    }

    if(type>=OSA_MEM_REGION_TYPE_MAX)
    {
        return 0;
    }

    pMemRegion = &gOsaMem.memRegion[type];

    if(physAddr < pMemRegion->physAddr
        ||
       physAddr >= (pMemRegion->physAddr + pMemRegion->size)
    )
    {
        return 0;
    }

    return (physAddr - pMemRegion->physAddr);
}

unsigned int OSA_memMapHeapIdToRegionId(Osa_HeapId heapId)
{

    OSA_assert(heapId < OSA_HEAPID_MAXNUMHEAPS);

    if(heapId == OSA_HEAPID_DDR_NON_CACHED_SR0)
    {
        return OSA_MEM_REGION_TYPE_SR0;
    }
    else if (heapId == OSA_HEAPID_DDR_CACHED_SR1)
    {
        return OSA_MEM_REGION_TYPE_SR1;
    }
    else
    {
        printf(" OSA: MEM: WARNING: Invalid heap ID !!!\n");
        return OSA_MEM_REGION_TYPE_INVALID;
    }
}

int OSA_memCacheInv(unsigned int virtAddr, unsigned int length)
{
    Uint32 cmd;
    DMA_CacheInvPrm prm;
    int status;

    prm.size = length;
    prm.virtAddr = virtAddr;

    cmd = DMA_IOCTL_CMD_MAKE(DMA_CMD_CACHEINV);
    status = ioctl(gOsaMem.cache_fd, cmd, &prm);

    return status;
}

int OSA_memInit()
{
    int status=OSA_SOK;
    int i;
    OSA_MemRegion *pMemRegion;
    char deviceName[20];

    memset(&gOsaMem, 0, sizeof(OSA_Mem));

    gOsaMem.non_cache_fd = open("/dev/mem",O_RDWR|O_SYNC);
    if(gOsaMem.non_cache_fd  < 0)
    {
        printf(" OSA: ERROR: /dev/mem open failed !!!\n");
        return -1;
    }

    sprintf(deviceName, "%s", DMA_DRV_NAME);

    gOsaMem.cache_fd = open(deviceName, O_RDWR);
    if(gOsaMem.cache_fd<0)
    {
        printf(" OSA: ERROR: %s open failed !!!\n", deviceName);
        return OSA_EFAIL;
    }

    /* Map required sections upfront */

    i = OSA_MEM_REGION_TYPE_SR0;
    pMemRegion = &gOsaMem.memRegion[i];

    pMemRegion->physAddr = SR0_ADDR;
    pMemRegion->size = SR0_SIZE;

    pMemRegion->virtAddr =
        OSA_memMap(pMemRegion->physAddr, pMemRegion->size);

    OSA_assert(pMemRegion->virtAddr != 0);

    i = OSA_MEM_REGION_TYPE_SR1;
    pMemRegion = &gOsaMem.memRegion[i];

    pMemRegion->physAddr = SR1_FRAME_BUFFER_MEM_ADDR;
    pMemRegion->size = SR1_FRAME_BUFFER_MEM_SIZE;

    pMemRegion->virtAddr =
        OSA_memMapFd(gOsaMem.cache_fd, pMemRegion->physAddr, pMemRegion->size);

    OSA_assert(pMemRegion->virtAddr != 0);

    i = OSA_MEM_REGION_TYPE_SYSTEM_IPC;
    pMemRegion = &gOsaMem.memRegion[i];

    pMemRegion->physAddr = SYSTEM_IPC_SHM_MEM_ADDR;
    pMemRegion->size = SYSTEM_IPC_SHM_MEM_SIZE;

    pMemRegion->virtAddr =
        OSA_memMap(pMemRegion->physAddr, pMemRegion->size);

    OSA_assert(pMemRegion->virtAddr != 0);

    i = OSA_MEM_REGION_TYPE_REMOTE_LOG;
    pMemRegion = &gOsaMem.memRegion[i];

    pMemRegion->physAddr = REMOTE_LOG_MEM_ADDR;
    pMemRegion->size = REMOTE_LOG_MEM_SIZE;

    pMemRegion->virtAddr =
        OSA_memMap(pMemRegion->physAddr, pMemRegion->size);

    OSA_assert(pMemRegion->virtAddr != 0);

    #if 1 //def OSA_DEBUG_MEM
    for(i=0; i<OSA_MEM_REGION_TYPE_AUTO; i++)
    {
        pMemRegion = &gOsaMem.memRegion[i];
        printf(" OSA: MEM: %d: Mapped 0x%08x to 0x%08x of size 0x%08x \n",
            i, pMemRegion->physAddr, pMemRegion->virtAddr, pMemRegion->size
            );
    }
    #endif

    return status;
}

int OSA_memDeInit()
{
    int status=OSA_SOK;
    int i;

    for(i=0; i<OSA_MEM_REGION_TYPE_AUTO; i++)
    {
        OSA_memUnMap(
            gOsaMem.memRegion[i].virtAddr,
            gOsaMem.memRegion[i].size
            );
    }

    close(gOsaMem.non_cache_fd);
    close(gOsaMem.cache_fd);

    return status;
}




