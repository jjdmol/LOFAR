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
// 08/08/2000 E.A. Nijboer            Initial release
// 12/01/2001 E.A. Nijboer            platform test.
// 09/02/2001 E.A. Nijboer            
//
//=========================================================================
*/
#include <stdlib.h>

#include "stepper.h"
#include "stores.h"
#include "codingstandard.h"
#include "string.h"
#include "bsegui.h"
#include "executor_master.h"
#include "general_lib.h"
#include "expirator.h"
#include "stdio.h"
#include "malloc.h"
#include "atdriver.h"
#include "executor_lib.h"
#include "parser_lib.h"
#include "atkernel.h"
#include "atkernel_intern.h"
#include "arithmetric.h"


#define   ASCII ((int16)(753))
#define   PROT  ((int16)(969))
#define   TCI   ((int16)(162))



/* FUNCTIONNR combines an OCF and SUBFIELD into a PROT function number.      */
#define FUNCTIONNR(x,y) (((x)) | ((y)<<10))

/*****************************************************************************/
/* Local Headers                                                             */
/*****************************************************************************/
int16     MakeBoardStep(
  struct TBoard *ptBoard,
  int16 iIOChannel);

/*int16 MakeStateMachineStep (struct TStateMachine *ptMachine, int16 iIOChannel); */
int16     SingleStep(
  struct TStateMachine *ptMachine,
  int16 iIOChannel);
int16     ExecuteFunction(
  struct TStateMachine *ptMachine,
  int16 iIOChannel);
int16     ExecuteStateMachines(
  struct TStateMachine *ptMachine,
  int16 iIOChannel);

void      SubstituteVariables(
  struct TVariableList *ptVarList,
  struct TParameterList *ptParameterList,
  struct TVariableList *ptValueList,
  struct TVariableList *ptGlobalVars);

void      Get_Random_Enumerated(
  char *pcValue,
  struct TValueDefList *ptValueDefs);

void      Get_Random_Limited(
  char *pcValue,
  char *pcLower,
  char *pcUpper);
void      Get_Random_Unlimited(
  char *pcValue,
  int iByteCount);
void      Get_Random_Bitfield(
  char *pcValue,
  struct TValueDefList *ptValueDefs);


int16     Intern_Timer(
  struct TVariableList *ptVarList,
  struct TStateMachine *ptStateMachine,
  struct TTransition *ptTransition);

int16     Intern_MTimer(
  struct TVariableList *ptVarList,
  struct TStateMachine *ptStateMachine,
  struct TTransition *ptTransition);

int16     Intern_RTimer(
  struct TVariableList *ptVarList,
  struct TStateMachine *ptStateMachine,
  struct TTransition *ptTransition);
int16     Intern_RMTimer(
  struct TVariableList *ptVarList,
  struct TStateMachine *ptStateMachine,
  struct TTransition *ptTransition);

int16     Intern_Clear(
  struct TVariableList *ptVarList);
int16     Intern_R_Sig(
  struct TVariableList *ptVarList);
int16     Intern_S_Sig(
  struct TVariableList *ptVarList);
int16     Intern_Wait(
  struct TVariableList *ptVarList);
int16     Intern_Comp(
  struct TVariableList *ptVarList);
int16     Intern_Print(
  struct TVariableList *ptVarList);
int16     Intern_PROT(
  int16 iIOChannel);
int16     Intern_TCI(
  int16 iIOChannel);
int16     Intern_ASCII(
  int16 iIOChannel);
int16     Intern_PROT_W(
  int16 iIOChannel,
  struct TVariableList *ptVarList);
int16     Intern_Term(
  char *pcReason);
int16     Intern_Rescue(
  );
int16     Intern_End(
  struct TStateMachine *pt);
int16     Intern_CtsHigh_Action(
  int16 iIOChannel);
int16     Intern_CtsLow_Action(
  int16 iIOChannel);
int16     Intern_DsrHigh_Action(
  int16 iIOChannel);
int16     Intern_DsrLow_Action(
  int16 iIOChannel);

int16     Intern_DtrHigh_Action(
  int16 iIOChannel);
int16     Intern_DtrLow_Action(
  int16 iIOChannel);
int16     Intern_SetCommBreak(
  int16 iIOChannel);
int16     Intern_ClearCommBreak(
  int16 iIOChannel);

int16     Intern_SOCKET_GENERAL_ERROR(
  int16 iIOChannel);
int16     Intern_SOCKET_SEND_ERROR(
  int16 iIOChannel);
int16     Intern_SOCKET_RECEIVE_ERROR(
  int16 iIOChannel);
int16     Intern_SOCKET_CONNECT_ERROR(
  int16 iIOChannel);
int16     Intern_SOCKET_DISCONNECT_ERROR(
  int16 iIOChannel);
int16     Intern_SOCKET_ACCEPT_ERROR(
  int16 iIOChannel);
int16     Intern_SOCKET_UNKNOWN_ERROR(
  int16 iIOChannel);
int16     Intern_CONNECTION_LOST(
  int16 iIOChannel);
int16     Intern_CONNECTION_ESTABLISHED(
  int16 iIOChannel);


void      SetProtocol(
  int16 iBoardNumber,
  int16 iProtocolType);
void      SendFunction(
  int16 iIOChannel,
  struct TFunction *ptFunction,
  struct TVariableList *ptVarList);
int16     EvaluateForSmartString(
  struct TVariable *ptVar,
  struct TVariableList *ptVarList);

void      Send_PROT_ACL_Data(
  int16 iBoardNumber,
  int16 iHandle,
  int16 iPBFlags,
  int16 iBCFlags,
  char *pcBuffer,
  int16 iBufferLength);

char     *FindBufferInGlobalDataList(
  char *pcBufferName);

struct TParameterList *NextParameter(
  struct TParameterList *ptParameterList,
  char *pcValue);

int TransferValue(
  char         *pcBuffer,
  char         *pcValue,
  struct TType *ptType,
  unsigned int  uiLenght);

void DetailledLog(
  struct TParameter *ptParameter,
  struct TVariable  *ptVar);
/* Sets a new timer returns NULL when failed otherwise the Timer */
static struct TTimer *SetTimer( int32 lTimerValue,
                                float fTimerScaling,
                                struct TStateMachine * ptStateMachine);
           
/* Converts a byte array according to the Endianess defined */
static int ConvertValue(char         *pcBuffer,
                        char         *pcValue,
                        int16         iCodeLength,
                        struct TType *ptType);
/* Converts the array in Value to the Endiness required */                                
static void ConvertArray(char         *pcBuffer,
                         char         *pcValue,
                         int           iCodeLength,
                         struct TType *ptType );
                                


/*****************************************************************************/
/* Local constants                                                           */
/*****************************************************************************/
char     *_NO_SIGNAL = " ";
char     *_SIGNAL = "*";

/* The following variables are bitmaps. The bitnumber of each bit set        */
/* indicates the channel on which the signal is received.                    */

uint32    ul_SOCKET_GENERAL_ERROR = 0;
uint32    ul_SOCKET_SEND_ERROR = 0;
uint32    ul_SOCKET_RECEIVE_ERROR = 0;
uint32    ul_SOCKET_CONNECT_ERROR = 0;
uint32    ul_SOCKET_DISCONNECT_ERROR = 0;
uint32    ul_SOCKET_ACCEPT_ERROR = 0;
uint32    ul_SOCKET_UNKNOWN_ERROR = 0;
uint32    ul_CONNECTION_LOST = 0;
uint32    ul_CONNECTION_ESTABLISHED = 0;

/*****************************************************************************/
/* Exported functions                                                        */
/*****************************************************************************/
int16 MakeStep(
  void)
{
  struct TBoardList *ptBoardWalker;

  int16     bModified;
  int16     bAnyModified;

  bAnyModified  = FALSE;
  ptBoardWalker = _ptBoardList;

  while (ptBoardWalker != NULL)
  {
    bModified = MakeBoardStep(ptBoardWalker->ptThis, ptBoardWalker->ptThis->iNumber);
    if (bModified == TRUE)  bAnyModified = TRUE;

    ptBoardWalker = ptBoardWalker->ptNext;
  }
  return bAnyModified;
}

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/

int16 MakeBoardStep(
  struct TBoard * ptBoard,
  int16 iIOChannel)
{
  struct TStateMachineList *ptStateMachineWalker;
  int16     bModified;
  int16     bAnyModified;

  bAnyModified = FALSE;
  bModified = FALSE;

  ptStateMachineWalker = ptBoard->ptStateMachines;
  while (ptStateMachineWalker != NULL)
  {
    bModified = SingleStep(ptStateMachineWalker->ptThis, iIOChannel);
    if (bModified == TRUE)
    {
      bAnyModified = TRUE;
    }
    ptStateMachineWalker = ptStateMachineWalker->ptNext;
  }
  return bAnyModified;
}




int16 SingleStep(
  struct TStateMachine * ptStateMachine,
  int16 iIOChannel)
/****************************************************************************/
/* Tries to perform a function or an internal event in the referred         */
/* statemachine. If succeeded, TRUE will be returned. Otherwise, FALSE will */
/* be returned. If a function is executed, the result of the executed       */
/* function will be redirected to the indicated IOChannel.                  */
/****************************************************************************/
{
  struct TTransitionList *ptTransitionWalker;
  struct TActionList *ptActionWalker;
  int16     bDone;
  int16     bFunctionDone;
  struct TVariableList *ptVarList;
  char     *pcState;
  char     *pcResult;
  int16     iChannel;



  /* first do some safety checks.                                           */

  if (ptStateMachine == NULL)
    return FALSE;
  if (ptStateMachine->iPending != RUNNING_SM)
    return FALSE;

  if (ptStateMachine->ptCurrentState == NULL)
  {
    ptStateMachine->ptCurrentState = ptStateMachine->ptStates->ptThis;
  }

  /* Try to execute a normal function.                                      */
  bFunctionDone = ExecuteFunction(ptStateMachine, iIOChannel);

  /* If failed, then try an internal action.                                */
  /* We don't make a fancy algorithm to choose between severel internal     */
  /* actions, we only pick the first one.                                   */

  if (bFunctionDone == FALSE)
  {
    bFunctionDone = ExecuteStateMachines(ptStateMachine, iIOChannel);
  }

  if (bFunctionDone == FALSE)
  {
    ptTransitionWalker = ptStateMachine->ptCurrentState->ptTransitionList;
    pcState = ptStateMachine->ptCurrentState->pcName;
    bDone = FALSE;
    while ((ptTransitionWalker != NULL) && (bDone == FALSE))
    {
      ptActionWalker = ptTransitionWalker->ptThis->ptActionList;
      while (ptActionWalker != NULL)
      {
        ptVarList = ptActionWalker->ptThis->ptVarList;

        if (ptTransitionWalker->ptThis->iAlternative == 0)
          iChannel = iIOChannel;
        else if (ptStateMachine->iAlternative == -1)
          iChannel = iIOChannel;
        else
          iChannel = ptStateMachine->iAlternative;

        switch (ptActionWalker->ptThis->iActionType)
        {
          case TIMER_ACTION:
            bDone |= Intern_Timer(ptVarList,
                                  ptStateMachine, 
                                  ptTransitionWalker->ptThis);
            break;
          case MTIMER_ACTION:
            bDone |= Intern_MTimer(ptVarList,
                                   ptStateMachine, 
                                   ptTransitionWalker->ptThis);
            break;
          case RTIMER_ACTION:
            bDone |= Intern_RTimer(ptVarList,
                                   ptStateMachine, 
                                   ptTransitionWalker->ptThis);
            break;
          case RMTIMER_ACTION:
            bDone |= Intern_RMTimer(ptVarList,
                                    ptStateMachine, 
                                    ptTransitionWalker->ptThis);
            break;
          case CLEAR_ACTION:
            bDone |= Intern_Clear(ptVarList);
            break;
          case R_SIG_ACTION:
            bDone |= Intern_R_Sig(ptVarList);
            break;
          case S_SIG_ACTION:
            bDone |= Intern_S_Sig(ptVarList);
            break;
          case WAIT_ACTION:
            bDone |= Intern_Wait(ptVarList);
            break;
          case TERMINATE_ACTION:
            bDone |= Intern_Term(pcState);
            break;
          case PRINT_ACTION:
            bDone |= Intern_Print(ptVarList);
            break;
          case COMPARE_ACTION:
            bDone |= Intern_Comp(ptVarList);
            break;
          case BREAK_ACTION:;
            break;
          case PROT_PROTOCOL_ACTION:
            bDone |= Intern_PROT(iChannel);
            break;
          case TCI_PROTOCOL_ACTION:
            bDone |= Intern_TCI(iChannel);
            break;
          case ASCII_PROTOCOL_ACTION:
            bDone |= Intern_ASCII(iChannel);
            break;
          case PROT_WRITEDATA_ACTION:
            bDone |= Intern_PROT_W(iChannel, ptVarList);
            break;
          case RESCUE_ACTION:
            bDone |= Intern_Rescue();
            break;
          case ARITHMETRIC_ACTION:
            bDone |= TRUE;
            pcResult = EvaluateExpression(ptActionWalker->ptThis->ptExpression);
            free(pcResult);
            break;
          case IF_ACTION:
            pcResult = EvaluateExpression(ptActionWalker->ptThis->ptExpression);
            bDone |= (strcmp(pcResult, "TRUE") == 0);
            free(pcResult);
            break;
          case END_ACTION:
            bDone |= Intern_End(ptStateMachine);
            break;
          case CTS_HIGH_ACTION:
            bDone |= Intern_CtsHigh_Action(iChannel);
            break;
          case CTS_LOW_ACTION:
            bDone |= Intern_CtsLow_Action(iChannel);
            break;
          case DSR_HIGH_ACTION:
            bDone |= Intern_DsrHigh_Action(iChannel);
            break;
          case DSR_LOW_ACTION:
            bDone |= Intern_DsrLow_Action(iChannel);
            break;
          case DTR_HIGH_ACTION:
            bDone |= Intern_DtrHigh_Action(iChannel);
            break;
          case DTR_LOW_ACTION:
            bDone |= Intern_DtrLow_Action(iChannel);
            break;
          case SET_COMM_BREAK_ACTION:
            bDone |= Intern_SetCommBreak(iChannel);
            break;
          case CLEAR_COMM_BREAK_ACTION:
            bDone |= Intern_ClearCommBreak(iChannel);
            break;
          case SOCKET_GENERAL_ERROR:
            bDone |= Intern_SOCKET_GENERAL_ERROR(iChannel);
            break;
          case SOCKET_SEND_ERROR:
            bDone |= Intern_SOCKET_SEND_ERROR(iChannel);
            break;
          case SOCKET_RECEIVE_ERROR:
            bDone |= Intern_SOCKET_RECEIVE_ERROR(iChannel);
            break;
          case SOCKET_CONNECT_ERROR:
            bDone |= Intern_SOCKET_CONNECT_ERROR(iChannel);
            break;
          case SOCKET_DISCONNECT_ERROR:
            bDone |= Intern_SOCKET_DISCONNECT_ERROR(iChannel);
            break;
          case SOCKET_ACCEPT_ERROR:
            bDone |= Intern_SOCKET_ACCEPT_ERROR(iChannel);
            break;
          case SOCKET_UNKNOWN_ERROR:
            bDone |= Intern_SOCKET_UNKNOWN_ERROR(iChannel);
            break;
          case CONNECTION_LOST:
            bDone |= Intern_CONNECTION_LOST(iChannel);
            break;
          case CONNECTION_ESTABLISHED:
            bDone |= Intern_CONNECTION_ESTABLISHED(iChannel);
            break;

        }
        ptActionWalker = ptActionWalker->ptNext;
      }
      if (bDone == FALSE)
      {
        /* If this action could not be executed, try the next.              */
        ptTransitionWalker = ptTransitionWalker->ptNext;
      }
    }
    if (bDone == TRUE)
    {
      /* A transition must be done now. Do that transition.                 */
      TransferState(ptStateMachine, ptTransitionWalker->ptThis);
    }
    return bDone;
  }
  else
  {
    return TRUE;
  }
}



int16 ExecuteFunction(
  struct TStateMachine * ptStateMachine,
  int16 iIOChannel)
/****************************************************************************/
/* Tries to perform a function in the referred statemachine. If succeeded,  */
/* TRUE will be returned. Otherwise, FALSE will be returned. The result of  */
/* the executed function will be redirected to the indicated IOChannel.     */
/****************************************************************************/
{
  struct TTransitionList *ptTransitionWalker;
  struct TActionList *ptActionWalker;
  int       iFunctionCounter;

  /* First count the number of function transitions in the actual state.    */
  /* PROT_SendData will be treated as a function..                           */
  /**************************************************************************/

  iFunctionCounter = 0;
  ptTransitionWalker = ptStateMachine->ptCurrentState->ptTransitionList;
  while (ptTransitionWalker)
  {
    if ((ptTransitionWalker->ptThis->ptActionList->ptThis->iActionType ==
         FUNCTION_ACTION) ||
        (ptTransitionWalker->ptThis->ptActionList->ptThis->iActionType ==
         PROT_WRITEDATA_ACTION))
    {
      iFunctionCounter++;
    }
    ptTransitionWalker = ptTransitionWalker->ptNext;
  }

  if (iFunctionCounter == 0)
  {
    /* No function transitions available in the active state. Return false. */
    return FALSE;
  }

  /* Number of function transitions is known now. Pick a random one, and    */
  /* execute it.                                                            */
  /**************************************************************************/

  iFunctionCounter = iRandom(iFunctionCounter);
  ptTransitionWalker = ptStateMachine->ptCurrentState->ptTransitionList;

  while (iFunctionCounter != 0)
  {
    ptTransitionWalker = ptTransitionWalker->ptNext;
    if ((ptTransitionWalker->ptThis->ptActionList->ptThis->iActionType ==
         FUNCTION_ACTION) ||
        (ptTransitionWalker->ptThis->ptActionList->ptThis->iActionType ==
         PROT_WRITEDATA_ACTION))
    {
      iFunctionCounter--;
    }
  }

  /* When a transition contains two functions and two events, and function  */
  /* number 2 is chosen, ptActionWalker can point to the following transi-  */
  /* tion:                                                                  */
  /* s00 : Function                                                         */
  /* s00 : Event     <-- ptActionWalker                                     */
  /* s00 : Event                                                            */
  /* s00 : Function                                                         */
  /* Therefore, ptActionWalker has to search now for the next function in   */
  /* the list of transitions.                                               */
  /**************************************************************************/
  while ((ptTransitionWalker != NULL)
         && (ptTransitionWalker->ptThis != NULL)
         && (ptTransitionWalker->ptThis->ptActionList->ptThis->iActionType !=
             FUNCTION_ACTION) &&
         (ptTransitionWalker->ptThis->ptActionList->ptThis->iActionType !=
          PROT_WRITEDATA_ACTION))
  {
    ptTransitionWalker = ptTransitionWalker->ptNext;
  }

  if ((ptTransitionWalker == NULL) || (ptTransitionWalker->ptThis == NULL))
  {
    LogLine("*** internal error. ***");
    return FALSE;
  }

  /**************************************************************************/
  /* Now execute all functions in the list of actions in the choosen        */
  /* transition.                                                            */
  /**************************************************************************/

  ptActionWalker = ptTransitionWalker->ptThis->ptActionList;

  while (ptActionWalker != NULL)
  {
    if (ptActionWalker->ptThis->iActionType == FUNCTION_ACTION)
    {
      SubstituteVariables(ptActionWalker->ptThis->ptVarList,
                          ptActionWalker->ptThis->ptFunction->ptSendParameters,
                          NULL, ptStateMachine->ptLocalVars);

      switch (ptTransitionWalker->ptThis->iAlternative)
      {
        case 0:
          SendFunction(iIOChannel,
                       ptActionWalker->ptThis->ptFunction,
                       ptActionWalker->ptThis->ptVarList);
          break;
        case 1:
          SendFunction(ptStateMachine->iAlternative,
                       ptActionWalker->ptThis->ptFunction,
                       ptActionWalker->ptThis->ptVarList);
          break;
        default:
          break;
      }
    }
    else if (ptActionWalker->ptThis->iActionType == PROT_WRITEDATA_ACTION)
    {
      Intern_PROT_W(iIOChannel, ptActionWalker->ptThis->ptVarList);
    }
    else
    {
      LogLine("*** internal error. ***");
      return FALSE;
    }
    ptActionWalker = ptActionWalker->ptNext;
  }

  /* Executed. Now set statemachine to the next state. Mission completed.   */
  /**************************************************************************/

  TransferState(ptStateMachine, ptTransitionWalker->ptThis);

  return TRUE;
}

int16 ExecuteStateMachines(
  struct TStateMachine * ptStateMachine,
  int16 iIOChannel)
{
  struct TActionList *ptActionWalker;
  struct TTransitionList *ptTransitionWalker;
  struct TBoardList *ptBoardList;
  struct TStateMachineList *ptStateMachineList;
  int16     iTransitionCounter;


  /* First grab a random transition.                                        */
  /* To be able to, first count the number of statemachine transitions.     */
  /**************************************************************************/
  ptTransitionWalker = ptStateMachine->ptCurrentState->ptTransitionList;
  iTransitionCounter = 0;

  while (ptTransitionWalker)
  {
    if (ptTransitionWalker->ptThis->ptActionList->ptThis->iActionType ==
        STATEMACHINE_ACTION)
    {
      iTransitionCounter++;
    }
    ptTransitionWalker = ptTransitionWalker->ptNext;
  }

  if (iTransitionCounter == 0)
  {
    /* No Statmachine transitions available in the active state.            */
    return FALSE;
  }

  /* Number of Statemachine  transitions is known now. Pick a random one,   */
  /* and execute it.                                                        */
  /**************************************************************************/

  iTransitionCounter = iRandom(iTransitionCounter);
  ptTransitionWalker = ptStateMachine->ptCurrentState->ptTransitionList;

  while (iTransitionCounter != 0)
  {
    ptTransitionWalker = ptTransitionWalker->ptNext;
    if (ptTransitionWalker->ptThis->ptActionList->ptThis->iActionType ==
        STATEMACHINE_ACTION)
    {
      iTransitionCounter--;
    }
  }

  while ((ptTransitionWalker != NULL)
         && (ptTransitionWalker->ptThis != NULL)
         && (ptTransitionWalker->ptThis->ptActionList->ptThis->iActionType !=
             STATEMACHINE_ACTION))
  {
    ptTransitionWalker = ptTransitionWalker->ptNext;
  }

  if ((ptTransitionWalker == NULL) || (ptTransitionWalker->ptThis == NULL))
  {
    LogLine("*** internal error. ***");
    return FALSE;
  }

  /* We finally decided which transition to take. Start all statemachines   */
  /* in this transition by adding them to the list of statemachines running */
  /* on the device of interest. First find the list of statemachines for    */
  /* this device.                                                           */
  /**************************************************************************/

  ptBoardList = _ptBoardList;

  while ((ptBoardList != NULL)
         && (ptBoardList->ptThis != NULL)
         && (ptBoardList->ptThis->iNumber != iIOChannel))
  {
    ptBoardList = ptBoardList->ptNext;
  }

  /* We've found the list of statemachines....                              */
  /* Go to the last entry in this list of statemachines.                    */

  ptStateMachineList = ptBoardList->ptThis->ptStateMachines;
  while (ptStateMachineList->ptNext != NULL)
  {
    ptStateMachineList = ptStateMachineList->ptNext;
  }

  /* And add (a copy of) all statemachines of this transition to this list  */
  /* of statemachines.                                                      */

  ptActionWalker = ptTransitionWalker->ptThis->ptActionList;
  while (ptActionWalker != NULL)
  {
    ptStateMachineList->ptNext = newStateMachineList();
    ptStateMachineList = ptStateMachineList->ptNext;
    ptStateMachineList->ptThis =
      Duplicate_StateMachine(ptActionWalker->ptThis->ptStateMachine);
    ptStateMachineList->ptThis->ptMaster = ptStateMachine;
    ptStateMachineList->ptThis->iPending = JUST_BORN;
    ptStateMachineList->ptThis->pcDeviceName =
      my_strdup(ptStateMachine->pcDeviceName);
    ptStateMachineList->ptThis->iIOChannel = ptStateMachine->iIOChannel;

    ptStateMachine->iPending = SLEEPING;

    InitStateMachine(ptStateMachineList->ptThis);

    Connect_Invocations(ptStateMachineList->ptThis->ptParameters,
                        ptActionWalker->ptThis->ptVarList);

    ptActionWalker = ptActionWalker->ptNext;
  }

  TransferState(ptStateMachine, ptTransitionWalker->ptThis);

  return TRUE;
}

void SubstituteVariables(
  struct TVariableList *ptVarList,
  struct TParameterList *ptParameterList,
  struct TVariableList *ptValueList,
  struct TVariableList *ptGlobalVars)
/****************************************************************************/
/* The variables in the ptVarList will be substituted with the values as    */
/* defined in the ptValueList. The values are _not_ checked whether they    */
/* are in range or not. This is expected to be done earlier (during decom-  */
/* position of a received teststring into variables). If the ptValueList    */
/* provides no values, the ptVarList variables are substituted with random  */
/* values based on what's allowed according to the ptParameterList.         */
/* The lists ptVarList and ptParameterList are expected to be equal in      */
/* length, the ptValueList is expected to be NULL, or a list of the same    */
/* length.                                                                  */
/****************************************************************************/
{
  int16     bSkipThisOne;
  int16     bVarHasAName;
  int16     bVarIsPreInitialised;
  int16     iVarSize;

  while (   (ptParameterList->ptThis->ptTypeDef == NULL)
         && (ptParameterList->aptNext[0]!=NULL)
        )
  {
    ptParameterList = ptParameterList->aptNext[0];
  }

  if (ptValueList == NULL)
  {
    while (ptVarList != NULL)
    {
/*        ptParameterList = NextParameter(ptParameterList,ptVarList->ptThis->ptValue->pcValue); */
/*    ptParameterList->ptThis->ptValue = ptVarList->ptThis->ptValue;*/

      bSkipThisOne = (ptVarList->ptThis->ptItsType == NULL);
      /* In general, only true for databuffer variables.                    */

      if (bSkipThisOne == FALSE)
      {
        /* Referencing to the pcValue can only be done, if it is not a      */
        /* casted pointer to a databuffer variable.                         */

        /* The variable has to be skipped from initalising, when it is      */
        /* already initialised.                                             */
        /* Unnamed variables which are not pre-initialised, must be reinit  */
        /* ialised all the time!                                            */

        bSkipThisOne = ((ptVarList->ptThis->ptValue != NULL)
                        && (ptVarList->ptThis->ptValue->pcValue != NULL)
                        && (ptVarList->ptThis->ptValue->pcValue[0] != '\0'));

        bVarHasAName = ((ptVarList->ptThis->pcName != NULL)
                        && (ptVarList->ptThis->pcName[0] != '\0'));

        bVarIsPreInitialised = ((ptVarList->ptThis->ptValue != NULL)
                                && (ptVarList->ptThis->ptValue->bFixed ==
                                    TRUE));

        if (TRUE == bVarIsPreInitialised)
        {
          bSkipThisOne = TRUE;
        }
        
        if ((bVarHasAName == FALSE) && (bVarIsPreInitialised == FALSE))
        {
          bSkipThisOne = FALSE;
        }
      }

      if (ptVarList->ptThis->ptValue == NULL)
      {
        BSEG_LogLine
          ("Internal error 4 file stepper.c, function SubstituteVariables");
        return;
      }
      else if (ptVarList->ptThis->ptValue->pcValue == NULL)
      {
        iVarSize = ptVarList->ptThis->ptItsType->iSizeInBytes;
        ptVarList->ptThis->ptValue->pcValue = malloc(iVarSize * 2 + 3);
        /* Now we have to calculate the value for the length field */
      }

      if (!bSkipThisOne)
      {
        /* The variable is not initialised yet, and can be initialised.     */
        /********************************************************************/
        if (ptParameterList->ptThis->ptTypeDef->pcLowerLimit == NULL)
        {

          Get_Random_Unlimited(ptVarList->ptThis->ptValue->pcValue,
                               ptVarList->ptThis->ptItsType->iSizeInBytes);
        }
        else
          switch (ptParameterList->ptThis->ptTypeDef->iKind)
          {
            case ENUMKIND:
              Get_Random_Enumerated(ptVarList->ptThis->ptValue->pcValue,
                                    ptParameterList->ptThis->ptTypeDef->
                                    ptDefinition);
              break;
            case BITFIELDKIND:
              Get_Random_Bitfield(ptVarList->ptThis->ptValue->pcValue,
                                  ptParameterList->ptThis->ptTypeDef->
                                  ptDefinition);
              break;
            default:
              Get_Random_Limited(ptVarList->ptThis->ptValue->pcValue,
                                 ptParameterList->ptThis->ptTypeDef->
                                 pcLowerLimit,
                                 ptParameterList->ptThis->ptTypeDef->
                                 pcUpperLimit);
              break;
          }
      }
      else
      {
        /* normal variable substitution is not needed. However, a long time */
        /* ago we've added "smart strings". At this point, we need to evalu */
        /* ate for smart strings.                                           */
        if ((ptVarList->ptThis->pcName != NULL)
            && (ptVarList->ptThis->pcName[0] == '"'))
        {
          /* A string. Maybe a smart string..                                 */
          /* EvaluateForSmartString(ptVarList->ptThis, ptGlobalVars);         */
          /* Smart strings are disabled now.                                  */

        }
      }
      if (ptParameterList != NULL)

        /* switch the two lists to the next record.                           */
            /**********************************************************************/
        if (ptParameterList != NULL)
        {
          ptParameterList = NextParameter(ptParameterList,
                                          ptVarList->ptThis->ptValue->pcValue);
        }
      ptVarList = ptVarList->ptNext;
    }
  }
  else
  {
    while (ptVarList != NULL)
    {
      ptParameterList =
        NextParameter(ptParameterList, ptVarList->ptThis->ptValue->pcValue);
      /*ptParameterList->ptThis->ptValue = ptVarList->ptThis->ptValue; */

      /* First check if memory has been allocated already.                 */
      /* If so, do a strdup. Else do a strcpy.                             */
             /*********************************************************************/
      if (ptVarList->ptThis->ptValue->pcValue == NULL)
      {
        ptVarList->ptThis->ptValue->pcValue =
          my_strdup(ptValueList->ptThis->ptValue->pcValue);
      }
      else
      {
        strcpy(ptVarList->ptThis->ptValue->pcValue,
               ptValueList->ptThis->ptValue->pcValue);
      }

      /* switch the two lists to the next record.                          */
             /*********************************************************************/
      if (ptParameterList != NULL)
      {
        ptParameterList = NextParameter(ptParameterList,
                                        ptVarList->ptThis->ptValue->pcValue);
      }
      ptVarList = ptVarList->ptNext;
    }
  }
}

int16 EvaluateForSmartString(
  struct TVariable *ptVar,
  struct TVariableList *ptVarList)
/***************************************************************************/
/* The ptVar is a variable containing a string. This must have been        */
/* checked before invoking this function. If the string refers to          */
/* variables via the {var} construction; the value of the var is substi-   */
/* tuted in place of the {var} section.                                    */
/***************************************************************************/
{
  int16     iNibble;

  char     *pcDest;
  char     *pcOrg;
  char     *pcVarOrg;

  char     *pcVarName;
  char     *pcError;

  struct TVariable *ptReferredVar;

  if (ptVar->ptValue->pcValue != NULL)
  {
    free(ptVar->ptValue->pcValue);
  }

  ptVar->ptValue->pcValue = (char *) malloc(1024);
  /* A better way is to calculate the needed amount of bytes, but that is  */
  /* a rather complex job. We rely that 1k will be enough.                 */

  pcDest = ptVar->ptValue->pcValue;
  pcOrg = ptVar->pcName;

  *(pcDest++) = '0';
  *(pcDest++) = 'x';

  pcOrg++;                      /* skip the leading " character.                                 */

  while (*pcOrg != '"')         /* String is terminated with a '"' and a '\0'.      */
  {
    if (*pcOrg == '{')
    {
      /* The situation here: "function({var})"                             */
      /*                               ^                                   */
      /*                               ^ pcOrg                             */

      pcVarName = pcOrg + 1;

      while ((*pcOrg != 0) && (*pcOrg != '}'))
        pcOrg++;
      if (*pcOrg == 0)
      {
        pcError = (char *) malloc(250);
        sprintf(pcError, "Error in smartstring %s", ptVar->pcName);
        LogLine(pcError);
        free(pcError);
        *(pcDest++) = '\0';
        return FALSE;
      }

      *pcOrg = 0;               /* Only temporary...                                    */

      /* The situation now: "function({var"                               */
      /*                               ^  ^                               */
      /*                               ^  ^pcOrg                          */
      /*                               ^pcVarName                         */

      ptReferredVar = FindVar(pcVarName, ptVarList);

      if (ptReferredVar == NULL)
      {
        pcError = (char *) malloc(250);
        sprintf(pcError, "Error in smartstring: %s", ptVar->pcName);
        LogLine(pcError);
        free(pcError);

        pcError = (char *) malloc(250);
        sprintf(pcError, "Variable %s not found.", pcVarName);
        LogLine(pcError);
        free(pcError);
        *(pcDest++) = '\0';
        return FALSE;
      }

      *pcOrg = '}';             /* Restore the original string.                        */
      pcOrg++;                  /* And go to the next character.                       */

      /* The intended variable is found, and the ptOrg pointer has the    */
      /* variable reference passed completely, including the brackets.    */
      /* Now we only have to copy the value of the variable in the des-   */
      /* tination string and we can continue the normal work.             */

      if ((ptReferredVar->ptValue != NULL)
          && (ptReferredVar->ptValue->pcValue != NULL)
          && (ptReferredVar->ptValue->pcValue[0] == 0))
      {
        pcError = (char *) malloc(250);
        sprintf(pcError, "Error in smartstring: %s", ptVar->pcName);
        LogLine(pcError);

        sprintf(pcError, "Variable %s not initialised.", ptReferredVar->pcName);
        LogLine(pcError);
        free(pcError);

      }
      else
      {
        pcVarOrg = ptReferredVar->ptValue->pcValue;
        while (*pcVarOrg)
        {
          iNibble = ((*pcVarOrg) & 0xF0);
          *(pcDest++) = (char) TO_ASCII(iNibble >> 4);
          iNibble = ((*pcVarOrg) & 0x0F);
          *(pcDest++) = (char) TO_ASCII(iNibble);
          pcVarOrg++;
        }
      }
    }
    else
    {
      if ((ptVar->ptItsType->iKind == UNICODEKIND)
          || (ptVar->ptItsType->iKind == UNICODE0KIND))
      {
        *(pcDest++) = (char) '0';
        *(pcDest++) = (char) '0';
      }
      iNibble = ((*pcOrg) & 0xF0);
      *(pcDest++) = (char) TO_ASCII(iNibble >> 4);
      iNibble = ((*pcOrg) & 0x0F);
      *(pcDest++) = (char) TO_ASCII(iNibble);
      pcOrg++;
    }
  }

  if (ptVar->ptItsType->iLessAllowed == 0)
  {
    while (pcDest <
           ptVar->ptValue->pcValue + ptVar->ptItsType->iSizeInBytes * 2 + 2)
    {
      *(pcDest++) = '0';
    }
  }

  *(pcDest) = 0;

  return TRUE;
}



void Get_Random_Enumerated(
  char *pcValue,
  struct TValueDefList *ptValueDefs)
/****************************************************************************/
/* ValueDefs is expected to point to a list of value definitions which      */
/* enumerate the set of allowed values.                                     */
/****************************************************************************/
{
  struct TValueDefList *ptValueWalker;
  int       iCounter;

  /* first count the number of enumerated values.                           */
  /**************************************************************************/
  iCounter = 0;
  ptValueWalker = ptValueDefs;
  while (ptValueWalker)
  {
    if (ptValueWalker->ptThis->pcDescription != NULL)
    {
      iCounter++;
    }
    ptValueWalker = ptValueWalker->ptNext;
  }

  /* then choose a random one.                                              */
  /**************************************************************************/

  iCounter = iRandom(iCounter);

  /* go to and copy that random one.                                        */
  /**************************************************************************/

  ptValueWalker = ptValueDefs;
  while (iCounter > 0)
  {
    if (ptValueWalker->ptThis->pcDescription)
      iCounter--;
    ptValueWalker = ptValueWalker->ptNext;
  }

  /* this random value will be choosen.                                     */
  /**************************************************************************/
  strcpy(pcValue, ptValueWalker->ptThis->pcValue);
}



void Get_Random_Bitfield(
  char *pcValue,
  struct TValueDefList *ptValueDefs)
/****************************************************************************/
/* ValueDefs is expected to point to a list of value definitions which      */
/* define (per bitmask) sets of allowed values. The bitmasks are expected   */
/* to be non-overlapping and grouped. But if they are not, (which is        */
/* actually an error in the specification), this function will not crash.   */
/* The results are undefined in that case.                                  */
/****************************************************************************/
{
  int       i;
  struct TValueDefList *ptNewMaskEntry;

  /* first make a string of zeros. This string must have the same length as */
  /* all pcValue fields in the TTValueDefList. We measure the length of the */
  /* first one... Imagine this first one is "0x000001"                      */

  i = strlen(ptValueDefs->ptThis->pcValue);     /* nr of chars in "0x000001"    */
  i = i - 2;                    /* nr of chars in   "000001"    */
  i = i / 2;                    /* nr of bytes in   "000001"    */

  strcpy(pcValue, "0x");
  while (i != 0)
  {
    strcat(pcValue, "00");
    i = i - 1;
  }

  /* Now pcValue is 0x000000, based on our example.                         */

  /* NewMaskEntry always points to the first ValueDef with a new BitMask.   */
  /* example: bitmask   value                                               */
  /*        a 0011      00                                                  */
  /*        b 0011      01                                                  */
  /*        c 0011      10                                                  */
  /*        d 0011      11                                                  */
  /*        e 1100      00                                                  */
  /*        f 1100      01                                                  */
  /*        g 1100      10                                                  */
  /*        h 1100      11                                                  */
  /* First NewMaskEntry points to a. and ValueDefs counts n from a to d     */
  /* incl. Then, a random value from these lines is choosen and or-d with   */
  /* the resultstring. Next, NewMaskEntry points to e. and ValueDefs counts */
  /* n from e to h incl. Then, a random value from these lines is choosen   */
  /* and or-d with the resultstring.                                        */

  ptNewMaskEntry = ptValueDefs;
  while (ptNewMaskEntry)
  {
    i = 0;
    if (ptValueDefs->ptThis->pcMask != NULL)
    {
      /* as long as the current mask is equal to the 'first' mask;          */
      while ((ptValueDefs != NULL)
             &&
             (strcmp
              (ptValueDefs->ptThis->pcMask,
               ptNewMaskEntry->ptThis->pcMask) == 0))
      {
        /* count this mask, and go to the next one.                         */
        if (ptValueDefs->ptThis->pcDescription)
          i++;
        ptValueDefs = ptValueDefs->ptNext;
      }

      /* The number of consecutive equal masks is counted. Now pick a       */
      /* random one from these entries.                                     */
          /**********************************************************************/
      i = iRandom(i);

      /* Start from the NewMaskEntry, and count to the random choosen one.  */
      /**********************************************************************/
      ptValueDefs = ptNewMaskEntry;
      while (i > 0)
      {
        if (ptValueDefs->ptThis->pcDescription)
          i--;
        ptValueDefs = ptValueDefs->ptNext;
      }

      /* this random value will be choosen.                                 */
      /**********************************************************************/
      ar_or(pcValue, ptValueDefs->ptThis->pcValue);

      /* and continue with the following mask.                              */
      /**********************************************************************/
      while ((ptValueDefs != NULL)
             &&
             (strcmp
              (ptValueDefs->ptThis->pcMask,
               ptNewMaskEntry->ptThis->pcMask) == 0))
      {
        ptValueDefs = ptValueDefs->ptNext;
      }
      ptNewMaskEntry = ptValueDefs;
    }
    else
    {
      /* Bitfield without mask: make it 50% change to use this bitfield or  */
      /* not.                                                               */
            /**********************************************************************/
      if (iRandom(2) == 1)
      {
        ar_or(pcValue, ptNewMaskEntry->ptThis->pcValue);
      }
      ptNewMaskEntry = ptNewMaskEntry->ptNext;
    }
  }
}

void Get_Random_Limited(
  char *pcValue,
  char *pcLower,
  char *pcUpper)
/* Get a random value between *pcLower and *pcUpper. No special care is     */
/* taken for the '0x' prefix, however the 'x' is not a hexadecimal          */
/* character, it is processed as desired by this function. The result will  */
/* start with the same prefix as the prefix of the limits.                  */
/* example :                                                                */
/* UpperLimit        : 0x5555555                                            */
/* UpperLimitNibble  : 0x55.....                                            */
/* Random Nibble     : 0x6536A41                                            */
/* LowerLimitNibble  : 0x777....                                            */
/* LowerLimit        : 0x7777777                                            */
{
  int       bLowerLimitValid, bUpperLimitValid;
  int       i;
  int       iUpperLimitNibble, iLowerLimitNibble;

  bLowerLimitValid = TRUE;
  bUpperLimitValid = TRUE;
  while (*pcLower)              /* or *pUpper, both have equal length...                 */
  {
    iUpperLimitNibble = (bUpperLimitValid) ? TO_HEX(*pcUpper) : 0xF;
    iLowerLimitNibble = (bLowerLimitValid) ? TO_HEX(*pcLower) : 0x0;
    i = iUpperLimitNibble - iLowerLimitNibble;
    i = iRandom(i + 1);
    (*pcValue) = (char) (TO_ASCII(i + iLowerLimitNibble));
    if ((*pcValue) < (*pcUpper))
      bUpperLimitValid = FALSE;
    if ((*pcValue) > (*pcLower))
      bLowerLimitValid = FALSE;
    pcValue++;
    pcLower++;
    pcUpper++;
  }
  (*pcValue) = 0;
}

void Get_Random_Unlimited(
  char *pcValue,
  int iByteCount)
/* Get a random value of n bytes, and write that value as a hex string in   */
/* pcValue.                                                                 */
{
  int       iRandomNibble;

  *(pcValue++) = '0';
  *(pcValue++) = 'x';
  while (iByteCount != 0)
  {
    iRandomNibble = iRandom(16);
    *(pcValue++) = (char) (TO_ASCII(iRandomNibble));
    iRandomNibble = iRandom(16);
    *(pcValue++) = (char) (TO_ASCII(iRandomNibble));

    iByteCount--;
  }
  (*pcValue) = 0;
}

int16 Intern_End(
  struct TStateMachine *pt)
/****************************************************************************/
/* Indicate that the statemachine is ended and can be removed from the list */
/* of statemachines.                                                        */
/****************************************************************************/
{
  pt->iPending = ENDING;
  pt->ptMaster->iPending = WAKING_UP;
  return TRUE;
}

int16 Intern_Timer(
  struct TVariableList * ptVarList,
  struct TStateMachine * ptStateMachine,
  struct TTransition * ptTransition)
/****************************************************************************/
/* Add a record in the _lot administration.                                 */
/****************************************************************************/
{
  int16             lParameterValue = 0;
  float             fTimeScaling;
  char              pLogLine[80];
  struct TVariable *ptVar = NULL;

  /* First determine if there is already a timer running for this state-   */
  /* machine. If so, don't add a second one.                               */
   /*************************************************************************/

  if (ptStateMachine->ptRunningTimer != NULL)
  {
    return FALSE;
  }

  /* Calculate the amount of milliseconds the timer should last.           */
   /*************************************************************************/
  if (ptVarList != NULL)
  {
    ptVar = ptVarList->ptThis;
  }

  fTimeScaling = (float) 1.00;  /* the default..                        */

  if (ptVar != NULL)
  {
    if (ptVar->ptItsType != NULL)
    {
      if (ptVar->ptItsType->iKind == TIMEKIND)
      {
        fTimeScaling = ptVar->ptItsType->fTimeScaling;
      }
    }

    if ((ptVar->ptValue != NULL) && (ptVar->ptValue->pcValue != NULL))
    {
      lParameterValue = GetInteger(ptVar->ptValue->pcValue);
    }
  }

  if ( lParameterValue != 0)
  {
    ptStateMachine->ptRunningTimer = 
      SetTimer(lParameterValue, fTimeScaling, ptStateMachine);
    if (ptStateMachine->ptRunningTimer == NULL)
    {
      LogLine("Warning: timer could not be set.");
      return TRUE;            /* Because timer is not set. Script can continue..   */
    }
    else
    {
      ptStateMachine->ptStateInTimer = ptStateMachine->ptCurrentState;
      ptStateMachine->ptRunningTimerTransition = ptTransition;
  
      sprintf(pLogLine, "%s:%s TIMER %.3f sec",
              ptStateMachine->pcName,
              ptStateMachine->ptCurrentState->pcName, 
              (lParameterValue*fTimeScaling));
       LogLine(pLogLine);
  
      return FALSE;           /* Because timer is set. Script cannot continue..   */
    }
  }
  /* If no Timer record is added in the List Of Timers, return TRUE to     */
  /* the invoker of this function.  This indicates that the timer is not   */
  /* added, and a state-transition can be made.                            */
   /*************************************************************************/
  return TRUE;
}

int16 Intern_MTimer(
  struct TVariableList * ptVarList,
  struct TStateMachine * ptStateMachine,
  struct TTransition * ptTransition)
/****************************************************************************/
/* Add a record in the _lot administration.                                 */
/****************************************************************************/
{
  int16             lParameterValue = 0;
  float             fTimeScaling;
  char              pLogLine[80];
  struct TVariable *ptVar = NULL;

  /* First determine if there is already a timer running for this state-   */
  /* machine. If so, don't add a second one.                               */
   /*************************************************************************/

  if (ptStateMachine->ptRunningTimer != NULL)
  {
    return FALSE;
  }

  /* Calculate the amount of milliseconds the timer should last.           */
   /*************************************************************************/
  if (ptVarList != NULL)
  {
    ptVar = ptVarList->ptThis;
  }

  fTimeScaling = (float) 0.001;  /* the default..                        */

  if (ptVar != NULL)
  {
    if ((ptVar->ptValue != NULL) && (ptVar->ptValue->pcValue != NULL))
    {
      lParameterValue = GetInteger(ptVar->ptValue->pcValue);
    }
  }
  if ( lParameterValue != 0)
  {
    ptStateMachine->ptRunningTimer = 
      SetTimer(lParameterValue, fTimeScaling, ptStateMachine);
    if (ptStateMachine->ptRunningTimer == NULL)
    {
      LogLine("Warning: timer could not be set.");
      return TRUE;            /* Because timer is not set. Script can continue..   */
    }
    else
    {
      ptStateMachine->ptStateInTimer = ptStateMachine->ptCurrentState;
      ptStateMachine->ptRunningTimerTransition = ptTransition;
  
      sprintf(pLogLine, "%s:%s MTIMER %.3f msec",
              ptStateMachine->pcName,
              ptStateMachine->ptCurrentState->pcName, 
              ((lParameterValue*fTimeScaling)*1000));
      LogLine(pLogLine);
  
      return FALSE;           /* Because timer is set. Script cannot continue..   */
    }
  }
  /* If no Timer record is added in the List Of Timers, return TRUE to     */
  /* the invoker of this function.  This indicates that the timer is not   */
  /* added, and a state-transition can be made.                            */
   /*************************************************************************/
  return TRUE;
}

int16 Intern_RTimer(
  struct TVariableList * ptVarList,
  struct TStateMachine * ptStateMachine,
  struct TTransition * ptTransition)
/****************************************************************************/
/* Add a record in the _lot administration.                                 */
/****************************************************************************/
{
  int16             lParameterValue  = 0;
  int16             lParameterValue1 = 0;
  int16             lParameterValue2 = 0;
  float             fTimeScaling;
  char              pLogLine[80];
  struct TVariable *ptVar = NULL;

  /* First determine if there is already a timer running for this state-   */
  /* machine. If so, don't add a second one.                               */
   /*************************************************************************/

  if (ptStateMachine->ptRunningTimer != NULL)
  {
    return FALSE;
  }

  fTimeScaling = (float) 1.0;  /* the default..                        */


  /* Calculate the amount of milliseconds the timer should last.           */
   /*************************************************************************/
  if (ptVarList != NULL)
  {
    ptVar = ptVarList->ptThis;
    if (ptVar != NULL)
    {
      if ((ptVar->ptValue != NULL) && (ptVar->ptValue->pcValue != NULL))
      {
        lParameterValue1 = GetInteger(ptVar->ptValue->pcValue);
      }
    }
  }
  if (NULL != ptVarList->ptNext) 
  {
    ptVar = ptVarList->ptNext->ptThis;
    if (ptVar != NULL)
    {
      if ((ptVar->ptValue != NULL) && (ptVar->ptValue->pcValue != NULL))
      {
        lParameterValue2 = GetInteger(ptVar->ptValue->pcValue);
      }
    }
  }
  if ( (lParameterValue1 != 0) && (lParameterValue2 != 0))
  {
    if (lParameterValue1 >= lParameterValue2)
    {
      LogLine("Warning: timer could not be set, first param should be less than param 2.");
      return TRUE;            /* Because timer is not set. Script can continue..   */
    }
    else
    {
      lParameterValue = 
      (int32) (((float)((lParameterValue2-lParameterValue1+1)*(rand()/(RAND_MAX+1.0))))+lParameterValue1);
      ptStateMachine->ptRunningTimer = 
        SetTimer(lParameterValue, fTimeScaling, ptStateMachine);
      if (ptStateMachine->ptRunningTimer == NULL)
      {
        LogLine("Warning: timer could not be set.");
        return TRUE;            /* Because timer is not set. Script can continue..   */
      }
      else
      {
        ptStateMachine->ptStateInTimer = ptStateMachine->ptCurrentState;
        ptStateMachine->ptRunningTimerTransition = ptTransition;
    
        sprintf(pLogLine, "%s:%s RTIMER %.3f sec",
                ptStateMachine->pcName,
                ptStateMachine->ptCurrentState->pcName, 
                lParameterValue*fTimeScaling);
        LogLine(pLogLine);
    
        return FALSE;           /* Because timer is set. Script cannot continue..   */
      }
    }
  }
  /* If no Timer record is added in the List Of Timers, return TRUE to     */
  /* the invoker of this function.  This indicates that the timer is not   */
  /* added, and a state-transition can be made.                            */
   /*************************************************************************/
  return TRUE;
}

int16 Intern_RMTimer(
  struct TVariableList * ptVarList,
  struct TStateMachine * ptStateMachine,
  struct TTransition * ptTransition)
/****************************************************************************/
/* Add a record in the _lot administration.                                 */
/****************************************************************************/
{
  int16             lParameterValue  = 0;
  int16             lParameterValue1 = 0;
  int16             lParameterValue2 = 0;
  float             fTimeScaling;
  char              pLogLine[80];
  struct TVariable *ptVar = NULL;

  /* First determine if there is already a timer running for this state-   */
  /* machine. If so, don't add a second one.                               */
   /*************************************************************************/

  if (ptStateMachine->ptRunningTimer != NULL)
  {
    return FALSE;
  }

  fTimeScaling = (float) 0.001;  /* the default..                        */


  /* Calculate the amount of milliseconds the timer should last.           */
   /*************************************************************************/
  if (ptVarList != NULL)
  {
    ptVar = ptVarList->ptThis;
    if (ptVar != NULL)
    {
      if ((ptVar->ptValue != NULL) && (ptVar->ptValue->pcValue != NULL))
      {
        lParameterValue1 = GetInteger(ptVar->ptValue->pcValue);
      }
    }
  }
  if (NULL != ptVarList->ptNext) 
  {
    ptVar = ptVarList->ptNext->ptThis;
    if (ptVar != NULL)
    {
      if ((ptVar->ptValue != NULL) && (ptVar->ptValue->pcValue != NULL))
      {
        lParameterValue2 = GetInteger(ptVar->ptValue->pcValue);
      }
    }
  }
  if ( (lParameterValue1 != 0) && (lParameterValue2 != 0))
  {
    if (lParameterValue1 >= lParameterValue2)
    {
      LogLine("Warning: timer could not be set, first param should be less than param 2.");
      return TRUE;            /* Because timer is not set. Script can continue..   */
    }
    else
    {
      lParameterValue = 
      (int32) (((float)((lParameterValue2-lParameterValue1+1)*(rand()/(RAND_MAX+1.0))))+lParameterValue1);
      ptStateMachine->ptRunningTimer = 
        SetTimer(lParameterValue, fTimeScaling, ptStateMachine);
      if (ptStateMachine->ptRunningTimer == NULL)
      {
        LogLine("Warning: timer could not be set.");
        return TRUE;            /* Because timer is not set. Script can continue..   */
      }
      else
      {
        ptStateMachine->ptStateInTimer = ptStateMachine->ptCurrentState;
        ptStateMachine->ptRunningTimerTransition = ptTransition;
    
        sprintf(pLogLine, "%s:%s RMTIMER %.3f msec",
                ptStateMachine->pcName,
                ptStateMachine->ptCurrentState->pcName, 
                (lParameterValue*fTimeScaling*1000));
        LogLine(pLogLine);
    
        return FALSE;           /* Because timer is set. Script cannot continue..   */
      }
    }
  }
  /* If no Timer record is added in the List Of Timers, return TRUE to     */
  /* the invoker of this function.  This indicates that the timer is not   */
  /* added, and a state-transition can be made.                            */
   /*************************************************************************/
  return TRUE;
}

struct TTimer *SetTimer(int32 lTimerValue,
                        float fTimerScaling,
                        struct TStateMachine *ptStateMachine)
{
  float            fTimeSpan;          
  struct timeval   tCurrentTime;
  struct timeval   tExpirationMoment;  
  struct TTimer   *ptTimer;
  
  ptTimer = NULL;
  
  if (lTimerValue != 0)
  {
    fTimeSpan = lTimerValue * fTimerScaling;

    /* The next statement is added because lTimeSpan showed to be 1 msec   */
    /* too less. The reason of this is unknown and not interesting. How-   */
    /* ever, because the time-span is logged in the logging-window and to  */
    /* avoid discussions about why the lTimeSpan is 1 msec too less, I     */
    /* just increment the lTimeSpan with 1. Just because it looks nicer in */
    /* the logging :-)                                                     */
     /***********************************************************************/
    /* lTimeSpan++; */

    /* The amount of milliseconds is now known.                            */
     /***********************************************************************/
    if (fTimeSpan != 0)
    {
      gettimeofday(&tCurrentTime, NULL);
      tExpirationMoment.tv_sec =  tCurrentTime.tv_sec + (long) fTimeSpan;
      fTimeSpan -= (long) fTimeSpan;
      fTimeSpan *= 1000000;
      tExpirationMoment.tv_usec = tCurrentTime.tv_usec + (long) fTimeSpan;
      if (tExpirationMoment.tv_usec > 1000000)
      {
        tExpirationMoment.tv_sec++;
        tExpirationMoment.tv_usec -= 1000000;
      }
      ptTimer = AddTimer(tExpirationMoment, ptStateMachine);
    }
  }
  return(ptTimer);
}

int Intern_Clear(
  struct TVariableList *ptVarList)
/****************************************************************************/
/* Clears all variables in ptVarList.                                       */
/****************************************************************************/
{
  struct TData *ptDataRecord;

  while (ptVarList != NULL)
  {
    if (ptVarList->ptThis != NULL)
    {
      if (ptVarList->ptThis->ptItsType == NULL)
      {
        /* Don't know what this is. Do nothing...                           */
        /*------------------------------------------------------------------*/
      }
      else if (ptVarList->ptThis->ptItsType == _ptDataType)
      {
        /* A Datatype variable. Clear it, if possible.                      */
        /*------------------------------------------------------------------*/
        ptDataRecord = (struct TData *) ptVarList->ptThis;
        if (ptDataRecord->pcValue != NULL)
        {
          (*ptDataRecord->pcValue) = '\0';
        }
      }
      else
      {
        /* An ordinary variable. Just clear it, if possible.                */
        /*------------------------------------------------------------------*/
        if (ptVarList->ptThis->ptValue != NULL)
        {
          if (ptVarList->ptThis->ptValue->pcValue)
          {
            if (ptVarList->ptThis->ptValue->pcValue == _SIGNAL)
            {
              (ptVarList->ptThis->ptValue->pcValue) = _NO_SIGNAL;
            }
            else
            {
              (*ptVarList->ptThis->ptValue->pcValue) = '\0';
            }
          }
        }
      }
    }

    ptVarList = ptVarList->ptNext;
  }
  return TRUE;
}


int16 Intern_R_Sig(
  struct TVariableList * ptVarList)
/****************************************************************************/
/* returns TRUE if a signal on the indicated variable was received. In that */
/* case, the signal is also removed.                                        */
/****************************************************************************/
{
  struct TVariable *ptVar = NULL;

  if (ptVarList != NULL)
  {
    ptVar = ptVarList->ptThis;
  }

  if (ptVar != NULL)
  {
    if (ptVar->ptValue != NULL)
    {
      if (ptVar->ptValue->pcValue == _SIGNAL)
      {
        /* Yes! signal received!  Remove it, and return TRUE                */
        ptVar->ptValue->pcValue = _NO_SIGNAL;
        return TRUE;
      }
    }
  }
  return FALSE;
}

int16 Intern_S_Sig(
  struct TVariableList * ptVarList)
/****************************************************************************/
/* Sets a signal on the indicated var.                                      */
/****************************************************************************/
{
  struct TVariable *ptVar = NULL;

  if (ptVarList != NULL)
  {
    ptVar = ptVarList->ptThis;
  }

  if (ptVar != NULL)
  {
    if (ptVar->ptValue != NULL)
    {
      if (ptVar->ptValue->pcValue == _SIGNAL)
      {
        return FALSE;
      }
      ptVar->ptValue->pcValue = _SIGNAL;

    }
  }
  return TRUE;
}

int16 Intern_Wait(
  struct TVariableList * ptVarList)
/****************************************************************************/
/* returns TRUE if a value in the indicated variable was detected. The      */
/* value remains intact.                                                    */
/****************************************************************************/
{
  struct TVariable *ptVar = NULL;

  if (ptVarList != NULL)
  {
    ptVar = ptVarList->ptThis;
  }

  if (ptVar != NULL)
  {
    if (ptVar->ptValue != NULL)
    {
      if (ptVar->ptValue->pcValue != NULL)
      {
        if (ptVar->ptValue->pcValue[0] != '\0')
        {
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

int16 Intern_Print(
  struct TVariableList * ptVarList)
/****************************************************************************/
/* logs the value of all parameters.                                        */
/****************************************************************************/
{
  struct TVariable *ptVar = NULL;
  char *LastChar;

  if (ptVarList != NULL)
  {
    ptVar = ptVarList->ptThis;
  }

  if (ptVar != NULL)
  {
    /* Don't print the first and the last character, because these are two "  */
    /* characters.                                                            */

    LastChar = ptVar->pcName;
    while (*LastChar++);
    LastChar--;
    LastChar--;
    *LastChar=0;

    LogLine("-------------------------------");
    LogLine(ptVar->pcName+1);
    LogLine("-------------------------------");
    *LastChar='"';
  }
  return TRUE;
}

int16 Intern_Comp(
  struct TVariableList * ptVarList)
/****************************************************************************/
/* Compares two data buffers. The names of the data buffers are provided    */
/* as var-names in the ptVarList. The function returns TRUE if              */
/* - The contents of both buffers is equal                                  */
/* - The name of the first buffer is uninitialised                          */
/* - The name of the second buffer is uninitialised                         */
/* In all other cases, the function returns FALSE.                          */
/* If the buffer name starts with an '@', the buffername is treated as a    */
/* filename, and the contents of the file is used for comparing.            */
/****************************************************************************/
{
  char     *pcBuffer1, *pcBuffer2;

  /* First check if we really have to compare two buffers.                  */
  /**************************************************************************/

  if (ptVarList == NULL)
    return TRUE;
  if (ptVarList->ptNext == NULL)
    return TRUE;

  /* we _have_ two parameters...  That's something.                         */
  /**************************************************************************/

  if (ptVarList->ptThis->pcName == NULL)
    return TRUE;
  if (ptVarList->ptNext->ptThis->pcName == NULL)
    return TRUE;

  /* we have two named parameters. Now we have to do some work.             */
  /**************************************************************************/

  pcBuffer1 = FindBufferInGlobalDataList(ptVarList->ptThis->pcName);
  pcBuffer2 = FindBufferInGlobalDataList(ptVarList->ptNext->ptThis->pcName);

  /* These functioncalls return pointers which we can not free. The memory  */
  /* is managed by the global data list, and is freed when the script file  */
  /* is unloaded.                                                           */
  /* The filename indirection is solved during loading the script.          */
  /**************************************************************************/

  return (strcmp(pcBuffer1, pcBuffer2) == 0);
}


int16 Intern_PROT(
  int16 iBoardNumber)
/****************************************************************************/
/* Sets the protocol to PROT protocol on the indicated board.                */
/****************************************************************************/
{
  SetProtocol(iBoardNumber, PROT);
  return TRUE;
}

int16 Intern_TCI(
  int16 iBoardNumber)
/****************************************************************************/
/* Sets the protocol to TCI protocol on the indicated board.                */
/****************************************************************************/
{
  SetProtocol(iBoardNumber, TCI);
  return TRUE;
}

int16 Intern_ASCII(
  int16 iBoardNumber)
/****************************************************************************/
/* Sets the protocol to ASCII protocol on the indicated board.              */
/****************************************************************************/
{
  SetProtocol(iBoardNumber, ASCII);
  return TRUE;
}


int16 Intern_PROT_W(
  int iBoardNumber,
  struct TVariableList * ptVarList)
/****************************************************************************/
/* Writes PROT data to the indicated board                                   */
/****************************************************************************/
{
  char     *pcBuffer;
  struct TVariable *ptVar = NULL;
  struct TData *ptData;

  uint16    uiHandle;
  uint16    uiPBFlag;
  uint16    uiBCFlag;

  uiHandle = 0;
  uiPBFlag = 0;
  uiBCFlag = 0;
  pcBuffer = NULL;

  if (ptVarList != NULL)
  {
    ptVar = ptVarList->ptThis;
    uiHandle = GetInteger(ptVar->ptValue->pcValue);
    ptVarList = ptVarList->ptNext;
  }

  if (ptVarList != NULL)
  {
    ptVar = ptVarList->ptThis;
    uiPBFlag = GetInteger(ptVar->ptValue->pcValue);
    ptVarList = ptVarList->ptNext;
  }

  if (ptVarList != NULL)
  {
    ptVar = ptVarList->ptThis;
    uiBCFlag = GetInteger(ptVar->ptValue->pcValue);
    ptVarList = ptVarList->ptNext;
  }

  if (ptVarList != NULL)
  {
    ptVar = ptVarList->ptThis;
    ptData = (struct TData *) ptVar;
    pcBuffer = ptData->pcValue;
  }

  Send_PROT_ACL_Data(iBoardNumber, uiHandle, uiPBFlag, uiBCFlag, pcBuffer,
                    strlen(pcBuffer));
  return TRUE;
}

int16 Intern_Term(
  char *pcReason)
/****************************************************************************/
/* Terminates the script immedeately.                                       */
/****************************************************************************/
{
  StopExecutor(pcReason);
  return TRUE;
}

int16 Intern_Rescue(
  )
/****************************************************************************/
/* Switches _all_ the statemachines that have a rescue state, to the rescue */
/* state.                                                                   */
/****************************************************************************/
{
  struct TBoardList *ptBoardList;
  struct TStateMachineList *ptStateMachineList;
  struct TStateMachine *ptM;
  char      pcLogLine[250];

  ptBoardList = _ptBoardList;
  while (ptBoardList != NULL)
  {
    ptStateMachineList = ptBoardList->ptThis->ptStateMachines;
    while (ptStateMachineList != NULL)
    {
      ptM = (ptStateMachineList->ptThis);
      if (ptM->ptRescueState != NULL)
      {
        /*******************************************************************/
        /* This is no formal transition, so we don't use TransferState     */
        /*******************************************************************/
        sprintf(pcLogLine, "%s:%s rescue-ing to state %s",
                ptBoardList->ptThis->pcName,
                ptM->pcName, ptM->ptRescueState->pcName);

        LogLine(pcLogLine);

        ptM->ptCurrentState = ptM->ptRescueState;
        if (ptM->ptRunningTimer != NULL)
        {
          DeleteTimer(&ptM->ptRunningTimer);
        }
      }
      ptStateMachineList = ptStateMachineList->ptNext;
    }
    ptBoardList = ptBoardList->ptNext;
  }

  return TRUE;
}


int16 Intern_CtsHigh_Action(
  int16 iIOChannel)
{
/*  return (BSED_GetCts(iIOChannel) == LINE_HIGH); */
  return FALSE;                 /* to stop scripts that use obsolete functionality */
}

int16 Intern_CtsLow_Action(
  int16 iIOChannel)
{
/*  return (BSED_GetCts(iIOChannel) == LINE_LOW); */
  return FALSE;                 /* to stop scripts that use obsolete functionality */
}

int16 Intern_DsrHigh_Action(
  int16 iIOChannel)
{
/*  return (BSED_GetDsr(iIOChannel) == LINE_HIGH); */
  return FALSE;                 /* to stop scripts that use obsolete functionality */
}

int16 Intern_DsrLow_Action(
  int16 iIOChannel)
{
/*  return (BSED_GetDsr(iIOChannel) == LINE_LOW); */
  return FALSE;                 /* to stop scripts that use obsolete functionality */
}

int16 Intern_DtrHigh_Action(
  int16 iIOChannel)
{
/*  BSED_SetDtr(iIOChannel, LINE_HIGH);
  return TRUE; */
  return FALSE;                 /* to stop scripts that use obsolete functionality */
}

int16 Intern_DtrLow_Action(
  int16 iIOChannel)
{
/*  BSED_SetDtr(iIOChannel, LINE_LOW);
  return TRUE; */
  return FALSE;                 /* to stop scripts that use obsolete functionality */
}

int16 Intern_SetCommBreak(
  int16 iIOChannel)
{
/*  BSED_SetCommBreak(iIOChannel);
  return TRUE; */
  return FALSE;                 /* to stop scripts that use obsolete functionality */
}

int16 Intern_ClearCommBreak(
  int16 iIOChannel)
{
/*  BSED_ClearCommBreak(iIOChannel);
  return TRUE; */
  return FALSE;                 /* to stop scripts that use obsolete functionality */
}

int16 Intern_SOCKET_GENERAL_ERROR(
  int16 iIOChannel)
{
  uint32    ulMask;

  ulMask = 1 << iIOChannel;
  if ((ul_SOCKET_GENERAL_ERROR & ulMask) == ulMask)
  {
    ul_SOCKET_GENERAL_ERROR &= ~(ulMask);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

int16 Intern_SOCKET_SEND_ERROR(
  int16 iIOChannel)
{
  uint32    ulMask;

  ulMask = 1 << iIOChannel;
  if ((ul_SOCKET_SEND_ERROR & ulMask) == ulMask)
  {
    ul_SOCKET_SEND_ERROR &= ~(ulMask);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

int16 Intern_SOCKET_RECEIVE_ERROR(
  int16 iIOChannel)
{
  uint32    ulMask;

  ulMask = 1 << iIOChannel;
  if ((ul_SOCKET_RECEIVE_ERROR & ulMask) == ulMask)
  {
    ul_SOCKET_RECEIVE_ERROR &= ~(ulMask);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

int16 Intern_SOCKET_CONNECT_ERROR(
  int16 iIOChannel)
{
  uint32    ulMask;
  uint32    ulx;

  ulMask = 1 << iIOChannel;
  if ((ul_SOCKET_CONNECT_ERROR & ulMask) == ulMask)
  {
    ulMask = ~ulMask;
    ulx = ulMask & ul_SOCKET_CONNECT_ERROR;
    ul_SOCKET_CONNECT_ERROR = ulx;
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

int16 Intern_SOCKET_DISCONNECT_ERROR(
  int16 iIOChannel)
{
  uint32    ulMask;

  ulMask = 1 << iIOChannel;
  if ((ul_SOCKET_DISCONNECT_ERROR & ulMask) == ulMask)
  {
    ul_SOCKET_DISCONNECT_ERROR &= ~(ulMask);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

int16 Intern_SOCKET_ACCEPT_ERROR(
  int16 iIOChannel)
{
  uint32    ulMask;

  ulMask = 1 << iIOChannel;
  if ((ul_SOCKET_ACCEPT_ERROR & ulMask) == ulMask)
  {
    ul_SOCKET_ACCEPT_ERROR &= ~(ulMask);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

int16 Intern_SOCKET_UNKNOWN_ERROR(
  int16 iIOChannel)
{
  uint32    ulMask;

  ulMask = 1 << iIOChannel;
  if ((ul_SOCKET_UNKNOWN_ERROR & ulMask) == ulMask)
  {
    ul_SOCKET_UNKNOWN_ERROR &= ~(ulMask);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

int16 Intern_CONNECTION_LOST(
  int16 iIOChannel)
{
  uint32    ulMask;

  ulMask = 1 << iIOChannel;
  if ((ul_CONNECTION_LOST & ulMask) == ulMask)
  {
    ul_CONNECTION_LOST &= ~(ulMask);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

int16 Intern_CONNECTION_ESTABLISHED(
  int16 iIOChannel)
{
  uint32    ulMask;

  ulMask = 1 << iIOChannel;
  if ((ul_CONNECTION_ESTABLISHED & ulMask) == ulMask)
  {
    ul_CONNECTION_ESTABLISHED &= ~(ulMask);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

void SetProtocol(
  int16 iBoardNumber,
  int16 iProtocolType)
/****************************************************************************/
/* Switches the protocol type on the indicated board.                       */
/****************************************************************************/
{
  switch (iProtocolType)
  {
    case PROT:
      BSED_SetProtocol(iBoardNumber, PROT, 0, NULL, 256);
      break;
  }
}

char     *FindBufferInGlobalDataList(
  char *pcBufferName)
{
  struct TDataList *ptDataWalker;

  ptDataWalker = _ptDataList;

  while (ptDataWalker != NULL)
  {
    if (ptDataWalker->ptThis != NULL)
    {
      if (strcmp(ptDataWalker->ptThis->pcName, pcBufferName) == 0)
      {
        return (ptDataWalker->ptThis->pcValue);
      }
    }
    ptDataWalker = ptDataWalker->ptNext;
  }
  return NULL;
}

int ConvertHexValue(
  char         *pcBuffer,
  char         *pcValue,
  int16         iCodeLength,
  struct TType *ptType
)
{
  int16 iIndex;
  int16 iEnd;
  int16 iBufferIndex;
  
  iIndex        = 0;           /* skip the 0x prefix.                                  */
  iEnd          = 0;
  iBufferIndex  = 0;

  if (iCodeLength >= 2)
  {
    iEnd = iCodeLength;
    while ((iIndex/2) < (iEnd/2))
    {
      if (ptType->iEndianess == TRUE)
      { 
        if( (iEnd % 2) == 0)
        {
          pcBuffer[((iEnd/2)-1)-iBufferIndex] = 
            (int8) TO_BYTE(pcValue[(iEnd-1)-iIndex-1], pcValue[(iEnd-1)-iIndex]);
        }
        else
        {
          pcBuffer[(iEnd/2)-iBufferIndex] = 
            (int8) TO_BYTE(pcValue[(iEnd-1)-iIndex-1], pcValue[(iEnd-1)-iIndex]);
        }
      }
      else
      {
        pcBuffer[iBufferIndex] = 
         (int8) TO_BYTE(pcValue[(iEnd-1)-iIndex-1], pcValue[(iEnd-1)-iIndex]);
      }
  
      iBufferIndex++;
      iIndex+=2;
    }
    if ((iCodeLength%2) != 0)
    {
       pcBuffer[iBufferIndex] = 
        (int8) TO_BYTE('0', pcValue[0]);
      iBufferIndex++;
    }
  }
  else
  {
    pcBuffer[iBufferIndex] = 
      (int8) TO_BYTE('0', pcValue[iIndex]);
    iBufferIndex++;
    
  }
  return(iBufferIndex);
}

int ConvertDecimalValue(
  char         *pcBuffer,
  char         *pcValue,
  int16         iCodeLength,
  struct TType *ptType
)
{
  char          cByte;
  int16         iIndex;
  int16         iEnd;
  int16         iBufferIndex;
  long          lNumber;
  unsigned long ulNumber;
  char          caNumberString[20];
  
  iIndex        = 0;
  iEnd          = iCodeLength;
  iBufferIndex  = 0;
  
  if ( pcValue[0] == '-')
  {
    sscanf( pcValue,"%ld", &lNumber);
    while (lNumber !=0 )
    { 
      cByte = lNumber & 0xF;
      lNumber >>= 4;
      cByte |= ((lNumber & 0xF) << 4);
      lNumber >>= 4;
      caNumberString[iBufferIndex] = cByte;
      iBufferIndex++;
    }
  }
  else
  {
    sscanf( pcValue,"%lu", &ulNumber);
    while (ulNumber !=0 )
    { 
      cByte = ulNumber & 0xF;
      ulNumber >>= 4;
      cByte |= ((ulNumber & 0xF) << 4);
      ulNumber >>= 4;
      caNumberString[iBufferIndex] = cByte;
      iBufferIndex++;
    }
  }
  for (iIndex = 0; iIndex < iBufferIndex; iIndex++)
  {
    if (ptType->iEndianess == TRUE)
    {
      pcBuffer[(ptType->iSizeInBytes-1) - iIndex] = caNumberString[iIndex];
    }
    else
    {
      pcBuffer[iIndex] = caNumberString[iIndex];
    }
  }
  return(iBufferIndex);
}

/* Converts a byte array according to the Endianess defined */
int ConvertValue(
  char         *pcBuffer,
  char         *pcValue,
  int16         iCodeLength,
  struct TType *ptType
)
{
  int16 iIndex;
  int16 iEnd;
  int16 iBufferIndex;
  
  iIndex        = 0;
  iEnd          = 0;
  iBufferIndex  = 0;

  if (iCodeLength != 0)
  {
    if (iCodeLength > 1)
    {
      if (pcValue[1]=='x') 
      {
        iBufferIndex = ConvertHexValue(pcBuffer,pcValue+2,iCodeLength,ptType);
      }
      else
      {
        iBufferIndex = ConvertDecimalValue(pcBuffer,pcValue,iCodeLength,ptType);
      }
    }
    else
    {
      /* Only passed 1 charachter */
      if (pcValue[1]=='x')
      {
        pcBuffer[iBufferIndex] = 
          (int8) TO_BYTE('0', pcValue[iIndex+2]);
        iBufferIndex++;
      }
      else
      {
        pcBuffer[iBufferIndex] = 
          (int8) TO_BYTE('0', pcValue[iIndex]);
        iBufferIndex++;
      }
    }
    if (ptType->iKind != ARRAYKIND)
    {
      iEnd = ptType->iSizeInBytes;
      /* Fill the remainder with zero's */
      if ((iBufferIndex < iEnd) && (ptType->iLessAllowed == 0))
      {
        while (iBufferIndex < iEnd)
        {
          if (ptType->iEndianess == TRUE)
          { 
             pcBuffer[(iEnd-1)-iBufferIndex] = 0;
          }
          else
          {
             pcBuffer[iBufferIndex] = 0;
          }
          iBufferIndex++;
        }
      }
    }
  }
  return(iBufferIndex);
}

void ConvertArray(
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

  if (pcValue[1]=='x') 
  {
    iIndexValue = 2;           /* skip the 0x prefix.                                  */
  }
  else
  {
    iIndexValue = 0;
  }
  iIndexBuffer = 0;
  
  for (; (iIndexValue/2) < (iCodeLength/2); iIndexValue += (ptType->uiSizeOfElement*2))
  {
    for (j=0; j < ptType->uiSizeOfElement; j++)
    {
      /* Little Endian... */
      if (ptType->iEndianess == FALSE)
      {
        i = (iIndexValue+((ptType->uiSizeOfElement*2)-(j*2)))-1;
        pcBuffer[iIndexBuffer] = (uint8) TO_BYTE(pcValue[i-1], 
                                                 pcValue[i]);
        iIndexBuffer++;
      }
      /* Big Endian... */
      else
      {
        
        pcBuffer[iIndexBuffer] = (uint8) TO_BYTE(pcValue[iIndexValue+(j*2)], 
                                                 pcValue[iIndexValue+(j*2)+1]);
        iIndexBuffer++;
      }
    }
  }
}

int TransferValue(
  char         *pcBuffer,
  char         *pcValue,
  struct TType *ptType,
  unsigned int  uiLenght)
{
  int16 i;
  int16 iCodeLength;
  int16 iReturnValue;

  if (ptType->iKind == FILEDATAKIND)
  { 
    memcpy(pcBuffer, pcValue, uiLenght);
    iReturnValue = uiLenght;
  }
  else
  {
    if (pcValue[1]=='x') 
    {
      iCodeLength  = strlen(pcValue+2);
    }
    else
    {
      iCodeLength  = strlen(pcValue);
    }
    iReturnValue = iCodeLength/2;
    
    if (ptType->iKind == ARRAYKIND)
    {
      if (ptType->uiSizeOfElement > 1)
      {
        ConvertArray(pcBuffer, pcValue, iCodeLength, ptType);
      }
      else
      {
        iReturnValue = ConvertValue(pcBuffer, pcValue, iCodeLength, ptType);
      }
    }
    else
    {
      iReturnValue = ConvertValue(pcBuffer, pcValue, iCodeLength, ptType);
    }
  }
  return iReturnValue;
}

void DetailledLog(struct TParameter *ptParameter,
                  struct TVariable  *ptVar)
{
  char *pcLogLine;
  struct TValueDefList *ptValueWalker;

  pcLogLine = (char *) malloc(1024);

  if (   (ptParameter->ptTypeDef->iKind == UNICODE0KIND)
      || (ptParameter->ptTypeDef->iKind == UNICODEKIND )
      || (ptParameter->ptTypeDef->iKind == ASCII0KIND  )
      || (ptParameter->ptTypeDef->iKind == ASCIIKIND   )
     )
  {
     sprintf(pcLogLine, "%26s <- %s",
             ptParameter->pcName,
             ptVar->pcName);
  }
  else if (ptParameter->ptTypeDef->iKind == ENUMKIND)
  {
    ptValueWalker = ptParameter->ptTypeDef->ptDefinition;
    while (   (ptValueWalker)
           && (ptValueWalker->ptThis)
           && (ptValueWalker->ptThis->pcValue)
           && (strcmp(ptValueWalker->ptThis->pcValue,ptVar->ptValue->pcValue) != 0)
          )
    {
      ptValueWalker = ptValueWalker->ptNext;
    }
    if (   (ptValueWalker)
        && (ptValueWalker->ptThis)
        && (ptValueWalker->ptThis->pcValue)
        && (strcmp(ptValueWalker->ptThis->pcValue,ptVar->ptValue->pcValue) == 0)
       )
    {
       sprintf(pcLogLine, "%26s <- %16s (%s)",
                    ptParameter->pcName,
                    ptVar->ptValue->pcValue,
                    ptValueWalker->ptThis->pcDescription);
    }
    else
    {
       sprintf(pcLogLine, "%26s <- %16s (un-enumerated)",
                    ptParameter->pcName,
                    ptVar->ptValue->pcValue);
    }
  }
  else
  {
    sprintf(pcLogLine, "%26s <- %16s", ptParameter->pcName, ptVar->ptValue->pcValue);
  }

  LogLine(pcLogLine);
  free(pcLogLine);
}


void SendFunction(
  int16 iIOChannel,
  struct TFunction *ptFunction,
  struct TVariableList *ptVarList)
/****************************************************************************/
/* This is the last function before the BSED_SendString function is invoked. */
/****************************************************************************/
{
  char     *pcValue;
  char      pcSendBuffer[204800];
  int       i = 0;
  int       j = 0;
  int       iDataLength = 0;
  struct TData *ptData;
  char     *pcLogLine = NULL;
  int16     iResult;
  struct TIOList *ptIOList;
  int       iLength;
  char      pBuffer[204800];
  char      pHex[10];
  char     *pcAsciiEntry;
  struct TParameterList *ptFunctionParameters = NULL;
  struct TType          *EmptyType;
  int16     iVarSize;

  int16     iLFCounter = 0;
  int16     iLFPosition[20];    /* Up to 20 length fields.. */
  int16     iLFStart[20];
  int16     iLFStop[20];
  int16     iLFLength[20];
  struct TParameter *ptLFInfo[20];

/*  struct TValueDefList *ptValueWalker; */


  struct TParameter *ptParameter[2048];

  /* First do some logging, if required.                                    */

  if (((_ptGlobals->iLogMode & BSEK_LOG_EXTENSIVE) == BSEK_LOG_EXTENSIVE)
/* ||  ((_ptGlobals->iLogMode & BSEK_LOG_STRING   ) == BSEK_LOG_STRING    ) */
    )
  {
    pcLogLine = (char *) malloc(204800);
    LogLine("");
    sprintf(pcLogLine, "Sending event %s", ptFunction->pcName);
    LogLine(pcLogLine);
    free(pcLogLine);
  }

  ptFunctionParameters = ptFunction->ptSendParameters;

  /* Now compose the string to be sent.                                     */


  while (ptFunctionParameters != NULL)
  {
    while (((ptFunctionParameters != NULL)
            && (ptFunctionParameters->ptThis != NULL)
            && (ptFunctionParameters->ptThis->ptTypeDef == NULL))
           || ((ptFunctionParameters != NULL)
               && (ptFunctionParameters->ptRepRec != NULL))
           || ((ptFunctionParameters != NULL)
               && (ptFunctionParameters->ptThis != NULL)
               && (ptFunctionParameters->ptThis->ptLengthFromField != NULL))
           || ((ptFunctionParameters != NULL)
               && (ptFunctionParameters->ptThis != NULL)
               && (ptFunctionParameters->ptThis->ptLengthToField != NULL)))
    {
      /* We have to test the presence of a value, but the real value      */
      /* is stored in the name then...                                    */
      /* If this is the case, it's a "constant" field.                    */
      /* which has to be transmitted, but is not in the variables list..  */

      ptParameter[i] = ptFunctionParameters->ptThis;


      if (ptFunctionParameters->ptRepRec != NULL)
      {
        if (ptFunctionParameters->ptRepRec->iBegin == TRUE)
        {
          /* When I'm on the beginning, I initialize */
          ptFunctionParameters->ptRepRec->iCounter =
            ptFunctionParameters->ptRepRec->ptParameter->iLoopCount;
        }
        else
        {
          /* When I'm on the end, I jump to the beginning. */
          ptFunctionParameters = ptFunctionParameters->aptNext[1];
        }
        if (ptFunctionParameters->ptRepRec->iCounter == 0)
        {
          /* No repetition at all or any more. skip repeat block            */
          ptFunctionParameters = ptFunctionParameters->aptNext[1]->aptNext[0];
        }
        else
        {
          ptFunctionParameters->ptRepRec->iCounter--;
          ptFunctionParameters = ptFunctionParameters->aptNext[0];
        }
      }
      else
      {
        if (ptFunctionParameters->ptThis->ptTypeDef == NULL)
        {
          EmptyType = (struct TType*) malloc( sizeof(struct TType) );
          EmptyType->pcName = NULL;
          EmptyType->iSizeInBytes = 0;
          EmptyType->iLessAllowed = 0;
          EmptyType->pcUpperLimit = NULL;
          EmptyType->pcLowerLimit = NULL;
          EmptyType->pcDecimalDescription = NULL;
          EmptyType->iKind = 0;
          EmptyType->fTimeScaling = (float) 1.00;       /* suprise! */
          EmptyType->ptDefinition = NULL;
          EmptyType->iEndianess = TRUE;
          EmptyType->iRefCount = 1;
           
          /* No typedef -> Predefined constant.                             */
          /* Retrieve value from ptFunctionParameters.                      */
          i += TransferValue( &(pcSendBuffer[i]),
                              ptFunctionParameters->ptThis->pcName,
                              EmptyType,
                              0);
          free(EmptyType);
          ptFunctionParameters = ptFunctionParameters->aptNext[0];
        }
        else if ((ptFunctionParameters->ptThis->ptLengthFromField != NULL)
                 || (ptFunctionParameters->ptThis->ptLengthToField != NULL))
        {
          iLFPosition[iLFCounter] = i;
          ptLFInfo[iLFCounter] = ptFunctionParameters->ptThis;
          iLFCounter++;

          /* Fill with '\0' bytes for the time being... */
          iVarSize = ptFunctionParameters->ptThis->ptTypeDef->iSizeInBytes;
          while (iVarSize--) pcSendBuffer[i++]=0;

          ptFunctionParameters = ptFunctionParameters->aptNext[0];
        }
        else
        {
          /* a Typedef. -> No predefined constant.                          */
          /* Retrieve value from ptVarList                                  */
/*                pcValue = ptVarList->ptThis->ptValue->pcValue; */
          ptFunctionParameters->ptThis->iLoopCount = GetInteger(pcValue);
          i += TransferValue(&(pcSendBuffer[i]),
                              ptVarList->ptThis->pcName,
                              ptVarList->ptThis->ptItsType,
                              ptVarList->ptThis->ptValue->uiLength);
        }
        if (ptFunctionParameters) ptParameter[i] = ptFunctionParameters->ptThis;
      }
    }
    if (ptVarList != NULL)
    {
      if (ptVarList->ptThis->ptItsType == NULL)
      {
        /* No type known -> it's a Data record.                               */

        ptData = (struct TData *) ptVarList->ptThis;
        pcValue = ptData->pcValue;
        while ((*pcValue) != '\0')
        {
          pcSendBuffer[i++] = *(pcValue++);
          iDataLength++;
        }
      }
      else
      {
        /* known type -> It's a variable record.                              */
        /* pcSendBuffer must be extended with the value of this variable.     */

        pcValue = ptVarList->ptThis->ptValue->pcValue;
        ptFunctionParameters->ptThis->iLoopCount = GetInteger(pcValue);

        pcAsciiEntry = &pcSendBuffer[i]; /* just in case we need to log ASCII */
        i += TransferValue(&(pcSendBuffer[i]),
                            pcValue,
                            ptVarList->ptThis->ptItsType,
                            ptVarList->ptThis->ptValue->uiLength);
        pcSendBuffer[i] = 0;             /* just in case we need to log ASCII.*/

        /* Value is added to the pcSendBuffer string. Terminate with some     */
        /* logging.                                                           */

        if ((_ptGlobals->iLogMode & BSEK_LOG_EXTENSIVE) == BSEK_LOG_EXTENSIVE)
        {
          DetailledLog(ptFunctionParameters->ptThis, ptVarList->ptThis);
        }
        else if ((_ptGlobals->iLogMode & BSEK_LOG_STRING) == BSEK_LOG_STRING)
        {
          pcLogLine = (char *) malloc(1024);
          sprintf(pcLogLine, "-> ");
          strcat(pcLogLine, pcAsciiEntry);
          LogLine(pcLogLine);
          free(pcLogLine);
        }

        if (ptFunctionParameters != NULL)
        {
          ptFunctionParameters = NextParameter(ptFunctionParameters,
                                               ptVarList->ptThis->ptValue->
                                               pcValue);
        }
      }

      ptVarList = ptVarList->ptNext;
    }
  }

  iLength = i;

  if (iLFCounter > 0)
  {
    /* iLength is lenght of stream until the end. Default is length measurement
       to the end of the stream, therefor this initialisation. */
    for (i = 0; i < iLFCounter; i++)
      iLFStop[i] = iLength;


    /*int16 iLFCounter=0;
      int16 iLFPosition[20]; 
      int16 iLFStart[20];
      int16 iLFStop[20];
      int16 iLFLength[20]; */

    for (i = 0; i < iLength; i++)
    {
      for (j = 0; j < iLFCounter; j++)
      {
        if (ptParameter[i] == ptLFInfo[j]->ptLengthFromField)
          iLFStart[j] = i;
        if (ptParameter[i] == ptLFInfo[j]->ptLengthToField)
          iLFStop[j] = i;
      }
    }

    for (j = 0; j < iLFCounter; j++)
    {
      iLFLength[j] = (iLFStop[j] - iLFStart[j]) + 1;

      if (ptLFInfo[j]->ptTypeDef->iEndianess == TRUE)
      {
        iLFPosition[j] += (ptLFInfo[j]->ptTypeDef->iSizeInBytes - 1);
      }

      while (iLFLength[j])
      {
        pcSendBuffer[iLFPosition[j]] = (int8) iLFLength[j] % 0xFF;
        iLFLength[j] /= 0x100;

        if (ptLFInfo[j]->ptTypeDef->iEndianess == TRUE)
        {
          iLFPosition[j]--;
        }
        else
        {
          iLFPosition[j]++;
        }
        iDataLength++;
        pcValue += 2;           /* Skip the just transferred byte                      */
      }
    }
  }

  /* Send buffer is ready. Maybe do some additional logging?             */

  if ((_ptGlobals->iLogMode & BSEK_LOG_STRING) != BSEK_LOG_STRING)
  {
     /********************************************************************/
    /* String log is switched off. Give a compact logging.              */
     /********************************************************************/

    ptIOList = _ptIOList;       /* Try to find the name of the device on      */
    /* on which the information is received...    */
    while ((ptIOList->ptThis->iNumber != iIOChannel)
           && (ptIOList->ptNext != NULL))
    {
      ptIOList = ptIOList->ptNext;
    }

    if (ptIOList->ptThis->iNumber == iIOChannel)
    {
      sprintf(pBuffer, "[-> %s (%d):", ptIOList->ptThis->pcName, iLength);
    }
    else
    {
      sprintf(pBuffer, "[-> %02d (%d) ", iIOChannel, iLength);
    }

    i = 0;
    while ((i < iLength) && (i < 255))
    {
      sprintf(pHex, "%02X ", pcSendBuffer[i]);
      if (pcSendBuffer[i] >= 0)
        strcat(pBuffer, pHex);
      else
        strcat(pBuffer, pHex + 6);      /* Because of e.g. 0xFFFFFF80          */
      i++;
    }
    LogLine(pBuffer);

  }

  /* Fill in the lenght fields now... */

  iResult = BSED_SendString(iIOChannel, iLength, (uint8 *)pcSendBuffer);

  if (iResult != OK)
  {
    LogLine("Cannot send data");
    StopExecutor("Cannot send data");
  }

}

#ifdef OBSOLETE_SENDFUNCTION
/*************************************************/
void SendFunction(
  int16 iIOChannel,
  struct TFunction *ptFunction,
  struct TVariableList *ptVarList)
/****************************************************************************/
/* This is the last function before the BSED_SendString function is invoked. */
/****************************************************************************/
{
  static int iSequenceNumber = 0;
  char     *pcValue;
  char      pcSendBuffer[1024];
  int16     i = 0;
  int16     j = 0;
  int16     iDataLength = 0;
  struct TData *ptData;
  char     *pcLogLine = NULL;
  int16     iResult;
  struct TIOList *ptIOList;
  int       iLength;
  char      pBuffer[2048];
  char      pHex[10];
  int       iEndian;
  int16     iCodeLength;
  char     *pcAsciiEntry;
  struct TParameterList *ptFunctionParameters = NULL;
  int16     iVarSize;

  int16     iLFCounter = 0;
  int16     iLFPosition[20];    /* Up to 20 length fields.. */
  int16     iLFStart[20];
  int16     iLFStop[20];
  int16     iLFLength[20];
  struct TParameter *ptLFInfo[20];

  struct TValueDefList *ptValueWalker;


  struct TParameter *ptParameter[2048];

  /* First do some logging, if required.                                    */

  if (((_ptGlobals->iLogMode & BSEK_LOG_EXTENSIVE) == BSEK_LOG_EXTENSIVE)
/* ||  ((_ptGlobals->iLogMode & BSEK_LOG_STRING   ) == BSEK_LOG_STRING    ) */
    )
  {
    pcLogLine = (char *) malloc(1024);
    LogLine("");
    sprintf(pcLogLine, "Sending event %s", ptFunction->pcName);
    LogLine(pcLogLine);
    free(pcLogLine);
  }

  ptFunctionParameters = ptFunction->ptSendParameters;

  /* Now compose the string to be sent.                                     */


  while (ptFunctionParameters != NULL)
  {
    while (((ptFunctionParameters != NULL)
            && (ptFunctionParameters->ptThis != NULL)
            && (ptFunctionParameters->ptThis->ptTypeDef == NULL))
           || ((ptFunctionParameters != NULL)
               && (ptFunctionParameters->ptRepRec != NULL))
           || ((ptFunctionParameters != NULL)
               && (ptFunctionParameters->ptThis != NULL)
               && (ptFunctionParameters->ptThis->ptLengthFromField != NULL))
           || ((ptFunctionParameters != NULL)
               && (ptFunctionParameters->ptThis != NULL)
               && (ptFunctionParameters->ptThis->ptLengthToField != NULL)))
    {
      /* We have to test the presence of a value, but the real value      */
      /* is stored in the name then...                                    */
      /* If this is the case, it's a "constant" field.                    */
      /* which has to be transmitted, but is not in the variables list..  */

      ptParameter[i] = ptFunctionParameters->ptThis;


      if (ptFunctionParameters->ptRepRec != NULL)
      {
        if (ptFunctionParameters->ptRepRec->iBegin == TRUE)
        {
          /* When I'm on the beginning, I initialize */
          ptFunctionParameters->ptRepRec->iCounter =
            ptFunctionParameters->ptRepRec->ptParameter->iLoopCount;
        }
        else
        {
          /* When I'm on the end, I jump to the beginning. */
          ptFunctionParameters = ptFunctionParameters->aptNext[1];
        }
        if (ptFunctionParameters->ptRepRec->iCounter == 0)
        {
          /* No repetition at all or any more. skip repeat block            */
          ptFunctionParameters = ptFunctionParameters->aptNext[1]->aptNext[0];
        }
        else
        {
          ptFunctionParameters->ptRepRec->iCounter--;
          ptFunctionParameters = ptFunctionParameters->aptNext[0];
        }
      }
      else
      {
        if (ptFunctionParameters->ptThis->ptTypeDef == NULL)
        {
          /* No typedef -> Predefined constant.                             */
          /* Retrieve value from ptFunctionParameters.                      */
          pcValue = ptFunctionParameters->ptThis->pcName;
          iEndian = TRUE;
          ptFunctionParameters = ptFunctionParameters->aptNext[0];
        }
        else if ((ptFunctionParameters->ptThis->ptLengthFromField != NULL)
                 || (ptFunctionParameters->ptThis->ptLengthToField != NULL))
        {
          iLFPosition[iLFCounter] = i;
          ptLFInfo[iLFCounter] = ptFunctionParameters->ptThis;
          iLFCounter++;

          iVarSize = ptFunctionParameters->ptThis->ptTypeDef->iSizeInBytes;
          pcValue = malloc(iVarSize * 2 + 3);
          strcpy(pcValue, "0x");
          while (iVarSize--)
            strcat(pcValue, "00");

          iEndian = ptFunctionParameters->ptThis->ptTypeDef->iEndianess;
          ptFunctionParameters = ptFunctionParameters->aptNext[0];
        }
        else
        {
          /* a Typedef. -> No predefined constant.                          */
          /* Retrieve value from ptVarList                                  */
          pcValue = ptVarList->ptThis->pcName;
/*                pcValue = ptVarList->ptThis->ptValue->pcValue; */
          iEndian = ptVarList->ptThis->ptItsType->iEndianess;
          ptFunctionParameters->ptThis->iLoopCount = GetInteger(pcValue);
        }
        pcValue += 2;           /* skip the 0x prefix.                                  */

        iCodeLength = strlen(pcValue) / 2;

        if (iEndian == FALSE)
        {
          i += (iCodeLength - 1);
        }

        while ((pcValue[0] != '\0') && (pcValue[1] != '\0'))
        {
          pcSendBuffer[i] = (int8) TO_BYTE(pcValue[0], pcValue[1]);

          if (iEndian == FALSE)
            i--;
          else
            i++;

          iDataLength++;
          pcValue += 2;         /* Skip the just transferred byte                      */
        }

        if (iEndian == FALSE)
        {
          i += (iCodeLength + 1);
        }
      }
    }

    ptParameter[i] = ptFunctionParameters->ptThis;

    if (ptVarList->ptThis->ptItsType == NULL)
    {
      /* No type known -> it's a Data record.                               */

      ptData = (struct TData *) ptVarList->ptThis;
      pcValue = ptData->pcValue;
      while ((*pcValue) != '\0')
      {
        pcSendBuffer[i++] = *(pcValue++);
        iDataLength++;
      }
    }
    else
    {
      /* known type -> It's a variable record.                              */
      /* pcSendBuffer must be extended with the value of this variable.     */

      pcValue = ptVarList->ptThis->ptValue->pcValue;
      ptFunctionParameters->ptThis->iLoopCount = GetInteger(pcValue);

      pcValue += 2;             /* skip the 0x prefix.                                  */

      pcAsciiEntry = &pcSendBuffer[i];  /* just in case we need to log ASCII */

      iCodeLength = strlen(pcValue) / 2;

      if (ptVarList->ptThis->ptItsType->iEndianess == FALSE)
      {
        i += (iCodeLength - 1);
      }

      while ((pcValue[0] != '\0') && (pcValue[1] != '\0'))
      {
        pcSendBuffer[i] = (int8) TO_BYTE(pcValue[0], pcValue[1]);

        if (ptVarList->ptThis->ptItsType->iEndianess == FALSE)
        {
          i--;
        }
        else
        {
          i++;
        }
        iDataLength++;
        pcValue += 2;           /* Skip the just transferred byte                      */
      }

      if (ptVarList->ptThis->ptItsType->iEndianess == FALSE)
      {
        i += (iCodeLength + 1);
      }

      pcSendBuffer[i] = 0;      /* just in case we need to log ASCII.            */

      /* Value is added to the pcSendBuffer string. Terminate with some     */
      /* logging.                                                           */

      if ((_ptGlobals->iLogMode & BSEK_LOG_EXTENSIVE) == BSEK_LOG_EXTENSIVE)
      {

        pcLogLine = (char *) malloc(1024);

        if ((ptFunctionParameters->ptThis->ptTypeDef->iKind == UNICODE0KIND)
            || (ptFunctionParameters->ptThis->ptTypeDef->iKind == UNICODEKIND)
            || (ptFunctionParameters->ptThis->ptTypeDef->iKind == ASCII0KIND)
            || (ptFunctionParameters->ptThis->ptTypeDef->iKind == ASCIIKIND))
        {
          sprintf(pcLogLine, "%26s <- %s",
                  ptFunctionParameters->ptThis->pcName,
                  ptVarList->ptThis->pcName);
        }
        else if (ptFunctionParameters->ptThis->ptTypeDef->iKind == ENUMKIND)
        {
          ptValueWalker = ptFunctionParameters->ptThis->ptTypeDef->ptDefinition;
          while ((ptValueWalker)
                 && (ptValueWalker->ptThis)
                 && (ptValueWalker->ptThis->pcValue)
                 &&
                 (strcmp
                  (ptValueWalker->ptThis->pcValue,
                   ptVarList->ptThis->ptValue->pcValue) != 0))
          {
            ptValueWalker = ptValueWalker->ptNext;
          }
          if ((ptValueWalker)
              && (ptValueWalker->ptThis)
              && (ptValueWalker->ptThis->pcValue)
              &&
              (strcmp
               (ptValueWalker->ptThis->pcValue,
                ptVarList->ptThis->ptValue->pcValue) == 0))
            sprintf(pcLogLine, "%26s <- %16s (%s)",
                    ptFunctionParameters->ptThis->pcName,
                    ptVarList->ptThis->ptValue->pcValue,
                    ptValueWalker->ptThis->pcDescription);
          else
          {
            sprintf(pcLogLine, "%26s <- %16s (un-enumerated)",
                    ptFunctionParameters->ptThis->pcName,
                    ptVarList->ptThis->ptValue->pcValue);
          }
        }
        else
        {
          sprintf(pcLogLine, "%26s <- %16s",
                  ptFunctionParameters->ptThis->pcName,
                  ptVarList->ptThis->ptValue->pcValue);
        }

        LogLine(pcLogLine);
        free(pcLogLine);
      }
      else if ((_ptGlobals->iLogMode & BSEK_LOG_STRING) == BSEK_LOG_STRING)
      {
        pcLogLine = (char *) malloc(1024);
        sprintf(pcLogLine, "-> ");
        strcat(pcLogLine, pcAsciiEntry);
        LogLine(pcLogLine);
        free(pcLogLine);
      }

      if (ptFunctionParameters != NULL)
      {

        ptFunctionParameters = NextParameter(ptFunctionParameters,
                                             ptVarList->ptThis->ptValue->
                                             pcValue);
      }
    }

    ptVarList = ptVarList->ptNext;
  }

  iLength = i;

  if (iLFCounter > 0)
  {
    /* iLength is lenght of stream until the end. Default is length measurement
       to the end of the stream, therefor this initialisation. */
    for (i = 0; i < iLFCounter; i++)
      iLFStop[i] = iLength;


    /*int16 iLFCounter=0;
      int16 iLFPosition[20]; 
      int16 iLFStart[20];
      int16 iLFStop[20];
      int16 iLFLength[20]; */

    for (i = 0; i < iLength; i++)
    {
      for (j = 0; j < iLFCounter; j++)
      {
        if (ptParameter[i] == ptLFInfo[j]->ptLengthFromField)
          iLFStart[j] = i;
        if (ptParameter[i] == ptLFInfo[j]->ptLengthToField)
          iLFStop[j] = i;
      }
    }

    for (j = 0; j < iLFCounter; j++)
    {
      iLFLength[j] = (iLFStop[j] - iLFStart[j]) + 1;

      if (ptLFInfo[j]->ptTypeDef->iEndianess == TRUE)
      {
        iLFPosition[j] += (ptLFInfo[j]->ptTypeDef->iSizeInBytes - 1);
      }

      while (iLFLength[j])
      {
        pcSendBuffer[iLFPosition[j]] = (int8) iLFLength[j] % 0xFF;
        iLFLength[j] /= 0x100;

        if (ptLFInfo[j]->ptTypeDef->iEndianess == TRUE)
        {
          iLFPosition[j]--;
        }
        else
        {
          iLFPosition[j]++;
        }
        iDataLength++;
        pcValue += 2;           /* Skip the just transferred byte                      */
      }
    }
  }

  /* Send buffer is ready. Maybe do some additional logging?             */

  if ((_ptGlobals->iLogMode & BSEK_LOG_STRING) != BSEK_LOG_STRING)
  {
     /********************************************************************/
    /* String log is switched off. Give a compact logging.              */
     /********************************************************************/

    ptIOList = _ptIOList;       /* Try to find the name of the device on      */
    /* on which the information is received...    */
    while ((ptIOList->ptThis->iNumber != iIOChannel)
           && (ptIOList->ptNext != NULL))
    {
      ptIOList = ptIOList->ptNext;
    }

    if (ptIOList->ptThis->iNumber == iIOChannel)
    {
      sprintf(pBuffer, "[-> %s (%d):", ptIOList->ptThis->pcName, iLength);
    }
    else
    {
      sprintf(pBuffer, "[-> %02d (%d) ", iIOChannel, iLength);
    }

    i = 0;
    while ((i < iLength) && (i < 255))
    {
      sprintf(pHex, "%02X ", pcSendBuffer[i]);
      if (pcSendBuffer[i] >= 0)
        strcat(pBuffer, pHex);
      else
        strcat(pBuffer, pHex + 6);      /* Because of e.g. 0xFFFFFF80          */
      i++;
    }
    LogLine(pBuffer);

  }

  /* Fill in the lenght fields now... */

  iResult = BSED_SendString(iIOChannel, iLength, pcSendBuffer);

  if (iResult != OK)
  {
    LogLine("Cannot send data");
    StopExecutor("Cannot send data");
  }

}

/*************************************************/
#endif



void Send_PROT_ACL_Data(
  int16 iBoardNumber,
  int16 iHandle,
  int16 iPacketBoundaryFlag,
  int16 iBroadCastFlag,
  char *pcBuffer,
  int16 iBufferLength)
{
  char      pcSendBuffer[204800];
  int16     i = 0;
  int16     iHandleAndFlags;
  int16     iResult;
  int16     iPB0, iPB1, iBC0, iBC1;

  iPB0 = (iPacketBoundaryFlag & 0x01) >> 0;
  iPB1 = (iPacketBoundaryFlag & 0x02) >> 1;

  iBC0 = (iBroadCastFlag & 0x01) >> 0;
  iBC1 = (iBroadCastFlag & 0x02) >> 1;

  iHandleAndFlags = (iHandle << 0) | (iPB0 << 12) | (iPB1 << 13) | (iBC0 << 14) | (iBC1 << 15); /* Big Endian Forever :-(((        */


  pcSendBuffer[i++] = 0x02;     /* PROT Command Packet                           */
  pcSendBuffer[i++] = (iHandleAndFlags & 0x00FF) >> 0;
  pcSendBuffer[i++] = (iHandleAndFlags & 0xFF00) >> 8;

  pcSendBuffer[i++] = ((iBufferLength + 4) & 0x00FF) >> 0;
  pcSendBuffer[i++] = ((iBufferLength + 4) & 0xFF00) >> 8;

  pcSendBuffer[i++] = ((iBufferLength + 0) & 0x00FF) >> 0;
  pcSendBuffer[i++] = ((iBufferLength + 0) & 0xFF00) >> 8;

  pcSendBuffer[i++] = 0;
  pcSendBuffer[i++] = 0;

  while (iBufferLength--)
  {
    pcSendBuffer[i++] = *(pcBuffer++);
  }

  iResult = BSED_SendString(iBoardNumber, i, (uint8 *)pcSendBuffer);

  if (iResult != OK)
  {
    LogLine("Cannot send data");
    StopExecutor("Cannot send data");
  }
  else
  {
    sprintf(pcSendBuffer, "%s sent %d data-bytes", DeviceName(iBoardNumber), i);
    LogLine(pcSendBuffer);

  }

}
