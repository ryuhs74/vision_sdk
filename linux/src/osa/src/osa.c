
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include <osa.h>
#include <osa_mem.h>
#include <osa_remote_log_if.h>

Void OSA_signalHandler_SIGSEGV( Int32 signum, siginfo_t* sig_info, Void* context);

int OSA_init()
{
    int status;
    struct sigaction sa;

    sa.sa_sigaction = (Void *)OSA_signalHandler_SIGSEGV;
    sigemptyset (&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);

    status = OSA_memInit();
    OSA_assertSuccess(status);

    status = OSA_globalTimerInit();
    OSA_assertSuccess(status);

    status = RemoteLog_init();
    OSA_assertSuccess(status);

    status = OSA_remoteLogClientInit();
    OSA_assertSuccess(status);

    return status;
}

int OSA_deInit()
{
    int status;

    status = OSA_globalTimerDeInit();
    OSA_assertSuccess(status);

    status = OSA_remoteLogClientDeInit();
    OSA_assertSuccess(status);

    status = OSA_memDeInit();
    OSA_assertSuccess(status);

    return status;
}

int OSA_attachSignalHandler(int sigId, void (*handler)(int ) )
{
  struct sigaction sigAction;

  /* insure a clean shutdown if user types ctrl-c */
  sigAction.sa_handler = handler;
  sigemptyset(&sigAction.sa_mask);
  sigAction.sa_flags = 0;
  sigaction(sigId, &sigAction, NULL);

  return OSA_SOK;
}

Uint32 OSA_getCurTimeInMsec()
{
  static int isInit = FALSE;
  static Uint32 initTime=0;
  struct timeval tv;

  if(isInit==FALSE)
  {
      isInit = TRUE;

      if (gettimeofday(&tv, NULL) < 0)
        return 0;

      initTime = (Uint32)(tv.tv_sec * 1000u + tv.tv_usec/1000u);
  }

  if (gettimeofday(&tv, NULL) < 0)
    return 0;

  return (Uint32)(tv.tv_sec * 1000u + tv.tv_usec/1000u)-initTime;
}

void OSA_waitMsecs(Uint32 msecs)
{
    struct timespec delayTime, remainTime;
    int ret;

    delayTime.tv_sec  = msecs/1000;
    delayTime.tv_nsec = (msecs%1000)*1000000;

    do
    {
        ret = nanosleep(&delayTime, &remainTime);
        if(ret < 0 && remainTime.tv_sec > 0 && remainTime.tv_nsec > 0)
        {
            /* restart for remaining time */
            delayTime = remainTime;
        }
        else
        {
            break;
        }
    } while(1);
}

static char xtod(char c) {
  if (c>='0' && c<='9') return c-'0';
  if (c>='A' && c<='F') return c-'A'+10;
  if (c>='a' && c<='f') return c-'a'+10;
  return c=0;        // not Hex digit
}

static int HextoDec(char *hex, int l)
{
  if (*hex==0)
    return(l);

  return HextoDec(hex+1, l*16+xtod(*hex)); // hex+1?
}

int xstrtoi(char *hex)      // hex string to integer
{
  return HextoDec(hex,0);
}



/**
 *******************************************************************************
 *
 * \brief Reset and init buffer skip context
 *
 * \param skipCtx           [OUT] Buffer skip context
 * \param inputFrameRate    [IN]  Expected input frame rate
 * \param outputFrameRate   [IN]  Required output frame rate
 *
 * \return None
 *
 *******************************************************************************
 */
Void OSA_resetSkipBufContext(OSA_BufSkipContext *skipCtx,
                               UInt32 inputFrameRate,
                               UInt32 outputFrameRate)
{
    skipCtx->firstTime = TRUE;
    skipCtx->inputFrameRate = inputFrameRate;
    skipCtx->outputFrameRate = outputFrameRate;
}

/**
 *******************************************************************************
 *
 * \brief This function tells the caller whether current buffer should be
 *        skipped or not
 *
 *        The function uses OSA_BufSkipContext to keep track of rate
 *        of incoming buffers and outgoing buffers and based on this tells
 *        if current buffer should be skipped (return TRUE) or not skipped
 *        i.e used by the application (return FALSE)
 *
 *        Application should call this function everytime a buffers is received
 *        or need to be output and this function will tell whether to skip
 *        this buffer or not
 *
 *        Application should make sure OSA_resetSkipBufContext() once before
 *        first buffer is received
 *
 *        NOTE, the OSA_BufSkipContext is just a counter, actuall buffer
 *        pointer need not be a input to this function.
 *
 * \param skipCtx           [IN/OUT] Buffer skip context
 *
 * \return TRUE, skip this buffer \n
 *         FALSE, no not skip this buffer
 *
 *******************************************************************************
 */
Bool OSA_doSkipBuf(OSA_BufSkipContext *skipCtx )
{
    /*
     * if the target bufferrate has changed, first time case
     * needs to be visited?
     */
    if(skipCtx->firstTime)
    {
        skipCtx->outCnt = 0;
        skipCtx->inCnt = 0;

        skipCtx->multipleCnt = skipCtx->inputFrameRate * skipCtx->outputFrameRate;
        skipCtx->firstTime = FALSE;
    }

    if (skipCtx->inCnt > skipCtx->outCnt)
    {
        skipCtx->outCnt += skipCtx->outputFrameRate;
        /* skip this frame, return true */
        return TRUE;
    }

    /* out will also be multiple */
    if (skipCtx->inCnt == skipCtx->multipleCnt)
    {
        /* reset to avoid overflow */
        skipCtx->inCnt = skipCtx->outCnt = 0;
    }

    skipCtx->inCnt += skipCtx->inputFrameRate;
    skipCtx->outCnt += skipCtx->outputFrameRate;

    /* display this frame, hence return false */
    return FALSE;
}

/**
 *******************************************************************************
 *
 * \brief   SIGSEGV signal handler
 *
 *          Prints details of backstrace in case of SIGSEGV generation
 *
 *******************************************************************************
*/
Void OSA_signalHandler_SIGSEGV( Int32 signum, siginfo_t* sig_info, Void* context)
{
    Void *str_array[16];
    Int32 size;
    Char **str = (Char **)NULL;
    Void *pnt = NULL;
    ucontext_t* uc = (ucontext_t*) context;
    Int32 i;

    printf("\n****** Segmentation fault caught ....\n");

    size = backtrace (str_array, 15);
    str = backtrace_symbols (str_array, size);
    pnt = (Void*) uc->uc_mcontext.arm_lr;
    str_array[1] = pnt;
    printf("Faulty address is %p, called from %p\n", sig_info->si_addr, pnt);

    printf ("Totally Obtained %zd stack frames. signal number =%d \n", size, signum);
    if(signum == SIGSEGV)
    {
        printf(" Signal number = %d, Signal errno = %d\n",
        sig_info->si_signo, sig_info->si_errno);

        switch(sig_info->si_code)
        {
            case 1: printf(" SI code = %d (Address not mapped to object)\n",
                                sig_info->si_code);
            break;
            case 2: printf(" SI code = %d (Invalid permissions for \
                                 mapped object)\n",sig_info->si_code);

            break;
            default: printf("SI code = %d (Unknown SI Code)\n",sig_info->si_code);
            break;
        }

        printf(" Fault addr = %p \n",sig_info->si_addr);
    }

    printf("[bt] Execution path:\n");

    for (i = 0; i < size; i++)
    {
        printf ("[bt] %s\n", str[i]);
    }
    exit(0);
}
