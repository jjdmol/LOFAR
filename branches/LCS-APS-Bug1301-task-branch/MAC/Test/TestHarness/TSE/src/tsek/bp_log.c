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
// Description   : This module handles the batch processing status logging to the
//                 GUI.
//
// Revisions:
//
// Date       Author                  Changes
// 09/02/2001 Widjai Lila             Initial release
//
//=========================================================================
*/


/*------------------------------------------------------------------------------
 * Copyright (c) 2001 Ericsson Mobile Communications AB, Sweden.
 * Any reproduction without written permission is prohibited by law.
 *------------------------------------------------------------------------------
 * Produced by Ericsson EuroLab Netherlands BV, the Netherlands.
 *------------------------------------------------------------------------------
 * Component: Batch Processor
 * File     : bp_log.c
 * Version  : 1.0     feb 09, 2001                                        
 *
 * Author   : Widjai Lila
 *------------------------------------------------------------------------------
 * Description: This module handles the batch processing status logging to the
 *              GUI.
 *------------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "bp.h"
#include "bp_log.h"
#include "bsegui.h"
#include "atkernel.h"
#include "general_lib.h"
#include "stores.h"

/*------------------------------------------------------------------------------*/
/* Global adminstration                                                         */
/*------------------------------------------------------------------------------*/
typedef struct tStatus
{
  int8      acStartTime[9];     /* Start time of the script         */
  int8     *pcFileName;         /* Script filename                  */
  int8     *pcStatus;           /* Status of processing the script  */
  struct tStatus *ptNext;       /* Next script status               */
} TStatus;

TStatus  *_ptStrStatus,         /* Start of the linked list         */
         *_ptNxtStatus;         /* Next entry of the list           */

int32     _lBufferSize;         /* Buffer size of the logging to    */
                                /* be displayed                     */
FILE     *_ptLogFile;           /* Handle to the log file           */

/*------------------------------------------------------------------------------*/
/* Local functions                                                              */
/*------------------------------------------------------------------------------*/
void      UpdateStatus(
  void);
void      AddItemToFile(
  int8 * pcStartTime,
  int8 * pcFileName,
  int8 * pcStatus);
void      LogFileHeader(
  void);

/*------------------------------------------------------------------------------*/
/* Module internal functions                                                    */
/*------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Function     :   bp_InitScriptStatus                                        */
/* Description  :   Initialization of the list pointers.                       */
/* Parameter    :   -                                                          */
/* Return value :   -                                                          */
/*-----------------------------------------------------------------------------*/
void bp_InitScriptStatus(
  void)
{
  _ptStrStatus = NULL;
  _ptNxtStatus = NULL;
  _lBufferSize = 0;
}

/*-----------------------------------------------------------------------------*/
/* Function     :   bp_ScriptStatus                                            */
/* Description  :   Adds a new item to the list or changes the status.         */
/* Parameter    :   iCommand,   add or change an item                          */ 
/*                  pcFileName, script filename                                */
/*                  pcStatus,   current processing status                      */
/* Return value :   -                                                          */
/*-----------------------------------------------------------------------------*/
void bp_ScriptStatus(
  int16 iCommand,
  int8 * pcFileName,
  int8 * pcStatus)
{
  static struct tm *ptStartTime = NULL;
  struct timeval    tNow;

  switch (iCommand)
  {

    case NEW_ITEM:             /* Add an item */

      if (_ptStrStatus == NULL)
      {
        /* First entry */
        _ptStrStatus = _ptNxtStatus = (TStatus *) malloc(sizeof(TStatus));
        _ptNxtStatus->ptNext = NULL;
      }

      else
      {
        /* Next entry */
        _ptNxtStatus->ptNext = (TStatus *) malloc(sizeof(TStatus));
        _ptNxtStatus = (TStatus *) (_ptNxtStatus->ptNext);
        _ptNxtStatus->ptNext = NULL;
      }

      /* Allocate memory */
      _ptNxtStatus->pcFileName = (int8 *) my_strdup((int8 *) pcFileName);
      _ptNxtStatus->pcStatus = (int8 *) my_strdup((int8 *) pcStatus);

      /* Store the start time */
      gettimeofday( &tNow, NULL);
      ptStartTime = localtime( &tNow.tv_sec );
      strftime( _ptNxtStatus->acStartTime, 
                9, 
                "%H:%M:%S", 
                ptStartTime);
      

      /* New buffersize */
      _lBufferSize += 255;

      break;

    case CHANGE_STATUS:        /* Change the status */

      if (pcStatus != NULL)
      {
        /* Delete the previous status */
        free((int8 *) (_ptNxtStatus->pcStatus));

        /* Copy the new status */
        _ptNxtStatus->pcStatus = (int8 *) my_strdup((int8 *) pcStatus);

        /* Add to the logging file */
        AddItemToFile(_ptNxtStatus->acStartTime, pcFileName, pcStatus);
      }

      break;
  }

}

/*-----------------------------------------------------------------------------*/
/* Function     :   bp_FreeMemScriptStatus                                     */
/* Description  :   Free the allocate memory for the status list               */
/* Parameter    :   -                                                          */
/* Return value :   -                                                          */
/*-----------------------------------------------------------------------------*/
void bp_FreeMemScriptStatus(
  void)
{

  if (_ptStrStatus != NULL)
  {
    /* First entry */
    _ptNxtStatus = _ptStrStatus;

    while (_ptNxtStatus != NULL)
    {
      /* Walk through the list and free the memory */
      _ptStrStatus = (TStatus *) _ptStrStatus->ptNext;

      if (_ptNxtStatus->pcStatus != NULL)
        free((int8 *) _ptNxtStatus->pcStatus);

      if (_ptNxtStatus->pcFileName != NULL)
        free((int8 *) _ptNxtStatus->pcFileName);

      free((TStatus *) _ptNxtStatus);

      _ptNxtStatus = (TStatus *) _ptStrStatus;
    }

    /* Reset the buffer size */
    _lBufferSize = 0;

  }

}

/*-----------------------------------------------------------------------------*/
/* Function     : bp_UpdateLogging                                             */
/* Description  : Updates the batch logging in the overview window             */
/* Parameter    : -                                                            */
/* Return value : -                                                            */
/*-----------------------------------------------------------------------------*/
void bp_UpdateLogging(
  void)
{

  UpdateStatus();

}

/*-----------------------------------------------------------------------------*/
/* Function     :   bp_Message                                                 */
/* Description  :                                                              */
/* Parameter    :   pcString, text to be displayed in the log window.          */
/* Return value :   -                                                          */
/*-----------------------------------------------------------------------------*/
void bp_Message(
  int8 * pcString)
{

  /* Text to be displayed in de logging window */
  BSEG_LogLine((int8 *) my_strdup((int8 *) pcString));

}

/*-----------------------------------------------------------------------------*/
/* Function     :   bp_StartFileLogging                                        */
/* Description  :   Creates a batch log file                                   */
/* Parameter    :   FileName, the batch filename                               */
/*                  Replay - indicates if a batch file is to be replayed(1) or */ 
/*                  not(0).                                                    */
/* Return value :   Error codes specified in                                   */
/*-----------------------------------------------------------------------------*/
int16 bp_StartFileLogging(
  int8 * pcFileName,
  int16 iReplay)
{
  int8     *pcLogFileName = NULL;
  int16     iStatus = BP_OK;
  int       iReturnValue;
  long      lOrgSeedValue;
  
  pcLogFileName = bp_CreateLogFileName((int8 *) pcFileName);
  if (iReplay == 1)
  {
    GetSeedValue(pcLogFileName);
  }
  if ((_ptLogFile = fopen(pcLogFileName, "w+t")) == NULL)
  {
    /* Couldn't create the file */
    iStatus = BP_LOG_FILE_NOT_CREATED;

    /* Notify the user that no file could be created */
    bp_Message((int8 *) "Warning: Log file could not be created !");
  }

  else
  {
    /* Put the header in the Log file */
    LogFileHeader();
  }

  free(pcLogFileName);

  return iStatus;
}

/*-----------------------------------------------------------------------------*/
/* Function     :   bp_StopFileLogging                                         */
/* Description  :   Closes the log file                                        */
/* Parameter    :   pcReason, Reason why to stop logging                       */
/* Return value :   -                                                          */
/*-----------------------------------------------------------------------------*/
void bp_StopFileLogging(
  int8 * pcReason)
{

  if (_ptLogFile != NULL)
  {
    if ( (pcReason != NULL) && (*pcReason != 0))
    {
      fprintf(_ptLogFile, "\n\n%s", pcReason);
      fflush(_ptLogFile);
    }

    fclose(_ptLogFile);
    _ptLogFile = NULL;
  }

  else
  {
    /* Notify the user that no file was created */
    bp_Message((int8 *) "Warning: Log file was not created !");
  }

}

/*-----------------------------------------------------------------------------*/
/* Function     :   bp_CreateLogFileName                                       */
/* Description  :   Replace the current extension with the "log" extension and */
/*                  when no file extension is used it will be added.           */
/* Parameter    :   pcFilename, script filename                                */
/* Return value :   Filename with the log extension                            */
/*-----------------------------------------------------------------------------*/
int8     *bp_CreateLogFileName(
  int8 * pcFileName)
{
  int8     *pcLogFileName, *pcTempAddress, *pcAddress;

  /* Store the file name */
  pcLogFileName = my_strdup((int8 *) pcFileName);

  pcTempAddress = (int8 *) pcLogFileName;
  pcAddress = (int8 *) pcTempAddress;

  /* Find the last "\" */
  while (pcTempAddress != NULL)
  {
    pcTempAddress = (int8 *) strchr(pcTempAddress, '\\');

    if (pcTempAddress != NULL)
    {
      pcAddress = (int8 *) pcTempAddress;
      pcTempAddress++;
    }
  }

  /* Find the dot in the last part of the path */
  pcAddress = (int8 *) strrchr(pcAddress, '.');

  if (pcAddress != NULL)
    *pcAddress = '\0';

  /* add the "log" extension to the filename   */
  strcat((int8 *) pcLogFileName, ".log");

  return pcLogFileName;
}

/*-----------------------------------------------------------------------------*/
/* Function     :   UpdateStatus                                               */
/* Description  :   Sends the status of the script to the GUI.                 */
/* Parameter    :   -                                                          */
/* Return value :   -                                                          */
/*-----------------------------------------------------------------------------*/
void UpdateStatus(
  void)
{
  TStatus  *ptTmpStatus = NULL;
  int8      acString[300] = { 0 }, *pacText = NULL;

  if (_ptGlobals->iOverviewMode == BSEK_PROCESSED_FILES)
  {
    /* User selected the overview window */

    /* Start with the first entry */
    ptTmpStatus = _ptStrStatus;

    if (_ptStrStatus != NULL)
    {
      /* Allocate memory */
      pacText = (int8 *) malloc(sizeof(int8) * _lBufferSize);
      pacText[0] = 0;

      while (ptTmpStatus != NULL)
      {

        /* Fromatting of the string */
        if (ptTmpStatus->pcStatus != NULL)
        {
          sprintf(acString, "%-20s  %s\r\n", ptTmpStatus->pcStatus,
                  ptTmpStatus->pcFileName);
          strcat(pacText, acString);
        }

        /* Next status */
        ptTmpStatus = (TStatus *) ptTmpStatus->ptNext;
      }
      /* Display the text */
      BSEG_Overview((int8 *) pacText);
    }

  }

}

/*-----------------------------------------------------------------------------*/
/* Function     :   AddItemToFile                                              */
/* Description  :   Write the status and filename to the log file              */
/* Parameter    :   pcFileName, script filename                                */
/*                  pcStatus,   reason why the script stopped                  */
/* Return value :   -                                                          */
/*-----------------------------------------------------------------------------*/
void AddItemToFile(
  int8 * pcStartTime,
  int8 * pcFileName,
  int8 * pcStatus)
{

  if (pcStatus != NULL)
  {
    /* Can we add the script status to the log file ? */
    if ((strcmp(pcStatus, BP_RUNNING)) != 0)
    {
      /* Script isn't running anymore */
      if (_ptLogFile != NULL)
      {
        fprintf(_ptLogFile, "%s    %-20s  %s\r\n", pcStartTime, pcStatus,
                pcFileName);
      }
    }
  }

}

/*-----------------------------------------------------------------------------*/
/* Function     :   LogFileHeader                                              */
/* Description  :   Writes information in the header of the logfile            */
/* Return value :   -                                                          */
/*-----------------------------------------------------------------------------*/
void LogFileHeader(
  void)
{
  int16     iVersion;
  int8      acTime[9];
  static struct tm *ptStartTime = NULL;
  struct timeval    tNow;

  if (_ptLogFile != NULL)
  {
    /* Get the current time */
    gettimeofday( &tNow, NULL);
    ptStartTime = localtime( &tNow.tv_sec );
    strftime( acTime, 
              9, 
              "%H:%M:%S", 
              ptStartTime);

    /* Add header information to the log file */
    fprintf(_ptLogFile, "Script Engine\n");
    fprintf(_ptLogFile, "%s\n\n", BSEK_Version(&iVersion));
    fprintf(_ptLogFile, "Batch started at: %s\n\n", acTime);
    fprintf(_ptLogFile, "Started at  Status                Script\n");
    fprintf(_ptLogFile, "Seed: %ld\n",lSeedValue);
    fflush(_ptLogFile);
  }

}
