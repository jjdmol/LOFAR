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
// Description   : 
//
// Revisions:
//
// Date       Author                  Changes
// 16/08/2000 E.A. Nijboer            Initial release
// 09/02/2001 E.A. Nijboer            
//
//=========================================================================
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "general_lib.h"

#include "atkernel.h"
#include "bsegui.h"
#include "atkernel_intern.h"
#include "atdriver.h"


#include "scanner.h"
#include "parser.h"
#include "parser_lib.h"
#include "executor_master.h"
#include "stores.h"
#include "expirator.h"
#include "eventreceiver.h"
#include "eventprocessor.h"

#include "bp.h"
#include "bp_log.h"

/* Only for test purposes: */
#include "teststuff.h"

/*****************************************************************************/
/*                                                                           */
/* Local macro's                                                             */
/*                                                                           */
/*****************************************************************************/

#define    BSEK_UNINITIALISED   ((int16) 0)
#define    BSEK_IDLE            ((int16) 1)
#define    BSEK_CONFIGURED      ((int16) 2)
#define    BSEK_BATCH_LOADED    ((int16) 3)
#define    BSEK_READY           ((int16) 4)
#define    BSEK_PAUSED          ((int16) 5)
#define    BSEK_RUNNING         ((int16) 6)
#define    BSEK_BATCH_RUNNING   ((int16) 7)

#define    UNDEFINED           ((int16) 0xA5A5)

#define    VERSIONNUMBER       ((int16) 90)
/****************************************************************************/
/*                                                                          */
/* Local variables, accessible via interface functions.                     */
/*                                                                          */
/****************************************************************************/

char     *pcVersionString = "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\
Test Script Engine\n\
Copyright (c) 2005\n\
Ordina Technical Automation b.v.\n\
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=";

char      pcFullVersionString[250] = { 0 };
char     *pcStartTime = NULL;
static char *pcCurrentFileName;

FILE     *tLogFile = NULL;
int16     iLogFileBlocked;
int16     iHcnt;

static int16     iCurrentState = BSEK_UNINITIALISED;
static int16     iCurrentState_Stepper = STEP_UNINITIALISED;

int16     iHiddenStateMachine = 0;

struct stat *ptScriptFile;
static char        *pcScriptFile;

static int16          iShuttingDown = 0;
static struct timeval tStartTime;
static long           lTicksPerSecond = 0;
static long           lTicksPerMilliSecond = 0;

/* Internal interface functions                                             */

static int16     CheckScriptFileStatus(  );

/****************************************************************************/
/*                                                                          */
/* Functions, accessible for the outer world.                               */
/*                                                                          */
/****************************************************************************/
void SetEnabledChannels(
  int *iEnabledChannel,
  char *pcEnabledDevice[16],
  char *pcProtocolName)
{
  int       i;

  piChannels = iEnabledChannel;

  for (i = 0; i < 16; i++)
  {
    pcDevice[i] = pcEnabledDevice[i];
  }

  pcProtocol = pcProtocolName;

  Add_IO();                     /* REMOVE IF NOT WORKING */

}

char     *BSEK_CreateHTML(
  char *pcFileName,
  struct BSEK_ObjectName *ptObjects,
  int16 bWithData,
  int16 iCharacterSize)
/* BSEK_CreateHTML will create an HTML file, based on the given logfile and */
/* the given sequence of objects, and other parameters.                     */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* pFileName               : ptr to string containing the log-file from     */
/*                           which the object names are retrieved.          */
/* *pObjects               : ptr to list of object names, defining the de-  */
/*                           sired sequence of objects in the to be gene-   */
/*                           rated MSC.                                     */
/* bWithData               : TRUE : The to be generated MSC will also con-  */
/*                                  tain the data in the body of each log-  */
/*                                  ged message.                            */
/*                           FALSE: The to be generated MSC will contain    */
/*                                  no additional data.                     */
/* iCharacterSize          : integer indicating the size of the characters  */
/*                           in the to be generated MSC.                    */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* char *                  : ptr to string containing the name of the       */
/*                           generated file.                                */
/* Remarks:                                                                 */
/* The GUI might want to start the default HTML viewer to display the       */
/* generated file?                                                          */
/* ------------------------------------------------------------------------ */
{
  iCharacterSize = 0;
  bWithData = 0;
  ptObjects = NULL;
  pcFileName = NULL;

  return ("Not implemented yet.");
}


void BSEK_ErrorPosition(
  char **ppcError)
/* BSEK_ErrorPosition describes the position where an error is found during */
/* reading an input file.                                                   */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* ppError                 : ptr to ptr to a string containing an easy      */
/*                           readable string. The kernel will allocate      */
/*                           memory for this string. The caller of this     */
/*                           function is responsible for freeing the memory */
/*                           block.                                         */
/* Return value:                                                            */
/* --                                                                       */
/* Remarks:                                                                 */
/* When during reading an input file, whether this is a specification file, */
/* a script file or a batch file, errors can occur. An error will be        */
/* reported by a returned error code, a detailled description of this error */
/* can be obtained using this function. This description is formatted with  */
/* cr/lf characters.                                                        */
/* An example of such a string can be:                                      */
/*                                                                          */
/*      Error reading file D:\scripts\file3.ats, line 417                   */
/*      Included by file D:\scripts\test_1.ats, line 112                    */
/*      Included by file D:\scripts\big_test.ats, line 15                   */
/*                                                                          */
/*      Found token : (                                                     */
/*      Expected token : {                                                  */
/*                                                                          */
/*                                                                          */
/* ------------------------------------------------------------------------ */
{
  int       iResponseLength;

  iResponseLength = strlen(_pcWarningText) + strlen(_pcErrorText) + strlen(_pcFatalText) + 1;   /* for a terminating zero.                           */

  *ppcError = (char *) malloc(iResponseLength);

  strcpy(*ppcError, _pcFatalText);
  strcat(*ppcError, _pcErrorText);
  strcat(*ppcError, _pcWarningText);

  if (iResponseLength > 2000)
  {
    /* Cut extreme long error reports. This error report will be presented  */
    /* to the user via a dialog. The dialog may not become too big, because */
    /* otherwise the OK button appears beyond the screen borders.           */

    /* The error report will now be presented in the overview window instead */
    /* of a modal window. */
    /*(*ppcError)[2000] = '\0'; */
  }

  /* To prevent the same errors presented to the user multiple times...     */
  ResetErrorLogging();
}


void BSEK_Init(
  void)
/* BSEK_Init intialises the KERNEL. This function must be called before any */
/* function can be called.                                                  */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return parameters:                                                       */
/* ------------------------------------------------------------------------ */
{
  if (iCurrentState == BSEK_UNINITIALISED)
  {
    InitScanner();
    InitParser();
    InitExecutor();
    BP_Init();
    BSED_Init();
    StartDebugger();

    MallocText();

    _ptGlobals->iOverviewMode = BSEK_STATEMACHINES_FULL;
    _ptGlobals->iLogMode = BSEK_DEFAULT_LOGGING;

    iCurrentState = BSEK_IDLE;
    iHiddenStateMachine = 0;

    ptScriptFile = (struct stat *) malloc(sizeof(struct stat));

    lTicksPerSecond = (sysconf(_SC_CLK_TCK));
    if (1000 < lTicksPerSecond)
    {
      lTicksPerMilliSecond = (lTicksPerSecond/1000);
    }
    
  }
}


struct BSEK_ObjectName *BSEK_GetObjectNames(
  char *pcFileName)
/* BSEK_GetObjectNames returns a list of object names from a log-file.      */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* char *pFileName         : ptr to a zero-terminated string, containing    */
/*                           a filename. This string must meet the same     */
/*                           requirements as the filename string used in    */
/*                           the standard fopen function. This file will    */
/*                           be opened to read a log file from.             */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* struct BSEK_ObjectName * : ptr to a list of object names. If the log file */
/*                           contains no object names, a NULL pointer will  */
/*                           be returned.                                   */
/*                           The kernel will allocate memory for the        */
/*                           records, the caller of this function will be   */
/*                           responsible for freeing the list.              */
/* ------------------------------------------------------------------------ */
{
  pcFileName = NULL;
  iHiddenStateMachine = 0;
  return (NULL);
}


int16 BSEK_LoadBatch(
  char *pcFileName)
/* BSEK_LoadBatch loads a test batch file.                                  */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* char *pFileName         : ptr to a zero-terminated string, containing    */
/*                           a filename. This string must meet the same     */
/*                           requirements as the filename string used in    */
/*                           the standard fopen function. This file will    */
/*                           be opened to read a batch file from.           */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for open file functions"                       */
/* ------------------------------------------------------------------------ */
{
  int       iReturnCode;
  int       iResult;

  switch (iCurrentState)
  {
    case BSEK_UNINITIALISED:
      /* finalisation:                               */
      iReturnCode = UNDEFINED;
      iHiddenStateMachine = 0;
      break;
    case BSEK_IDLE:
    case BSEK_BATCH_LOADED:

    case BSEK_READY:
    case BSEK_CONFIGURED:
      iResult = BP_LoadBatch(pcFileName);
      switch (iResult)
      {
        case BP_OK:
          /* First retrieve some statistics about    */
          /* the just loaded batch file..            */
          iCurrentState = BSEK_BATCH_LOADED;
          iReturnCode = BSEK_OK;
          break;
        case BP_FILE_CANNOT_BE_OPENED:
          iReturnCode = BSEK_FILE_CANNOT_BE_OPENED;
          break;
        case BP_FILE_READ_FAILURE:
          iReturnCode = BSEK_SCRIPT_ERROR;
          break;
        case BP_BATCH_NO_END:
          iReturnCode = BSEK_SCRIPT_ERROR;
          break;
        case BP_NO_SCRIPTS_IN_BATCH:
          iReturnCode = BSEK_SCRIPT_ERROR;
          break;
        default:
          iReturnCode = BSEK_SCRIPT_ERROR;
          break;
      }

      /* finalisation:                               */
      iHiddenStateMachine = 0;
      break;
    case BSEK_PAUSED:
      /* finalisation:                               */
      iReturnCode = BSEK_SCRIPT_RUNNING;
      iHiddenStateMachine = 0;
      break;
    case BSEK_RUNNING:
      /* finalisation:                               */
      iReturnCode = BSEK_SCRIPT_RUNNING;
      iHiddenStateMachine = 0;
      break;
    case BSEK_BATCH_RUNNING:
      /* finalisation:                               */
      iReturnCode = BSEK_BATCH_ALREADY_RUNS;
      iHiddenStateMachine = 0;
      break;
    default:
      /* finalisation:                               */
      iReturnCode = UNDEFINED;
      iHiddenStateMachine = 0;
      break;
  }
  return (iReturnCode);

}

int16 BSEK_LoadIOFile       ( char                          *pcFileName)
/* BSEK_LoadIOFile loads a io definition file. This io file contains a list */
/* of device names which the user can map to communication ports.           */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* char *pFileName         : ptr to a zero-terminated string, containing    */
/*                           a filename. This string must meet the same     */
/*                           requirements as the filename string used in    */
/*                           the standard fopen function. This file will    */
/*                           be opened to read a test script from.          */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for open file functions"                       */
/* ------------------------------------------------------------------------ */
{

  int iResult     = UNDEFINED;
  int iReturnCode = UNDEFINED;

  switch (iCurrentState)
  {
    case BSEK_UNINITIALISED :
                             /* finalisation:                               */
                             iReturnCode = UNDEFINED;
                             iHiddenStateMachine = 0;
                              break;
    case BSEK_IDLE          :
    case BSEK_CONFIGURED    :
    case BSEK_BATCH_LOADED  :
    case BSEK_READY         : iResult = Parse_IO_File(pcFileName);
                             
                             /* finalisation:                               */
                             if (iResult < 0)
                             {
                               iReturnCode = BSEK_FILE_CANNOT_BE_OPENED;
                             }
                             else if (iResult > 0)
                             {
                               iReturnCode   = BSEK_SCRIPT_ERROR;
                               iCurrentState = BSEK_IDLE;
                             }
                             else if (iResult == 0)
                             {
#ifdef _NO_CONTROL_PANEL_
                               iResult = BSED_StartConfiguration();
                               if (0 == iResult)
                               {
                                 iReturnCode   = BSEK_OK;
                                 iCurrentState = BSEK_CONFIGURED;
                               }
                               else
                               {
                                 iReturnCode   = BSEK_SCRIPT_ERROR;
                                 iCurrentState = BSEK_IDLE;
                               }
#else
                               iReturnCode   = BSEK_OK;
                               iCurrentState = BSEK_CONFIGURED;
#endif /*_NO_CONTROL_PANEL_ */
                             }
                             iHiddenStateMachine = 0;
                              break;
    case BSEK_PAUSED        :
                             /* finalisation:                               */
                             iHiddenStateMachine = 0;
                             iReturnCode = BSEK_KERNEL_BUSY;
                              break;
    case BSEK_RUNNING       :
                             /* finalisation:                               */
                             iHiddenStateMachine = 0;
                             iReturnCode = BSEK_SCRIPT_RUNNING;
                              break;
    case BSEK_BATCH_RUNNING :
                             /* finalisation:                               */
                             iHiddenStateMachine = 0;
                             iReturnCode = BSEK_BATCHFILE_RUNNING;
                              break;
  	default                :
                             /* finalisation:                               */
                             iHiddenStateMachine = 0;
                             iReturnCode = UNDEFINED;
                              break;
  }
  return (iReturnCode);
}

int16 BSEK_LoadScript(
  char *pcFileName)
/* BSEK_LoadScript loads a test script. This testscript can contain a list  */
/* of device names which the user can map to communication ports.           */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* char *pFileName         : ptr to a zero-terminated string, containing    */
/*                           a filename. This string must meet the same     */
/*                           requirements as the filename string used in    */
/*                           the standard fopen function. This file will    */
/*                           be opened to read a test script from.          */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for open file functions"                       */
/* ------------------------------------------------------------------------ */
{
  int       iResult = UNDEFINED;
  int       iReturnCode = UNDEFINED;

  switch (iCurrentState)
  {
    case BSEK_UNINITIALISED:   /* do nothing. Undefined behaviour allowed.    */
      /* finalisation:                               */
      iReturnCode = UNDEFINED;
      iHiddenStateMachine = 0;
      break;
    case BSEK_IDLE:
      /* finalisation:                               */
      iReturnCode = BSEK_NO_SPECIFICATION;
      iHiddenStateMachine = 0;
      break;
    case BSEK_CONFIGURED:
    case BSEK_BATCH_LOADED:
    case BSEK_READY:
      iResult = Parse_Script_File(pcFileName);

      /* finalisation:                               */
      if (iResult < 0)
      {
        iReturnCode = BSEK_FILE_CANNOT_BE_OPENED;
      }
      else if (iResult > 0)
      {
        iReturnCode = BSEK_SCRIPT_ERROR;
        iCurrentState = BSEK_CONFIGURED;
      }
      else if (iResult == 0)
      {
        /* First retrieve some statistics about      */
        /* the just loaded script file..             */
        stat(pcFileName, ptScriptFile);

        pcScriptFile = my_strdup(pcFileName);

        iReturnCode = BSEK_OK;
        iCurrentState = BSEK_READY;
      }
      Update_Overview();
      iHiddenStateMachine = 0;
      break;
    case BSEK_PAUSED:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_KERNEL_BUSY;
      break;
    case BSEK_RUNNING:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_SCRIPT_RUNNING;
      break;
    case BSEK_BATCH_RUNNING:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_BATCHFILE_RUNNING;
      break;
    default:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = UNDEFINED;
      break;
  }
  return (iReturnCode);
}


int16 BSEK_LoadSpecification(
  char *pcFileName)
/* BSEK_LoadSpecification loads the specification file. This file configures */
/* the communication module, and defines all possible events, commands      */
/* and parameters possible between AutoTest and the Software Under Test.    */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* char *pcFileName        : ptr to a zero-terminated string, containing    */
/*                           a filename. This string must meet the same     */
/*                           requirements as the filename string used in    */
/*                           the standard fopen function.                   */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for open file functions"                       */
/* ------------------------------------------------------------------------ */
{

  int       iResult = UNDEFINED;
  int       iReturnCode = UNDEFINED;

  switch (iCurrentState)
  {
    case BSEK_UNINITIALISED:
      /* finalisation:                               */
      iReturnCode = UNDEFINED;
      iHiddenStateMachine = 0;
      break;
    case BSEK_IDLE:
    case BSEK_CONFIGURED:
    case BSEK_BATCH_LOADED:
    case BSEK_READY:
      iResult = Parse_Specification_File(pcFileName);

      /* finalisation:                               */
      if (iResult < 0)
      {
        iReturnCode = BSEK_FILE_CANNOT_BE_OPENED;
      }
      else if (iResult > 0)
      {
        iReturnCode = BSEK_SCRIPT_ERROR;
        iCurrentState = BSEK_IDLE;
      }
      else if (iResult == 0)
      {
        iReturnCode = BSEK_OK;
        iCurrentState = BSEK_CONFIGURED;
      }
      iHiddenStateMachine = 0;
      break;
    case BSEK_PAUSED:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_KERNEL_BUSY;
      break;
    case BSEK_RUNNING:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_SCRIPT_RUNNING;
      break;
    case BSEK_BATCH_RUNNING:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_BATCHFILE_RUNNING;
      break;
    default:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = UNDEFINED;
      break;
  }
  return (iReturnCode);
}

int16 BSEK_OpenLogFile(
  char *pcFileName,
  int16 iReplay)
/* BSEK_OpenLogFile opens a logfile, to which exactly the same logging as   */
/* sent to the GUI by the BSEG_LogLine function.                            */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/*               Replay - indicates if a scripy file is to be replayed(1) or*/ 
/*               not(0).                                                    */
/* --                                                                       */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for reset script/batch function"               */
/* ------------------------------------------------------------------------ */
{

  char        *pString;
  char        *pcLogFilename;
  int          iVersionNumber;
  struct stat  tNewStat;
  char         pcTimeStartedLog[25];
  int16        iResult;
  /*---start------------------------------------------------------------ */
  FILE        *tScriptFile;
  char         c;
  char        *ScriptHeader;       /*changes by epkkaal */

  /*---finish------------------------------------------------------------*/

  pcLogFilename = NULL;
  if (tLogFile != NULL)
  {
    CloseLogFile();
  }

  pcLogFilename = bp_CreateLogFileName(pcFileName);
  if (iReplay == 1)
  {
    GetSeedValue(pcLogFilename);
  }
  tLogFile = fopen(pcLogFilename, "w");

  if (tLogFile == NULL)
  {
    iResult = BSEK_FILE_CANNOT_BE_OPENED;
  }
  else
  {
    /*fseek(tLogFile, iHcnt, SEEK_SET); */

    pString = CurrentFile();
    pcCurrentFileName = malloc(strlen(CurrentFile()) + 1);
    strcpy(pcCurrentFileName, CurrentFile());
    stat(pString, &tNewStat);

    pString = BSEK_Version(&iVersionNumber);
    fprintf(tLogFile, "%s\n\n", pString);

    fprintf(tLogFile, "Logfile of script   : %s\n", CurrentFile());
    fprintf(tLogFile, "Script file created : %s", ctime(&(tNewStat.st_ctime)));
    fprintf(tLogFile, "Last change         : %s\n",
            ctime(&(tNewStat.st_mtime)));

    fprintf(tLogFile, "Start time          : %s\n", "xx:xx:xx (xxxx-xx-xx)");
    fprintf(tLogFile, "Stop time           : %s\n", "xx:xx:xx (xxxx-xx-xx)");
    fprintf(tLogFile, "Duration            : %s\n", "xx:xx:xx");
    fprintf(tLogFile, "Seed: %s\n", "x");

    fprintf(tLogFile, "\n\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");

    iLogFileBlocked = FALSE;

    /*---start----------changes by epkkaal--------------------------- */
    pString = CurrentFile();

    tScriptFile = fopen(CurrentFile(), "r");

    if (tScriptFile == NULL)
    {
      return BSEK_FILE_CANNOT_BE_OPENED;
    }

    if (BSEG_GetIncludeHeader())
    {
      c = 0;
      iHcnt = 0;
      fseek(tScriptFile, 0L, SEEK_SET);

      do
      {
        fread(&c, 1, 1, tScriptFile);
        iHcnt++;
      }
      while (c != '[');



      if ((ScriptHeader = (char *) calloc( iHcnt, sizeof(char))) != NULL)
      {
        fseek(tScriptFile, 0L, SEEK_SET);
        fread(ScriptHeader, 1, iHcnt - 1, tScriptFile);
        fprintf(tLogFile, "%s", ScriptHeader);
        fflush(tLogFile);

        free(( void*) ScriptHeader);
      }

      fclose(tScriptFile);

      /*----finish-------------------------------------------------------- */
    }

    iResult = BSEK_OK;
  }
  if (NULL != pcLogFilename)
  {
    free(pcLogFilename);
  }
  return(iResult);
}

int16 BSEK_ResetScript(
  void)
/* BSEK_ResetScript sets back a script in its initial state.                */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for reset script function"                     */
/* Remarks:                                                                 */
/* Resetting means that all variables are set back to uninitialized, and    */
/* that the "current state" is reinitialised to the first state.            */
/* Resetting is only possible if a script is loaded and not running.        */
/* ------------------------------------------------------------------------ */
{
  int       iReturnCode;

  switch (iCurrentState)
  {
    case BSEK_UNINITIALISED:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = UNDEFINED;
      break;
    case BSEK_IDLE:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_NO_SCRIPT_LOADED;
      break;
    case BSEK_CONFIGURED:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_NO_SCRIPT_LOADED;
      break;
    case BSEK_BATCH_LOADED:
      InitBoards(_ptBoardList);
      Update_Overview();
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_OK;
      break;
    case BSEK_READY:
    case BSEK_PAUSED:
      InitBoards(_ptBoardList);
      Update_Overview();
      /* finalisation:                               */
      iCurrentState = BSEK_READY;
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_OK;
      break;
    case BSEK_RUNNING:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_SCRIPT_RUNNING;
      break;
    case BSEK_BATCH_RUNNING:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_BATCHFILE_RUNNING;
      break;
    default:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = UNDEFINED;
      break;
  }
  return (iReturnCode);
}


int16 BSEK_RunBatch(
  int16 iReplay)
/* BSEK_RunBatch will run a batch file.                                     */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for run script/batch function"                 */
/* ------------------------------------------------------------------------ */
{
  int       iReturnCode;

  if (pcStartTime != NULL)
  {
    free(pcStartTime);
    pcStartTime = NULL;
  }

  switch (iCurrentState)
  {
    case BSEK_UNINITIALISED:   /* do nothing. Undefined behaviour allowed.    */
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = UNDEFINED;
      break;
    case BSEK_IDLE:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_NO_BATCH_TO_RUN;
      break;
    case BSEK_CONFIGURED:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_NO_BATCH_TO_RUN;
      break;
    case BSEK_BATCH_LOADED:
      iReturnCode = BP_RunBatch(iReplay);
      if (iReturnCode == BP_OK)
      {
        iCurrentState = BSEK_BATCH_RUNNING;
        iReturnCode = BSEK_OK;
      }
      else
      {
        iCurrentState = BSEK_BATCH_LOADED;
        iReturnCode = BSEK_OTHER_ERROR;
      }
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      break;
    case BSEK_READY:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_NO_BATCH_TO_RUN;
      break;
    case BSEK_PAUSED:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_NO_BATCH_TO_RUN;
      break;
    case BSEK_RUNNING:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_SCRIPT_RUNNING;
      break;
    case BSEK_BATCH_RUNNING:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_BATCH_ALREADY_RUNS;
      break;
    default:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = UNDEFINED;
      break;
  }
  return (iReturnCode);
}


int16 BSEK_RunScript(
  int16 iReplay)
/* BSEK_RunScript starts/continues a test script.                           */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/*    Replay - indicates if the script is to be replayed. 1 replay, 0 not   */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for run script/batch function"                 */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
{
  int            iReturnCode;
  struct timeval tNow;
  if (pcStartTime != NULL)
  {
    free(pcStartTime);
    pcStartTime = NULL;
  }

  switch (iCurrentState)
  {
    case BSEK_UNINITIALISED:   /* do nothing. Undefined behaviour allowed.    */
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = UNDEFINED;
      break;
    case BSEK_IDLE:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_NO_SCRIPT_TO_RUN;
      break;
    case BSEK_CONFIGURED:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_NO_SCRIPT_TO_RUN;
      break;
    case BSEK_BATCH_LOADED:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_NO_SCRIPT_TO_RUN;
      break;

    case BSEK_READY:
    case BSEK_PAUSED:
      iReturnCode = CheckScriptFileStatus();
      if (iReturnCode == 0)
      {
        if (iReplay == 1)
        {
          if (lSeedValue == 0)
          {
            gettimeofday( &tNow, NULL);
            lSeedValue = tNow.tv_sec + tNow.tv_usec;
            LogLine("Failed to read seed value from log file");
          }
        }
        else
        {
          gettimeofday( &tNow, NULL);
          lSeedValue = tNow.tv_sec + tNow.tv_usec;
        }
        srandom(lSeedValue);
        StartExecutor();
        /* finalisation:                             */
        iHiddenStateMachine = 0;
        iReturnCode = BSEK_OK;
        iCurrentState = BSEK_RUNNING;
      }
      else
      {
        iHiddenStateMachine = 0;
        iReturnCode = BSEK_SCRIPT_ERROR;
      }
      break;
    case BSEK_RUNNING:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_SCRIPT_ALREADY_RUNS;
      break;
    case BSEK_BATCH_RUNNING:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_BATCHFILE_RUNNING;
      break;
    default:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = UNDEFINED;
      break;
  }
  return (iReturnCode);
}

int16 BSEK_Shutdown(
  void)
/* BSEK_Shutdown stops all scripts and frees all allocated memory.          */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for shutdown"                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
{
  int       iReturnCode;

  FreeText();

  switch (iCurrentState)
  {
    case BSEK_UNINITIALISED:
      /* finalisation:                               */
      iReturnCode = UNDEFINED;
      iHiddenStateMachine = 0;
      break;
    case BSEK_IDLE:
      /* finalisation:                               */
      iReturnCode = BSEK_OK;
      iHiddenStateMachine = 0;
      break;
    case BSEK_CONFIGURED:
    case BSEK_BATCH_LOADED:
      delEventList(&_ptEventList);
      delFunctionList(&_ptFunctionList);
      delTypeList(&_ptTypeList);

      /* finalisation:                               */
      iReturnCode = BSEK_OK;
      iHiddenStateMachine = 0;
      break;
    case BSEK_READY:
    case BSEK_PAUSED:
      delBoardList(&_ptBoardList);
      delStateMachineList(&_ptStateMachineList);
      delEventList(&_ptEventList);
      delFunctionList(&_ptFunctionList);
      delTypeList(&_ptTypeList);
      delVariableList(&_ptGlobalVars);
      ClosedownTimer();

      /* finalisation:                               */
      iReturnCode = BSEK_OK;
      iHiddenStateMachine = 0;
      break;
    case BSEK_RUNNING:
      iShuttingDown = 1;
      StopExecutor("BSE killed");

      /* give script executor thread time to die     */
      WaitForEndExecutor(); 
      /*sleep(50); */

      delBoardList(&_ptBoardList);
      delStateMachineList(&_ptStateMachineList);
      delEventList(&_ptEventList);
      delFunctionList(&_ptFunctionList);
      delTypeList(&_ptTypeList);
      delVariableList(&_ptGlobalVars);

      /* finalisation:                               */
      iReturnCode = BSEK_OK;
      iHiddenStateMachine = 0;
      break;
    case BSEK_BATCH_RUNNING:
      iShuttingDown = 1;
      BP_StopBatch("BSE killed");

      /* give script executor thread time to die     */
      sleep(50);                /* Modified from 500 */

      delBoardList(&_ptBoardList);
      delStateMachineList(&_ptStateMachineList);
      delEventList(&_ptEventList);
      delFunctionList(&_ptFunctionList);
      delTypeList(&_ptTypeList);
      delVariableList(&_ptGlobalVars);

      /* finalisation:                               */
      iReturnCode = BSEK_OK;
      iHiddenStateMachine = 0;
      break;
    default:
      /* finalisation:                               */
      iReturnCode = UNDEFINED;
      iHiddenStateMachine = 0;
      break;
  }

  if (_pcWarningText)
  {
    free(_pcWarningText);
    _pcWarningText = NULL;
  }

  if (_pcErrorText)
  {
    free(_pcErrorText);
    _pcErrorText = NULL;
  }

  if (_pcFatalText)
  {
    free(_pcFatalText);
    _pcFatalText = NULL;
  }

  if (_pcLine)
  {
    free(_pcLine);
    _pcLine = NULL;
  }

  if (ptScriptFile)
  {
    free(ptScriptFile);
    ptScriptFile = NULL;
  }


  if (iReturnCode == BSEK_OK)
  {
    ShutDownScanner();
    ShutDownParser();
    BSED_ShutDown();
    BP_Finalize();
    StopDebugger();
  }

  return (iReturnCode);

}

int16 BSEK_StepScript(
  void)
/* BSEK_StepScript steps through a script.                                  */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for run script/batch function"                 */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
{
  int       iReturnCode;

  switch (iCurrentState)
  {
    case BSEK_UNINITIALISED:   /* do nothing. Undefined behaviour allowed.    */
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = UNDEFINED;
      break;
    case BSEK_IDLE:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_NO_SCRIPT_TO_RUN;
      break;
    case BSEK_CONFIGURED:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_NO_SCRIPT_TO_RUN;
      break;
    case BSEK_BATCH_LOADED:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_NO_SCRIPT_TO_RUN;
      break;
    case BSEK_READY:
      StepExecutor();
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_OK;
      iCurrentState = BSEK_PAUSED;
      break;
    case BSEK_PAUSED:
      StepExecutor();
      /*(Added by epkragi) Before only the else part existed   */
      if (iCurrentState_Stepper == STEP_FINISH_OK ||
          iCurrentState_Stepper == STEP_FINISH_ERROR ||
          iCurrentState_Stepper == STEP_FINISH_WARNING)
      {
        /* finalisation:                               */
        iHiddenStateMachine = 0;
        iReturnCode = BSEK_OK;
        iCurrentState = BSEK_READY;
        /*Now the user starts over stepping or running */
        iCurrentState_Stepper = STEP_UNINITIALISED; 
      }
      else
      {
        /* finalisation:                               */
        iHiddenStateMachine = 0;
        iReturnCode = BSEK_OK;
        iCurrentState = BSEK_PAUSED;
      }
      break;
    case BSEK_RUNNING:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_SCRIPT_ALREADY_RUNS;
      break;
    case BSEK_BATCH_RUNNING:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = BSEK_BATCHFILE_RUNNING;
      break;
    default:
      /* finalisation:                               */
      iHiddenStateMachine = 0;
      iReturnCode = UNDEFINED;
      break;
  }
  return (iReturnCode);
}


int16 BSEK_SetLoggingMode(
  int16 iLoggingMode)
/* BSEK_SetLoggingMode will set the logging mode.                           */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* iLoggingMode            : enumerated value as defined in "logging modes" */
/*                           A bitwise OR can be used to combine different  */
/*                           values.                                        */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* --                                                                       */
/* Remarks:                                                                 */
/* The kernel has full control over the contents of the log-window. The     */
/* GUI shouldn't add extra functionality to this log window, because        */
/* this log-window and the contents of a log-file should be identical.      */
/* Using this function, the user can influence the level of logging of both */
/* output devices (window and file)                                         */
/* ------------------------------------------------------------------------ */
{
  _ptGlobals->iLogMode = iLoggingMode;

  return (BSEK_OK);

}

int16 BSEK_SetViewMode(
  int16 iViewMode)
/* BSEK_SetViewMode will set the view mode, which is the contents of the    */
/* right window in the GUI.                                                 */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* iViewMode               : enumerated value as defiend in "view modes"    */
/*                                                                          */
/* Output parameters:      :                                                */
/* Return value:                                                            */
/* --                                                                       */
/* Remarks:                                                                 */
/* The kernel has full control over the contents of the view-window. The    */
/* GUI shoudn't add extra functionality to this view window.                */
/* ------------------------------------------------------------------------ */
{
  _ptGlobals->iOverviewMode = iViewMode;

  if ((iViewMode == BSEK_STATEMACHINES_HEADERS) && (iHiddenStateMachine == 0))
    iHiddenStateMachine = 1;
  else if ((iViewMode == BSEK_PROCESSED_FILES) && (iHiddenStateMachine == 1))
    iHiddenStateMachine = 2;
  else if ((iViewMode == BSEK_STATEMACHINES_HEADERS) &&
           (iHiddenStateMachine == 2))
    iHiddenStateMachine = 3;
  else if ((iViewMode == BSEK_STATEMACHINES_FULL) && (iHiddenStateMachine == 3))
    iHiddenStateMachine = 4;
  else if ((iViewMode == BSEK_PROCESSED_FILES) && (iHiddenStateMachine == 4))
    iHiddenStateMachine = 5;
  else if ((iViewMode == BSEK_STATEMACHINES_FULL) && (iHiddenStateMachine == 5))
    iHiddenStateMachine = 6;
  else if ((iViewMode == BSEK_STATEMACHINES_HEADERS) &&
           (iHiddenStateMachine == 6))
    iHiddenStateMachine = 7;
  else if ((iViewMode == BSEK_PROCESSED_FILES) && (iHiddenStateMachine == 7))
    iHiddenStateMachine = 8;
  else if ((iViewMode == BSEK_STATEMACHINES_FULL) && (iHiddenStateMachine == 8))
    iHiddenStateMachine = 9;
  else if ((iViewMode == BSEK_STATEMACHINES_HEADERS) &&
           (iHiddenStateMachine == 9))
    iHiddenStateMachine = 10;
  else if ((iViewMode == BSEK_PROCESSED_FILES) && (iHiddenStateMachine == 10))
    iHiddenStateMachine = 11;
  else if ((iViewMode == BSEK_STATEMACHINES_HEADERS) &&
           (iHiddenStateMachine == 11))
    iHiddenStateMachine = 12;
  else
    iHiddenStateMachine = 0;

  if (iHiddenStateMachine == 12)
    _ptGlobals->iDebugLevel = 1;

  Update_Overview();
  BP_UpdateLogging();

  return (BSEK_OK);
}


int16 BSEK_StopBatch(
  char *pcReason)
/* BSEK_StopBatch will stop a batch file.                                   */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for stop script/batch function"                */
/* ------------------------------------------------------------------------ */
{
  int       iReturnCode;

  switch (iCurrentState)
  {
    case BSEK_UNINITIALISED:   /* do nothing. Undefined behaviour allowed.    */
      /* finalisation:                               */
      iReturnCode = UNDEFINED;
      break;
    case BSEK_IDLE:
      /* finalisation:                               */
      iReturnCode = BSEK_OK;
      break;
    case BSEK_CONFIGURED:
      /* finalisation:                               */
      iReturnCode = BSEK_OK;
      break;
    case BSEK_BATCH_LOADED:
      /* finalisation:                               */
      iReturnCode = BSEK_OK;
      break;
    case BSEK_READY:
      /* finalisation:                               */
      iReturnCode = BSEK_OK;
      break;
    case BSEK_PAUSED:
      /* finalisation:                               */
      iReturnCode = BSEK_OK;
      break;
    case BSEK_RUNNING:
      /* finalisation:                               */
      iReturnCode = BSEK_SCRIPT_RUNNING;
      break;
    case BSEK_BATCH_RUNNING:
      iCurrentState = BSEK_READY;
      if (pcReason != NULL)
      {
        BP_StopBatch(pcReason);
      }
      else
      {
        BP_StopBatch(NULL);
      }
      /* finalisation:                               */
      iReturnCode = BSEK_OK;
      break;
    default:
      /* finalisation:                               */
      iReturnCode = UNDEFINED;
      break;
  }
  return (iReturnCode);
}



int16 BSEK_StopScript(
  char *pcReason)
/* BSEK_StopScript stops a test script.                                     */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for stop script/batch function"                */
/* Remarks:                                                                 */
/* Stopping means that the current state of the testscript will be "frozen" */
/* i.e. the testscript will not generate test commands any more, and in-    */
/* coming events will not influence the state of the test-script. However,  */
/* these events will be logged using the BSEG_LogLine function.             */
/* ------------------------------------------------------------------------ */
{
  int       iReturnCode;

  switch (iCurrentState)
  {
    case BSEK_UNINITIALISED:   /* do nothing. Undefined behaviour allowed.    */
      /* finalisation:                               */
      iReturnCode = UNDEFINED;
      break;
    case BSEK_IDLE:
      /* finalisation:                               */
      iReturnCode = BSEK_OK;
      break;
    case BSEK_CONFIGURED:
      /* finalisation:                               */
      iReturnCode = BSEK_OK;
      break;
    case BSEK_BATCH_LOADED:
      /* finalisation:                               */
      iReturnCode = BSEK_OK;
      break;
    case BSEK_READY:
      /* finalisation:                               */
      iReturnCode = BSEK_OK;
      break;
    case BSEK_PAUSED:          /* single step functionality here? */

      /* finalisation:                               */
      iReturnCode = BSEK_OK;
      break;
    case BSEK_RUNNING:
      PauseExecutor();

      /* The logfile is _not_ automatically closed   */
      /* hen a script stops. This is to prevent      */
      /* unclear problems when reading the following */
      /* script in a batch file.                     */
      /* Therefore this close logfile is not invoked */
      /* when stopping a script while in batch mode  */

      /*if (tLogFile != NULL)
         {
         CloseLogFile();
         } */

      /* finalisation:                               */
      iReturnCode = BSEK_OK;
      iCurrentState = BSEK_PAUSED;
      break;
    case BSEK_BATCH_RUNNING:
      PauseExecutor();
      /* finalisation:                               */
      iReturnCode = BSEK_BATCHFILE_RUNNING;
      break;
    default:
      /* finalisation:                               */
      iReturnCode = UNDEFINED;
      break;
  }
  return (iReturnCode);

}

void BSEK_InternalEvent(
  int16 iDeviceNumber,
  char *pcData)
/* BSEK_InternalEvent acts as an interface to trigger internal events.      */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* iDeviceNumber           : The device number for which a string is in-    */
/*                           tended.                                        */
/* pcData                  : ptr to the ascii string. The following ascii   */
/*                           strings are currently defined:                 */
/*                           SOCKET_GENERAL_ERROR                           */
/*                           SOCKET_SEND_ERROR                              */
/*                           SOCKET_RECEIVE_ERROR                           */
/*                           SOCKET_CONNECT_ERROR                           */
/*                           SOCKET_DISCONNECT_ERROR                        */
/*                           SOCKET_ACCEPT_ERROR                            */
/*                           SOCKET_UNKNOWN_ERROR                           */
/*                           CONNECTION_LOST                                */
/*                           CONNECTION_ESTABLISHED                         */
/* --                                                                       */
/* ------------------------------------------------------------------------ */
{
  /* Minus 1 as lenght indicates an internal event, identified by the       */
  /* 0-terminated ascii string refferred to by pcData                       */
  StoreThisEvent(iDeviceNumber, -1, pcData);
}

char     *BSEK_Version(
  int16 * piVersionNumber)
/* BSEK_Version provides version information.                               */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* piVersionNumber         : div 100 : major version number                 */
/*                           mod 100 : minor version number (major.minor)   */
/* Return value:                                                            */
/* char *                  : Ptr to three lines of ASCII text in the format */
/*                              AutoTest version 0.60                       */
/*                              sept 25, 2000                               */
/*                              Copyright (c) ...                           */
/*                           This memory cannot be freed.                   */
/* ------------------------------------------------------------------------ */
{

  *piVersionNumber = VERSIONNUMBER;

  sprintf(pcFullVersionString, "%s\nBuild: %s", pcVersionString, __DATE__);

  return pcFullVersionString;
}

void BSEK_ProcessIncomingBuffer(
  void)
/* BSEK_ProcessIncomingBuffer gives us an interface for handling the buffer.*/
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* piVersionNumber         : div 100 : major version number                 */
/*                           mod 100 : minor version number (major.minor)   */
/* Return value:                                                            */
/* char *                  : Ptr to three lines of ASCII text in the format */
/*                              AutoTest version 0.60                       */
/*                              sept 25, 2000                               */
/*                              Copyright (c) ...                           */
/* ------------------------------------------------------------------------ */
{
  ProcessIncomingBuffer();
}

/****************************************************************************/
/*                                                                          */
/* Functions, accessible for the rest of the kernel                         */
/*                                                                          */
/****************************************************************************/

void internal_ScriptStopped(
  char *pcReason)
/* ------------------------------------------------------------------------ */
/* To close the logfile, to reset the iCurrentState statemachine, and pass  */
/* stop information to the right components.                                */
{
  int       iCnt;
  char     *pcText;
  char     *pcPtr;

  int16     iOldOverviewMode;

/*--- When Stepping we need to know when we are at the last step so */
/*--- here we set the state for what should happend after the last action */
/*--- (Added by epkragi) */
  char     *text;
  int       i;

  text = my_strdup(pcReason);
  iCnt = 0;
  for (i = 0; i < 1; i++)
  {
    iCnt = 0;
  }
  while (1)
  {
    if (text[iCnt] != '\0')
    {
      text[iCnt] = tolower(text[iCnt]);
    }
    else
    {
      break;
    }
    iCnt++;
  }

  if (strpos("ok", text))
  {
    iCurrentState_Stepper = STEP_FINISH_OK;
  }
  else if (strpos("warning", text))
  {
    iCurrentState_Stepper = STEP_FINISH_WARNING;
  }
  else if (strpos("error", text))
  {
    iCurrentState_Stepper = STEP_FINISH_ERROR;
  }
  else if (strpos("timeout", text))     /*Used for General_timeout*/
  {
    iCurrentState_Stepper = STEP_FINISH_ERROR;
  }

  free( (void*) text);
/*-----------End of Step state check */

  if (tLogFile != NULL)
  {
    iOldOverviewMode = _ptGlobals->iOverviewMode;
    _ptGlobals->iOverviewMode = BSEK_STATEMACHINES_FULL;

    pcText = (char *) malloc(1000000);
    (*pcText) = '\0';

    WriteBoardList(pcText, _ptBoardList);

    /* Remove all \r bytes from the pcText string. The string contains      */
    /* \r\n bytes at the end of each line. fprintf converts \n bytes to     */
    /* \r\n bytes, thereby implicityl converting \r\n to \r\r\n sequences.  */
    /* On some text editors, the \r\r\n sequence is converted to a single   */
    /* space _without_ a \r or \n byte. This would result in the text       */
    /* appearing on one single line.                                        */

    pcPtr = pcText;
    while (*pcPtr != 0)
    {
      if (*pcPtr == '\r')
      {
        *pcPtr = ' ';
      }
      pcPtr++;
    }

    if (tLogFile != NULL)
    {
      fprintf(tLogFile, "\n\n\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
      fprintf(tLogFile, "Statemachines:\n\n");
      fprintf(tLogFile, "%s", pcText);
      fflush(tLogFile);
    }

    free(pcText);
    pcText = NULL;

    if (tLogFile != NULL)
    {
      iLogFileBlocked = TRUE;
    }


    _ptGlobals->iOverviewMode = iOldOverviewMode;
  }

/*
  if(strcmp(pcReason,"BP_SCRIPT_ERROR")==0)
  {
    iCurrentState=BSEK_BATCH_RUNNING;
  }
*/
  switch (iCurrentState)
  {
    case BSEK_UNINITIALISED:   /* do nothing. Undefined behaviour allowed.    */
      /* finalisation:                               */
      break;
    case BSEK_IDLE:
      /* finalisation:                               */
      break;
    case BSEK_CONFIGURED:
      /* finalisation:                               */
      break;
    case BSEK_BATCH_LOADED:
      /* finalisation:                               */
      break;
    case BSEK_READY:
      /* finalisation:                               */
      break;
    case BSEK_PAUSED:
      {
	      BSEG_ScriptStopped(pcReason);     /* Removed to test new development */
	
	      if (tLogFile != NULL)
	      {
	        CloseLogFile();
	      }
	
	      /* finalisation:                               */
	      iCurrentState = BSEK_READY;
      }
      break;
    case BSEK_RUNNING:
      if (tLogFile != NULL)
      {
        CloseLogFile();
      }

      /* finalisation:                               */
      iCurrentState = BSEK_READY;
      BSEG_ScriptStopped(pcReason);
      break;
    case BSEK_BATCH_RUNNING:
      /* finalisation:                               */
      BSEG_ClearLogging();
      BP_ScriptStopped(pcReason);
      break;
    default:
      /* finalisation:                               */
      break;
  }

}

void internal_BatchStopped(
  char *pcReason)
/* ------------------------------------------------------------------------ */
/* To reset the iCurrentState statemachine.                                 */
{
  /* First update the administration then update the GUI */
  iCurrentState = BSEK_READY;   /*BSEK_BATCH_LOADED; */

  /* The logfile is _not_ automatically closed when a script stops.         */
  /* This is to prevent unclear problems when reading the following script  */
  /* in a batch file.                                                       */

  if (tLogFile != NULL)
  {
    CloseLogFile();
  }

  BSEG_BatchStopped(pcReason);
}

void LogLine(
  char *pcString)
/* LogLine sends logging information to the GUI and a logfile.              */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* pcString : pointer to zero terminated string.                            */
/* --                                                                       */
/* Globals:                                                                 */
/* iLogmode : If the timestamp flag is set, a timestamp will be added.      */
/*                                                                          */
/* For the timestamp, the Windows OS provides one function:                 */
/* GetLocalTime gives the time of the day.                                  */
/*                                                                          */
/* In the logline we want the time of the day with millisecond resolution.  */
/* To achieve this, a stamp of the system time is taken when the first      */
/* logline is generated. The number elapsed milliseconds is added to this.  */
{
  char             *pcCompleteString;
  static struct tm *ptStartTime = NULL;
  struct timeval    tNow;
  char              acMilliString[5];
  long              lMilliSeconds;

  gettimeofday( &tNow, NULL);
  ptStartTime = localtime( &tNow.tv_sec );
  if (NULL != ptStartTime)
  {
		pcCompleteString = (char *) calloc(4096, sizeof(char));
		
		strftime( pcCompleteString, 
              4096, 
              "%H:%M:%S", 
              ptStartTime);

 	  /* Compute milliseconds from microseconds. */
 	  lMilliSeconds = tNow.tv_usec / 1000;
    sprintf( acMilliString,":%03ld ",lMilliSeconds);
    strcat(pcCompleteString, acMilliString);
    
    strcat( pcCompleteString, pcString);
		if (pcStartTime == NULL)
		{
      /* Mark the start of the duration */
		  gettimeofday( &tStartTime, NULL );
		
		  /* Added by epkragi */
		  /*For compensating the milliseconds we are missing on StartTime */
		  tStartTime.tv_usec -= (lMilliSeconds*1000);
		
		  pcStartTime = malloc(32);
		  strftime( pcStartTime, 
                32, 
                "%H:%M:%S (%Y-%m-%d)",
                localtime(&tStartTime.tv_sec));
		}
		
		if ((tLogFile != NULL) && (iLogFileBlocked == FALSE))
		{
		  fprintf(tLogFile, "%s\n", pcCompleteString);
		  fflush(tLogFile);
		}
		
		if (iShuttingDown != 1)
		{
		  BSEG_LogLine(pcCompleteString);
		}
  }
}

void CloseLogFile(
  )
{
  char        *pString;
  char        *pcStopTime = NULL;
  char        *pcDuration = NULL;
  int          iVersionNumber;
  struct stat  tNewStat;

  struct tm      *ptStopTime = NULL;
  struct timeval  tNow;
  long            lDuration;

  int       iDurHour, iDurMin, iDurSec;
  
  gettimeofday( &tNow, NULL);
  ptStopTime = localtime( &tNow.tv_sec );
  lDuration = tNow.tv_sec - tStartTime.tv_sec;
  
  pcDuration = malloc(32);

  iDurHour = lDuration / 3600;
  lDuration -= iDurHour * 3600;
  iDurMin = lDuration / 60;
  lDuration -= iDurMin * 60;
  iDurSec = lDuration;  /*Modified by epkragi (Added +500 which is 0,5sec) */
  sprintf(pcDuration, "%.2d:%.2d:%.2d", iDurHour, iDurMin, iDurSec);

  pcStopTime = malloc(32);

  strftime( pcStopTime, 
            32, 
            "%H:%M:%S (%Y-%m-%d)",
            ptStopTime);

  stat(pcCurrentFileName, &tNewStat);

  pString = BSEK_Version(&iVersionNumber);

  fseek(tLogFile, 0L, SEEK_SET);

  fprintf(tLogFile, "%s\n\n", pString);
  fflush(tLogFile);
  fprintf(tLogFile, "Logfile of script   : %s\n",
          pcCurrentFileName /*CurrentFile() */ );
  fflush(tLogFile);
          
  fprintf(tLogFile, "Script file created : %s\n", ctime(&(tNewStat.st_ctime)));
  fflush(tLogFile);
  fprintf(tLogFile, "Last change         : %s\n", ctime(&(tNewStat.st_mtime)));
  fflush(tLogFile);

  fprintf(tLogFile, "Start time          : %s\n", pcStartTime);
  fflush(tLogFile);
  fprintf(tLogFile, "Stop time           : %s\n", pcStopTime);
  fflush(tLogFile);
  fprintf(tLogFile, "Duration            : %s\n", pcDuration);
  fflush(tLogFile);
  fprintf(tLogFile, "Seed: %ld\n", lSeedValue);
  fflush(tLogFile);

  fprintf(tLogFile, "\n\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
  fflush(tLogFile);

  if (tLogFile != NULL)
  {
    if (fclose(tLogFile))
    {
      /* File could not be closed */
    }
    tLogFile = NULL;
  }

  free(pcDuration);
  pcDuration = NULL;

  free(pcStartTime);
  pcStartTime = NULL;

  free(pcStopTime);
  pcStopTime = NULL;

  free(pcCurrentFileName);
  pcCurrentFileName = NULL;
}

int16 CheckScriptFileStatus(
  )
{
  struct stat tNewStat;
  char     *pcFileName;
  int16     iReturnValue;
  int16     iRet;

  /* First retrieve the latest statistics of the batch file                 */
  if (stat(pcScriptFile, &tNewStat) != 0)
  {
    iRet = 0;
  }


  /* Script file changed?                                                   */
  if (tNewStat.st_mtime > ptScriptFile->st_mtime)
  {
    /* Yes, reload it. We cannot just call BSEK_LoadScript, because a memory */
    /* leak would be introduced: the function contains the statement        */
    /* pcScriptFile = strdup(pcFileName);                                   */

    pcFileName = pcScriptFile;
    iReturnValue = BSEK_LoadScript(pcFileName);

    /* now pcScriptFile is redirected to a new memory location. Remove the  */
    /* old memory location.                                                 */
    free(pcFileName);
    pcFileName = NULL;
  }
  else
  {
    iReturnValue = 0;
  }

  return iReturnValue;
}
