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
// 12/01/2001 E.A. Nijboer            platform test.
// 09/02/2001 E.A. Nijboer            
//
//=========================================================================
*/


#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "general_lib.h"
#include "bsegui.h"
#include "parser_lib.h"
#include "stores.h"
#include "atkernel.h"
#include "atkernel_intern.h"
#include "expirator.h"
#include "executor_lib.h"       /* for the _Signal constants. */


#define TO_HEX(a)   (((a)>'9')?((a)-'A'+10):((a)-'0'))
#define LINELENGTH           ((int8) 100)

char     *pcOverview;
long      lSeedValue = 0;

void MallocText(
  void)
{
  pcOverview = (char *) malloc(1000000);
}
/* Reads the seed value from given filename */
void GetSeedValue(char * pcFilename)
{
  FILE  *pFile;
  int16  iFound;
  char  *pcReturnValue;
  char   pcLine[LINELENGTH];
  
  pFile  = NULL; 
  iFound = 0; 
  pcReturnValue = NULL;
  
  if (NULL != pcFilename)
  {
    pFile = fopen(pcFilename,"r");
    if (NULL != pFile)
    {
      while((feof( pFile ) == 0) && 
            (iFound == 0) )
      {
        pcReturnValue = fgets(pcLine, LINELENGTH, pFile);
        if (NULL != pcReturnValue)
        {
          if( sscanf( pcLine,"Seed: %ld\n", &lSeedValue) != 0)
          {
            iFound = TRUE;
          }
        }
      }
      fclose(pFile); 
    }
  }
}
void FreeText(
  void)
{
  free(pcOverview);
  pcOverview = NULL;
}


int16 GetInteger(
  char *pcToken)
{
  int       i = 0;
  int       iBase = 10;
  int       iDigit;

  while (((pcToken) != 0)
         && ((((*pcToken) >= '0')
              &&
              ((*pcToken) <= '9'))
             ||
             (((*pcToken) >= 'A')
              &&
              ((*pcToken) <= 'F'))
             || ((*pcToken) == 'b') || ((*pcToken) == 'x')))
  {
    iDigit = 0;

    if (*pcToken == 'x')
      iBase = 16;
    else if (*pcToken == 'b')
      iBase = 2;
    else
    {
      if (*pcToken >= 'a')
        *pcToken += ('A' - 'a');
      iDigit =
        ((*pcToken) <= '9') ? ((*pcToken) - '0') : ((*pcToken) - 'A' + 10);
    }

    i = (iBase * i) + iDigit;

    pcToken++;

  }
  return i;
}

struct TParameterList *qNextRepParameter(
  struct TParameterList *ptParameterList)
{
  int       i;

  while (ptParameterList->ptRepRec != NULL)
  {
    if (ptParameterList->ptRepRec->iBegin)
    {
      /* We found the begin of a repeated block. Now find out how     */
      /* many times we can expect the referenced block..              */
      if (ptParameterList->ptRepRec->ptParameter == NULL)
      {
        AddError2("Parameter %s should be eigther constant or unspecified",
                  ptParameterList->ptRepRec->ptParameter->pcName);
        i = 0;
      }
      else
      {
        if (ptParameterList->ptRepRec->ptParameter->ptValue == NULL)
        {
          AddError2("Parameter %s must be a constant!",
                    ptParameterList->ptRepRec->ptParameter->pcName);
          i = 0;
        }

        else
          i =
            GetInteger(ptParameterList->ptRepRec->ptParameter->ptValue->
                       pcValue);
      }

      if (i == 0)
        ptParameterList = ptParameterList->aptNext[1];
      else
        ptParameterList->ptRepRec->iCounter = i;

      ptParameterList = ptParameterList->aptNext[0];
    }
    else
    {
      /* We found the end of a repeated block. Jump to the beginning */
      /* Decrement counter and continue..                            */

      ptParameterList = ptParameterList->aptNext[1];
      ptParameterList->ptRepRec->iCounter--;

      if (ptParameterList->ptRepRec->iCounter == 0)
        ptParameterList = ptParameterList->aptNext[1];

      ptParameterList = ptParameterList->aptNext[0];
    }
  }
  return ptParameterList;
}

struct TParameterList *NextParameter(
  struct TParameterList *ptParameterList,
  char *pcValue)
/****************************************************************************/
/* Determines which of the array of next pointers has to be choosen. This   */
/* array of next pointers is used to implement the union in function defi-  */
/* nitions. If there is no union, the normal next parameter is returned.    */
/****************************************************************************/
{
  int       iValue;
  char     *pcErrorLine;
  struct TParameterList *ptNewList;

  iValue = GetInteger(pcValue);

  if (ptParameterList->iFollowedByUnion == TRUE)
  {
    if ((iValue < 0) || (iValue > UNIONARRAYSIZE + 1))
    {
      pcErrorLine = (char *) malloc(64);
      sprintf(pcErrorLine, "Runtime error, %d out of union boundary", iValue);
      LogLine(pcErrorLine);
      free(pcErrorLine);
      return NULL;
    }
    else
    {
      ptNewList = ptParameterList->aptNext[iValue];
    }
  }
  else
  {
    ptParameterList->ptThis->iLoopCount = iValue;
    ptNewList = ptParameterList->aptNext[0];
  }

  if (ptNewList != NULL)
    while ((ptNewList != NULL) && (ptNewList->ptRepRec != NULL))
    {
      if (ptNewList->ptRepRec->iBegin)
      {
        /* We found the begin of a repeated block. Now find out how     */
        /* many times we can expect the referenced block..              */
        if (ptNewList->ptRepRec->ptParameter == NULL)
        {
          AddError2("Parameter %s should be eigther constant or unspecified",
                    ptNewList->ptRepRec->ptParameter->pcName);
          iValue = 0;
        }
        else
        {
          if (ptNewList->ptRepRec->ptParameter->ptValue == NULL)
          {
            AddError2("Parameter %s must be a constant!",
                      ptNewList->ptRepRec->ptParameter->pcName);
            iValue = 0;
          }
          else
            iValue = ptNewList->ptRepRec->ptParameter->iLoopCount;
        }

        if (iValue == 0)
          ptNewList = ptNewList->aptNext[1];
        else
          ptNewList->ptRepRec->iCounter = iValue;

        ptNewList = ptNewList->aptNext[0];
      }
      else
      {
        /* We found the end of a repeated block. Jump to the beginning */
        /* Decrement counter and continue..                            */

        ptNewList = ptNewList->aptNext[1];
        ptNewList->ptRepRec->iCounter--;

        if (ptNewList->ptRepRec->iCounter == 0)
          ptNewList = ptNewList->aptNext[1];
        ptNewList = ptNewList->aptNext[0];
      }
    }
  return ptNewList;
}


struct TState *FindState(
  char *pcName,
  struct TStateList *ptStateList)
/****************************************************************************/
/* the state with name pcName is searched, and a pointer to it is returned. */
/* If not found, a NULL pointer is returned.                                */
/****************************************************************************/
{
  if (pcName == NULL)
  {
    return NULL;
  }

  while (ptStateList != NULL)
  {
    if (ptStateList->ptThis != NULL)
    {
      if (strcmp(ptStateList->ptThis->pcName, pcName) == 0)
      {
        return ptStateList->ptThis;
      }
    }
    ptStateList = ptStateList->ptNext;
  }
  return NULL;
}


void InitFileVar(
  struct TVariable *ptVar)
{
  FILE     *inv;
  int32     lFileLength;
/*  char     *pcPtr; */

  if (strstr(ptVar->ptValue->pcFileMode,"w") != NULL)
  {
    ptVar->ptValue->pcValue = (char *) malloc(204800);
  }
  else
  {  
    inv = fopen(ptVar->ptValue->pcFileName, "r");
    if (inv != NULL)
    {
      /* First determine the file length.                              */
      fseek(inv, 0, SEEK_END);
      lFileLength = ftell(inv);
      fseek(inv, 0, SEEK_SET);
  
      /* Next, remove old allocated space and allocate new space.      */
      if (ptVar->ptValue->pcValue != NULL)
      {
        free(ptVar->ptValue->pcValue);
      }
      ptVar->ptValue->pcValue = (char *) malloc(lFileLength + 1);
      /* 32 bits should be more than enough */
      ptVar->ptValue->uiLength = (unsigned int) lFileLength;
      /* Finally, Load the file in the new allocated space.            */
      fread( (void*) ptVar->ptValue->pcValue,
             sizeof(char),
             lFileLength,
             inv);
  /* This was done using the hard way           
      pcPtr = ptVar->ptValue->pcValue;
      while (ftell(inv) != lFileLength)
      {
        *(pcPtr++) = getc(inv);
      }
      *pcPtr = 0;
      */
      fclose(inv);
    }
  }
}


void InitVar(
  struct TVariable *ptVar)
{

  struct TData *ptDataRecord;

  if ((ptVar->ptItsType == NULL) || (ptVar->ptItsType == _ptBooleanType))
  {
    if (ptVar->ptValue != NULL)
    {
      if (ptVar->ptValue->bFixed == FALSE)
      {
        if (ptVar->ptValue->pcValue != NULL)
        {
          free(ptVar->ptValue->pcValue);
          ptVar->ptValue->pcValue = NULL;
        }
      }
      if (ptVar->ptValue->pcFileName != NULL)
      {
        InitFileVar(ptVar);
      }
    }
  }
  else if (ptVar->ptItsType == _ptDataType)
  {
    ptDataRecord = (struct TData *) ptVar;

    if (ptDataRecord->pcValue != NULL)
    {
      if (ptDataRecord->bFixed == FALSE)
      {
        (*ptDataRecord->pcValue) = '\0';
      }
    }
  }
  else
  {                             /* An ordinary variable. Just initialise it, if possible.               */
    /*----------------------------------------------------------------------*/
    if (ptVar->ptValue != NULL)
    {
      if (ptVar->ptValue->bFixed == FALSE)
      {
        if ((ptVar->ptValue->pcValue != NULL)
            && (ptVar->ptValue->pcValue != _NO_SIGNAL)
            && (ptVar->ptValue->pcValue != _SIGNAL))
        {
          if (ptVar->ptValue->pcValue[0] != '\0')
          {
            ptVar->ptValue->pcValue[0] = '\0';
          }
        }
        else if (ptVar->ptValue->pcValue == _SIGNAL)
        {
          ptVar->ptValue->pcValue = _NO_SIGNAL;
        }
      }
      else if (ptVar->ptValue->pcFileName != NULL)
      {
        InitFileVar(ptVar);
      }
    }
  }
}


void InitVarList(
  struct TVariableList *ptVarList)
{
  if (ptVarList != NULL)
  {
    if (ptVarList->ptThis != NULL)
      InitVar(ptVarList->ptThis);
    if (ptVarList->ptNext != NULL)
      InitVarList(ptVarList->ptNext);
  }
}

void InitStateMachine(
  struct TStateMachine *ptMachine)
/****************************************************************************/
/* Initialises one single statemachine.                                     */
/* The current state is initalised, and all action records are updated.     */
/* In each action record, the ptNextState pointer is substituted.           */
/* If the corresponding pcNextStateName is not found in the list of states, */
/* an error is generated. (In the previous internal releases of AutoTest    */
/* the ptNextState field was not available, and the states were searched    */
/* during runtime.)                                                         */
/****************************************************************************/
{
  struct TStateList *ptStateListWalker;
  struct TActionList *ptActionListWalker;
  struct TTransitionList *ptTransitionListWalker;
  struct TState *ptState;

  /* Initialise the initial state. Which is the first mentioned state..     */

  ptMachine->ptCurrentState = ptMachine->ptStates->ptThis;

  ptMachine->ptActionInProgress = NULL;
  ptMachine->ptRunningTimer = NULL;
  ptMachine->ptRunningTimerTransition = NULL;
  ptMachine->ptStateInTimer = NULL;


  /* Next, reset all variables                                              */

  InitVarList(ptMachine->ptLocalVars);

  /* Next, initialise an eventually rescue state                            */
  if (ptMachine->pcRescueStateName != NULL)
  {
    ptMachine->ptRescueState = FindState(ptMachine->pcRescueStateName,
                                         ptMachine->ptStates);
    if (ptMachine->ptRescueState == NULL)
    {
      AddError2("Rescue State %s not defined.", ptMachine->pcRescueStateName);
    }
  }
  else
  {
    ptMachine->ptRescueState = NULL;
  }

  /* Next, initialise all states.                                           */
  for (ptStateListWalker = ptMachine->ptStates;
       ptStateListWalker != NULL; ptStateListWalker = ptStateListWalker->ptNext)
  {
    if (ptStateListWalker->ptThis != NULL)
    {
      /* In a state, initialise all transitions:                            */

      for (ptTransitionListWalker = ptStateListWalker->ptThis->ptTransitionList;
           ptTransitionListWalker != NULL;
           ptTransitionListWalker = ptTransitionListWalker->ptNext)
      {

        ptTransitionListWalker->ptThis->lTransitionCounter = 0;

        /* In a transition: Initialise all actions                          */
        /*  - set the lActionCounter to 0,                                  */
        for (ptActionListWalker = ptTransitionListWalker->ptThis->ptActionList;
             ptActionListWalker != NULL;
             ptActionListWalker = ptActionListWalker->ptNext)

        {
          ptActionListWalker->ptThis->lActionCounter = 0;

          InitVarList(ptActionListWalker->ptThis->ptVarList);
        }

        /* In a state, initialise all transitions:                          */
        /*  - initialise all ptNextState, based on pcNextStateName          */

        if (ptTransitionListWalker->ptThis != NULL)
        {
          if (ptTransitionListWalker->ptThis->pcNextStateName == NULL)
          {
            /* No new statename intended. This is only allowed by a         */
            /* transition containing a TERMINATE transition...              */

            if ((ptTransitionListWalker->ptThis->ptActionList->ptThis->
                 iActionType != TERMINATE_ACTION)
/* && (ptTransitionListWalker->ptThis->ptActionList->ptThis->iActionType != END_ACTION) */
              )
            {
              AddError2("In line nr. %d, a next state must be defined",
                        ptTransitionListWalker->ptThis->ptActionList->ptThis->
                        iSourceLineNumber);
            }
          }
          else if (*(ptTransitionListWalker->ptThis->pcNextStateName) == '\0')
          {
            /* No statename mentinoned, the next state is intended..        */
            ptTransitionListWalker->ptThis->ptNextState =
              ptStateListWalker->ptNext->ptThis;
            if (ptTransitionListWalker->ptThis->ptNextState == NULL)
            {
              AddError3("In line nr. %d, implicit next state is defined. %s",
                        ptTransitionListWalker->ptThis->ptActionList->ptThis->
                        iSourceLineNumber,
                        "This is only allowed if there IS a next state..");

            }
          }
          else
          {
            ptState = FindState(ptTransitionListWalker->ptThis->pcNextStateName,
                                ptMachine->ptStates);
            if (ptState == NULL)
            {
              AddError3("In line nr. %d, state %s does not exist",
                        ptTransitionListWalker->ptThis->ptActionList->ptThis->
                        iSourceLineNumber,
                        ptTransitionListWalker->ptThis->pcNextStateName);
            }
            else
            {
              ptTransitionListWalker->ptThis->ptNextState = ptState;
              Increment(&(ptState->iRefCount));
            }
          }
        }
      }
    }
  }
}

void InitStateMachineList(
  struct TStateMachineList *ptList)
/****************************************************************************/
/* Initialises _all_ statemachines in the indicated list.                   */
/****************************************************************************/
{
  while (ptList != NULL)
  {
    if (ptList->ptThis != NULL)
      InitStateMachine(ptList->ptThis);
    ptList = ptList->ptNext;
  }
}

void InitBoard(
  struct TBoard *ptBoard)
/****************************************************************************/
/* Initialises _all_ statemachines on the indicated board.                  */
/****************************************************************************/
{
  InitStateMachineList(ptBoard->ptStateMachines);
}


void InitBoards(
  struct TBoardList *ptBoardList)
/****************************************************************************/
/* Initialises _all_ statemachines on all boards.                           */
/****************************************************************************/
{
  while (ptBoardList != NULL)
  {
    if (ptBoardList->ptThis != NULL)
      InitBoard(ptBoardList->ptThis);
    ptBoardList = ptBoardList->ptNext;
  }
}


void Update_Overview(
  void)
{
  /* Make an empty string of ca. 1 MBytes. That should be enough for     */
  /* all statemachines. Maybe we can make a more sophisticated esti-     */
  /* mation later..                                                      */

  char     *pcText;

  switch (_ptGlobals->iOverviewMode)
  {
    case BSEK_STATEMACHINES_FULL:
    case BSEK_STATEMACHINES_HEADERS:

      pcText = (char *) malloc(1000000);
      (*pcText) = '\0';
      WriteBoardList(pcText, _ptBoardList);

      if (_ptGlobals->iDebugLevel > 0)
      {
        WriteTimerList(pcText);
      }
      BSEG_Overview(pcText);
      break;
    case BSEK_PROCESSED_FILES:

      break;
    default:
      break;
  }
}


char     *strpos(
  char *pcShortString,
  char *pcLongString)
{
  int16     iStrLength = 0;

  /* first determine the length of the short string.                        */
  while (pcShortString[iStrLength] != 0)
    iStrLength++;

  /* search in the long string if the first iStrLength bytes are equal      */
  while (*pcLongString)
  {
    if (strncmp((char *) pcShortString, (char *) pcLongString, iStrLength) == 0)
    {
      return pcLongString;
    }
    else
    {
      pcLongString++;
    }
  }

  /* not found... Return a NULL pointer                                     */
  return NULL;
}

char     *my_strdup(
  const char *p)
{
  size_t  tStringSize;
  char   *pcResultString;
  
  tStringSize    = 0;
  pcResultString = NULL;
  
  if (p != NULL)
  {
    tStringSize = strlen(p);
    pcResultString = (char*) calloc(tStringSize+1, sizeof(char));
    strcpy(pcResultString, p);
  }
  return(pcResultString);
}



void asciicat(
  char *pcBuffer,
  char *pcString)
/* The one and only function that violates all coding standards.            */
{
  char      x[2];
  char      cHighNibble, cLowNibble;

  x[0] = 0;
  x[1] = 0;

  pcString += 2;                /* skip the 0x prefix */

  strcat(pcBuffer, "\"");

  while (*pcString != 0)
  {
    cHighNibble = *pcString++;
    cLowNibble = *pcString++;

    *x = TO_HEX(cHighNibble) << 4 | TO_HEX(cLowNibble);

    if (((*x) >= ' ') && ((*x) <= '~'))
      strcat(pcBuffer, x);
    else
      strcat(pcBuffer, ".");
  }

  strcat(pcBuffer, "\"");
}
