#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "codingstandard.h"
#include "ini_parser.h"


/* ------------------------------------------------------------------------ */
/* LOCAL DEFINES                                                            */
/* ------------------------------------------------------------------------ */
#define MAX_LINE_BUFFER       (132)
#define TOKEN_COMMENT_CPP     ("//")
#define NOF_OPTIONS           ((uint16) 6)
#define LOOP                  ((uint16) 0)
#define STOP_ON_ERROR         ((uint16) 1)
#define LOG_VIEW              ((uint16) 2)
#define LOG_LINE              ((uint16) 3)
#define LOG_TO_FILE           ((uint16) 4)
#define REPLAY                ((uint16) 5)
/*------------------------------------------------------------------------- */
/* LOCAL FUNCTIONS                                                          */
/* ------------------------------------------------------------------------ */
static void     RemoveWhiteSpaces( char *pcLine );
static void     ProcessLine      ( char* pcLine, int16 iLineCounter );
static void     LowerLine        ( char* pcLine );
static int16    iLineCounter;

/* ------------------------------------------------------------------------ */
/* LOCAL VARIABLES                                                          */
/* ------------------------------------------------------------------------ */
/* The known options of the ini file parser */
static const char *pcOptions[] = { "loop",
                                    "stoponerror",
                                    "logview",
                                    "logline",
                                    "logtofile",
                                    "replay"
                                  };


/* ------------------------------------------------------------------------ */
/* EXTERNAL VARIABLES                                                       */
/* ------------------------------------------------------------------------ */
/* The known keywords or options of the ini file parser */
struct tIniSettings ParsedIniSettings;

/* ParseIniFile                                                             */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*  Filename - filename of the ini file                                     */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/*  Parses line for line of the ini file                                    */
/* ------------------------------------------------------------------------ */
int16 ParseIniFile( char *pcFilename )
{
  int16  iResult;
  int16  iEndOfFile;
  char   pcLineBuffer[MAX_LINE_BUFFER];
  char  *pcGetsResult;
  FILE  *pIniFile;

  /* Setting the resulting settings to the default values */
  ParsedIniSettings.iLoop        = 0;
  ParsedIniSettings.iStopOnError = 1;
  ParsedIniSettings.iLogLine     = 0;
  ParsedIniSettings.iLogView     = 0;
  ParsedIniSettings.iLogToFile   = 0;
  ParsedIniSettings.iReplay      = 0;

  iResult      = FALSE;
  iEndOfFile   = FALSE;
  iLineCounter = 0;
  /* Try opening the ini file for reading */
  pIniFile     = fopen( pcFilename,"r");

  /* When succesfull, process the file */ 
  if (NULL != pIniFile)
  {
    while( iEndOfFile == FALSE )
    {
      /* Read a line from the file */ 
      pcGetsResult = fgets( pcLineBuffer, MAX_LINE_BUFFER, pIniFile );
      if( pcGetsResult == NULL )
      {
        iEndOfFile = TRUE;
      }
      else
      {
        /* and process it */ 
        iLineCounter++;
        ProcessLine( pcLineBuffer, iLineCounter );
      }
    }
    iResult = TRUE; 
  }
  return(iResult);
}

/* ProcessLine                                                              */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*  Line - Line to be processed                                             */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/*  Skips empty lines and lines that start with a comment.                  */
/* ------------------------------------------------------------------------ */
static void     ProcessLine( char* pcLine, int16 iLineCounter )
{
  char*   pcToken;
  uint16  uiCounter;
  uint8   ucFound;


  uiCounter = 0;
  ucFound   = 0;
  /* Skip the empty lines */
  if( (pcLine[0] != '\n') && (pcLine[0] != '\0') && (pcLine[0] != '\r') )
  {
    /* Remove the whitespace from the line, easier to process */
    RemoveWhiteSpaces(pcLine);
    /* check for comments */
    pcToken = strstr( pcLine, TOKEN_COMMENT_CPP );
    /* If there is not a comment on the start of the line, process the line */
    if (pcLine != pcToken)
    {
      if (NULL != pcToken)
      {
        *pcToken = '\0';
      }
      /* all characters of the line to lower case */
      LowerLine(pcLine);
      while ((uiCounter < NOF_OPTIONS) && (ucFound==0))
      {
        /* Check for current option */
        pcToken = strstr(pcLine, pcOptions[uiCounter]);

        if (NULL != pcToken)
        {
          switch (uiCounter)
          {
          case LOOP:
            sscanf(pcLine,"loop=%d",&ParsedIniSettings.iLoop);
            ucFound = 1;
            break;
          case STOP_ON_ERROR:
            sscanf(pcLine,"stoponerror=%d",&ParsedIniSettings.iStopOnError);
            ucFound = 1;
            break;
          case LOG_VIEW:
            sscanf(pcLine,"logview=%d",&ParsedIniSettings.iLogView);
            ucFound = 1;
            break;
          case LOG_LINE:
            sscanf(pcLine,"logline=%d",&ParsedIniSettings.iLogLine);
            ucFound = 1;
            break;
          case LOG_TO_FILE:
            sscanf(pcLine,"logtofile=%d",&ParsedIniSettings.iLogToFile);
            ucFound = 1;
            break;
          case REPLAY:
            sscanf(pcLine,"replay=%d",&ParsedIniSettings.iReplay);
            ucFound = 1;
            break;
          default:
            printf("Error while parsing ini file\n");
            break;
          }
        }
        uiCounter++;
      }
      if (ucFound == 0)
      {
        printf("unknown keyword at line %d\n",iLineCounter);
      }
    }
  }
}

/* RemoveWhiteSpaces                                                        */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*  Line - Line to be processed                                             */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/*  Removes the whitespaces from the Line.                                  */
/* ------------------------------------------------------------------------ */
static void     RemoveWhiteSpaces( char *pcLine )
{
  int iSrcLength;
  int iSrcCounter;
  int isWhiteSpace;
  int iDestCounter;
  char pcStrippedString[MAX_LINE_BUFFER];

  iSrcCounter = 0;
  iSrcLength = (int) (strlen(pcLine)+1);
  iDestCounter = 0;

  if (pcLine != NULL)
  {
    while( iSrcCounter < iSrcLength)
    {
      /* check if the character is a ' ' or '\n' or '\r' or '\t' */
      isWhiteSpace = isspace(pcLine[iSrcCounter]);
      if( isWhiteSpace == 0 )
      {
        pcStrippedString[iDestCounter] = pcLine[iSrcCounter];
        iDestCounter++;
      }
      iSrcCounter++;
    }
    strcpy( pcLine, pcStrippedString);
  }
}

/* LowerLine                                                                */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*  Line - Line to be processed                                             */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value     : None                                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/*  Lowers the case of all characters in Line                               */
/* ------------------------------------------------------------------------ */
static void     LowerLine( char* pcLine )
{
  int16  iIndex;
  int16  iLineLength;
  char   pcLoweCaseString[MAX_LINE_BUFFER];

  iIndex      = 0;
  iLineLength = (int16)(strlen(pcLine)+1);

  for (iIndex = 0; iIndex < iLineLength; iIndex++)
  {
    pcLoweCaseString[iIndex] = tolower(pcLine[iIndex]);
  }
  strcpy(pcLine,pcLoweCaseString);
}
