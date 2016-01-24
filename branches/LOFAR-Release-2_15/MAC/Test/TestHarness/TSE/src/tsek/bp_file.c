/*=========================================================================
// (c)   Ordina Technical Automation BV
//
// The Intellectual Property Rights of this source code file is not transferable.
// For information on this subject, contact Ordina Technical Automation B.V.
// Any attempt or participation in deciphering, decoding, reverse engineering or
// in any way altering the source code is strictly prohibited, unless the prior
// written consent of Ordina Technical Automation B.V. is obtained.
//
// Project       : Script Engine
// Module/Systeem: Script Engine
//
// --- Version Control ---
//    $RCSfile$
//    $Revision$
//    $Date$
// -----------------------
//
// Description   : This file contains the file reading and converting routines.
//                 Also batch file error checking is performed.
//
// Revisions:
//
// Date       Author                  Changes
// 09/02/2001 Widjai Lila             Initial release
//
//=========================================================================
*/


#include <stdio.h>              /* File access and read routines */
#include <stdlib.h>             /* Memory routines               */
#include <string.h>             /* String routines               */

#include "codingstandard.h"     /* Our codingstandard            */
#include "general_lib.h"
#include "bp.h"
#include "bp_file.h"            /* Batch processor               */
#include "bp_log.h"


#define  MAX_FILE_LENGTH    255 /* Maximum 255 characters for the filename */


/*-----------------------------------------------------------------------------*/
/* Global variables                                                            */ 
/*-----------------------------------------------------------------------------*/

FILE     *_ptFile;              /* Batch file handle */

/*-----------------------------------------------------------------------------*/
/* Local functions                                                             */
/*-----------------------------------------------------------------------------*/
int16     ReadLines(
  TScriptFiles **);
TScriptFiles *ReadFromFile(
  int16 *,
  int32 *);
TScriptFiles *CheckFile(
  int16 *,
  int32,
  TScriptFiles *);
void      FreeMemory(
  TScriptFiles *);

/*-----------------------------------------------------------------------------*/
/* Module exported functions                                                   */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Function     :   bp_LoadBatch                                               */
/* Description  :   This functions is used by the Batch Processor for reading  */
/*                  the content of the batch file                              */
/* Parameter    :   -                                                          */
/* Return value :   Returnvalues specified in ...                              */
/*-----------------------------------------------------------------------------*/
int16 bp_LoadBatch(
  int8 * pcString,
  TScriptFiles ** pptFiles)
{
  int16     iStatus = BP_OK;

  if ((_ptFile = fopen(pcString, "rt")) == NULL)
  {
    /* ERROR */
    iStatus = BP_FILE_CANNOT_BE_OPENED;
  }

  else
  {
    iStatus = ReadLines(pptFiles);

    if (fclose(_ptFile) == 0)
    {
      /* File couldn't be closed, what went wrong ?? */
    }
  }

  return iStatus;
}

/*-----------------------------------------------------------------------------*/
/* Function     :   FreeMemory                                                 */
/* Description  :   This functions frees the memory allocated for the linked   */
/*                  list.                                                      */
/* Parameter    :   Pointer to the linked list.                                */
/* Return value :   -                                                          */
/*-----------------------------------------------------------------------------*/
void bp_FreeMemFileList(
  TScriptFiles * ptFiles)
{
  TScriptFiles *ptNextFile = NULL;

  while (ptFiles != NULL)
  {
    ptNextFile = (TScriptFiles *) ptFiles->ptNext;

    if (ptFiles->pcName != NULL)
      free(ptFiles->pcName);

    free(ptFiles);

    ptFiles = (TScriptFiles *) ptNextFile;
  }

}

/*-----------------------------------------------------------------------------*/
/* Local functions                                                             */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Function     :   ReadLines                                                  */
/* Description  :   This functions handles reading and checking of the file.   */
/* Parameter    :   pptFiles, address where to store the data from the file.   */
/* Return value :   Returnvalues specified in ...                              */
/*-----------------------------------------------------------------------------*/
int16 ReadLines(
  TScriptFiles ** pptFiles)
{
  int16     iStatus = BP_OK;
  int32     lLinesRead = 0;
  TScriptFiles *ptTmpFile = NULL;

  /* Read the data */
  ptTmpFile = ReadFromFile(&iStatus, &lLinesRead);

  if (iStatus == BP_OK)
  {
    /* Checking and filtering of the batch file */
    *pptFiles = (TScriptFiles *) CheckFile(&iStatus, lLinesRead, ptTmpFile);

    /* Free the temporarily allocated memory */
    bp_FreeMemFileList(ptTmpFile);
  }

  return iStatus;
}

/*-----------------------------------------------------------------------------*/
/* Function     :   ReadFromFile                                               */
/* Description  :   This functions reads the data from the batch file.         */
/* Parameter    :   piStatus,    pointer where to store the status             */
/*                  piLinesRead, pointer where to store the number of lines    */
/*                   read.                                                     */
/* Return value :   Pointer to a linked list containing the data read.         */
/*-----------------------------------------------------------------------------*/
TScriptFiles *ReadFromFile(
  int16 * piStatus,
  int32 * plLinesRead)
{
  int8      acTemp[MAX_FILE_NAME];
  TScriptFiles *ptStrFile = NULL, *ptNxtFile = NULL;

  while (!feof(_ptFile))
  {
    if (fgets(acTemp, MAX_FILE_NAME, _ptFile) == NULL)
    {
      /* Reading failed, what is the reason */
      if (!feof(_ptFile))
      {
        /* Hmmm, not the end of the file (what went wrong)     */
        *piStatus = BP_FILE_READ_FAILURE;
      }

    }

    else
    {
      /* Store the text read */

      if (ptStrFile == NULL)
      {
        /* First entry */
        ptStrFile = ptNxtFile = (TScriptFiles *) malloc(sizeof(TScriptFiles));
        ptNxtFile->ptNext = NULL;

      }

      else
      {
        /* Next entry */
        ptNxtFile = ptNxtFile->ptNext =
          (TScriptFiles *) malloc(sizeof(TScriptFiles));
        ptNxtFile->ptNext = NULL;
      }

      ptNxtFile->pcName = (int8 *) my_strdup((int8 *) acTemp);

      (*plLinesRead)++;

    }

  } /* EndWhile */

  return (TScriptFiles *) ptStrFile;

}

/*-----------------------------------------------------------------------------*/
/* Function     :   CheckFile                                                  */
/* Description  :   This function filters comments and checks if the file ends */
/*                  with the end of batch command ("end.").                    */
/* Parameter    :   piStatus,   pointer where to store the status              */
/*                  iLinesRead, number of lines read from the file             */
/*                  ptTmpFile,  pointer to the linked list                     */
/* Return value :   Pointer to the filtered linked list.                       */
/*-----------------------------------------------------------------------------*/
TScriptFiles *CheckFile(
  int16 * piStatus,
  int32 lLinesRead,
  TScriptFiles * ptTmpFile)
{
  int8      bEndBatchFilePresent = FALSE;
  int32     i;
  TScriptFiles *ptStrFile = NULL, *ptNxtFile = NULL;
  int8     *pcAddress = NULL;

  for (i = 0; i < lLinesRead; i++)
  {
    /* Filter the comments (starting with ';' and '/') and empty lines */
    if ((*(ptTmpFile->pcName) != ';') && (*(ptTmpFile->pcName) != '/') &&
        (*(ptTmpFile->pcName) != '\n'))
    {
      /* Line contains no comments */

      /* Find the '\n' and replace it with the '\0' */
      pcAddress = strchr(ptTmpFile->pcName, '\n');
      if (pcAddress != NULL)
        *pcAddress = '\0';

      /* Allocate memory */
      if (ptStrFile == NULL)
      {
        /* First entry */
        ptStrFile = ptNxtFile = (TScriptFiles *) malloc(sizeof(TScriptFiles));
        ptNxtFile->ptNext = NULL;
      }
      else
      {
        /* Next entry */
        ptNxtFile->ptNext = (TScriptFiles *) malloc(sizeof(TScriptFiles));
        ptNxtFile = (TScriptFiles *) (ptNxtFile->ptNext);
        ptNxtFile->ptNext = NULL;
      }

      ptNxtFile->pcName = (int8 *) strdup(ptTmpFile->pcName);

      if (strcmp(ptNxtFile->pcName, "end.") == 0)
      {
        bEndBatchFilePresent = TRUE;
      }

    }

    else
    {
      if (ptTmpFile->pcName != NULL)
        free((int8 *) ptTmpFile->pcName);
      ptTmpFile->pcName = NULL;
    }

    ptTmpFile = (TScriptFiles *) ptTmpFile->ptNext;

  }

  /* Check batch file */
  if (bEndBatchFilePresent == FALSE)
  {
    *piStatus = BP_BATCH_NO_END;

#ifdef BP_DEBUG
    printf("\nNo end. in batch file");
#endif
  }

  return (TScriptFiles *) ptStrFile;

}
