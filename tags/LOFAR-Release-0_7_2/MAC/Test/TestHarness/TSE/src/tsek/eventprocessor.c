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
//    $Date$
// -----------------------
//
// Description   : 
//
// Revisions:
//
// Date       Author                  Changes
// 08/08/2000 E.A. Nijboer            Initial release
// 12/01/2001 E.A. Nijboer            platform test.
// 09/02/2001 E.A. Nijboer            
//
//=========================================================================
*/


#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "eventprocessor.h"
#include "eventreceiver.h"
#include "general_lib.h"
#include "parser_lib.h"
#include "bsegui.h"
#include "executor_lib.h"
#include "executor_master.h"
#include "atkernel_intern.h"
#include "stores.h"
#include "atkernel.h"
#include "stepper.h"

#define  PROT_RECEIVE_MODE           ((int16) (0))
#define  ASCII_RECEIVE_MODE         ((int16) (1))

#define  PROT_COMMAND_PACKET         (0x01)
/*#define  PROT_ACL_DATA_PACKET        (0x02) */
/*#define  PROT_SCO_DATA_PACKET        (0x03) */
#define  PROT_EVENT_PACKET           (0x04)

extern int iRunningMode;        /* Steal from the executor-master. */

/****************************************************************************/
/* Local function headers.                                                  */
/****************************************************************************/
char     *DeviceName(
  int16 i);

void      ProcessInternalSignal(
  int16 iDeviceNumber,
  char *pcEvent);

void      ProcessPROTEvent(
  char *pcBuffer,
  int16 iLength,
  int16 iBoard);

void      ProcessPROTData(
  char *pcBuffer,
  int iLength,
  int iBoard);

void      ProcessASCIIData(
  char *pcBuffer,
  int iLength,
  int iBoard);

void      ProcessPROT_ACL(
  char *pcBuffer,
  uint16 uiLength,
  int16 iBoard);



void      ParseReceivedVariables(
  char *pcBuffer,
  int16 iTotalLength,
  int16 iBoard,
  struct TVariableList **pptVarList,
  struct TParameterList *ptParameterList);

void      ProcessDecomposedAcl(
  int16 iHandle,
  int16 iPbFlag,
  int16 iBcFlag,
  int16 iIndicatedLength,
  char *pcBuffer,
  int16 iBoard);

void      ProcessDecomposedAcl2(
  int16 iHandle,
  int16 iPbFlag,
  int16 iBcFlag,
  int16 iIndicatedLength,
  char *pcBuffer,
  struct TStateMachineList *ptStateMachines);

int16     ProcessDecomposedAcl3(
  int16 iHandle,
  int16 iPbFlag,
  int16 iBcFlag,
  int16 iIndicatedLength,
  char *pcBuffer,
  struct TStateMachine *ptStateMachine);

int16     ProcessDecomposedAcl4(
  int16 iHandle,
  int16 iPbFlag,
  int16 iBcFlag,
  int16 iIndicatedLength,
  char *pcBuffer,
  struct TTransition *ptTransition);

int16     ProcessDecomposedAcl5(
  int16 iHandle,
  int16 iPbFlag,
  int16 iBcFlag,
  int16 iIndicatedLength,
  char *pcBuffer,
  struct TAction *ptAction);


void      ProcessDecomposedPROTEvent(
  struct TEvent *ptEvent,
  struct TVariableList *ptVarList,
  int16 iBoard);

void      ProcessDecomposedPROTEvent2(
  struct TEvent *ptEvent,
  struct TVariableList *ptVarList,
  struct TStateMachineList *ptSMList);

int16     ProcessDecomposedPROTEvent3(
  struct TEvent *ptEvent,
  struct TVariableList *ptVarList,
  struct TStateMachine *ptSM);

int16     ProcessDecomposedPROTEvent4(
  struct TEvent *ptEvent,
  struct TVariableList *ptVarList,
  struct TVariableList *ptLocalVars,
  struct TTransition *ptTrans);

int16     CheckBoundaries(
  char *pcValue,
  struct TType *ptType);

int16     ParseReceivedVariable(
  char *pcValue,
  char *pcInputStream,
  char *pcName,
  struct TType *ptTypeDef,
  int16 iReducedLength,
  int16 iBufferLength);

int16     TestAction(
  struct TEvent *ptEvent,
  struct TVariableList *ptVarList,
  struct TAction *ptAction,
  struct TVariableList *ptStatemachineVarList);

int16     TestVariable(
  struct TVariable *ptReceivedVar,
  struct TVariableList *ptVarList,
  struct TVariable *ptVariable);

void      SubstituteReceivedVariables(
  struct TVariableList *ptVarList,
  struct TVariableList *ptReceivedVars,
  struct TVariableList *ptRefVars);

void      SubstituteReceivedVariable(
  struct TVariable *ptVar,
  struct TVariable *ptReceivedVar,
  struct TVariableList *ptRefVar);

int16     smartcmp(
  int iKind,
  char *pcVarName,
  char *pcReceivedValue);

int16     smartSubstitute(
  char *pcVarName,
  struct TVariableList *ptVarList,
  char *pcReceivedValue);

int16     SingleCharMatch(
  char cByte,
  char *pHexString);

static int ConvertArray(char         *pcBuffer,
                        char         *pcValue,
                        int           iCodeLength,
                        struct TType *ptType);
static int16 ValueCmp(
  struct TVariable * ptReceivedVar,
  struct TVariable * ptVariable);


/****************************************************************************/
/* Exported function bodies.                                                */
/****************************************************************************/


int16 ProcessIncomingBuffer(
  void)
/****************************************************************************/
/****************************************************************************/
{
  struct TRawEvent *ptEvent;
  char      pBuffer[800];
  char      pHex[10];
  int       i, iByteCount;
  int       iDeviceNumber;

  ptEvent = GetNextEvent();

  while (ptEvent != NULL)
  {
    iDeviceNumber = ptEvent->iDeviceNumber;
    iByteCount = ptEvent->iEventLength;

    if (iByteCount < 0)
    {
      /* Message from the Control Panel itself, not a Protocol Message */
      /* The message is coded as a 0-terminated ASCII string.          */

      sprintf(pBuffer, "CP : %s [%s]", DeviceName(iDeviceNumber),
              ptEvent->pcEvent);
      LogLine(pBuffer);
      ProcessInternalSignal(iDeviceNumber, ptEvent->pcEvent);
    }
    else
    {
      /* Protocol Message...                                           */
      LogLine("");

      sprintf(pBuffer, "[<- %s (%d):", DeviceName(iDeviceNumber), iByteCount);

      i = 0;

      while ((i < iByteCount) && (i < 256))
      {
        sprintf(pHex, "%02X ", ptEvent->pcEvent[i]);
        if (ptEvent->pcEvent[i] >= 0)
          strcat(pBuffer, pHex);
        else
          strcat(pBuffer, pHex + 6);    /* Because of e.g. 0xFFFFFF80          */
        i++;
      }
      LogLine(pBuffer);

      ProcessPROTData(ptEvent->pcEvent,
                     ptEvent->iEventLength, ptEvent->iDeviceNumber);
    }

    free(ptEvent->pcEvent);
    free(ptEvent);

    if (iRunningMode != HALTED)
    {
      ptEvent = GetNextEvent();
    }
    else
    {
      ptEvent = NULL;
    }

    /* end while */
  }


  return 0;
}


/****************************************************************************/
/* Local function bodies                                                    */
/****************************************************************************/

void ProcessInternalSignal(
  int16 iDeviceNumber,
  char *pcEvent)
{
  uint32    ulMask;

  ulMask = 1 << iDeviceNumber;
  if (strcmp(pcEvent, "SOCKET_GENERAL_ERROR") == 0)
  {
    ul_SOCKET_GENERAL_ERROR |= ulMask;
  }
  else if (strcmp(pcEvent, "SOCKET_SEND_ERROR") == 0)
  {
    ul_SOCKET_SEND_ERROR |= ulMask;
  }
  else if (strcmp(pcEvent, "SOCKET_RECEIVE_ERROR") == 0)
  {
    ul_SOCKET_RECEIVE_ERROR |= ulMask;
  }
  else if (strcmp(pcEvent, "SOCKET_CONNECT_ERROR") == 0)
  {
    ul_SOCKET_CONNECT_ERROR |= ulMask;
  }
  else if (strcmp(pcEvent, "SOCKET_DISCONNECT_ERROR") == 0)
  {
    ul_SOCKET_DISCONNECT_ERROR |= ulMask;
  }
  else if (strcmp(pcEvent, "SOCKET_ACCEPT_ERROR") == 0)
  {
    ul_SOCKET_ACCEPT_ERROR |= ulMask;
  }
  else if (strcmp(pcEvent, "SOCKET_UNKNOWN_ERROR") == 0)
  {
    ul_SOCKET_UNKNOWN_ERROR |= ulMask;
  }
  else if (strcmp(pcEvent, "CONNECTION_LOST") == 0)
  {
    ul_CONNECTION_LOST |= ulMask;
  }
  else if (strcmp(pcEvent, "CONNECTION_ESTABLISHED") == 0)
  {
    ul_CONNECTION_ESTABLISHED |= ulMask;
  }
}

void ProcessPROTData(
  char *pcBuffer,
  int iLength,
  int iBoard)
/****************************************************************************/
/****************************************************************************/
{

  ProcessPROTEvent(pcBuffer, iLength, iBoard);
}

void ProcessASCIIData(
  char *pcBuffer,
  int iLength,
  int iBoard)
/****************************************************************************/
/****************************************************************************/
{
  pcBuffer = 0;
  iLength = 0;
  iBoard = 0;

  LogLine("Cannot process ASCII data yet.");
}


void ProcessPROT_ACL(
  char *pcBuffer,
  uint16 uiLength,
  int16 iBoard)
{
  int16     iHandleAndFlags;
  int16     iHandle;
  int16     iPbFlag, iPbFlag0, iPbFlag1;
  int16     iBcFlag, iBcFlag0, iBcFlag1;
  uint16    uiIndicatedLength;
  char      pLine[240];

  uint8     ucByte;

  ucByte = *(pcBuffer++);
  iHandleAndFlags = ucByte << 0;

  ucByte = *(pcBuffer++);
  iHandleAndFlags |= ucByte << 8;

  ucByte = *(pcBuffer++);
  uiIndicatedLength = ucByte << 0;

  ucByte = *(pcBuffer++);
  uiIndicatedLength |= ucByte << 8;

  iHandle = iHandleAndFlags & 0x0FFF;

  iPbFlag0 = (iHandleAndFlags & 0x1000) >> 12;
  iPbFlag1 = (iHandleAndFlags & 0x2000) >> 13;
  iBcFlag0 = (iHandleAndFlags & 0x4000) >> 14;
  iBcFlag1 = (iHandleAndFlags & 0x8000) >> 15;

  iPbFlag = iPbFlag0 | (iPbFlag1 << 1);
  iBcFlag = iBcFlag0 | (iBcFlag1 << 1);

  pcBuffer += 4;                /* skip additional length field.. */

  if (uiIndicatedLength != uiLength - 4)
  {
    LogLine("Received invalid ACL packet! Lenght is not correct");
  }
  else
  {
    sprintf(pLine, "%s received %d bytes", DeviceName(iBoard), uiLength);
    LogLine(pLine);

    ProcessDecomposedAcl(iHandle,
                         iPbFlag,
                         iBcFlag, uiIndicatedLength - 4, pcBuffer, iBoard);
  }
}


void ProcessDecomposedAcl(
  int16 iHandle,
  int16 iPbFlag,
  int16 iBcFlag,
  int16 iIndicatedLength,
  char *pcBuffer,
  int16 iBoard)
{
  struct TBoardList *ptBoardList;
  char     *pcErrorLine;

  ptBoardList = _ptBoardList;

  while ((ptBoardList != NULL)
         && (ptBoardList->ptThis != NULL)
         && (ptBoardList->ptThis->iNumber != iBoard))
  {
    ptBoardList = ptBoardList->ptNext;
  }

  if ((ptBoardList != NULL) && (ptBoardList->ptThis != NULL))
  {
    ProcessDecomposedAcl2(iHandle,
                          iPbFlag,
                          iBcFlag,
                          iIndicatedLength,
                          pcBuffer, ptBoardList->ptThis->ptStateMachines);
  }
  else
  {
    pcErrorLine = malloc(256);
    sprintf(pcErrorLine, "Internal Error: Board #%d not found!", iBoard);
    LogLine(pcErrorLine);
    free(pcErrorLine);
  }
}

void ProcessDecomposedAcl2(
  int16 iHandle,
  int16 iPbFlag,
  int16 iBcFlag,
  int16 iIndicatedLength,
  char *pcBuffer,
  struct TStateMachineList *ptSMList)
{
  int16     bProcessed = FALSE;

  while (ptSMList != NULL)
  {
    if ((ptSMList->ptThis != NULL)
        && (ptSMList->ptThis->iPending == RUNNING_SM))
    {
      bProcessed = ProcessDecomposedAcl3(iHandle,
                                         iPbFlag,
                                         iBcFlag,
                                         iIndicatedLength,
                                         pcBuffer, ptSMList->ptThis);
      if (bProcessed == TRUE)
      {
        ptSMList = NULL;
      }
      else
      {
        ptSMList = ptSMList->ptNext;
      }
    }
  }
  if (bProcessed == FALSE)
  {
    LogLine("Warning: Acl Data not processed!");
  }

}

int16 ProcessDecomposedAcl3(
  int16 iHandle,
  int16 iPbFlag,
  int16 iBcFlag,
  int16 iIndicatedLength,
  char *pcBuffer,
  struct TStateMachine *ptSM)
{

  struct TTransitionList *ptWalker;
  int16     bProcessed;

  if (ptSM == NULL)
    return FALSE;
  if (ptSM->ptCurrentState == NULL)
    return FALSE;
  if (ptSM->ptCurrentState->ptTransitionList == NULL)
    return FALSE;

  ptWalker = ptSM->ptCurrentState->ptTransitionList;
  bProcessed = FALSE;

  while (ptWalker != NULL)
  {
    bProcessed = ProcessDecomposedAcl4(iHandle,
                                       iPbFlag,
                                       iBcFlag,
                                       iIndicatedLength,
                                       pcBuffer, ptWalker->ptThis);

    if (bProcessed == TRUE)
    {
      TryTransferState(ptSM, ptWalker->ptThis);
      ptWalker = NULL;
    }
    else
    {
      ptWalker = ptWalker->ptNext;
    }
  }
  return bProcessed;
}


int16 ProcessDecomposedAcl4(
  int16 iHandle,
  int16 iPbFlag,
  int16 iBcFlag,
  int16 iIndicatedLength,
  char *pcBuffer,
  struct TTransition * ptTransition)
{
  struct TActionList *ptWalker;

  if (ptTransition == NULL)
    return FALSE;
  ptWalker = ptTransition->ptActionList;
  while (ptWalker != NULL)
  {
    if (ptWalker->ptThis->iActionType == PROT_READDATA_ACTION)
    {
      if (ProcessDecomposedAcl5(iHandle,
                                iPbFlag,
                                iBcFlag,
                                iIndicatedLength,
                                pcBuffer, ptWalker->ptThis) == TRUE)
      {
        return TRUE;
      }
    }
    ptWalker = ptWalker->ptNext;

  }
  return FALSE;

}

int16 ProcessDecomposedAcl5(
  int16 iHandle,
  int16 iPbFlag,
  int16 iBcFlag,
  int16 iIndicatedLength,
  char *pcBuffer,
  struct TAction * ptAction)
{
  int16     iExpHandle;
  int16     iExpPbFlag;
  int16     iExpBcFlag;
  char     *pcExpData;
  struct TData *ptData;
  char      pcString[32];

  struct TVariableList *ptVarList;

  iExpHandle = -1;
  iExpPbFlag = -1;
  iExpBcFlag = -1;

  ptVarList = ptAction->ptVarList;

  if ((ptVarList->ptThis->ptValue != NULL)
      && (ptVarList->ptThis->ptValue->pcValue != NULL)
      && (ptVarList->ptThis->ptValue->pcValue[0] != 0))
  {
    iExpHandle = GetInteger(ptVarList->ptThis->ptValue->pcValue);
  }
  ptVarList = ptVarList->ptNext;

  if ((ptVarList->ptThis->ptValue != NULL)
      && (ptVarList->ptThis->ptValue->pcValue != NULL)
      && (ptVarList->ptThis->ptValue->pcValue[0] != 0))
  {
    iExpPbFlag = GetInteger(ptVarList->ptThis->ptValue->pcValue);
  }
  ptVarList = ptVarList->ptNext;

  if ((ptVarList->ptThis->ptValue != NULL)
      && (ptVarList->ptThis->ptValue->pcValue != NULL)
      && (ptVarList->ptThis->ptValue->pcValue[0] != 0))
  {
    iExpBcFlag = GetInteger(ptVarList->ptThis->ptValue->pcValue);
  }
  ptVarList = ptVarList->ptNext;

  ptData = (struct TData *) ptVarList->ptThis;
  pcExpData = ptData->pcValue;

  if ((iExpHandle != -1) && (iExpHandle != iHandle))
    return FALSE;
  if ((iExpPbFlag != -1) && (iExpPbFlag != iPbFlag))
    return FALSE;
  if ((iExpBcFlag != -1) && (iExpBcFlag != iBcFlag))
    return FALSE;

  if ((pcExpData != NULL)
      && (pcExpData[0] != 0)
      && (memcmp(pcExpData, pcBuffer, iIndicatedLength) != 0))
  {
    return FALSE;
  }

  /* Finally, we know for sure that we've found a PROT_READDATA action      */
  /* that fully matches the received Acl Data string. Now updata all used   */
  /* variables.                                                             */

  /* Variable one. */
  ptVarList = ptAction->ptVarList;

  if (ptVarList->ptThis->pcName != NULL)
  {
    if (ptVarList->ptThis->ptValue == NULL)
    {
      ptVarList->ptThis->ptValue = newValue();
    }

    if (ptVarList->ptThis->ptValue->pcValue == NULL)
    {
      sprintf(pcString, "0x%04x", iHandle);
      ptVarList->ptThis->ptValue->pcValue = my_strdup(pcString);
    }
  }

  /* Variable two. */
  ptVarList = ptVarList->ptNext;

  if (ptVarList->ptThis->pcName != NULL)
  {
    if (ptVarList->ptThis->ptValue == NULL)
    {
      ptVarList->ptThis->ptValue = newValue();
    }

    if (ptVarList->ptThis->ptValue->pcValue == NULL)
    {
      switch (iPbFlag)
      {
        case 0x00:
          sprintf(pcString, "0b00");
          break;
        case 0x01:
          sprintf(pcString, "0b01");
          break;
        case 0x02:
          sprintf(pcString, "0b10");
          break;
        case 0x03:
          sprintf(pcString, "0b11");
          break;
        default:
          sprintf(pcString, "????");
          break;
      }
      ptVarList->ptThis->ptValue->pcValue = my_strdup(pcString);
    }
  }

  /* variable three */
  ptVarList = ptVarList->ptNext;

  if (ptVarList->ptThis->pcName != NULL)
  {
    if (ptVarList->ptThis->ptValue == NULL)
    {
      ptVarList->ptThis->ptValue = newValue();
    }

    if (ptVarList->ptThis->ptValue->pcValue == NULL)
    {
      switch (iBcFlag)
      {
        case 0x00:
          sprintf(pcString, "0b00");
          break;
        case 0x01:
          sprintf(pcString, "0b01");
          break;
        case 0x02:
          sprintf(pcString, "0b10");
          break;
        case 0x03:
          sprintf(pcString, "0b11");
          break;
        default:
          sprintf(pcString, "????");
          break;
      }
      ptVarList->ptThis->ptValue->pcValue = my_strdup(pcString);
    }
  }

  /* and finally, the data variable. */

  if (ptData->pcName != NULL)
  {
    if ((ptData->pcValue != NULL) && (ptData->pcValue[0] == 0))
    {
      free(ptData->pcValue);
      ptData->pcValue = NULL;
    }

    if (ptData->pcValue == NULL)
    {
      ptData->pcValue = malloc(iIndicatedLength + 1);
      memmove(ptData->pcValue, pcBuffer, iIndicatedLength);
      *(ptData->pcValue + iIndicatedLength) = 0;        /* make it zero-terminated */
    }
  }

  ptAction->lActionCounter++;

  return TRUE;

}

void ProcessPROTEvent(
  char *pcBuffer,
  int iLength,
  int16 iBoard)
/****************************************************************************/
/****************************************************************************/
{
  struct TEvent *ptEvent;
  char     *pcLogLine;
  struct TVariableList *ptReceivedVarList = NULL;

  /* First find out which PROT Event is intended...                          */

  ptEvent = FindMatchingEvent(pcBuffer, iLength);
  if (ptEvent == NULL)
  {
    pcLogLine = malloc(250);
    sprintf(pcLogLine, "**Error: Unknown Event received...");
    LogLine(pcLogLine);
    free(pcLogLine);
  }
  else
  {
    if (((_ptGlobals->iLogMode & BSEK_LOG_EXTENSIVE) == BSEK_LOG_EXTENSIVE)
/*   ||  ((_ptGlobals->iLogMode & BSEK_LOG_STRING   ) == BSEK_LOG_STRING    ) */
      )
    {
      pcLogLine = (char *) malloc(1024);
      sprintf(pcLogLine, "Receive function %s", ptEvent->pcName);
      LogLine(pcLogLine);
      free(pcLogLine);
    }
    ParseReceivedVariables(pcBuffer,
                           iLength, 
                           iBoard,
                           &ptReceivedVarList, 
                           ptEvent->ptParameters);
    ProcessDecomposedPROTEvent(ptEvent, ptReceivedVarList, iBoard);

    if (ptReceivedVarList != NULL)
    {
      delVariableList(&ptReceivedVarList);
    }
  }
}

void ParseReceivedVariables(
  char *pcBuffer,
  int16 iTotalLength,
  int16 iBoard,
  struct TVariableList **pptVarList,
  struct TParameterList *ptParameterList)
/****************************************************************************/
/****************************************************************************/
{
  int16     bErrorFree = TRUE;
  int16     bReady = FALSE;
  int16     iBytesParsed = 0;
  struct TVariableList *ptLastVarList;
/*  struct TVariableList *ptVarList; */

  char     *pLogLine = NULL;
  char      pLengthValue[20];
  int16     i;
  int16     bScriptParameter;
  int16     iReducedLength;
  char      cByte[3];
  char      c;

  struct TValueDefList *ptValueWalker;


  struct TBoardList *ptBoardList;
  char     *pcErrorLine;

  ptLastVarList = NULL;

  ptBoardList = _ptBoardList;

  while ((ptBoardList != NULL)
         && (ptBoardList->ptThis != NULL)
         && (ptBoardList->ptThis->iNumber != iBoard))
  {
    ptBoardList = ptBoardList->ptNext;
  }

  if ((ptBoardList != NULL) && (ptBoardList->ptThis != NULL))
  {
    while ((bErrorFree == TRUE) && (bReady == FALSE))
    {
      /* Are there remaining definitions for next parameters in the buffer?  */
      if (ptParameterList == NULL)
      {
        /* No. We are ready.                                                 */
        bReady = TRUE;
      }
      if (bReady == FALSE)
      {
        /* Find out whether the parameter is mentioned in the script.        */
        bScriptParameter = TRUE;
        if ((ptParameterList->ptThis->ptTypeDef == NULL)
            || (ptParameterList->ptThis->pcLengthFromField != NULL)
            || (ptParameterList->ptThis->pcLengthToField != NULL))
        {
          bScriptParameter = FALSE;
        }
  
        if (bScriptParameter == TRUE)
        {
          if (*pptVarList == NULL)
            *pptVarList = newVariableList();
          if (ptLastVarList == NULL)
            ptLastVarList = *pptVarList;
  
          if (ptLastVarList->ptThis == NULL)
          {
            ptLastVarList->ptThis =
              newVariable(ptParameterList->ptThis->ptTypeDef);
          }
          if (ptParameterList->ptThis->ptLengthIndicator)
          {
            if (ptParameterList->ptThis->ptTypeDef->iKind == ARRAYKIND)
            {
              iReducedLength = 
                ptParameterList->ptThis->ptLengthIndicator->iLoopCount *
                ptParameterList->ptThis->ptTypeDef->uiSizeOfElement;
            }
            else
            {
              iReducedLength =
                ptParameterList->ptThis->ptLengthIndicator->iLoopCount;
            }
          }
          else
          {
            iReducedLength = -1;
          }
          iBytesParsed =
            ParseReceivedVariable(ptLastVarList->ptThis->ptValue->pcValue,
                                  pcBuffer, 
                                  ptParameterList->ptThis->pcName,
                                  ptParameterList->ptThis->ptTypeDef,
                                  iReducedLength, iTotalLength);
          ptLastVarList->ptThis->ptValue->uiLength = iBytesParsed;
        }
        else if (ptParameterList->ptThis->ptTypeDef != NULL)
        {
          iBytesParsed = ParseReceivedVariable(pLengthValue,
                                               pcBuffer,
                                               ptParameterList->ptThis->pcName,
                                               ptParameterList->ptThis->ptTypeDef,
                                               -1, iTotalLength);
        }
        else if (ptParameterList->ptThis->ptValue != NULL)
        {
          iBytesParsed =
            (strlen(ptParameterList->ptThis->ptValue->pcValue) - 2) / 2;
        }
        else
        {
          LogLine("Foutje? I don't know...");
        }
  
        if ((_ptGlobals->iLogMode & BSEK_LOG_EXTENSIVE) == BSEK_LOG_EXTENSIVE)
        {
          pLogLine = (char *) malloc(1024);
  
          /* Extensive log type 1. */
          if (    (ptParameterList->ptThis->ptTypeDef != NULL)
               && (ptParameterList->ptThis->ptTypeDef->iKind == ENUMKIND))
          {
            sprintf(pLogLine, "%26s <- %16s",
                      ptParameterList->ptThis->pcName,
                      ptLastVarList->ptThis->ptValue->pcValue);
  
            ptValueWalker = ptParameterList->ptThis->ptTypeDef->ptDefinition;
            while ((ptValueWalker != NULL)
                   && (ptValueWalker->ptThis != NULL)
                   &&
                   (strcmp
                    (ptValueWalker->ptThis->pcValue,
                     ptLastVarList->ptThis->ptValue->pcValue) != 0))
            {
              ptValueWalker = ptValueWalker->ptNext;
            }
            if ((ptValueWalker != NULL)
                && (ptValueWalker->ptThis != NULL)
                &&
                (strcmp
                 (ptValueWalker->ptThis->pcValue,
                  ptLastVarList->ptThis->ptValue->pcValue) == 0))
            {
              strcat(pLogLine, " (");
              strcat(pLogLine, ptValueWalker->ptThis->pcDescription);
              strcat(pLogLine, ")");
            }
          }
          /* Extensive log type 2. */
          else if ((ptLastVarList != NULL)
              && (ptLastVarList->ptThis != NULL)
              && (ptLastVarList->ptThis->ptValue != NULL)
              && (ptLastVarList->ptThis->ptValue->pcValue != NULL)
              && ~(   (ptParameterList->ptThis->ptTypeDef != NULL)
                   && (ptParameterList->ptThis->ptTypeDef->iKind == ENUMKIND))
                 )
          {
            if (ptLastVarList->ptThis->ptItsType->iKind == ASCIIKIND)
            {
              sprintf(pLogLine, "%26s <- ", ptParameterList->ptThis->pcName);
              i = 2;
              while (ptLastVarList->ptThis->ptValue->pcValue[i])
              {
                cByte[0] = ptLastVarList->ptThis->ptValue->pcValue[i++];
                cByte[1] = ptLastVarList->ptThis->ptValue->pcValue[i++];
                cByte[2] = 0;
                sscanf(cByte, "%x", (unsigned int*)&c);
                sprintf(cByte, "%c", (char) c);
                strcat(pLogLine, cByte);
              }
            }
            else if (ptLastVarList->ptThis->ptItsType->iKind == ASCII0KIND)
            {
              sprintf(pLogLine, "%26s <- ", ptParameterList->ptThis->pcName);
              i = 2;
              while (ptLastVarList->ptThis->ptValue->pcValue[i])
              {
                cByte[0] = ptLastVarList->ptThis->ptValue->pcValue[i++];
                cByte[1] = ptLastVarList->ptThis->ptValue->pcValue[i++];
                cByte[2] = 0;
                sscanf(cByte, "%x", (unsigned int*)&c);
                sprintf(cByte, "%c", (char) c);
                strcat(pLogLine, cByte);
              }
            }
            else if (ptLastVarList->ptThis->ptItsType->iKind == UNICODEKIND)
            {
              sprintf(pLogLine, "%26s <- ", ptParameterList->ptThis->pcName);
              i = 2;
              while (ptLastVarList->ptThis->ptValue->pcValue[i])
              {
                i += 2;           /* skip the first (empty) byte of an unicode word */
                if (ptLastVarList->ptThis->ptValue->pcValue[i])
                {
                  cByte[0] = ptLastVarList->ptThis->ptValue->pcValue[i++];
                  cByte[1] = ptLastVarList->ptThis->ptValue->pcValue[i++];
                  cByte[2] = 0;
                  sscanf(cByte, "%x",(unsigned int*) &c);
                  sprintf(cByte, "%c", (char) c);
                  strcat(pLogLine, cByte);
                }
              }
            }
            else if (ptLastVarList->ptThis->ptItsType->iKind == UNICODE0KIND)
            {
              sprintf(pLogLine, "%26s <- ", ptParameterList->ptThis->pcName);
            }
            else
              sprintf(pLogLine, "%26s <- %16s",
                      ptParameterList->ptThis->pcName,
                      ptLastVarList->ptThis->ptValue->pcValue);
          }
  
          /* Extensive log type 3. */
          else if ((ptParameterList->ptThis)
                   && (ptParameterList->ptThis->ptValue)
                   && (ptParameterList->ptThis->ptValue->pcValue)
                   && (ptParameterList->ptThis->ptTypeDef == NULL))
          {
            sprintf(pLogLine, "%26s <- %16s",
                    "(constant field)",
                    ptParameterList->ptThis->ptValue->pcValue);
          }
  
          /* Extensive log type 4. For length fields */
          else if (TRUE)
          {
            sprintf(pLogLine, "%26s <- %16s", "(length field)", pLengthValue);
          }
          if (pLogLine != NULL)
          {
            LogLine(pLogLine);
            free(pLogLine);
            pLogLine = NULL;
          }
        }
  
        if ((_ptGlobals->iLogMode & BSEK_LOG_STRING) == BSEK_LOG_STRING)
        {
          pLogLine = (char *) malloc(1024);
          sprintf(pLogLine, "<- %s", pcBuffer);
  
          LogLine(pLogLine);
          free(pLogLine);
        }
  
        pcBuffer += iBytesParsed;
  
        iTotalLength -= iBytesParsed;
  
        /* If the variabeles were in range, prepare the checking of the next */
        /* parameter.                                                        */
  
        if (bErrorFree == TRUE)
        {
  
          if ((ptLastVarList != NULL) && (ptLastVarList->ptThis != NULL))
          {
            /* Normal variable parsed. Store it's value in case the value in */
            /* dicates a repetition-counter                                  */
            i = GetInteger(ptLastVarList->ptThis->ptValue->pcValue);
            ptParameterList->ptThis->iLoopCount = i;
            ptParameterList = NextParameter(ptParameterList,
                                            ptLastVarList->ptThis->ptValue->
                                            pcValue);
            if (ptParameterList != NULL)
            {
              ptLastVarList->ptNext = newVariableList();
              ptLastVarList = ptLastVarList->ptNext;
            }
          }
          else
          {
            /* Constant parsed. Constant's don't show up in scripts, so the  */
            /* value isn't stored at all.                                    */
            /* Therefore we can't refer to it, and we use 0 instead..        */
            ptParameterList = NextParameter(ptParameterList, 0);
          }
        }
      }
    }
    if (bErrorFree == FALSE)
    {
      delVariableList(pptVarList);
    }

  }
  else
  {
    pcErrorLine = malloc(256);
    sprintf(pcErrorLine, "Warning: Board #%d not used in this test!", iBoard);
    LogLine(pcErrorLine);
  }


}

int ConvertArray(
  char         *pcBuffer,
  char         *pcValue,
  int           iCodeLength,
  struct TType *ptType
)
{
  int   iIndexValue;
  int   iIndexBuffer;
  int   i;
  int   j;
  uint8 ucByte;


  iIndexValue = 0;
  iIndexBuffer = 2;
  strcpy(pcBuffer,"0x");
  for (; iIndexValue < iCodeLength; iIndexValue += ptType->uiSizeOfElement)
  {
    for (j=0; j < ptType->uiSizeOfElement; j++)
    {
      /* Little Endian... */
      if (ptType->iEndianess == FALSE)
      {
        i = (iIndexValue+(ptType->uiSizeOfElement-j))-1;
        sprintf(&pcBuffer[iIndexBuffer], "%02X", (uint8) pcValue[i]);
        iIndexBuffer+=2;
      }
      /* Big Endian... */
      else
      {
        sprintf(&pcBuffer[iIndexBuffer], "%02X", (uint8) pcValue[iIndexValue+j]);
        iIndexBuffer+=2;
      }
    }
  }
}

int16 ParseReceivedVariable(
  char         *pcValue,
  char         *pcInputStream,
  char         *pcName,
  struct TType *ptTypeDef,
  int16         iReducedLength,
  int16         iBufferLength)
/****************************************************************************/
/* Return the amount of bytes parsed for one single variable. The parsed    */
/* bytes are stored in pcValue in the format 0x..,,.. Parsing is done accor-*/
/* ding the type definitino in *ptTypeDef.                                  */
/* If iReducedLength >= 0, the value overrules the length as specified in   */
/* ptTypeDef. If iReducedLength<0 the value in ptTypeDef is used as length. */
/****************************************************************************/
{
  int16     iParameterLength;
  int16     i;
  int16     iEndianess;
  int16     bErrorFree;
  char     *pErrorString;
  char     *pLogLine;
  char     *pcNewValue;
  char      ByteString[3];
  
  unsigned char bSingleByte;

  iParameterLength = ptTypeDef->iSizeInBytes;
  iEndianess       = ptTypeDef->iEndianess;
  bErrorFree       = TRUE;

  if (iReducedLength > 0)
  {
    if (iReducedLength > iParameterLength)
    {
      pLogLine = (char *) malloc(250);
      sprintf(pLogLine, "****Actual length exceeds allowed length for %s ",
              pcName);
      LogLine(pLogLine);
      free(pLogLine);
    }
    iParameterLength = iReducedLength;
  }

  if (iParameterLength > iBufferLength)
  {
    /* The received buffer is smaller than the expected number of bytes.  */
    /* Therefore, adjust the number of to be parsed bytes to a safe limit */

    if (iEndianess == 0x00)
    {
      pLogLine = (char *) malloc(250);
      LogLine("*******************************************");
      sprintf(pLogLine, "No or too less bytes received for parameter %s ",
              pcName);
      LogLine(pLogLine);
      LogLine("This event does not match the specification file!!");
      LogLine("*******************************************");
      free(pLogLine);
    }
    if (ptTypeDef->iKind != ARRAYKIND)
    {
      iParameterLength = iBufferLength;
    }
    else
    {
      LogLine("Parameter is an array not processing any further");
      return 0;
    }
  }
  if (FILEDATAKIND != ptTypeDef->iKind)
  {
    if (ptTypeDef->iKind == ARRAYKIND)
    {
      if (ptTypeDef->uiSizeOfElement > 1)
      {
        ConvertArray(pcValue, pcInputStream, iParameterLength, ptTypeDef);
        pcInputStream += iParameterLength;
      }
      else
      {
        /* if iEndianess is 0, we have to scan the next n bytes backwards. If    */
        /* iEndianess != 0, we have to scan the next n bytes forwards.           */
        if (iEndianess == 0)
        {
          pcInputStream += iParameterLength;
        }
      
        strcpy(pcValue, "0x");
      
        i = iParameterLength;
        while (i > 0)
        {
      
          if (iEndianess == 0)
            bSingleByte = *(--pcInputStream);
          else
            bSingleByte = *(pcInputStream++);
      
          sprintf(ByteString, "%02X", bSingleByte);
          strcat(pcValue, ByteString);
          i--;
        }
      
        if (iEndianess == 0)
        {
          pcInputStream += iParameterLength;
        }
      }
    }
    else
    {
      /* if iEndianess is 0, we have to scan the next n bytes backwards. If    */
      /* iEndianess != 0, we have to scan the next n bytes forwards.           */
      if (iEndianess == 0)
      {
        pcInputStream += iParameterLength;
      }
    
      strcpy(pcValue, "0x");
    
      i = iParameterLength;
      while (i > 0)
      {
    
        if (iEndianess == 0)
          bSingleByte = *(--pcInputStream);
        else
          bSingleByte = *(pcInputStream++);
    
        sprintf(ByteString, "%02X", bSingleByte);
        strcat(pcValue, ByteString);
        i--;
      }
    
      if (iEndianess == 0)
      {
        pcInputStream += iParameterLength;
      }
    }
    
  
    /* Now we have stored the indicated amount of bytes from the input   */
    /* buffer. Check the received value on boundaries.                   */
  
    bErrorFree = CheckBoundaries(pcValue, ptTypeDef);
    if (bErrorFree == FALSE)
    {
      pErrorString = (char *) malloc(250);
      sprintf(pErrorString,
              "Error: parameter %s has value %s. This is out of range",
              pcName, pcValue);
      LogLine(pErrorString);
      free(pErrorString);
    }
  }  
  else
  {
    memcpy( pcValue, pcInputStream, iParameterLength);
    
  }
  if (bErrorFree)
  {
    return iParameterLength;
  }
  else
  {
    return 0;
  }

}


int16 CheckBoundaries(
  char *pcValue,
  struct TType * ptType)
/****************************************************************************/
/* Check if a particular value matches a particular type specification.     */
/* Returns TRUE if the value matches the specification.                     */
/* Returns FALSE otherwise.                                                 */
/**                                                                        **/
/* For the time beiing, no check is been done, and TRUE is retrurned always */
/****************************************************************************/
{
  pcValue = NULL;
  ptType = NULL;
  return TRUE;
}



void ProcessDecomposedPROTEvent(
  struct TEvent *ptEvent,
  struct TVariableList *ptVarList,
  int16 iBoard)
/****************************************************************************/
/* A received event must have been decomposed in an event number and a      */
/* parameter list. The corresponding tEvent structure must be available,    */
/* and the received parameters must be stored as separate values in a value */
/* list. Having that, this function can be called to check whether the re-  */
/* ceived event is able to trigger one of the statemachines running on the  */
/* indicated board.                                                         */
/*                                                                          */
/* This function finds the statemachine list of the indicated board. If the */
/* indicated board is not found, an error is logged. In real life, this may */
/* never happen.                                                            */
/****************************************************************************/
{
  struct TBoardList *ptBoardList;
  char     *pcErrorLine;

  ptBoardList = _ptBoardList;

  while ((ptBoardList != NULL)
         && (ptBoardList->ptThis != NULL)
         && (ptBoardList->ptThis->iNumber != iBoard))
  {
    ptBoardList = ptBoardList->ptNext;
  }

  if ((ptBoardList != NULL) && (ptBoardList->ptThis != NULL))
  {
    ProcessDecomposedPROTEvent2(ptEvent,
                               ptVarList, ptBoardList->ptThis->ptStateMachines);
  }
  else
  {
    pcErrorLine = malloc(256);
    sprintf(pcErrorLine, "Warning: Board #%d not used in this test!", iBoard);
    LogLine(pcErrorLine);
  }
}

void ProcessDecomposedPROTEvent2(
  struct TEvent *ptEvent,
  struct TVariableList *ptVarList,
  struct TStateMachineList *ptSMList)
/****************************************************************************/
/* The same precondition as for ProcessDecomposedPROTevent is applicable,   */
/* with the extention that a statemachine list must be provided instead of  */
/* a board list.                                                            */
/* If none of the statemachines is able to accept the received event, a     */
/* warning is logged to the user interface. In real life, this can happen   */
/* a lot; and it's not an internal error. Maybe it is a script error, maybe */
/* a bug is found in the system under test.                                 */
/****************************************************************************/
{
  int16                     bProcessed = FALSE;
  int16                     iNumberOfSM;
  char                     *pErrorLine;
  struct TStateMachineList *ptStartSMList;
  struct TTransition       *ptTransition;

  iNumberOfSM   = 0;
  pErrorLine    = NULL;
  ptStartSMList = ptSMList; 
  ptTransition  = NULL;      
  
  while (ptSMList != NULL)
  {
    if ((ptSMList->ptThis != NULL)
        && (ptSMList->ptThis->iPending == RUNNING_SM))
    {
      iNumberOfSM++;
      bProcessed = ProcessDecomposedPROTEvent3(ptEvent,
                                              ptVarList, ptSMList->ptThis);
      if (bProcessed == TRUE)
      {
        ptSMList = NULL;
      }
      else
      {
        ptSMList = ptSMList->ptNext;
      }
    }
  }

  if (bProcessed == FALSE)
  {
    
    if (iNumberOfSM == 1)
    {
      ptTransition                    = newTransition();
      ptTransition->iInstantNextState = FALSE;
      ptTransition->ptNextState       = FindState("UnhandledEvent",
                                                  ptStartSMList->ptThis->ptStates);
      
      if (NULL != ptTransition->ptNextState)
      {
        TransferState(ptStartSMList->ptThis, ptTransition);
      }
      free(ptTransition);
      
    }
    if((_ptGlobals->iLogMode & BSEK_LOG_STRING) != BSEK_LOG_STRING)
    {
      pErrorLine = malloc(250);
      sprintf(pErrorLine, "Warning: Event %s not processed!", ptEvent->pcName);
      LogLine(pErrorLine);
      free(pErrorLine);
    }
  }
}

int16 ProcessDecomposedPROTEvent3(
  struct TEvent *ptEvent,
  struct TVariableList *ptVarList,
  struct TStateMachine *ptSM)
/****************************************************************************/
/* This function tries to match the received event on one particular state  */
/* machine. If it succeeds, the variables are substituted, the statemachine */
/* transfers to the next state and TRUE is returned. If it fails, FALSE is  */
/* returned.                                                                */
/* To test if the statemachine is able to accept the event, all EVENT       */
/* actions are checked until one of the EVENT actions accepts the event.    */
/****************************************************************************/
{
  struct TTransitionList *ptTransitionList;
  int16     bFits;

  if (ptSM == NULL)
    return FALSE;
  if (ptSM->ptCurrentState == NULL)
    return FALSE;

  ptTransitionList = ptSM->ptCurrentState->ptTransitionList;

  while (ptTransitionList != NULL)
  {
    bFits = ProcessDecomposedPROTEvent4(ptEvent,
                                       ptVarList,
                                       ptSM->ptLocalVars,
                                       ptTransitionList->ptThis);
    if (bFits)
    {
      TryTransferState(ptSM, ptTransitionList->ptThis);
      return TRUE;
    }
    else
    {
      ptTransitionList = ptTransitionList->ptNext;
    }
  }
  return FALSE;
}


int16 ProcessDecomposedPROTEvent4(
  struct TEvent * ptEvent,
  struct TVariableList * ptVarList,
  struct TVariableList * ptLocalVars,
  struct TTransition * ptTrans)
/****************************************************************************/
/* This function tries to match the received event on one particular state  */
/* machine. If it succeeds, the variables are substituted, the statemachine */
/* transfers to the next state and TRUE is returned. If it fails, FALSE is  */
/* returned.                                                                */
/* To test if the statemachine is able to accept the event, all EVENT       */
/* actions are checked until one of the EVENT actions accepts the event.    */
/****************************************************************************/
{
  struct TActionList *ptActionList;
  int16     bFits;
  int16     iValidEvent;

  if (ptTrans == NULL)
    return FALSE;
  if (ptTrans->ptActionList == NULL)
    return FALSE;

  ptActionList = ptTrans->ptActionList;

  while (ptActionList != NULL)
  {
    /************************************************************************/
    /* First determine if the action is a valid event. A valid event has    */
    /* - no predefined iNrOfReceptions                                      */
    /* - or, if there is a predefined iNrOfReceptions                       */
    /*       - not been 'triggered' more than indicated by iNrOfReceptions  */
    /************************************************************************/
    iValidEvent = TRUE;
    if (ptActionList->ptThis->iNrOfReceptions > 0)
    {
      iValidEvent =
        (ptActionList->ptThis->iNrOfReceptions >
         (int) ptActionList->ptThis->lActionCounter);
    }

    if (iValidEvent == TRUE)
    {
      iValidEvent = (ptActionList->ptThis->iActionType == EVENT_ACTION);
    }

    if (iValidEvent)
    {
      bFits = TestAction(ptEvent, ptVarList, ptActionList->ptThis, ptLocalVars);
      if (bFits == TRUE)
      {
        SubstituteReceivedVariables(ptActionList->ptThis->ptVarList,
                                    ptVarList, ptLocalVars);
        ptActionList->ptThis->lActionCounter++;
        return TRUE;
      }
    }
    ptActionList = ptActionList->ptNext;
  }
  return FALSE;
}

int16 TestAction(
  struct TEvent * ptEvent,
  struct TVariableList * ptReceivedVarList,
  struct TAction * ptAction,
  struct TVariableList * ptStatemachineVarList)
/****************************************************************************/
/* One single action is checked if it is able to match the received event   */
/* and parameter list. If so, TRUE is returned. Open variabeles are NOT     */
/* substituted yet, this will be done by the function SubstituteReceivedVa- */
/* riables().                                                               */
/* If the length of the received parameter list and the expected parameter  */
/* list do not match, there is an internal error. This internal error is    */
/* logged in the script log file.                                           */
/****************************************************************************/
{
  int16     bVariableFits;
  struct TVariableList *ptVarList;

  if (ptAction->ptEvent != ptEvent)
  {
    return FALSE;
  }

  ptVarList = ptAction->ptVarList;

  while (ptReceivedVarList != NULL)
  {
    if (ptVarList == NULL)
    {
      LogLine("Internal error. More parameters parsed than possible");
      return FALSE;
    }
    bVariableFits = TestVariable(ptReceivedVarList->ptThis,
                                 ptStatemachineVarList, ptVarList->ptThis);
    if (bVariableFits == FALSE)
    {
      return FALSE;
    }

    ptVarList = ptVarList->ptNext;
    ptReceivedVarList = ptReceivedVarList->ptNext;
  }

  if (ptVarList != NULL)
  {
    LogLine("Internal error. Not enough parameters parsed");
  }
  return TRUE;
}


int16 TestVariable(
  struct TVariable * ptReceivedVar,
  struct TVariableList * ptVarList,
  struct TVariable * ptVariable)
/****************************************************************************/
/* This function checks if a received value matches an expected value. The  */
/* function returns TRUE if:                                                */
/* - the received value is of no interest: Thers is no expected ptValue     */
/* - the expected value is not known: the variable is not initialised yet.  */
/* - the expected value is equal to the received value.                     */
/* In all other cases, FALSE is returned.                                   */
/* If the expected value is a so called "smart string", the expected value  */
/* is treated as such.                                                      */
/****************************************************************************/
{
  int16     bNamedVariable;

  if (ptVariable == NULL)
    return FALSE;

  if (ptVariable->ptValue == NULL)
    return TRUE;

  if (ptVariable->ptValue->pcValue == NULL)
    return TRUE;

  if (ptVariable->ptValue->pcValue[0] == '\0')
    return TRUE;

  bNamedVariable = ((ptVariable->pcName != NULL)
                    && (ptVariable->pcName[0] != 0));

  if ((bNamedVariable == FALSE) && (ptVariable->ptValue->bFixed == FALSE))
  {
    /* no name and not fixed is an empty variable. This always matches.     */
    return TRUE;
  }

  /* At this point we're sure that                                           */
     /* - the variable has a value : we can compare something                  */
  /* - the variable has a name or is fixed : we must compare something.     */

  if ((bNamedVariable == TRUE)
      && (ptVariable->pcName != NULL) && (ptVariable->pcName[0] == '"'))
  {
    return (smartcmp(ptVariable->ptItsType->iKind,
                     ptVariable->pcName,
                     ptReceivedVar->ptValue->pcValue));
  }

  if (strcmp(ptReceivedVar->ptValue->pcValue,
             ptVariable->ptValue->pcValue) == 0)
  {
    return TRUE;
  }
  else
  {
    return(ValueCmp(ptVariable, ptReceivedVar));
  }

  return FALSE;
}

void SubstituteReceivedVariables(
  struct TVariableList *ptVarList,
  struct TVariableList *ptReceivedVars,
  struct TVariableList *ptRefVars)
/****************************************************************************/
/****************************************************************************/
{

  while (ptVarList != NULL)
  {
    SubstituteReceivedVariable(ptVarList->ptThis,
                               ptReceivedVars->ptThis, ptRefVars);
    ptVarList = ptVarList->ptNext;
    ptReceivedVars = ptReceivedVars->ptNext;
  }
}


void SubstituteReceivedVariable(
  struct TVariable *ptVar,
  struct TVariable *ptReceivedVar,
  struct TVariableList *ptRefVar)
/****************************************************************************/
/****************************************************************************/
{
  FILE *pFile;
  int   iResult;
  
  if (ptVar == NULL)
    return;

  if (ptVar->ptValue == NULL)
    return;

  if (ptVar->ptValue->pcFileName != NULL)
  {
    if( strstr( ptVar->ptValue->pcFileMode, "w") != NULL )
    {
      pFile = fopen(ptVar->ptValue->pcFileName, "w+");
      if (pFile != NULL)
      {
        iResult = strlen(ptReceivedVar->ptValue->pcValue);
        fwrite( ptReceivedVar->ptValue->pcValue, 
                sizeof(char), 
                ptReceivedVar->ptValue->uiLength, 
                pFile);
        fclose(pFile);
      }
    }
  }
  else
  {
    if ((ptVar->pcName != NULL) && (ptVar->pcName[0] == '"'))
    {
      smartSubstitute(ptVar->pcName, ptRefVar, ptReceivedVar->ptValue->pcValue);
    }
  
    if (ptVar->ptValue->pcValue == NULL)
    {
      ptVar->ptValue->pcValue = my_strdup(ptReceivedVar->ptValue->pcValue);
      return;
    }
  
    if (ptVar->ptValue->pcValue[0] == '\0')
    {
      strcpy(ptVar->ptValue->pcValue, ptReceivedVar->ptValue->pcValue);
    }
  }
}
int16 ValueCmp(
  struct TVariable * ptReceivedVar,
  struct TVariable * ptVariable)
{
  int16 iResult;
  int32 ulReceivedValue;
  int32 ulVariableValue;
  iResult = FALSE;
  
  if ((ptReceivedVar->ptValue->pcValue[0] == '0') && (ptReceivedVar->ptValue->pcValue[1] == 'x'))
  {
    sscanf(ptReceivedVar->ptValue->pcValue,"%x", &ulReceivedValue);
  }
  else
  {
    sscanf(ptReceivedVar->ptValue->pcValue,"%ul", &ulReceivedValue);
  }
  if ((ptVariable->ptValue->pcValue[0] == '0') && (ptVariable->ptValue->pcValue[1] == 'x'))
  {
    sscanf(ptVariable->ptValue->pcValue,"%x", &ulVariableValue);
  }
  else
  {
    sscanf(ptVariable->ptValue->pcValue,"%ul", &ulVariableValue);
  }
  if (ulReceivedValue == ulVariableValue)
  {
    iResult = TRUE;
  }
  return(iResult);
}

/****************************************************************************/
/* From this point, only so called "smart string" functions are defined.    */
/* There are actually two types of "smart strings". The smart strings used  */
/* in commands, and the smart strings used in events.                       */
/*                                                                          */
/* ________________________Smart strings in commands________________________*/
/* A 'smart string' is a string with references to variables. A typical     */
/* smart string looks like "COM.FNC:L2CA_Init({seqnr},{init})". The fragment*/
/* between brackets {} will be replaced then by the value of the variables. */
/* When the pcValue string of the variable 'seqnr' is e.g. '0x00' and the   */
/* pcValue string of the variable 'init' is e.g. 'TRUE', the smart string   */
/* will be expanded to "COM.FNC:L2CA_Init(0x00,TRUE)" and this string will  */
/* be sent out when the command in which this smart string was used, is     */
/* executed.                                                                */
/*                                                                          */
/* _________________________Smart strings in events_________________________*/
/* A 'smart string' is a string which refers to varialbes. A typical smart  */
/* string in an event looks like: "COM:MSG[L2CA_INIT_COMP({4,a},{4,=b})"    */
/* The {4,a} fragment indicates that four incoming characters will be       */
/* compared with the value of a. If the mentioned variable (a) is uninitia- */
/* lised, the comparisation will always match. If a is initialised, the     */
/* value MUST be equal to the received characters, otherwise the transition */
/* does not take place.                                                     */
/* The second fragment, {4,=b} is an example of an explicit comparisation.  */
/* In this case, b MUST be initialised and equal to four received characters*/
/* Instead of the '=' character, also an '<', '>' or an '!' can be used.    */
/* Only if all comparisations match, uninitialised variables will be initia-*/
/* lised with the received characters.                                      */
/****************************************************************************/

int16     smartcmp(
  int iKind,
  char *pcVarName,         /* As it appears in the script file  ("\"ab\"")  */
  char *pcReceivedValue)   /* As received from the DUT  ("0x00610062")      */

{
  int i=0,j=0;
  char ch;

  if (pcVarName[i++]==0) return TRUE; /* Empty string is always ok. */

  if (pcReceivedValue[j++] != '0') return FALSE;
  if (pcReceivedValue[j++] != 'x') return FALSE;

  while (pcVarName[i] != '"')
  {
    /* Test character i on position j, j+1 (,j+2 and j+3) of received value   */
    if ((iKind == ASCIIKIND) || (iKind == ASCII0KIND))
    {
      ch = TO_BYTE(pcReceivedValue[j],pcReceivedValue[j+1]);

      if (pcReceivedValue[j++]==0) return FALSE;
      if (pcReceivedValue[j++]==0) return FALSE;
      if (ch != pcVarName[i])      return FALSE;
    }
    else if ((iKind == UNICODEKIND) || (iKind == UNICODE0KIND))
    {
      if (pcReceivedValue[j++] != '0') return FALSE;
      if (pcReceivedValue[j++] != '0') return FALSE;

      ch = TO_BYTE(pcReceivedValue[j],pcReceivedValue[j+1]);

      if (pcReceivedValue[j++]==0) return FALSE;
      if (pcReceivedValue[j++]==0) return FALSE;
      if (ch != pcVarName[i])      return FALSE;
    }
    else
    {
      return FALSE;
    }

    i++; /* Go to next character. Index j is already incremented.             */
  }

  if ((iKind==ASCIIKIND) || (iKind == UNICODEKIND))
  {
    return TRUE;
  }
  else if (iKind == ASCII0KIND)
  {
    if (pcReceivedValue[j++] != '0') return FALSE;
    if (pcReceivedValue[j++] != '0') return FALSE;
    if (pcReceivedValue[j]==0) return TRUE;
    else                       return FALSE;
  }
  else if (iKind == UNICODE0KIND)
  {
    if (pcReceivedValue[j++] != '0') return FALSE;
    if (pcReceivedValue[j++] != '0') return FALSE;
    if (pcReceivedValue[j++] != '0') return FALSE;
    if (pcReceivedValue[j++] != '0') return FALSE;
    if (pcReceivedValue[j]==0) return TRUE;
    else                       return FALSE;
  }
  else                       return FALSE;
}
#ifdef USE_OLD_CODE_FOR_SMARTCMP
{

  char     *pReceivedChar;
  char     *pExpectedChar;
  char     *pExpectedVar;
  struct TVariable *ptVar;
  char     *pCompareChar;
  char     *pError;
  uint16    uiExpectedLength;
  int16     iEqual;

  pReceivedChar = pcReceivedValue + 2;  /* skip the 0x prefix                */
  pExpectedChar = pcVarName + 1;        /* skip the " prefix                 */

  if (*pExpectedChar == '"')
  {
    /* A "" string is used to compare against. This compare always fails.   */
    /* (This is an empty string)                                            */
    return FALSE;
  }

  while (*pExpectedChar != '"')
  {

    if (*pExpectedChar == '{')
    {
      pExpectedChar++;
      while (*pExpectedChar == ' ')
        pExpectedChar++;        /* skip spaces        */
      uiExpectedLength = GetInteger(pExpectedChar);
      if (uiExpectedLength == 0)
      {
        pError = (char *) malloc(250);
        sprintf(pError, "Error: Expected length must be greater than zero");
        LogLine(pError);
        free(pError);
        return FALSE;
      }

      while ((*pExpectedChar != ',') && (*pExpectedChar != '\0'))
      {
        pExpectedChar++;        /* find the comma.    */
      }

      if (*pExpectedChar == '\0')
      {
        pError = (char *) malloc(250);
        sprintf(pError, "Error: comma expected after variable size");
        LogLine(pError);
        free(pError);
        return FALSE;
      }


      pExpectedChar++;          /* skip the comma.    */

      while (*pExpectedChar == ' ')
        pExpectedChar++;        /* skip spaces        */

      pCompareChar = NULL;
      if ((*pExpectedChar == '<')
          || (*pExpectedChar == '>')
          || (*pExpectedChar == '=') || (*pExpectedChar == '!'))
      {
        pCompareChar = pExpectedChar;
        pExpectedChar++;
      }

      while (*pExpectedChar == ' ')
        pExpectedChar++;        /* skip spaces        */

      pExpectedVar = pExpectedChar;     /* here the variable name starts.      */
      while (*pExpectedChar != '}')
        pExpectedChar++;        /* find the } char    */

      *pExpectedChar = '\0';    /* temporarily. To isolate the variable name. */
      ptVar = FindVar(pExpectedVar, ptVarList);
      if (ptVar == NULL)
      {
        pError = (char *) malloc(250);
        sprintf(pError, "Error: Cannot find variable '%s'", pExpectedVar);
        LogLine(pError);
        free(pError);
        return FALSE;
      }

      *pExpectedChar = '}';     /* restore the temporary '\0' character.       */

      /* At this point, the pExpectedChar pointer just passed the          */
      /* {length,var} section. Nowe we have to check the expected variable. */


      if ((ptVar->ptValue != NULL)
          && (ptVar->ptValue->pcValue != NULL)
          && (ptVar->ptValue->pcValue[0] != '\0'))
      {
        /* Expected variable has a value.                                  */

        if (pCompareChar == NULL)
        {
          /* but we're not interested..                                    */
          pReceivedChar += 2 * uiExpectedLength;
        }
        else if (*pCompareChar == '=')
        {
          /* and we're interested :-)                                      */
          pExpectedVar = ptVar->ptValue->pcValue;

          if (strlen(pExpectedVar) != uiExpectedLength)
          {
            return FALSE;
          }
          while (*pExpectedVar != 0)
          {
            if (!SingleCharMatch(*pExpectedVar, pReceivedChar))
            {
              return FALSE;
            }
            pExpectedVar++;
            pReceivedChar += 2;
          }
        }
        else if (*pCompareChar == '!')
        {
          /* and we're interested :-)                                      */
          pExpectedVar = ptVar->ptValue->pcValue;

          if (strlen(pExpectedVar) == uiExpectedLength)
          {
            return FALSE;
          }

          iEqual = TRUE;
          while (*pExpectedVar != 0)
          {
            if (!SingleCharMatch(*pExpectedVar, pReceivedChar))
            {
              iEqual = FALSE;
            }
            pExpectedVar++;
            pReceivedChar += 2;
            if (iEqual == TRUE)
            {
              return FALSE;
            }
          }
        }
        else if (*pCompareChar == '<')
        {
          /* and we're interested :-)                                      */
          LogLine("< is not implemented.. Sorry");
          return FALSE;
        }
        else if (*pCompareChar == '>')
        {
          /* and we're interested :-)                                      */
          LogLine("> is not implemented.. Sorry");
          return FALSE;
        }

      }
      else
      {
        /* Indicated variable is not initialised yet. Skip the right amount */
        /* of bytes in the received string.                                */

        pReceivedChar += 2 * uiExpectedLength;

        if (pCompareChar != NULL)
        {
          if (*pCompareChar == '=');    /* accept          */
          else if (*pCompareChar == '<')
            return FALSE;       /* don't accept    */
          else if (*pCompareChar == '>')
            return FALSE;       /* don't accept    */
          else if (*pCompareChar == '!')
            return FALSE;       /* don't accept    */
          else
            return FALSE;       /* don't accept    */
        }
      }
    }
    else
    {
      if (!SingleCharMatch(*pExpectedChar, pReceivedChar))
      {
        return FALSE;
      }
      pReceivedChar += 2;
    }

    pExpectedChar++;
  }
  return TRUE;
}

#endif

int16 smartSubstitute(
  char *pcVarName,
  struct TVariableList * ptVarList,
  char *pcReceivedValue)
/****************************************************************************/
/* pcVarName is e.g. ""COM.L2CA_VERSION_CNF({abc,4})""                      */
/* ptVarList is a pointer to the list of vars to which abc belongs. (should)*/
/* pcReceivedValue is e.g. "0x808080                                        */
/****************************************************************************/
{

  char     *pReceivedChar;
  char     *pSubstituteChar;
  char     *pExpectedChar;
  char     *pExpectedVar;
  struct TVariable *ptVar;
  char     *pCompareChar;
  uint16    uiExpectedLength;
  uint8     ucLowNibble;
  uint8     ucHighNibble;

  pReceivedChar = pcReceivedValue + 2;  /* skip the 0x prefix                */
  pExpectedChar = pcVarName + 1;        /* skip the " prefix                 */

  while (*pExpectedChar != '"')
  {
    pCompareChar = NULL;
    if (*pExpectedChar == '{')
    {
      pExpectedChar++;
      while (*pExpectedChar == ' ')
        pExpectedChar++;        /* skip spaces        */
      uiExpectedLength = GetInteger(pExpectedChar);

      while ((*pExpectedChar != ',') && (*pExpectedChar != '\0'))
      {
        pExpectedChar++;        /* find the comma.    */
      }

      pExpectedChar++;          /* skip the comma.    */

      while (*pExpectedChar == ' ')
        pExpectedChar++;        /* skip spaces        */

      if ((*pExpectedChar == '<')
          || (*pExpectedChar == '<')
          || (*pExpectedChar == '=') || (*pExpectedChar == '!'))
      {
        pCompareChar = pExpectedChar;
        pExpectedChar++;
      }

      while (*pExpectedChar == ' ')
        pExpectedChar++;        /* skip spaces        */

      pExpectedVar = pExpectedChar;     /* here the variable name starts.      */
      while (*pExpectedChar != '}')
        pExpectedChar++;        /* find the } char    */

      *pExpectedChar = '\0';    /* temporarily. To isolate the variable name. */
      ptVar = FindVar(pExpectedVar, ptVarList); /* always found. Guaranteed. */

      if (ptVar->ptItsType == NULL)
      {
        /* This is a dirty trick. I'm sorry. When ptItsType == NULL,       */
        /* the WriteVar routine consideres it as a DataVar, and will not   */
        /* write the value of it. Therefore, we have to give it "some"     */
        /* type.                                                           */
        ptVar->ptItsType = _ptTypeList->ptThis;
      }

      *pExpectedChar = '}';     /* restore the temporary '\0' character.       */

      /* At this point, the pExpectedChar pointer just passed the          */
      /* {length,var} section. Nowe we have to check the expected variable. */

      if ((ptVar->ptValue != NULL)
          && (ptVar->ptValue->pcValue != NULL)
          && (ptVar->ptValue->pcValue[0] != '\0'))
      {
        /* Expected variable has already a value. Delete it.               */
        ptVar->ptValue->pcValue[0] = '\0';
      }

      if (ptVar->ptValue == NULL)
        ptVar->ptValue = newValue();
      if (ptVar->ptValue->pcValue == NULL)
      {
        ptVar->ptValue->pcValue = (char *) malloc(uiExpectedLength + 1);
      }

      pSubstituteChar = ptVar->ptValue->pcValue;

      while (uiExpectedLength > 0)
      {
        ucHighNibble = *(pReceivedChar++);
        ucLowNibble = *(pReceivedChar++);
        *pSubstituteChar = (char) (TO_HEX(ucHighNibble) << 4);
        *pSubstituteChar |= (char) (TO_HEX(ucLowNibble));

        uiExpectedLength--;
        pSubstituteChar++;
      }
      *pSubstituteChar = '\0';
    }
    else
    {
      pReceivedChar += 2;
    }

    pExpectedChar++;
  }
  return TRUE;
}


int16 SingleCharMatch(
  char cByte,
  char *pHexString)
{
  if (cByte == '~')
    return TRUE;                /* this is the wildcard character */

  if (TO_ASCII((cByte >> 4) & 0x0F) != pHexString[0])
    return FALSE;
  if (TO_ASCII((cByte) & 0x0F) != pHexString[1])
    return FALSE;
  return TRUE;
}
