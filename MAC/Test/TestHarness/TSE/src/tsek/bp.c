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
// Description   : This file contains the main functionality of the batch processor.
//
// Revisions:
//
// Date       Author                  Changes
// 09/02/2001 Widjai Lila             Initial release
//
//=========================================================================
*/

#include <stdio.h>
#include <stdlib.h>             /* Memory allocation                        */
#include <string.h>             /* String compare                           */
#include <unistd.h>
#include <errno.h>

#include "bp.h"                 /* Exported header file                     */
#include "bp_admin.h"           /* Administration of the module             */
#include "bp_file.h"            /* File reading and converting functions    */
#include "bp_log.h"             /* Status logging of the batch processor    */

#include "general_lib.h"
#include "parser.h"             /* Script parser                            */
#include "executor_master.h"    /* Script executor                          */
#include "atkernel_intern.h"    /* Kernel internal routines                 */
#include "atkernel.h"           /* Error messages                           */
#include "parser_lib.h"         /* Error messages                           */

/*-----------------------------------------------------------------------------*/
/* Definitions                                                                 */
/*-----------------------------------------------------------------------------*/
#define MAX_REASON_LENGTH 20
/*-----------------------------------------------------------------------------*/
/* Global variables                                                            */
/*-----------------------------------------------------------------------------*/

TAdmin    _tAdmin;              /* Global administration         */
int8      cSemaphore = 1;       /* Semaphore for synchronization */

/*-----------------------------------------------------------------------------*/
/* Local function prototypes                                                   */                                 
/*-----------------------------------------------------------------------------*/
int16     RunScript(
  void);
void      FirstScript(
  int16 iReplay);
int16     ConvertReturnValue(
  int16 iValue);
void      ProcessFlowControl(
  char *pcReason);

/*-----------------------------------------------------------------------------*/
/* External accessible functions                                               */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Function     :   BP_Init                                                    */
/* Description  :   This function initializes the administration. This function*/
/*                  MUST be called at startup of BSE.                          */
/* Parameter    :   -                                                          */
/* Return value :   -                                                          */
/*-----------------------------------------------------------------------------*/
void BP_Init(
  void)
{
  /* Set all variables to default */
  _tAdmin.pcBatchFile = NULL;
  _tAdmin.pcSpecFile = NULL;
  _tAdmin.ptStrScript = NULL;
  _tAdmin.ptNxtScript = NULL;
  _tAdmin.bRunning = FALSE;
  _tAdmin.bStopBatch = FALSE;

  /* Initialization of the script status logging */
  bp_InitScriptStatus();
}

/*-----------------------------------------------------------------------------*/
/* Function     :   BP_LoadBatch                                               */
/* Description  :   Stores the batch file in the administration and calls the  */
/*                  converting rountine.                                       */
/* Parameter    :   pcFileName, batch filename                                 */
/* Return value :   Return values specified in                                 */
/*-----------------------------------------------------------------------------*/
int16 BP_LoadBatch(
  int8 * pcFileName)
{
  int16         iStatus         = BP_OK;
  TScriptFiles *ptTmpScriptFile = NULL;

  /* If there was a batch file name, delete it */
  if (_tAdmin.pcBatchFile != NULL)
    free((int8 *) _tAdmin.pcBatchFile);

  /* Store the batch filename */
  _tAdmin.pcBatchFile = my_strdup((int8 *) pcFileName);

  /* If a batch file was loaded, clear the file list */
  if (_tAdmin.ptStrScript != NULL)
  {
    /* Free the previous allocated memory */
    bp_FreeMemFileList((TScriptFiles *) _tAdmin.ptStrScript);
    bp_FreeMemScriptStatus();
  }

  /* Read the batch file */
  iStatus =
    bp_LoadBatch(_tAdmin.pcBatchFile,
                 (TScriptFiles **) & (_tAdmin.ptStrScript));

  if (iStatus == BP_OK)
  {
    /* No errors in the batch file */

    /* Store the specification filename */
    if (_tAdmin.pcSpecFile != NULL)
      free(_tAdmin.pcSpecFile);
    _tAdmin.pcSpecFile = _tAdmin.ptStrScript->pcName;

    /* Store the first list item */
    ptTmpScriptFile = _tAdmin.ptStrScript;

    /* Only the script file are in the files admin */
    _tAdmin.ptStrScript = (TScriptFiles *) _tAdmin.ptStrScript->ptNext;

    /* And now free the first item */
    free(ptTmpScriptFile);

    if (_tAdmin.ptStrScript == NULL)
    {
      /* No scripts in the batch file */
      AddMsg1("No script in batch");

      internal_BatchStopped(my_strdup("Batch error"));

      iStatus = BP_NO_SCRIPTS_IN_BATCH;
    }

    else if (strcmp(_tAdmin.ptStrScript->pcName, "end.") == 0)
    {
      /* An end. ! */
      /* This can only happen when there is no specification or script file */
      AddMsg1("No script or specification file");

      internal_BatchStopped(my_strdup("Batch error"));

      iStatus = BP_NO_SCRIPTS_IN_BATCH;
    }

    else
    {
      /* Parse the specification file */
      iStatus = Parse_Specification_File((int8 *) _tAdmin.pcSpecFile);

      /* Convert the return value of the parser */
      iStatus = ConvertReturnValue(iStatus);
    }
  }

  else
  {
    if (iStatus == BP_BATCH_NO_END)
    {
      /* No "end." in the batch file */
      AddMsg1("Batch contains no 'end.' ");
    }

  }

  return iStatus;
}

/*-----------------------------------------------------------------------------*/
/* Function     : BP_RunBatch                                                  */
/* Description  : This function starts the execution of the script             */
/* Parameter    : -                                                            */
/* Return value : Return values specified in                                   */
/*-----------------------------------------------------------------------------*/
int16 BP_RunBatch(
  int16 iReplay)
{
  int16          iStatus = BP_OK;
  struct timeval tNow;

  /* Set to default again */
  _tAdmin.bRunning = FALSE;
  _tAdmin.bStopBatch = FALSE;

  /* Use the first script */
  gettimeofday( &tNow, NULL);
  lSeedValue = tNow.tv_sec + tNow.tv_usec;
  /* This function opens/creates the batch log file */
  FirstScript(iReplay);
  srandom(lSeedValue);
  /* There are scripts in the batch file, so continue */
  ProcessFlowControl("");
  iStatus = RunScript();

  return iStatus;
}

/*-----------------------------------------------------------------------------*/
/* Function     : BP_StopBatch                                                 */
/* Description  : This function stops processing the batch file                */
/* Parameter    : -                                                            */
/* Return value : -                                                            */
/*-----------------------------------------------------------------------------*/
void BP_StopBatch(
  int8 * pcReason)
{
  
  while (cSemaphore <= 0)
  {
    usleep(100000);
  }
  cSemaphore--;

  _tAdmin.bStopBatch = TRUE;
  _tAdmin.bRunning = FALSE;

  /* Update the status */
  bp_ScriptStatus(CHANGE_STATUS, _tAdmin.ptNxtScript->pcName,
                  (int8 *) USER_INTERRUPTION);

  /* Stop the executor */
  StopExecutor((int8 *) "User Interruption");

  while (ExecutorStopped() == EX_THREAD_BUSY)
  {
    usleep(500000);
  }
  
  /* Stop the batch logging */
  bp_StopFileLogging("Batch stopped by user");

  /* Let the user know that we stopped and why */
  internal_BatchStopped((int8 *) my_strdup("User Interruption"));

  cSemaphore++;

  bp_UpdateLogging();

}

/*-----------------------------------------------------------------------------*/
/* Function     : BP_ScriptStopped                                             */
/* Description  : Runs the next script                                         */
/* Parameter    : Reason why it stopped                                        */
/* Return value : -                                                            */
/*-----------------------------------------------------------------------------*/
void BP_ScriptStopped(
  int8 * pcReason)
{
  int16     iStatus = BP_OK;
  
  while (cSemaphore <= 0)
  {
    usleep(100000);
  }

  cSemaphore--;

  if (_tAdmin.bStopBatch == FALSE)
  {
    /* limit the number of characters */
    if (strlen((int8 *) pcReason) > MAX_REASON_LENGTH)
    {
      pcReason[MAX_REASON_LENGTH] = 0;
      pcReason[MAX_REASON_LENGTH - 1] = '~';
    }

    /* Update script status */
    bp_ScriptStatus(CHANGE_STATUS, (int8 *) _tAdmin.ptNxtScript->pcName,
                    (int8 *) pcReason);

    /* Point to the next script */
    _tAdmin.ptNxtScript = (TScriptFiles *) _tAdmin.ptNxtScript->ptNext;

    ProcessFlowControl(pcReason);

    /* Execute next script */
    iStatus = RunScript();

    if (iStatus == BP_ALL_SCRIPTS_PROCESSED)
    {
      /* Not running anymore */
      _tAdmin.bRunning = FALSE;

      /* Stop the batch logging */
      bp_StopFileLogging("Batch finished");

      /* Notify the master that the batch is finished */
      internal_BatchStopped((int8 *) pcReason);

    }
  }

  cSemaphore++;

  bp_UpdateLogging();
  Update_Overview();

}

/*-----------------------------------------------------------------------------*/
/* Function     : BP_UpdateLogging                                             */
/* Description  : Updates the batch logging in the overview window             */
/* Parameter    : -                                                            */
/* Return value : -                                                            */
/*-----------------------------------------------------------------------------*/
void BP_UpdateLogging(
  void)
{

  bp_UpdateLogging();

}

/*-----------------------------------------------------------------------------*/
/* Function     : BP_Finalize                                                  */
/* Description  : Free allocated memory                                        */
/* Parameter    : -                                                            */
/* Return value : -                                                            */
/*-----------------------------------------------------------------------------*/
void BP_Finalize(
  void)
{

  /* Free the file list */
  bp_FreeMemFileList((TScriptFiles *) _tAdmin.ptStrScript);

  /* Free the allocated memory of the other varaibles */
  if (_tAdmin.pcSpecFile != NULL)
    free((int8 *) _tAdmin.pcSpecFile);
  if (_tAdmin.pcBatchFile != NULL)
    free((int8 *) _tAdmin.pcBatchFile);

  /* Free the log list */
  bp_FreeMemScriptStatus();

  /* Close the log file */
  bp_StopFileLogging(NULL);

}


/*-----------------------------------------------------------------------------*/
/* Local functions                                                             */
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/
/* Function     : RunScript                                                    */
/* Description  : Sends the script file to the parser and starts the executor. */
/* Parameter    : -                                                            */
/* Return value :                                                              */
/*-----------------------------------------------------------------------------*/
int16 RunScript(
  void)
{
  int16     iStatus = BP_OK;
  int8     *pcLogFile = NULL;

  if (_tAdmin.ptStrScript != NULL)
  {
    /* A batch file is loaded */

    if ((strcmp(_tAdmin.ptNxtScript->pcName, "end.") != 0))
    {

      /* Parse the script file */
      iStatus = (int16) Parse_Script_File((int8 *) _tAdmin.ptNxtScript->pcName);

      /* Convert the return value of the parser */
      iStatus = ConvertReturnValue(iStatus);

      if (iStatus == BP_OK)
      {
        /* Create the script log file */
        pcLogFile =
          (int8 *) bp_CreateLogFileName((int8 *) _tAdmin.ptNxtScript->pcName);
        /* A test script is not replayed a batch is, so all scripts in the batch uses */
        /* the same seed value */   
        iStatus = BSEK_OpenLogFile((int8 *) pcLogFile, 0);
        if (pcLogFile != NULL)
          free((int8 *) pcLogFile);

        /* Execute this script */
        iStatus = StartExecutor();

        /* Convert the return value of the parser */
        iStatus = ConvertReturnValue((int16) iStatus);

        if (iStatus == BP_OK)
        {
          /* Update processing status */
          bp_ScriptStatus((int16) NEW_ITEM,
                          (int8 *) _tAdmin.ptNxtScript->pcName,
                          (int8 *) BP_RUNNING);
        }

      }

      else
      {
        if (iStatus == BP_FILE_CANNOT_BE_OPENED)
        {
          /* The script couldn't be opened */
          AddMsg2("Script %s not found", _tAdmin.ptNxtScript->pcName);
        }

        /* Error while parsing the script file */
        internal_BatchStopped((int8 *) my_strdup("Script error"));

      }
    }

    else
    {
      /* This is the "end." command */
      iStatus = BP_ALL_SCRIPTS_PROCESSED;
    }

  }

  return iStatus;
}

/*-----------------------------------------------------------------------------*/
/* Function     : FirstScript                                                  */
/* Description  : Retreives the first script from the list and checks if there */
/*                are scripts in the batch file.                               */
/* Parameter    : -                                                            */
/* Return value : Return values specified in                                   */
/*-----------------------------------------------------------------------------*/
void FirstScript(
  int16 iReplay)
{

  /* Start with the first script file */
  _tAdmin.ptNxtScript = _tAdmin.ptStrScript;
  _tAdmin.bRunning = TRUE;

  /* Start the batch logging */
  bp_StartFileLogging((int8 *) _tAdmin.pcBatchFile, iReplay);

  /* Free the possible previous allocated memory */
  bp_FreeMemScriptStatus();

}

/*-----------------------------------------------------------------------------*/
/* Function     : CovertReturnValue                                            */
/* Description  : Converts ATK return values into BP values.                   */
/* Parameter    : ATK return value                                             */
/* Return value : BP return value                                              */
/*-----------------------------------------------------------------------------*/
int16 ConvertReturnValue(
  int16 iValue)
{
  switch (iValue)
  {
    case BSEK_OK:
      iValue = BP_OK;
      break;
    case BSEK_FILE_CANNOT_BE_OPENED:
      iValue = BP_FILE_CANNOT_BE_OPENED;
      break;
    case BSEK_SCRIPT_ERROR:
      iValue = BP_SCRIPT_ERROR;
      break;
    case BSEK_NO_SPECIFICATION:
      iValue = BP_NO_SPECIFICATION;
      break;
    case BSEK_KERNEL_BUSY:
      iValue = BP_KERNEL_BUSY;
      break;
    default:
      iValue = BP_UNDEFINED;
      break;
  }
  return iValue;
}


void ProcessFlowControl(
  char *pcReason)
{
  char     *pcFlowCommand;
  char     *pc;
  char     *pcCondition;

  struct TScriptFiles *pt;

  int       n;


  if (_tAdmin.ptStrScript != NULL)      /* If a script is loaded.. */
  {

    if (_tAdmin.ptNxtScript->pcName[0] == ':')  /* If flow control is used here */
    {
      pcFlowCommand = my_strdup((_tAdmin.ptNxtScript->pcName) + 1);

      /* Now replace the first space with a '\0' character. */
      pc = pcFlowCommand;
      while ((*pc) && (*pc != ' '))
        pc++;
      if (*pc == 0)
      {                         /* Just a label. Ignore line and continue. */
        _tAdmin.ptNxtScript = _tAdmin.ptNxtScript->ptNext;
        ProcessFlowControl(pcReason);
        free(pcFlowCommand);
      }


      (*pc) = 0;

      if (strcmp(pcFlowCommand, "if") == 0)
      {
        pcCondition = pc + 1;
        while (*pcCondition && *pcCondition != '"')
          pcCondition++;        /* Opening " found */
        if (*pcCondition == 0)
        {
          free(pcFlowCommand);
          return;               /* On error we don't do anything.. */
        }
        pcCondition++;
        pc = pcCondition;
        while (*pc && *pc != '"')
          pc++;                 /* Closing " found */
        if (*pc == 0)
        {
          free(pcFlowCommand);
          return;               /* On error we don't do anything.. */
        }
        *pc = 0;
        if (strcmp(pcCondition, pcReason) == 0) /* Condition is matching */
        {
          pcCondition = pc + 1;
          while (*pcCondition && *pcCondition == ' ')
            pcCondition++;
          /* pcCondition is now pointing to the label we're looking for. */

          pcCondition--;
          *pcCondition = ':';   /* This is useful in looking for the label. */
          n = strlen(pcCondition);

          pt = _tAdmin.ptStrScript;
          while (pt && strncmp(pt->pcName, pcCondition, n) != 0)
          {
            pt = pt->ptNext;
          }
          if (pt == NULL)
          {
            free(pcFlowCommand);
            return;             /* On error we don't do anything.. */
          }
          /* Label is found. !! */
          _tAdmin.ptNxtScript = pt->ptNext;
          ProcessFlowControl(pcReason);
          free(pcFlowCommand);
          /* Ready :-) */
        }
        else                    /* condition is not matching. Ignore line and continue. */
        {
          _tAdmin.ptNxtScript = _tAdmin.ptNxtScript->ptNext;
          ProcessFlowControl(pcReason);
          free(pcFlowCommand);
        }

      }
      else if (strcmp(pcFlowCommand, "goto") == 0)
      {
        pcCondition = pc + 1;
        while (*pcCondition && *pcCondition == ' ')
          pcCondition++;
        /* pcCondition is now pointing to the label we're looking for. */

        pcCondition--;
        *pcCondition = ':';     /* This is useful in looking for the label. */
        n = strlen(pcCondition);

        pt = _tAdmin.ptStrScript;
        while (pt && strncmp(pt->pcName, pcCondition, n) != 0)
        {
          pt = pt->ptNext;
        }
        if (pt == NULL)
        {
          free(pcFlowCommand);
          return;               /* On error we don't do anything.. */
        }
        /* Label is found. !! */
        _tAdmin.ptNxtScript = pt->ptNext;
        ProcessFlowControl(pcReason);
        free(pcFlowCommand);
        /* Ready :-) */
      }
    }
  }
}
