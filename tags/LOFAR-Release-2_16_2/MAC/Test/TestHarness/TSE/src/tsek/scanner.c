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
// 21/05/2000 E.A. Nijboer            Initial release
// 12/01/2001 E.A. Nijboer            platform test.
// 09/02/2001 E.A. Nijboer            
//
//=========================================================================
*/


#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "general_lib.h"
#include "scanner.h"
#include "atdriver.h" 

typedef int boolean;

#define DELIMITER_CHARS ( char*) " \t\n\r"
                                      /* space, tab, newline, carriage return */
#define CR              '\n'    /* carriage return                      */
#define LF              '\r'    /* line feed                            */
#define ILLEGAL_CHAR    '\377'

#ifndef _MAX_DRIVE
#define _MAX_DRIVE    (3)
#endif
#ifndef _MAX_DIR
#define _MAX_DIR      (256)
#endif
#ifndef _MAX_FNAME
#define _MAX_FNAME    (256)
#endif
#ifndef _MAX_EXT
#define _MAX_EXT      (256)
#endif

 

char     *pHexDigits = "FEDCBA9876543210";
char     *pDecDigits = "9876543210";
char     *pBinDigits = "10";


struct tIncludeRecord
{
  char     *pcFileName;
  int       iLineNumber;
  long      lFilePointer;
  struct tIncludeRecord *ptIncludedBy;
};

struct tDefineRecord
{
  char     *pcIdentifier;
  char     *pcReplacement;
  int       iReplacementType;
  struct tDefineRecord *ptNextDefine;
};


char     *pcCurrentFileName = NULL;
FILE     *InputStream = NULL;
int16     iCurrentLineNumber = 1;
struct tIncludeRecord *ptIncludedBy = NULL;
char      cPutBackChar;
char     *pcPutBackToken = NULL;
boolean   bPutBackCharValid = FALSE;
boolean   bPutBackTokenValid = FALSE;
int16     iPutBackTokenType;
struct tDefineRecord *ptDefineList = NULL;
boolean   bCrCounting = FALSE;
boolean   bLfCounting = FALSE;
boolean   bBlockCrLf = FALSE;

static void      splitpath(
  char    *inpath,
  char    *drv,
  char    *dir,
  char    *fname,
  char    *ext );
char      GetChar(
  void);
void      UngetChar(
  void);
char      GetNewChar(
  void);

int       GetNumberToken(
  char cCharacter,
  char *pcToken);
int       GetIdentifierToken(
  char cCharacter,
  char *pcToken);
int       GetStringToken(
  char cCharacter,
  char *pcToken);
int       GetPrecompilerToken(
  char cCharacter,
  char *pcToken);
int       GetOperatorToken(
  char cCharacter,
  char *pcToken);
int       GetNewToken(
  char *pcToken);

int       AddOrReplaceDefine(
  char *pcMacro,
  char *pcReplacement,
  int RepType);
void      LocateDefine(
  char *pcMacro,
  struct tDefineRecord **pptRecord);
void      DeleteDefineList(
  struct tDefineRecord *ptRecord);
int       PreProcess(
  char *pcToken);
void      DeleteIncludedByRecord(
  void);
void      DeleteIncludeList(
  struct tIncludeRecord *ptRecord);
int       EvaluateMacroList(
  char *pcToken);
int16     AddIncludeRecord(
  void);

struct tDefineRecord *NewDefineRecord(
  );

/******************************************************************************/
/* returns a ptr to the first occurence of cChar in pcString. Returns NULL if */
/* not found.                                                                 */
/******************************************************************************/
void CodeNrToString(
  int16 iCode,
  char *pCode,
  char *pcToken)
{
  if (iCode == DEC_NUMBER_TOKEN)
    sprintf(pCode, "Decimal number");
  else if (iCode == HEX_NUMBER_TOKEN)
    sprintf(pCode, "Hexadecimal number");
  else if (iCode == BIN_NUMBER_TOKEN)
    sprintf(pCode, "Binary number");
  else if (iCode == STRING_TOKEN)
    sprintf(pCode, "String");
  else if (iCode == OPERATOR_TOKEN)
    sprintf(pCode, "Operator");
  else if (iCode == PRECOMPILER_TOKEN)
    sprintf(pCode, "Precompiler token");
  else if (iCode == OPEN_PARENTHESES_TOKEN)
    sprintf(pCode, "( token");
  else if (iCode == CLOSE_PARENTHESES_TOKEN)
    sprintf(pCode, ") token");
  else if (iCode == OPEN_BRACKETS_TOKEN)
    sprintf(pCode, "{ token");
  else if (iCode == CLOSE_BRACKETS_TOKEN)
    sprintf(pCode, "} token");
  else if (iCode == IDENTIFIER_TOKEN)
    sprintf(pCode, "Identifier");
  else if (iCode == DOT_TOKEN)
    sprintf(pCode, "Dot");
  else if (iCode == COMMA_TOKEN)
    sprintf(pCode, "Comma");
  else if (iCode == SEMICOLON_TOKEN)
    sprintf(pCode, "Semicolon");
  else if (iCode == COLON_TOKEN)
    sprintf(pCode, "Colon");
  else if (iCode == LEFT_ARROW_TOKEN)
    sprintf(pCode, "Left arrow");
  else if (iCode == RIGHT_ARROW_TOKEN)
    sprintf(pCode, "Right arror");
  else if (iCode == COMMENT_TOKEN)
    sprintf(pCode, "Comment");

  else if (iCode == UNEXPECTED_CHAR_READ)
    sprintf(pCode, "Unexpected char %s", pcToken);
  else if (iCode == UNTERMINATED_STRING_READ)
    sprintf(pCode, "Unterminated string");
  else if (iCode == BAD_MACRO_DEFINITION)
    sprintf(pCode, "Bad macro definition");
  else if (iCode == INCLUDE_FILE_PROBLEM)
    sprintf(pCode, "Cannot open include file %s", pcToken);
  else if (iCode == UNKNOWN_PREPROCESSOR)
    sprintf(pCode, "%s is an unknown preprocessor directive", pcToken);
  else if (iCode == OUT_OF_MEMORY)
    sprintf(pCode, "Out of memory.");
  else if (iCode == CANNOT_OPEN_FILE)
    sprintf(pCode, "Cannot open file %s", pcToken);
  else if (iCode == END_OF_FILE)
    sprintf(pCode, "End of file");
  else
    sprintf(pCode, "Internal error. Cannot convert iCode %d to a string",
            iCode);
}

/******************************************************************************/
/* returns a ptr to the first occurence of cChar in pcString. Returns NULL if */
/* not found.                                                                 */
/******************************************************************************/
char     *chrpos(
  char cChar,
  char *pcString)
{
  while (*pcString)
  {
    if (cChar == *pcString)
      return pcString;
    else
      pcString++;
  }
  return NULL;
}

boolean isDigit(
  char cChar)
{
  return ((cChar >= '0') && (cChar <= '9'));
}


boolean isAlpha(
  char cChar)
{
  boolean   bResult = FALSE;

  if ((cChar >= 'A') && (cChar <= 'Z'))
    bResult = TRUE;
  if ((cChar >= 'a') && (cChar <= 'z'))
    bResult = TRUE;
  if ((cChar == '_'))
    bResult = TRUE;
  return bResult;
}


void ShutDownScanner(
  void)
{
  struct tDefineRecord *ptCurrentDefineList;
  
  if (pcCurrentFileName != NULL)
  {
    free(pcCurrentFileName);
    pcCurrentFileName = NULL;
  }

  while (NULL != ptDefineList)
  {
    if (NULL != ptDefineList->pcIdentifier)
    {
      free(ptDefineList->pcIdentifier);
    }
    if (NULL != ptDefineList->pcReplacement)
    {
      free(ptDefineList->pcReplacement);
    }
    ptCurrentDefineList = ptDefineList;
    ptDefineList = ptDefineList->ptNextDefine;
    free(ptCurrentDefineList);
  }

  if (pcPutBackToken != NULL)
  {
    free(pcPutBackToken);
    pcPutBackToken = NULL;
  }
}



int InitScanner(
  void)
/******************************************************************************/
/* All open files are beiing closed, all NULL pointers are replaced by        */
/* valid pointers to a memory block of predefined size, except the tInclude-  */
/* Record chain. This chain is completely freed and initialized to NULL       */
/*                                                                            */
/* Return values: NO_SCANNER_ERROR      : function completed successfully     */
/*                OUT_OF_MEMORY : initialisation failed due to lack of memory */
/******************************************************************************/
{
  int16     iReturnValue = NO_SCANNER_ERROR;

  if (pcPutBackToken == NULL)
  {
    pcPutBackToken = malloc(TOKENSIZE);
    if (pcPutBackToken == NULL) /* if failed,                          */
    {
      iReturnValue = OUT_OF_MEMORY;     /* return an error.                    */
    }
    bPutBackTokenValid = FALSE;
  }

  bPutBackCharValid = FALSE;

  if (pcCurrentFileName == NULL)
  {
    if (iReturnValue == NO_SCANNER_ERROR)       /* if previous malloc succeeded */
    {
      pcCurrentFileName = malloc(TOKENSIZE);
      if (pcCurrentFileName == NULL)    /* if this malloc failed,              */
      {
        iReturnValue = OUT_OF_MEMORY;   /* return an error.                    */
      }
    }
  }

  if (InputStream != NULL)      /* if there is a file open             */
  {
    fclose(InputStream);        /* close that file                     */
    InputStream = NULL;
  }

  if (ptIncludedBy != NULL)     /* if there is an include chain        */
  {
    DeleteIncludeList(ptIncludedBy);
    ptIncludedBy = NULL;
  }

  /* an empty DefineList consist of one record containing all NULL pointers   */
  if (ptDefineList == NULL)
  {
    if (iReturnValue == NO_SCANNER_ERROR)
    {
      ptDefineList = malloc(sizeof(struct tDefineRecord));
      if (ptDefineList == NULL)
      {
        iReturnValue = OUT_OF_MEMORY;
      }
      else
      {
        ptDefineList->pcIdentifier = NULL;
        ptDefineList->pcReplacement = NULL;
        ptDefineList->ptNextDefine = NULL;
      }
    }
  }
  else
  {
    if (ptDefineList->ptNextDefine != NULL)
    {
      DeleteDefineList(ptDefineList->ptNextDefine);
      ptDefineList->ptNextDefine = NULL;
    }
  }
  return iReturnValue;          /* return the result                   */
}

char     *CurrentFile(
  void)
/******************************************************************************/
/* Return a pointer to the name of the current file.                          */
/******************************************************************************/
{
  return pcCurrentFileName;
}

int CurrentLine(
  void)
/******************************************************************************/
/* Return the current line number.                                            */
/******************************************************************************/
{
  return iCurrentLineNumber;
}

char     *NameOf(
  int iTokenType)
{
  switch (iTokenType)
  {
    case DEC_NUMBER_TOKEN:
      return ("DEC_NUMBER_TOKEN");
      break;
    case HEX_NUMBER_TOKEN:
      return ("HEX_NUMBER_TOKEN");
      break;
    case BIN_NUMBER_TOKEN:
      return ("BIN_NUMBER_TOKEN");
      break;
    case STRING_TOKEN:
      return ("STRING_TOKEN");
      break;
    case OPERATOR_TOKEN:
      return ("OPERATOR_TOKEN");
      break;
    case PRECOMPILER_TOKEN:
      return ("PRECOMPILER_TOKEN");
      break;
    case OPEN_PARENTHESES_TOKEN:
      return ("OPEN_PARENTHESES_TOKEN");
      break;
    case CLOSE_PARENTHESES_TOKEN:
      return ("CLOSE_PARENTHESES_TOKEN");
      break;
    case OPEN_BRACKETS_TOKEN:
      return ("OPEN_BRACKETS_TOKEN");
      break;
    case CLOSE_BRACKETS_TOKEN:
      return ("CLOSE_BRACKETS_TOKEN");
      break;
    case IDENTIFIER_TOKEN:
      return ("IDENTIFIER_TOKEN");
      break;
    case DOT_TOKEN:
      return ("DOT_TOKEN");
      break;
    case COMMA_TOKEN:
      return ("COMMA_TOKEN");
      break;
    case SEMICOLON_TOKEN:
      return ("SEMICOLON_TOKEN");
      break;
    case COLON_TOKEN:
      return ("COLON_TOKEN");
      break;
    case LEFT_ARROW_TOKEN:
      return ("LEFT_ARROW_TOKEN");
      break;
    case RIGHT_ARROW_TOKEN:
      return ("RIGHT_ARROW_TOKEN");
      break;
    case COMMENT_TOKEN:
      return ("COMMENT_TOKEN");
      break;
    case TILDE_TOKEN:
      return ("TILDE_TOKEN");
      break;
    case BACKSLASH_TOKEN:
      return ("BACKSLASH_TOKEN");
      break;
    case FLOAT_TOKEN:
      return ("FLOAT_TOKEN");
      break;
    default:
      return ("-- internal error --");
      break;
  }
}

char     *FileIncludedBy(
  void)
{
  char     *pcText;
  char     *pcLine;
  struct tIncludeRecord *ptWalker;

  pcText = malloc(10240);
  pcLine = malloc(1024);
  (*pcText) = 0;
  (*pcLine) = 0;
  ptWalker = ptIncludedBy;

  if (iPutBackTokenType == INCLUDE_FILE_PROBLEM)
  {
    if (ptWalker != NULL)
    {
      ptWalker = ptWalker->ptIncludedBy;
    }
  }

  while (ptWalker != NULL)
  {
    sprintf(pcLine,
            "Included by %s, line %d\n",
            ptWalker->pcFileName, ptWalker->iLineNumber);
    strcat(pcText, pcLine);
    ptWalker = ptWalker->ptIncludedBy;
  }

  free(pcLine);
  return pcText;
}

char GetChar(
  void)
/******************************************************************************/
/* get a char from the input file. Return a put back char if available.       */
/******************************************************************************/
{
  char      cReturnValue;

  if (bPutBackCharValid == TRUE)
  {
    /* "re-read" the ungetted char. Correct the linecounter if nessecary. */
    cReturnValue = cPutBackChar;
    bPutBackCharValid = FALSE;

    if (((cPutBackChar == LF) && (bLfCounting == TRUE))
        || ((cPutBackChar == CR) && (bCrCounting == TRUE)))
    {
      iCurrentLineNumber++;
    }
  }
  else
  {
    cReturnValue = GetNewChar();
  }

  if ((cReturnValue == CR) || (cReturnValue == LF))
  {
    if (bBlockCrLf)
    {
      UngetChar();
      cReturnValue = ILLEGAL_CHAR;
    }
  }

  return cReturnValue;
}

char GetNewChar(
  void)
/******************************************************************************/
/* get a new char from the input file, don't check for a put back char        */
/******************************************************************************/
{
  boolean   bLastFile = FALSE;
  boolean   bEscapeUsed = FALSE;

  if (InputStream != NULL)
  {
    cPutBackChar = (char) getc(InputStream);
    if (cPutBackChar == '\\')
    {
      bEscapeUsed = TRUE;
      cPutBackChar = (char) getc(InputStream);
      switch (cPutBackChar)
      {
        case 'n':
          cPutBackChar = '\n';
          break;
        case 'r':
          cPutBackChar = '\r';
          break;
        case 't':
          cPutBackChar = '\t';
          break;
        case '\\':
          cPutBackChar = '\\';
          break;
      }
    }
  }

  /* feof(InputStream) is not evaluated if InputStream equals NULL            */
  while (((InputStream == NULL) || (feof(InputStream))) && !bLastFile)
  {
    /* last read char is invalid. Try to return to previous file.           */
    if (ptIncludedBy != NULL)
    {
      DeleteIncludedByRecord();
      cPutBackChar = (char) getc(InputStream);
      if (cPutBackChar == '\\')
      {
        bEscapeUsed = TRUE;
        cPutBackChar = (char) getc(InputStream);
        switch (cPutBackChar)
        {
          case 'n':
            cPutBackChar = '\n';
            break;
          case 'r':
            cPutBackChar = '\r';
            break;
          case 't':
            cPutBackChar = '\t';
            break;
          case '\\':
            cPutBackChar = '\\';
            break;
        }
      }
    }
    else
    {
      /* this file is not included by another file. Escape from the while    */
      /* loop to prevent an endless loop.                                    */
      bLastFile = TRUE;
      cPutBackChar = 0;
    }
  }

  if ((cPutBackChar == LF) && (bEscapeUsed == FALSE))
  {
    if (bCrCounting == FALSE)
      bLfCounting = TRUE;
    if (bLfCounting == TRUE)
      iCurrentLineNumber++;
  }
  else if ((cPutBackChar == CR) && (bEscapeUsed == FALSE))
  {
    if (bLfCounting == FALSE)
      bCrCounting = TRUE;
    if (bCrCounting == TRUE)
      iCurrentLineNumber++;
  }
  return cPutBackChar;
}

void UngetChar(
  void)
/******************************************************************************/
/* put the last read char back in the input stream.                           */
/******************************************************************************/
{
  bPutBackCharValid = TRUE;

  /* adjust line counter, if a cr or lf is put back into the stream. */
  if (((cPutBackChar == LF) && (bLfCounting == TRUE))
      || ((cPutBackChar == CR) && (bCrCounting == TRUE)))

  {
    iCurrentLineNumber--;
  }
}



int GetToken(
  char *pcToken)
/******************************************************************************/
/* get a token from the input file. Return a put back token if available      */
/******************************************************************************/
{
  int       iResult;

  if (bPutBackTokenValid == TRUE)
  {
    strcpy((char *) pcToken, (char *) pcPutBackToken);
    bPutBackTokenValid = FALSE;
    iResult = iPutBackTokenType;
  }
  else
  {
    iResult = GetNewToken(pcToken);

    while (iResult == PRECOMPILER_TOKEN)
    {
      iResult = PreProcess(pcToken);
      if (iResult == NO_SCANNER_ERROR)
      {
        iResult = GetNewToken(pcToken);
      }
    }

    if (iResult == IDENTIFIER_TOKEN)
    {
      iResult = EvaluateMacroList(pcToken);
    }

    iPutBackTokenType = iResult;
  }

  strcpy((char *) pcPutBackToken, (char *) pcToken);

  return iResult;
}



void UngetToken(
  void)
/******************************************************************************/
/* put a token back in the input stream: Make GetToken gets the same token as */
/* the last time..                                                            */
/******************************************************************************/
{
  bPutBackTokenValid = TRUE;
}


int GetNewToken(
  char *pcToken)
/******************************************************************************/
/* get a new token from the input file. Don't check for a put back token      */
/* characters are classified as:                                              */
/* cr        :== carriage return character                                    */
/* lf        :== line feed character                                          */
/* tab       :== the tab character                                            */
/* dquote    :== the " character                                              */
/* digit     :== 0123456789                                                   */
/* bindigit  :== 01                                                           */
/* hexdigit  :== 0123456789ABCDEF                                             */
/* alpha     :== _abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ        */
/* any       :== all prtble chars except cr, lf and dqoute                    */
/* tab       :== the tab character                                            */
/* alphanum  :== alpha | digit                                                */
/*                                                                            */
/* <x>       :== zero or more x                                               */
/* [x]       :== one or more x                                                */
/*  x        :== exactly one x                                                */
/*                                                                            */
/* recognized tokens are :                                                    */
/*                                                                            */
/* FLOAT_TOKEN              :== <digit> '.' [digit]                           */
/* DEC_NUMBER_TOKEN         :== [digit]                                       */
/* HEX_NUMBER_TOKEN         :== '0' 'x' [hexdigit]                            */
/* BIN_NUMBER_TOKEN         :== '0' 'b' [bindigit]                            */
/* IDENTIFIER_TOKEN         :== alpha <alphanum>                              */
/* STRING_TOKEN             :== dquote <any> dquote                           */
/* OPERATOR_TOKEN           :== '='                                           */
/*                          :== '=' '='                                       */
/*                          :== '<'                                           */
/*                          :== '>'                                           */
/*                          :== '<' '='                                       */
/*                          :== '>' '='                                       */
/*                          :== '<' '>'                                       */
/*                          :== '+'                                           */
/*                          :== '-'                                           */
/*                          :== '*'                                           */
/*                          :== '/'                                           */
/*                          :== '<' '<'                                       */
/*                          :== '>' '>'                                       */
/*                          :== '&'                                           */
/*                          :== '|'                                           */
/*                          :== '[' ']'                                       */
/*                          :== '['                                           */
/*                          :== ']'                                           */
/* DOT_TOKEN                :== '.'                                           */
/* DOTDOT_TOKEN             :== '.' '.'                                       */
/* COMMA_TOKEN              :== ','                                           */
/* SEMICOLON_TOKEN          :== ';'                                           */
/* COLON_TOKEN              :== ':'                                           */
/* LEFT_ARROW_TOKEN         :== '<' '-'                                       */
/* RIGHT_ARROW_TOKEN        :== '-' '>'                                       */
/* OPEN_PARENTHESE_TOKEN    :== '('                                           */
/* CLOSE_PARENTHESE_TOKEN   :== ')'                                           */
/* PRECOMPILER_TOKEN        :== '#' [alpha]                                   */
/* DELIMITER_TOKEN          :== ' '                                           */
/*                          :== tab                                           */
/*                          :== cr                                            */
/*                          :== lf                                            */
/******************************************************************************/
{
  char      cCharacter;
  int       iResult;
  int       iCurrentLine;
  boolean   bEoLn;

  char     *pcTokenWalker;

  pcTokenWalker = pcToken;

  cCharacter = GetChar();
  /* first skip all delimiter chars */
  while (chrpos((char) cCharacter, DELIMITER_CHARS) != NULL)
  {
    cCharacter = GetChar();
  }

  if (isDigit(cCharacter))
  {
    /*    read a DEC_NUMBER_TOKEN */
    /* or read a HEX_NUMBER_TOKEN */
    /* or read a BIN_NUMBER_TOKEN */
    iResult = GetNumberToken(cCharacter, pcToken);
  }
  else if (isAlpha(cCharacter))
  {
    /* read an IDENTIFIER_TOKEN */
    iResult = GetIdentifierToken(cCharacter, pcToken);
  }
  else if (cCharacter == '"')
  {
    /* read a STRING_TOKEN */
    iResult = GetStringToken(cCharacter, pcToken);
  }
  else if (chrpos(cCharacter, "=<>+-*/&|[].") != NULL)
  {
    /* read an OPERATOR_TOKEN */
    iResult = GetOperatorToken(cCharacter, pcToken);
    if (strcmp((char *) pcToken, "->") == 0)
      iResult = RIGHT_ARROW_TOKEN;
    else if (strcmp((char *) pcToken, "<-") == 0)
      iResult = LEFT_ARROW_TOKEN;
    else if (strcmp((char *) pcToken, "..") == 0)
      iResult = DOTDOT_TOKEN;
    else if (strcmp((char *) pcToken, "//") == 0)
    {
      iCurrentLine = CurrentLine();

      /* bEoLn (EndOfLine) will be set to true if end of line is reached    */
      /* undetected because of end of file. (This will also happen in the   */
      /* unlikely situation that an inputfile contains something like:      */
      /* #define COMMENT //this is comment                                  */
      /* because while parsing a preprocessor macro, the scanner refuses to */
      /* go to the next line.                                               */

      bEoLn = FALSE;
      while ((iCurrentLine == CurrentLine()) && (!bEoLn))
      {
        cCharacter = GetChar();
        if ((cCharacter != ILLEGAL_CHAR) && (cCharacter != '\0'))
        {
          *(pcToken++) = cCharacter;
        }
        else
        {
          bEoLn = TRUE;
        }
      }
      iResult = COMMENT_TOKEN;

      if (!bEoLn)
      {
        /* End of comment detected by reading a Cr/Lf character. This       */
        /* read character is added to pcToken.                              */
        /* Put cr/lf back, only to adjust the line counter...               */
        /* (only needed if not in the middle of a macro.)                   */
        UngetChar();

        /* Delete cr or lf character from pcToken by overwriting it with    */
        /* the zero character (for terminating pcToken)                     */
        *(--pcToken) = 0;

      }
      else
      {
        /* Terminate pcToken by adding a zero character                     */
        *(pcToken) = 0;
      }

    }

  }
  else if (cCharacter == '#')
  {
    /* read a PRECOMPILER_TOKEN */
    iResult = GetPrecompilerToken(cCharacter, pcToken);
  }
  else if (cCharacter == '{')
  {
    iResult = OPEN_BRACKETS_TOKEN;
    (*pcTokenWalker++) = cCharacter;
    *pcTokenWalker = 0;
  }
  else if (cCharacter == '}')
  {
    iResult = CLOSE_BRACKETS_TOKEN;
    (*pcTokenWalker++) = cCharacter;
    *pcTokenWalker = 0;
  }
  else if (cCharacter == '(')
  {
    iResult = OPEN_PARENTHESES_TOKEN;
    (*pcTokenWalker++) = cCharacter;
    *pcTokenWalker = 0;
  }
  else if (cCharacter == ')')
  {
    iResult = CLOSE_PARENTHESES_TOKEN;
    (*pcTokenWalker++) = cCharacter;
    *pcTokenWalker = 0;
  }
  else if (cCharacter == '.')
  {
    iResult = DOT_TOKEN;
    (*pcTokenWalker++) = cCharacter;

    cCharacter = GetChar();

    while (isDigit(cCharacter))
    {
      iResult = FLOAT_TOKEN;
      (*pcTokenWalker++) = cCharacter;
      cCharacter = GetChar();
    }
    UngetChar();
    *pcTokenWalker = 0;
  }
  else if (cCharacter == ',')
  {
    iResult = COMMA_TOKEN;
    (*pcTokenWalker++) = cCharacter;
    *pcTokenWalker = 0;
  }
  else if (cCharacter == ';')
  {
    iResult = SEMICOLON_TOKEN;
    (*pcTokenWalker++) = cCharacter;
    *pcTokenWalker = 0;
  }
  else if (cCharacter == ':')
  {
    iResult = COLON_TOKEN;
    (*pcTokenWalker++) = cCharacter;
    *pcTokenWalker = 0;
  }
  else if (cCharacter == '~')
  {
    iResult = TILDE_TOKEN;
    (*pcTokenWalker++) = cCharacter;
    *pcTokenWalker = 0;
  }
  else if (cCharacter == '\\')
  {
    iResult = BACKSLASH_TOKEN;
    (*pcTokenWalker++) = cCharacter;
    *pcTokenWalker = 0;
  }
  else if (cCharacter == 0)
  {
    /* End of file reached. */
    iResult = END_OF_FILE;
    (*pcTokenWalker++) = cCharacter;
  }
  else
  {
    /* unexpected character read */
    iResult = UNEXPECTED_CHAR_READ;
    (*pcTokenWalker++) = cCharacter;
    *pcTokenWalker = 0;
  }
  return iResult;
}

int GetNumberToken(
  char cCharacter,
  char *pcToken)
/* DEC_NUMBER_TOKEN         :== [digit]                                       */
/* HEX_NUMBER_TOKEN         :== '0' 'x' [hexdigit]                            */
/* BIN_NUMBER_TOKEN         :== '0' 'b' [bindigit]                            */
/* FLOAT_TOKEN              :== <digit> '.' [digit]                           */
{
  char     *pDigitClass;
  int       iResult = DEC_NUMBER_TOKEN;

  *(pcToken++) = cCharacter;
  cCharacter = GetChar();
  if (cCharacter == 'x')
  {
    *(pcToken++) = cCharacter;
    pDigitClass = pHexDigits;
    cCharacter = GetChar();
    iResult = HEX_NUMBER_TOKEN;
  }
  else if (cCharacter == 'b')
  {
    *(pcToken++) = cCharacter;
    pDigitClass = pBinDigits;
    cCharacter = GetChar();
    iResult = BIN_NUMBER_TOKEN;
  }
  else
    pDigitClass = pDecDigits;

  while (chrpos(cCharacter, pDigitClass) != NULL)
  {
    *(pcToken++) = cCharacter;
    cCharacter = GetChar();
  }

  if ((iResult == DEC_NUMBER_TOKEN) && (cCharacter == '.'))
  {
    *(pcToken++) = cCharacter;
    cCharacter = GetChar();

    iResult = FLOAT_TOKEN;

    while (chrpos(cCharacter, pDigitClass) != NULL)
    {
      *(pcToken++) = cCharacter;
      cCharacter = GetChar();
    }

    UngetChar();
  }

  *pcToken = 0;
  UngetChar();

  return iResult;
}

int GetIdentifierToken(
  char cCharacter,
  char *pcToken)
/* IDENTIFIER_TOKEN         :== alpha <alphanum>                              */
{
  *(pcToken++) = cCharacter;
  cCharacter = GetChar();
  while (isAlpha(cCharacter) || isDigit(cCharacter) || (cCharacter == '_'))
  {
    *(pcToken++) = cCharacter;
    cCharacter = GetChar();
  }

  *pcToken = 0;
  UngetChar();

  return IDENTIFIER_TOKEN;
}


int GetStringToken(
  char cCharacter,
  char *pcToken)
/* STRING_TOKEN             :== dquote <any> dquote                           */
{
  int       iResult;
  int       iCurrentLine;

  iCurrentLine = CurrentLine();

  cCharacter = GetChar();
  while ((cCharacter != '"') && (iCurrentLine == CurrentLine()))
  {
    *(pcToken++) = cCharacter;
    cCharacter = GetChar();
  }

  /* end of line, or " is found. Check for error and try to recover           */
  if (iCurrentLine == CurrentLine())
  {
    iResult = STRING_TOKEN;
  }
  else
  {
    iResult = UNTERMINATED_STRING_READ;
    UngetChar();
  }

  *pcToken = 0;
  return iResult;
}

int GetPrecompilerToken(
  char cCharacter,
  char *pcToken)
/* PRECOMPILER_TOKEN        :== '#' [alpha]                                   */
{
  *(pcToken++) = cCharacter;
  cCharacter = GetChar();
  while (isAlpha(cCharacter))
  {
    *(pcToken++) = cCharacter;
    cCharacter = GetChar();
  }
  UngetChar();
  *pcToken = 0;

  return PRECOMPILER_TOKEN;
}


int GetOperatorToken(
  char cCharacter,
  char *pcToken)
/* OPERATOR_TOKEN           :== '='                                           */
/*                          :== '=' '='                                       */
/*                          :== '<'                                           */
/*                          :== '>'                                           */
/*                          :== '<' '='                                       */
/*                          :== '>' '='                                       */
/*                          :== '<' '>'                                       */
/*                          :== '+'                                           */
/*                          :== '-'                                           */
/*                          :== '*'                                           */
/*                          :== '/'                                           */
/*                          :== '<' '<'                                       */
/*                          :== '>' '>'                                       */
/*                          :== '&'                                           */
/*                          :== '|'                                           */
/*                          :== '[' ']'                                       */
/* The following two tokens are read as OPERATOR_TOKEN, but are translated    */
/* to LEFT_ARROW_TOKEN or RIGHT_ARROW_TOKEN before leaving the scanner.       */
/*                          :== '-' '>'                                       */
/*                          :== '<' '-'                                       */
{
  char     *pcCompleteToken;

  pcCompleteToken = pcToken;    /* store pointer to first token char */
  *(pcToken++) = cCharacter;    /* assign first character to token   */
  if (chrpos(cCharacter, "-=<>[/.") != NULL)    /* if second character possible      */
  {
    cCharacter = GetChar();     /*  read second character          */
    if (chrpos(cCharacter, "-=<>]/.") != NULL)  /*  if valid second character      */
    {
      *pcToken = cCharacter;    /*     add character               */
      pcToken[1] = 0;           /*     and make it zero terminated */

      /* now a 2 byte operator is read. Check if this is a valid two byte     */
      /* operator. If not, put back the second character in the input stream. */
      if (strpos(pcCompleteToken, "== <= >= <> << >> [] -> <- // ..") == NULL)
      {
        UngetChar();
      }
      else
      {
        pcToken++;
      }
    }
    else
    {
      UngetChar();              /*    else put second character back */
    }
  }
  *pcToken = 0;                 /* terminate token                   */
  return OPERATOR_TOKEN;
}


int AddOrReplaceDefine(
  char *pcMacro,
  char *pcReplacement,
  int iRepType)
/******************************************************************************/
/* Adds or replaces a record in the #define macro list                        */
/*                                                                            */
/* Return values: NO_SCANNER_ERROR      : function completed successfully     */
/*                OUT_OF_MEMORY : addition failed due to lack of memory       */
/******************************************************************************/
{
  struct tDefineRecord *ptDefineRecord;
  int       iResult = NO_SCANNER_ERROR;

  /* first find matching (or empty) record */
  LocateDefine(pcMacro, &ptDefineRecord);

  if (ptDefineRecord == NULL)
  {
    iResult = OUT_OF_MEMORY;
  }
  else
  {
    if (ptDefineRecord->pcIdentifier == NULL)
    {
      /* found an empty one. Initialise empty record */
      ptDefineRecord->pcIdentifier = malloc(strlen(pcMacro) + 1);
      strcpy(ptDefineRecord->pcIdentifier, pcMacro);
    }
    if (ptDefineRecord->pcReplacement != NULL)
    {
      /* there was already a replacement. Delete old replacement */
      free(ptDefineRecord->pcReplacement);
    }
    ptDefineRecord->pcReplacement = malloc(strlen(pcReplacement) + 1);
    strcpy(ptDefineRecord->pcReplacement, pcReplacement);
    ptDefineRecord->iReplacementType = iRepType;
  }
  return iResult;
}




boolean RecordIsMatching(
  struct tDefineRecord * ptRecord,
  char *pcMacro)
/******************************************************************************/
/*                                                                            */
/* returns true if the record is eigther matching or empty.                   */
/*                                                                            */
/******************************************************************************/
{
  boolean   bResult;

  if (ptRecord == NULL)
  {
    bResult = TRUE;
  }
  else if (ptRecord->pcIdentifier == NULL)
  {
    bResult = TRUE;
  }
  else
  {
    bResult = (strcmp(ptRecord->pcIdentifier, pcMacro) == 0);
  }
  return bResult;
}




void LocateDefine(
  char *pcMacro,
  struct tDefineRecord **pptRecord)
/******************************************************************************/
/*                                                                            */
/* Returns a pointer to eighter an empty record, or to a pcMacro matching     */
/* record. If there is no matching and no empty record in the list, an empty  */
/* record is added to the list.                                               */
/* if pptRecord is pointing to a NULL pointer, a malloc runned out of memory. */
/*                                                                            */
/******************************************************************************/
{
  struct tDefineRecord *ptRecord;

  ptRecord = ptDefineList;

  while (RecordIsMatching(ptRecord, pcMacro) == FALSE)
  {
    if (ptRecord->ptNextDefine == NULL)
    {
      ptRecord->ptNextDefine = NewDefineRecord();
    }
    ptRecord = ptRecord->ptNextDefine;
  }
  *pptRecord = ptRecord;
}




struct tDefineRecord *NewDefineRecord(
  )
{
  struct tDefineRecord *ptRecord;
  ptRecord = (struct tDefineRecord *) malloc(sizeof(struct tDefineRecord));
  if (ptRecord != NULL)
  {
    ptRecord->pcIdentifier = NULL;
    ptRecord->pcReplacement = NULL;
    ptRecord->ptNextDefine = NULL;
    ptRecord->iReplacementType = 0;
  }
  return ptRecord;
}


int EvaluateMacroList(
  char *pcToken)
{
  struct tDefineRecord *ptDefineRecord;
  int       iReturnTokenType;

  LocateDefine(pcToken, &ptDefineRecord);

  if (ptDefineRecord == NULL)
  {
    /* this should never happen, LocateDefine should always return an (even */
    /* tually empty) record. However, to be failsafe, this condition is     */
    /* checked.                                                             */
    iReturnTokenType = IDENTIFIER_TOKEN;
  }
  else if (ptDefineRecord->pcIdentifier == NULL)
  {
    /* An empty record is found, this means the just read identifier is not */
    /* in the macro list. A translation does not need to be made.           */
    iReturnTokenType = IDENTIFIER_TOKEN;
  }
  else
  {
    /* A filled record is found, this is a macro. The identifier will be    */
    /* replaced by it's replacement.                                        */
    strcpy(pcToken, ptDefineRecord->pcReplacement);
    iReturnTokenType = ptDefineRecord->iReplacementType;
  }
  return iReturnTokenType;
}






struct tIncludeRecord *NewIncludeRecord(
  )
{
  struct tIncludeRecord *ptRecord;
  ptRecord = (struct tIncludeRecord *) malloc(sizeof(struct tIncludeRecord));
  if (ptRecord != NULL)
  {
    ptRecord->pcFileName = NULL;
    ptRecord->iLineNumber = 0;
    ptRecord->lFilePointer = 0;
    ptRecord->ptIncludedBy = NULL;
  }
  return ptRecord;
}




void DeleteDefineList(
  struct tDefineRecord *ptRecord)
{
  if (ptRecord->ptNextDefine != NULL)
  {
    DeleteDefineList(ptRecord->ptNextDefine);
  }
  free(ptRecord->pcIdentifier);
  free(ptRecord->pcReplacement);
  free(ptRecord);
}



void DeleteIncludeList(
  struct tIncludeRecord *ptRecord)
{
  if (NULL != ptRecord)
  {
    if (ptRecord->ptIncludedBy != NULL)
    {
      DeleteIncludeList(ptRecord->ptIncludedBy);
    }
    if (NULL != ptRecord->pcFileName)
    {
      free(ptRecord->pcFileName);
    }
    free(ptRecord);
  }
}


int PreProcessDefine(
  char *pcToken)
{
  int       iResult;
  int       iTokenType;

  char     *pcExpansion;

  pcExpansion = malloc(TOKENSIZE);
  if (pcExpansion == NULL)
  {
    iResult = OUT_OF_MEMORY;
  }
  else
  {
    iTokenType = GetNewToken(pcToken);
    if (iTokenType != IDENTIFIER_TOKEN)
    {
      /* Identifier expected but not found. Skip next token as an attempt to   */
      /* keep in sync.                                                         */
      iResult = GetNewToken(pcToken);

      /* hide last read token, and report the error                            */
      *pcToken = 0;
      iResult = BAD_MACRO_DEFINITION;
    }
    else
    {
      iTokenType = GetNewToken(pcExpansion);
      AddOrReplaceDefine(pcToken, pcExpansion, iTokenType);
      iResult = NO_SCANNER_ERROR;
    }
    free(pcExpansion);
  }
  return iResult;
}


int PreProcessInclude(
  char *pcToken)
{
  int16     iResult;

  char     *pcDrive;
  char     *pcDir;

  char     *pcNewDrive;
  char     *pcNewDir;
  char     *pcNewFilename;
  char     *pcNewExtension;

  char     *pcNewFullFilename;

  iResult = GetNewToken(pcToken);
  if (iResult == STRING_TOKEN)
  {
    /* switch to the included file. First add a record in the IncludedBy     */
    /* list, fill this record with the current filename, and position of the */
    /* read pointer. Then close the current file and open the include file.  */
    /* Continue with the newly opened file.                                  */

    iResult = AddIncludeRecord();
    if (iResult == NO_SCANNER_ERROR)
    {
      fclose(InputStream);

      /* Now do some difficult things to retrieve the current directory,     */
      /* which is the directory in which the current file is found.          */
      /* This is to prevent errors when the include file is in the same      */
      /* directory as the main file; the path to the main file is absolute   */
      /* and the path to the include file is relative. When the current      */
      /* directory of the OS is another directory, the wrong directory is    */
      /* searched for the include file.                                      */

      pcDrive = (char *) malloc(_MAX_DRIVE+1);
      pcDir = (char *) malloc(_MAX_DIR+1);

      pcNewDrive = (char *) malloc(_MAX_DRIVE+1);
      pcNewDir = (char *) malloc(_MAX_DIR+1);
      pcNewFilename = (char *) malloc(_MAX_FNAME+1);
      pcNewExtension = (char *) malloc(_MAX_EXT+1);

      splitpath(pcCurrentFileName, pcDrive, pcDir, NULL, NULL);
      splitpath(pcToken, pcNewDrive, pcNewDir, pcNewFilename, pcNewExtension);

      /* Both the current filename and the include file name are split into  */
      /* interesting parts now.                                              */
      /* When the include file has no specified drive, use the drive of the  */
      /* current file.                                                       */

      if (*pcNewDir == 0)
      {
        free(pcNewDir);
        pcNewDir = pcDir;
      }
      else
      {
        free(pcDir);
      }

      /* When the include file has no specified directory, use the directory  */
      /* of the current file.                                                 */

      if (*pcNewDrive == 0)
      {
        free(pcNewDrive);
        pcNewDrive = pcDrive;
      }
      else
      {
        free(pcDrive);
      }

      /* Now merge the four components into a full file path. When parts are  */
      /* not known, their string will be empty.                               */

      pcNewFullFilename = malloc(_MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT);

      strcpy(pcNewFullFilename, pcNewDrive);
      strcat(pcNewFullFilename, pcNewDir);
      strcat(pcNewFullFilename, pcNewFilename);
      strcat(pcNewFullFilename, pcNewExtension);

      InputStream = fopen(pcNewFullFilename, "r");

      free(pcNewDrive);
      free(pcNewDir);
      free(pcNewFilename);
      free(pcNewExtension);

      if (InputStream == NULL)
      {
        /* failed. Report the problem. Requesting the next token will result */
        /* in detecting the current file to be closed. Because of that, the  */
        /* list of IncludeRecords will be consulted, and the last closed     */
        /* file will be re-opened.                                           */
        iResult = INCLUDE_FILE_PROBLEM;
      }
      else
      {
        /* Succeeded. Update the administration for the new file             */
        strcpy(pcCurrentFileName, pcNewFullFilename);
        iCurrentLineNumber = 1;
      }

      free(pcNewFullFilename);
    }
    else
    {
      /* internal error, or out of memory problem. There is no need to       */
      /* assign an error-code to iResult, because this has been done         */
      /* already.                                                            */
    }
  }
  else
  {
    iResult = BAD_MACRO_DEFINITION;
  }
  return iResult;
}

int PreProcess(
  char *pcToken)
{
  int       iResult;

  /* don't go to the next line. The macro must be fully */
  /* defined on one line.                               */
  bBlockCrLf = TRUE;

  if (strcmp(pcToken, "#define") == 0)
  {
    iResult = PreProcessDefine(pcToken);
    iCurrentLineNumber++; /* for some reason, this correction is needed.. */
  }
  else if (strcmp(pcToken, "#include") == 0)
  {
    iResult = PreProcessInclude(pcToken);
  }
  else
  {
    iResult = UNKNOWN_PREPROCESSOR;
  }

  /* re-enable going to the next line                   */
  bBlockCrLf = FALSE;

  return iResult;
}



int OpenInputFile(
  char *pcFileName)
{
  int16     iResult;

  if (pcCurrentFileName == NULL)
  {
    iResult = OUT_OF_MEMORY;
  }
  else
  {
    strcpy(pcCurrentFileName, pcFileName);
    InputStream = fopen(pcFileName, "r");
    if (InputStream == NULL)
    {
      iResult = CANNOT_OPEN_FILE;
    }
    else
    {
      iCurrentLineNumber = 1;
      iResult = NO_SCANNER_ERROR;
    }
  }
  return iResult;
}

void CloseInputFile(
  void)
{
  if (InputStream != NULL)
  {
    fclose(InputStream);
    InputStream = NULL;
  }
}


int16 AddIncludeRecord(
  void)
/******************************************************************************/
/* a new include record is added at the head of the include list.             */
/* on succes, NO_SCANNER_ERROR is returned. On lack of memory, OUT_OF_MEMORY  */
/* is returned. In that case, all eventually allocated memory is freed.       */
/******************************************************************************/
{
  struct tIncludeRecord *ptNewRecord;
  int16     iResult;

  ptNewRecord = (struct tIncludeRecord *) malloc(sizeof(struct tIncludeRecord));
  if (ptNewRecord == NULL)
  {
    iResult = OUT_OF_MEMORY;
  }
  else
  {
    ptNewRecord->pcFileName = malloc(strlen(pcCurrentFileName) + 1);
    if (ptNewRecord->pcFileName == NULL)
    {
      /* failed. Free the just allocated record, because we can't make use   */
      /* of it.                                                              */
      free(ptNewRecord);
      iResult = OUT_OF_MEMORY;
    }
    else
    {
      strcpy(ptNewRecord->pcFileName, pcCurrentFileName);
      ptNewRecord->iLineNumber = iCurrentLineNumber;
      ptNewRecord->lFilePointer = ftell(InputStream);
      ptNewRecord->ptIncludedBy = ptIncludedBy;
      ptIncludedBy = ptNewRecord;
      iResult = NO_SCANNER_ERROR;
    }
  }
  return iResult;
}

void DeleteIncludedByRecord(
  void)
/******************************************************************************/
/* This function can be called for two reasons:                               */
/* a. The current file is fully read, and can be closed; or                   */
/* b. The to be included file cannot be opened. But the current file is       */
/*    already closed.                                                         */
/*                                                                            */
/* In both cases, an "IncludedBy" record can be deleted, and the correspon-   */
/* ding file must be reopened.                                                */
/******************************************************************************/
{
  struct tIncludeRecord *ptNextRecord;

  ptNextRecord = ptIncludedBy->ptIncludedBy;
  if (InputStream != NULL)
  {
    /* case a. */
    fclose(InputStream);
  }
  /* else case b. */
  strcpy(pcCurrentFileName, ptIncludedBy->pcFileName);
  InputStream = fopen(pcCurrentFileName, "r");
  if (InputStream != NULL)
  {
    fseek(InputStream, ptIncludedBy->lFilePointer, SEEK_SET);
    iCurrentLineNumber = ptIncludedBy->iLineNumber;
  }
  else
  {
    /* internal error. This is not handled.. */
  }
  free(ptIncludedBy);
  ptIncludedBy = ptNextRecord;

}
/******************************************************************************/
/* This function splist the path into sections                                */
/******************************************************************************/
static void      splitpath(
  char    *inpath,
  char    *drv,
  char    *dir,
  char    *fname,
  char    *ext )
{
  int   iIndexDir;
  int   iIndexFname;
  int   iLastDirSeperator;
  int   iExtIndex;
  char *pcInPathTemp;
  
  
  if (NULL != inpath)
  {
    pcInPathTemp = inpath;
    if (NULL != drv)
    {
      if (inpath[1] == ':')
      {
        drv[0] = *pcInPathTemp;
        pcInPathTemp++;
        drv[1] = *pcInPathTemp;
        pcInPathTemp++;
        drv[2] = '\0';
      } 
      else
      {
        drv[0] = 0;
      }
    }
    iLastDirSeperator = 0;
    iIndexDir = 0;
    if (NULL != dir)
    {
      while ( (*pcInPathTemp != '.') && 
              (*pcInPathTemp != 0) && 
              (iIndexDir < _MAX_DIR) )
      {
        dir[iIndexDir] = *pcInPathTemp;
        if ((*pcInPathTemp == '\\') || (*pcInPathTemp == '/'))
        {
          iLastDirSeperator = iIndexDir;
        }
        pcInPathTemp++;
        iIndexDir++;
      }
    }
    if (*pcInPathTemp == 0)
    {
      fname[0] = 0;
      ext[0]   = 0;
    }
    else
    {
      if( iLastDirSeperator > 0)
      {
        iLastDirSeperator++;
      }
      iIndexFname = 0;
      if (NULL != fname)
      {
        while ( ((iIndexFname+iLastDirSeperator) < iIndexDir) && 
                (_MAX_FNAME > iIndexFname) )
        {
          fname[iIndexFname] = dir[iLastDirSeperator+iIndexFname];
          iIndexFname++;
        }
        fname[iIndexFname]     = 0;
      }
      dir[iLastDirSeperator] = 0;

      iExtIndex = 0;
      if (NULL != ext)
      {
        while ((*pcInPathTemp != 0) && (iExtIndex < _MAX_EXT))
        {
          ext[iExtIndex] = *pcInPathTemp;
          pcInPathTemp++;
          iExtIndex++;
        }
        ext[iExtIndex] = 0;
      }
    }
  }
}
