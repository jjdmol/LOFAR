/*****************************************************************************/
/*                                                                           */
/* File       : dbgtrace.c                                                   */
/* Start date : 2000-11-08                                                   */
/* Author     : AU-System Lars Eberson (lars.eberson@ausys.se)               */
/* Version    : 0.0                                                          */
/* Description: Contains a system global char buffer for debug outputs.      */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#ifdef WIN32
#include <windows.h>
#else
#include <stdio.h>
#include <pthread.h>
#endif

/*****************************************************************************/
/*                                                                           */
/* Type definitions                                                          */
/*                                                                           */
/*****************************************************************************/
#define ATD_SOCKET_MUTEX          ( (char*) "TSE DEBUGGER Mutex")


/*****************************************************************************/
/*                                                                           */
/* Constants                                                                 */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/* External references                                                       */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/* Variables                                                                 */
/*                                                                           */
/*****************************************************************************/
char chDbgBuf[8192];
static FILE *pFile = NULL;
static pthread_mutex_t  tDebuggerMutex  = PTHREAD_MUTEX_INITIALIZER;

/*****************************************************************************/
/*                                                                           */
/* Functions                                                                 */
/*                                                                           */
/*****************************************************************************/

void StartDebugger()
{
  pFile = fopen("debug.log","w");
}
/*****************************************************************************/
/*                                                                           */
/* Function    :                                                             */
/*                                                                           */
/* Description :                                                             */
/*                                                                           */
/* Input       :                                                             */
/*                                                                           */
/* Output      :                                                             */
/*                                                                           */
/* Globals     :                                                             */
/*                                                                           */
/* Returns     :                                                             */
/*                                                                           */
/* Comment     :                                                             */
/*                                                                           */
/*****************************************************************************/
void OutputDebugString(char *pcString)
{
  pthread_mutex_lock( &tDebuggerMutex );
  if (pFile == NULL)
  {
    pFile = fopen("debug.log","w");
  } 
  if (pFile != NULL)
  {
    fprintf(pFile, pcString);
    fprintf(pFile, "\n");
    fflush(pFile);
  }
  pthread_mutex_unlock( &tDebuggerMutex );
}

void StopDebugger()
{
  pthread_mutex_lock( &tDebuggerMutex );
  if (pFile != NULL)
  {
    fflush(pFile);
    fclose(pFile);
    pFile = NULL;
  }
  pthread_mutex_unlock( &tDebuggerMutex );
  pthread_mutex_destroy( &tDebuggerMutex );
}
