/****************************************************************************/
/*                                                                          */
/* file       : bsegui.c                                                    */
/* start date : Apr 20, 2005                                                */
/* author     : R v.d. Bunte                                                */
/* version    : 0.0     Apr 20, 2005                                        */
/*                                                                          */
/****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef WIN32
#include <direct.h>
#include <Windows.h>
#else
#include <unistd.h>
#include <signal.h>
#endif

#include "bsegui.h"
#include "atkernel.h"
#include "codingstandard.h"
#include "ini_parser.h"

/* ------------------------------------------------------------------------ */
/* LOCAL DEFINES                                                            */
/* ------------------------------------------------------------------------ */

/* Number of arguments that can be passed from the CLI */
#define NR_ARGUMENTS_TESTSCRIPT        ((int) 5)
#define NR_ARGUMENTS_TESTBATCH         ((int) 3)

#define OK                  ((int)  1)
#define NOT_OK              ((int)  0)
#define PROCESSING_ERROR    ((int) -1)

/* The type of test currently busy */
#define TEST_SCRIPT         ((int)  0)
#define TEST_BATCH          ((int) -1)
#define _MAX_PATH           ((int) 256)
/*------------------------------------------------------------------------- */
/* LOCAL FUNCTIONS                                                          */
/* ------------------------------------------------------------------------ */
static char  *GetIniDirectoryName ( char  *pcFilename );
static int    LoadProtocolFile    ( char  *pcFilename );
static int    LoadIOFile          ( char  *pcFilename );
static int    LoadTestScript      ( char  *pcFilename );
static int    LoadBatchFile       ( char  *pcFilename );
static int    OpenLogFile         ( char  *pcFilename, 
                                    int16  iReplay );
static void   RunBatch            ( void );
static void   RunScript           ( int16  iReplay );
static void   StartBatch          ( char  *pcIOFilename );
static void   StartScript         ( char  *pcProtocolFilename, 
                                    char  *pcLogFilename, 
                                    int16  iReplay);
static int    SetupSignals        ( void );

/* ------------------------------------------------------------------------ */
/* LOCAL VARIABLES                                                          */
/* ------------------------------------------------------------------------ */
static const char *pcUsage = 
  "%s usage:\n  Testscript : <protocol-file> <io-filename> <test-script> <log-filename>\n  TestBatch  : <io-filename> <test-batch>\n";

/* known file extensions */
static const char *pcScriptFileExtension   = ".btsw";
static const char *pcBatchFileExtension    = ".testbatch";
static const char *pcIOFileExtension       = ".io";
static const char *pcProtocolFileExtension = ".prot";
static const char *pcLogFileExtension      = ".log";

/* Variables to determine the state of current test*/
static int         iErrorOccured         = FALSE;
static int         iTestMode             = TEST_SCRIPT;
static int         iRunning              = FALSE;
static FILE       *fpLogLine             = NULL;
static FILE       *fpLogView             = NULL;
static char       *pcIOFilename          = NULL;
static char       *pcScriptFilename      = NULL;
static char       *pcBatchFilename       = NULL;
/* ------------------------------------------------------------------------ */
/* EXTERNAL FUNCTIONS                                                       */
/* ------------------------------------------------------------------------ */
void SigIntHandler ( int iSignalNo );


void SigIntHandler ( int iSignalNo )
{
  if (TRUE == iRunning)
  { 
    if (TEST_BATCH == iTestMode)
    {
      BSEK_StopBatch(NULL);
    }
    else
    {
      BSEK_StopScript(NULL);
    }  
  }  
}

/* BSEG_BatchStopped will be called when a batch file is stopped.           */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* pReason                 : ptr to a string with the reason for stopping   */
/* Return value:                                                            */
/* --                                                                       */
/* Remarks:                                                                 */
/* The function only reports errors.                                        */
/* ------------------------------------------------------------------------ */
void BSEG_BatchStopped(char* pcReason)
{
  char *pcOKLocation;
  char *pcWarningLocation;

  char *pcLowerReason;
  int   iIndex;
  int   iLenghtReason;
  
  if (TEST_BATCH == iTestMode)
  {
    iLenghtReason = (int) strlen(pcReason);
    
    pcLowerReason = (char*) malloc( (iLenghtReason+1) * sizeof(char));
    for (iIndex = 0; iIndex < iLenghtReason; iIndex++)
    {
      pcLowerReason[iIndex] = (char) tolower(pcReason[iIndex]);
    }
    pcOKLocation   = strstr( pcLowerReason,"ok");
    pcWarningLocation = strstr( pcLowerReason,"warning");

    if( (NULL == pcOKLocation) &&
        (NULL == pcWarningLocation) )
    {
      printf("Batch stopped because of error:\n %s\n", pcReason);
      free(pcReason);
      pcReason = NULL;
      iErrorOccured = TRUE;
      iRunning = FALSE;
    }
    else
    {
      BSEG_Loop();
    }
  }
}

/* BSEG_CheckUnselected not used                                            */
/* ------------------------------------------------------------------------ */
/* Input parameters : Ready                                                 */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/* returns -1 to be compatible                                              */
/* ------------------------------------------------------------------------ */
int BSEG_CheckUnselected(char* pcFileName)
{
  return -1;
}

/* BSEG_ClearLogging will delete the current content of the log window.     */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --																	                                    	*/
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* --																		                                    */
/* Remarks:                                                                 */
/* This function is not used in the CLI version.                            */
/* ------------------------------------------------------------------------ */
void BSEG_ClearLogging(void)
{
  /* For now don't do anything with the logging, because we don have a GUI */
}

/* BSEG_GetDirectoryName                                                    */
/* ------------------------------------------------------------------------ */
/* Input parameters : None                                                  */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : char*                                                 */
/* --                                                                       */
/* Remarks:                                                                 */
/* returns the current working directory                                    */
/* ------------------------------------------------------------------------ */
char* BSEG_GetDirectoryName()
{
  char *pcResultString;

  pcResultString = getcwd(NULL, _MAX_PATH);
  return pcResultString;
}

/* BSEG_GetIncludeHeader used to allow the user to disable the inclusion of */
/* an include header                                                        */
/* ------------------------------------------------------------------------ */
/* Input parameters : None                                                  */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : int                                                   */
/* --                                                                       */
/* Remarks:                                                                 */
/* returns (1==1) to always include the header. Maybe extended in the       */
/* future with an option in the .ini file                                   */
/* ------------------------------------------------------------------------ */
int BSEG_GetIncludeHeader(void)
{
  /* get the include header whatever that maybe? */
  return (1==1);
}

/* BSEG_LogLine will log the string to a log window.                        */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* pLogLine                : ptr to string which will be added to the text  */
/*                           in the log window. After logging, the GUI will */
/*                           free the memory block.                         */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
void BSEG_LogLine(char* pcLogLine)
{
  if (NULL != fpLogLine)
  {
    fprintf(fpLogLine,pcLogLine);
    fprintf(fpLogLine,"\n");
    fflush(fpLogLine);
  }
  else
  {
    if ((ParsedIniSettings.iLogLine == 1) &&
        (ParsedIniSettings.iLogToFile == 0))
    {
      printf(pcLogLine);
    }
  }
  free(pcLogLine);
  pcLogLine = NULL;
}

/* BSEG_Loop will be called when the scripts ends to determine if the script*/
/* or batch has to be looped                                                */
/* ------------------------------------------------------------------------ */
/* Input parameters : None                                                  */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/* This function restarts the batch or test script.                         */
/* ------------------------------------------------------------------------ */
void BSEG_Loop( void )
{
  int iResult;
  /* When stop on error check if errors occurred */
  if (OK == ParsedIniSettings.iStopOnError)
  {
    /* On Error quit! */
    if (TRUE == iErrorOccured)
    {
      if (TRUE == iRunning)
      {
        iRunning      = FALSE;
        return;
      }
    }
  }
  if (FALSE != iRunning)
  {
    /* Check if program currently is in looping mode */
    if (OK == ParsedIniSettings.iLoop)
    {
      /* Don't reset if looping batchfiles */
      if (TEST_SCRIPT == iTestMode)
      {
        /* Call function in kernel to reset script*/
        iResult = BSEK_ResetScript();

        switch(iResult)
        {
          case(BSEK_SCRIPT_RUNNING):
          {
            printf("Script must be stopped before resetting.\n");
            break;
          }
          case(BSEK_BATCHFILE_RUNNING):
          {
            printf("Reset is no option when running batch files.\n");
            break;
          }
          case(BSEK_NO_SCRIPT_LOADED):
          {
            printf("No script loaded to reset.\n");
            break;
          }
          case(BSEK_OK):
          {
            break;
          }
          default:
          {
            /* Unknown error occurred */
            printf("Internal error in file %s on line %s\n",
                    (char*)__FILE__, (char*)__LINE__);
            break;
          }
        }
        RunScript(ParsedIniSettings.iReplay);
      }
      else
      {
        BSEK_ResetScript();
        LoadBatchFile(pcBatchFilename);
        RunBatch();
      }
    }
    /* Else not in looping mode */
    else
    {
      iRunning = FALSE;
    }
  }
}

/* BSEG_Overview will view the string in the status window, if visible.     */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* pOverview               : ptr to string containing an overview of the    */
/*                           running state machines and test script.        */
/*                           The GUI will be responsible for freeing the    */
/*                           memory block.                                  */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
void BSEG_Overview(char* pcOverview)
{
  if (NULL != fpLogView)
  {
    fprintf(fpLogView,pcOverview);
    fflush(fpLogView);
  }
  else
  {
    if ((ParsedIniSettings.iLogView == 1) &&
        (ParsedIniSettings.iLogToFile == 0))
    {
      printf(pcOverview);
    }
  }
  free(pcOverview);
  pcOverview = NULL;
}

/* BSEG_ScriptStopped will be called when a single script is stopped.       */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* pReason                 : ptr to a string telling the reason for stopping*/
/* Return value:                                                            */
/* --                                                                       */
/* Remarks:                                                                 */
/* The function only reports errors.                                        */
/* ------------------------------------------------------------------------ */
void BSEG_ScriptStopped(char* pcReason)
{
  char *pcOKLocation;
  char *pcWarningLocation;
  char *pcErrorLocation;

  char *pcLowerReason;
  int   iIndex;
  int   iLenghtReason;
  
  pcLowerReason     = NULL;
  
  if (TEST_SCRIPT == iTestMode)
  {
   iLenghtReason = (int) strlen(pcReason);
   if (iLenghtReason > 0)
   {
    
    pcLowerReason = (char*) calloc( (iLenghtReason+1), sizeof(char));
    if (NULL != pcLowerReason)
    {
      pcLowerReason[0] = 0;
      for (iIndex = 0; iIndex < iLenghtReason; iIndex++)
      {
        pcLowerReason[iIndex] = (char) tolower(pcReason[iIndex]);
      }
  
      pcOKLocation      = NULL;
      pcWarningLocation = NULL;
      
      if (iLenghtReason >= strlen("ok"))
      {
        pcOKLocation = strstr( pcLowerReason, "ok");
      }
      if (iLenghtReason >= strlen("warning"))
      {
        pcWarningLocation = strstr( pcLowerReason, "warning");
      }
  
      if( (NULL == pcOKLocation) &&
          (NULL == pcWarningLocation) )
      {
        printf("Script stopped because:\n %s\n", pcReason);
        BSEK_ErrorPosition(&pcErrorLocation);
        printf("Script error: %s\n",pcErrorLocation);
        free(pcErrorLocation);
        pcErrorLocation = NULL;
        iErrorOccured = TRUE;
        iRunning = FALSE;
      }
      else
      {
        BSEG_Loop();
      }
      free(pcLowerReason);
      pcLowerReason = NULL;
    }
   }
  }
  free(pcReason);
  pcReason = NULL;
}

/* BSEG_SetBatchStateImages not used                                        */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*    Filename, status - combination to identify the node of which the image*/
/*     is updated.                                                          */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/* Does not do anything                                                     */
/* ------------------------------------------------------------------------ */
void    BSEG_SetBatchStateImages( char *pcFileName, char* pcStatus)
{
}

/* BSEG_StopOnError determines if the program stops on errors               */
/* ------------------------------------------------------------------------ */
/* Input parameters : Status: The reason of stopping.                       */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : int                                                   */
/* --                                                                       */
/* Remarks:                                                                 */
/* returns 0 to indicate not stop, 1 is when there is an error and          */
/*   StopOnError is enabled                                                 */
/* ------------------------------------------------------------------------ */
int BSEG_StopOnError(char *pcStatus)
{
  int iResult;

  /* returned when it should not stop on the error */
  iResult = 0;

  if ((OK == ParsedIniSettings.iStopOnError) &&
      ( (NULL == strstr(pcStatus,"ok")) ||
        (NULL == strstr(pcStatus,"warning"))
      ) 
     )
  {
    if (TEST_BATCH == iTestMode)
    {
      BSEK_StopBatch(pcStatus); 
    }
    else if (TEST_SCRIPT == iTestMode)
    {
      BSEK_StopScript(pcStatus);
    }
    iRunning = FALSE;
    iResult  = 1;
  }

  return(iResult);
}

/* BSEG_UpdateProgressBar not used                                          */
/* ------------------------------------------------------------------------ */
/* Input parameters : Ready                                                 */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/* returns -1 to be compatible                                              */
/* ------------------------------------------------------------------------ */
int BSEG_UpdateProgressBar(int bReady)
{
  printf("*");
  return -1;
}

/* main                                                                     */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*    argc - number of arguments                                            */
/*    argv - the arguments                                                  */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/* 1) Check the parameters                                                  */
/* 2) Read the ini file with the user settings                              */
/* 3) Initialise the Kernel                                                 */
/* 4) Read the protocol file with the protocol information                  */
/* 5) Read the IO file with the IO connection information                   */
/* 6) Read the test script or batch file                                    */
/* 7) Open the logfile                                                      */
/* 8) Run the test                                                          */
/* 9) Shutdown the Kernel                                                   */
/* ------------------------------------------------------------------------ */
int main( int argc, char* argv[])
{
  char  pcResultString[100];
  char *pcIniDirectoryName;
  int   iResult;
  
  getcwd(pcResultString,_MAX_PATH);
  
  /* arguments check */
  if ((NR_ARGUMENTS_TESTSCRIPT != argc) &&
      (NR_ARGUMENTS_TESTBATCH  != argc) )
  {
    printf(pcUsage, argv[0]);
  }
  else
  {
    /* Read the ini file */
    pcIniDirectoryName = GetIniDirectoryName(argv[0]);
    strcat(pcIniDirectoryName,"tse.ini");
    if (TRUE == ParseIniFile( pcIniDirectoryName ))
    {
      free(pcIniDirectoryName);
      /* Opening the log line file */
      if ((ParsedIniSettings.iLogToFile == 1) &&
          (ParsedIniSettings.iLogLine   == 1) )
      {
        fpLogLine = fopen("logline.log","w");
        if (NULL == fpLogLine)
        {
          printf("Failed to open log file logline.log, logline disabled\n");
        }
      }

      /* Opening the log view file */
      if ((ParsedIniSettings.iLogToFile == 1) &&
          (ParsedIniSettings.iLogView   == 1) )
      {
        fpLogView = fopen("logview.log","w");
        if (NULL == fpLogView)
        {
          printf("Failed to open log file logview.log, logview disabled\n");
        }
      }

      /* Initialise the Kernel (and driver) */
      BSEK_Init();
      SetupSignals();
      if (argc == NR_ARGUMENTS_TESTSCRIPT)
      {
        /* first store the global filename(s) */
        pcIOFilename     = argv[2];
        pcScriptFilename = argv[3];
        StartScript(argv[1], argv[4],(int16)ParsedIniSettings.iReplay);
      }
      else if (argc == NR_ARGUMENTS_TESTBATCH)
      {
        /* first store the global filename(s) */
        pcBatchFilename = argv[2];
        StartBatch(argv[1]);
      }
      /* Wait for it to finish */
      while (TRUE == iRunning)
      {
        sleep(10);
      }
        
      /* Closes also all of the connections */
      BSEK_Shutdown();
    }
    else
    {
      printf("Failed to open %s\n",pcIniDirectoryName);
      free(pcIniDirectoryName);
    }
  }
  if (NULL != fpLogLine)
  {
    fclose( fpLogLine );
  }
  if (NULL != fpLogView)
  {
    fclose( fpLogView );
  }
  if ( FALSE == iErrorOccured)
  {
    iResult = 0;
  }
  else
  {
    iResult = -1;
  }
  return( iResult );
}

/* ------------------------------------------------------------------------ */
/* INTERNAL FUNCTIONS                                                       */
/* ------------------------------------------------------------------------ */

static char *GetIniDirectoryName(char *pcFilename)
{
  int  iLengthFilename;
  int  iLastBackSlashIndex;
  int  iNOBackSlashes;
  int  iDone;
  char *pcResultString;
  
  iLengthFilename     = (int) strlen(pcFilename);
  iLastBackSlashIndex = 0;
  iNOBackSlashes      = 2;
  iDone               = NOT_OK;
  pcResultString      = (char*)calloc(80, sizeof(char));
  
  while ((iLengthFilename>=0) && (iDone == NOT_OK))
  {
    if( pcFilename[iLengthFilename] == '/')
    {
      iLastBackSlashIndex = iLengthFilename;
      iNOBackSlashes--;
      if ( 0 == iNOBackSlashes)
      {
        iDone = OK;
      }
    }
    iLengthFilename--;
  } 
  if (OK == iDone)
  {
    strncpy( pcResultString, pcFilename, iLastBackSlashIndex);
    strcat( pcResultString, "/etc/" );
  }
  else
  {
    if (iNOBackSlashes == 2)
    {
      /* Hmm not enough backslashes found */
      strcpy( pcResultString, "../etc/");
    }
    else
    {
      /* if the first character is a period then the command is like ./tse */
      if (pcFilename[0] == '.')
      {
        /* Then we have to go up one directory */
        strcpy( pcResultString, "../etc/");
      }
      else
      {
        strcpy( pcResultString, "etc/");
      }
    }
  }
  return(pcResultString);
}

/* LoadProtocolFile                                                         */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*    Filename - name of the protocol file                                  */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/* Checks the file for the correct extension. If it has the correct         */
/*  extension let the Kernel load the protocol or specification file.       */
/* ------------------------------------------------------------------------ */
static int LoadProtocolFile( char *pcFilename )
{
  int     iResult;
  int     iOperationResult;
  char*   pcReason;
  char*   pcExtensionIndex;

  iResult = NOT_OK;
 /* first determine if the filename extension is a test script file extension */
  pcExtensionIndex = strstr( (const char *)pcFilename, pcProtocolFileExtension);
  if (NULL != pcExtensionIndex)
  {
    iOperationResult = BSEK_LoadSpecification(pcFilename);

    switch (iOperationResult)
    {
    case BSEK_FILE_CANNOT_BE_OPENED:
      printf("Protocol file \"%s\" could not be openend\n",pcFilename);
      break;
    case BSEK_SCRIPT_ERROR:
      BSEK_ErrorPosition(&pcReason);
      printf("Error in protocol file %s\nReason %s\n",pcFilename, pcReason);

      free(pcReason);
      pcReason = NULL;
      break;
    case BSEK_KERNEL_BUSY:
      printf("Internal error in file %s on line %s", 
              (char*) __FILE__, (char*)__LINE__);
      break;
    case BSEK_OK:
      iResult = OK;
      break;
    default:
      BSEK_ErrorPosition(&pcReason);
      printf("Unknown error in protocol file %s\nError:%s\n",pcFilename, pcReason);

      free(pcReason);
      pcReason = NULL;
      break;
    }
  }
  return(iResult);
}

/* LoadTestScript                                                           */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*    Filename - name of the test script file                               */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/* Checks the file for the correct extension. If it has the correct         */
/*  extension let the Kernel load the Test script file.                     */
/* ------------------------------------------------------------------------ */
static int LoadTestScript( char *pcFilename )
{
  int     iResult;
  int     iOperationResult;
  char*   pcReason;
  char*   pcExtensionIndex;

  iResult = NOT_OK;
 /* first determine if the filename extension is a test script file extension */
  pcExtensionIndex = strstr( (const char *)pcFilename, pcScriptFileExtension);
  if (NULL != pcExtensionIndex)
  {
    /* Call function in kernel to load and parse the script file */
    iOperationResult = BSEK_LoadScript(pcFilename);

    switch (iOperationResult)
    {
    case BSEK_FILE_CANNOT_BE_OPENED:
      printf("Script file \"%s\" could not be opened.\n", pcFilename);
      BSEK_Shutdown();
      iResult = PROCESSING_ERROR;
      break;
    case BSEK_SCRIPT_ERROR:
      BSEK_ErrorPosition(&pcReason);
      printf("Script error: %s\n",pcReason);

      free(pcReason);
      pcReason = NULL;

      BSEK_Shutdown();
      iResult = PROCESSING_ERROR;
      break;
    case BSEK_NO_SPECIFICATION:
      printf("No specification loaded yet.");
      BSEK_Shutdown();
      iResult = PROCESSING_ERROR;
      break;
    case BSEK_KERNEL_BUSY:
      printf("Internal error in file %s on line %s",
              (char*)__FILE__,(char*)__LINE__);
      BSEK_Shutdown();
      iResult = PROCESSING_ERROR;
      break;
    /* BSEK_LoadScript ok */
    case BSEK_OK:
      iResult = OK;
      break;
    default:
      /* Unknown error occurred */
      /* Kernel function allocates memory for the string */
      BSEK_ErrorPosition(&pcReason);
      printf("Test script %s unknown error: %s\n",pcFilename, pcReason);

      free(pcReason);
      pcReason = NULL;

      BSEK_Shutdown();
      iResult = PROCESSING_ERROR;
      break;
    }
  }

  return(iResult);
}

/* LoadIOFile                                                               */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*    Filename - name of the test script file                               */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/* Checks the file for the correct extension. If it has the correct         */
/*  extension let the Kernel load the IO specification file.                */
/* ------------------------------------------------------------------------ */
static int LoadIOFile( char *pcFilename )
{
  int     iResult;
  int     iOperationResult;
  char*   pcReason;
  char*   pcExtensionIndex;

  iResult = NOT_OK;

 /* first determine if the filename extension is a IO file extension */
  pcExtensionIndex = strstr( (const char *)pcFilename, pcIOFileExtension);
  if (NULL != pcExtensionIndex)
  {
    /* Call function in kernel to parse the IO file */
    iOperationResult = BSEK_LoadIOFile(pcFilename);

    switch (iOperationResult)
    {
    case BSEK_FILE_CANNOT_BE_OPENED:
      printf("IO file \"%s\" cannot be opened.\n",pcFilename);
      BSEK_Shutdown();
      iResult = PROCESSING_ERROR;
      break;
    case BSEK_SCRIPT_ERROR:
      BSEK_ErrorPosition(&pcReason);
      printf("IO File error: %s\n",pcReason);

      free(pcReason);
      pcReason = NULL;

      BSEK_Shutdown();
      iResult = PROCESSING_ERROR;
      break;
    case BSEK_NO_SPECIFICATION:
      printf("No specification loaded yet.");
      BSEK_Shutdown();
      iResult = PROCESSING_ERROR;
      break;
    case BSEK_KERNEL_BUSY:
      printf("Internal error in file %s on line %s",
              (char*)__FILE__,(char*)__LINE__);
      BSEK_Shutdown();
      iResult = PROCESSING_ERROR;
      break;
    /* BSEK_LoadScript ok */
    case BSEK_OK:
      iResult = OK;
      break;
    default:
      /* Unknown error occurred */
      /* Kernel function allocates memory for the string */
      BSEK_ErrorPosition(&pcReason);
      printf("Script unknown error: %s\n",pcReason);

      free(pcReason);
      pcReason = NULL;

      BSEK_Shutdown();
      iResult = PROCESSING_ERROR;
      break;
    }
  }

  return(iResult);
}

/* LoadBatchFile                                                            */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*    Filename - name of the test script file                               */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/* Checks the file for the correct extension. If it has the correct         */
/*  extension let the Kernel load the Batch script     file.                */
/* ------------------------------------------------------------------------ */
static int LoadBatchFile ( char *pcFilename )
{
  int     iResult;
  int     iOperationResult;
  char*   pcReason;
  char*   pcExtensionIndex;

  iResult = NOT_OK;

 /* first determine if the filename extension is a batch file extension */
  pcExtensionIndex = strstr( (const char *)pcFilename, pcBatchFileExtension);
  if (NULL != pcExtensionIndex)
  {
    iOperationResult = BSEK_LoadBatch(pcFilename);

    switch (iOperationResult)
    {
    case BSEK_FILE_CANNOT_BE_OPENED:
      printf("Batch file \"%s\" cannot be opened.\n",pcFilename);

      BSEK_Shutdown();
      iResult = PROCESSING_ERROR;

      break;
    case BSEK_SCRIPT_ERROR:
      /* Kernel function allocates memory for the string */
      BSEK_ErrorPosition(&pcReason);
      printf("Script error in batch: %s\n",pcReason);

      free(pcReason);
      pcReason = NULL;
      iResult = PROCESSING_ERROR;
      break;
    case BSEK_NO_SPECIFICATION:
      printf("No specification loaded yet.");
      BSEK_Shutdown();
      iResult = PROCESSING_ERROR;
      break;
    case BSEK_KERNEL_BUSY:
      printf("While reading batch file:Internal error in file %s on line %s\n",
              (char*)__FILE__,(char*)__LINE__);
      BSEK_Shutdown();
      iResult = PROCESSING_ERROR;
      break;
    case BSEK_OK:
      iResult = OK;
      break;
    default:
      /* Kernel function allocates memory for the string */
      BSEK_ErrorPosition(&pcReason);
      printf("Batch unknown error: %s\n",pcReason);

      free(pcReason);
      pcReason = NULL;

      BSEK_Shutdown();

      iResult = PROCESSING_ERROR;
      break;
    }

  }
  return(iResult);
}

/* OpenLogFile                                                              */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*    Filename - name of the log-file                                       */
/*    Replay   - Indicates if the script is replayed                        */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/* Checks the file for the correct extension. If it has the correct         */
/*  extension let the Kernel open the log-file.                             */
/* ------------------------------------------------------------------------ */
static int OpenLogFile( char *pcFilename, int16 iReplay )
{
  int   iResult;
  int   iOperationResult;
  char *pcReason;
  char *pcExtensionIndex;

  iResult = NOT_OK;
 /* first determine if the filename extension is a test script file extension */
  pcExtensionIndex = strstr( (const char *)pcFilename, pcLogFileExtension);
  if (NULL != pcExtensionIndex)
  {
    iOperationResult = BSEK_OpenLogFile(pcFilename, iReplay);

    switch (iOperationResult)
    {
    case BSEK_FILE_CANNOT_BE_OPENED:
      printf("Log file \"%s\" could not be openend\n",pcFilename);
      break;
    case BSEK_KERNEL_BUSY:
      printf("Internal error in file %s on line %s", (char*)__FILE__, 
            (char*)__LINE__);
      break;
    case BSEK_OK:
      iResult = OK;
      break;
    default:
      BSEK_ErrorPosition(&pcReason);
      printf("Unknown error in log file %s\nError:%s\n",pcFilename, pcReason);

      free(pcReason);
      pcReason = NULL;
      break;
    }
  }
  return(iResult);
}

/* StartScript                                                              */
/* ------------------------------------------------------------------------ */
/* Input parameters :  None                                                 */
/*     ProtocolFilename - The protocol file to be loaded                    */
/*     LogFilename      - The log file to be loaded                         */
/*     Replay           - Replay the script                                 */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/* Loads the necessary files and when all succesfully loaded runs the       */
/* script                                                                   */
/* ------------------------------------------------------------------------ */
static void StartScript(char *pcProtocolFilename, 
                        char *pcLogFilename,
                        int16 iReplay)
{
  int iResult;
  
  /* Load the protocol file */
  if (OK == LoadProtocolFile(pcProtocolFilename))
  {
    /* Load the IO file */
    if (OK == LoadIOFile(pcIOFilename))
    {
        /* Load the Test script file */
      if (OK == LoadTestScript(pcScriptFilename))
      { 
        if (OK == OpenLogFile(pcLogFilename, iReplay))
        {
          RunScript(iReplay);
        }
        else
        {
          printf("Failed to log file %s\n",pcLogFilename);
        }
      }
      else
      {
        printf("Failed to open script file %s\n",pcScriptFilename);
      }
    }
    else
    {
      printf("Failed to open IO file %s\n",pcIOFilename);  
    }
  }
  else
  {
    printf("Failed to protocol file %s\n", pcProtocolFilename);
  }
}
/* RunScript                                                              */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                   */
/*     Replay           - Replay the script                                 */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/* Runs the test script loaded during the LoadTestScript.                   */
/* ------------------------------------------------------------------------ */
static void RunScript( int16 iReplay )
{
  int     iOperationResult;
  char*   pcReason;

  iOperationResult = BSEK_RunScript(iReplay);

  switch (iOperationResult)
  {
  case BSEK_NO_SCRIPT_TO_RUN:
    printf("No script selected.");
    iRunning = FALSE;
    break;
  case BSEK_SCRIPT_ALREADY_RUNS:
    printf("A script is already running.");
    break;
    /* If error occur, prompt the user in the memo log overview. */
  case BSEK_SCRIPT_ERROR:
    /* Kernel function allocates memory for the string */
    BSEK_ErrorPosition(&pcReason);
    printf("Running Test script failure %s\n",pcReason);

    free(pcReason);
    pcReason = NULL;

    iRunning = FALSE;
    break;
   case BSEK_OK:
    iRunning = TRUE;
    iTestMode = TEST_SCRIPT;
    break;
   default:
         /* Unknown error occurred */
    printf("While running script Internal error in file %s on line %s",
          (char*)__FILE__, (char*)__LINE__);
    BSEK_Shutdown();
    iRunning = FALSE;
    break;
  }
}

/* StartBatch                                                               */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*        IOFilename    - filename of the IO file to be used.               */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/* Loads the IO file and batch file, when succesfull it starts the Batch.   */
/* ------------------------------------------------------------------------ */
static void StartBatch( char *pcIOFilename )
{
  int iResult;
  /* Load the IO file */
  if (OK == LoadIOFile(pcIOFilename))
  {
    iResult = LoadBatchFile(pcBatchFilename);
    if (OK == iResult)
    {
      RunBatch();
    }
    else
    {
      printf("Failed to open batch file %s\n",pcBatchFilename);
    }
  }
  else
  {
    printf("Failed to open IO file %s\n",pcIOFilename);  
  }
}

/* RunBatch                                                               */
/* ------------------------------------------------------------------------ */
/* Input parameters :  None                                                 */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/* Runs the batch script loaded during the LoadTestBatch.                   */
/* ------------------------------------------------------------------------ */
static void RunBatch(void)
{
  int16    iResult;
  char    *pcReason;
  
  iResult = BSEK_RunBatch(ParsedIniSettings.iReplay);

  switch (iResult)
  {
  case BSEK_OK:
    iRunning = TRUE;
    iTestMode = TEST_BATCH;
    break;
  case BSEK_NO_BATCH_TO_RUN:
    printf("No batch file selected.\n");
    iRunning = FALSE;
    break;
  case BSEK_BATCH_ALREADY_RUNS:
    printf("Batch already running.\n");
    break;
  default:
    /* Kernel function allocates memory for the string */
    BSEK_ErrorPosition(&pcReason);
    printf("Running Batch file failure %s\n",pcReason);

    free(pcReason);
    pcReason = NULL;
    iRunning = FALSE;
    break;
  }
}

static int SetupSignals( void )
{
  int              iResult;
  struct sigaction SigAct;
  
  iResult = NOT_OK;
   
  SigAct.sa_handler = SigIntHandler;
  SigAct.sa_flags   = 0;
  sigemptyset( &SigAct.sa_mask );
  /* catch the Ctrl-C interrupts for gracefull shutdown during looping*/
  sigaddset( &SigAct.sa_mask, SIGINT );
  if ( sigaction( SIGINT, &SigAct, NULL) == 0 )
  {
    iResult = OK;
  }
  return( iResult );
}
