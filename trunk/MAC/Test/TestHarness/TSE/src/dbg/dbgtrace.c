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
#endif

/*****************************************************************************/
/*                                                                           */
/* Type definitions                                                          */
/*                                                                           */
/*****************************************************************************/


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
char chDbgBuf[2048];
static FILE *pFile = NULL;

/*****************************************************************************/
/*                                                                           */
/* Functions                                                                 */
/*                                                                           */
/*****************************************************************************/


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
  if (pFile == NULL)
  {
    pFile = fopen("debug.log","w");
  } 
  fprintf(pFile, pcString);
  fprintf(pFile, "\n");
  fflush(pFile);
}
