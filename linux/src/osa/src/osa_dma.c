
#include <osa.h>
#include <osa_dma.h>
#include <dev_memcache.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>


typedef struct {

  int fd;
  Bool isInitDone;

} OSA_DmaCtrl;

OSA_DmaCtrl gOSA_dmaCtrl = { -1, FALSE };

int OSA_dmaInit()
{
  char deviceName[20];

  if(!gOSA_dmaCtrl.isInitDone) {

    sprintf(deviceName, "/dev/%s", DMA_DRV_NAME);

    gOSA_dmaCtrl.fd = open(deviceName, O_RDWR);
    if(gOSA_dmaCtrl.fd<0)
      return OSA_EFAIL;

    gOSA_dmaCtrl.isInitDone = TRUE;
  }

  return OSA_SOK;
}

int OSA_dmaExit()
{
  gOSA_dmaCtrl.isInitDone = FALSE;
  return close(gOSA_dmaCtrl.fd);
}

Uint8 *OSA_dmaMapMem(Uint8 *physAddr, Uint32 size, Uint32 mapType)
{
    DMA_MmapPrm prm;
    Uint32 cmd;

    prm.physAddr = (unsigned int)physAddr;
    prm.size     = size;
    prm.mapType  = mapType;
    prm.virtAddr = 0;

    cmd = DMA_IOCTL_CMD_MAKE(DMA_CMD_MMAP);
    ioctl(gOSA_dmaCtrl.fd, cmd, &prm);

	if (prm.virtAddr==0)
  {
		OSA_ERROR(" OSA_dmaMapMem() failed !!!\n");
    return NULL;
  }

	prm.virtAddr = (UInt32)mmap(
	        (void	*)physAddr,
	        size,
					PROT_READ|PROT_WRITE|PROT_EXEC,MAP_SHARED,
					gOSA_dmaCtrl.fd,
					(unsigned int)physAddr
					);

    return (Uint8*)prm.virtAddr;
}

int OSA_dmaUnmapMem(Uint8 *virtAddr, Uint32 size)
{
  if(virtAddr)
    munmap((void*)virtAddr, size);

  return 0;
}

