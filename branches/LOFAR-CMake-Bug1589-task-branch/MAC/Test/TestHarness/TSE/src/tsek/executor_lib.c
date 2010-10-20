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
// 08/10/2000 E.A. Nijboer            Initial release
// 12/01/2001 E.A. Nijboer            platform test.
// 09/02/2001 E.A. Nijboer            
//
//=========================================================================
*/


#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>


#include "executor_lib.h"
#include "general_lib.h"
#include "parser_lib.h"
#include "atkernel.h"
#include "stores.h"
#include "expirator.h"
#include "bsegui.h"
#include "atkernel_intern.h"
#include "stepper.h"


/****************************************************************************/
/* Local function headers.                                                  */
/* To keep the compiler happy.                                              */
/****************************************************************************/

struct TState *FindState(
  char *pcName,
  struct TStateList *ptStateList);
void      InitStateMachine(
  struct TStateMachine *ptMachine);
void      InitStateMachineList(
  struct TStateMachineList *ptList);
void      InitBoard(
  struct TBoard *ptBoard);



int16 iRandom(
  int16 iMax)
/****************************************************************************/
/* returns a random value between 0 and (iMax-1)                            */
/* E(iRandom(n)==c) = 0    for c <  0                                       */
/*                  = 1/n  for 0 <= c < n                                   */
/*                  = 0    for c >  n                                       */
/****************************************************************************/
{
  return (rand() % iMax);
}



void ar_or(
  char *pcA,
  char *pcB)
/****************************************************************************/
/* The value (in AutoTest Representation) in pcA will be or-ed with the     */
/* value in pcB.                                                            */
/* Example : pcA = "0x0101"                                                 */
/*           pcB = "0x0200"                                                 */
/*     pcA becomes "0x0301"                                                 */
/****************************************************************************/
{
  while ((*pcA) && (*pcB))
  {
    (*pcA) = (char) (TO_ASCII(TO_HEX(*pcA) | TO_HEX(*pcB)));
    pcA++;
    pcB++;
  }
}


void ar_and(
  char *pcA,
  char *pcB)
/****************************************************************************/
/* The value (in AutoTest Representation) in pcA will be and-ed with the    */
/* value in pcB.                                                            */
/* Example : pcA = "0x010D"                                                 */
/*           pcB = "0x030C"                                                 */
/*     pcA becomes "0x010C"                                                 */
/****************************************************************************/
{
  while ((*pcA) && (*pcB))
  {
    (*pcA) = (char) (TO_ASCII(TO_HEX(*pcA) & TO_HEX(*pcB)));
    pcA++;
    pcB++;
  }
}


void TryTransferState(
  struct TStateMachine *ptStateMachine,
  struct TTransition *ptTransition)
{
  struct TActionList *ptActionWalker;
  int16     bAllDone;

  ptActionWalker = ptTransition->ptActionList;
  bAllDone = TRUE;

  /**************************************************************************/
  /* First check whether all actions are 'done'.                            */
  /**************************************************************************/
  while (ptActionWalker != NULL)
  {
    if ((ptActionWalker->ptThis != NULL) && (bAllDone == TRUE))
    {
      if (ptActionWalker->ptThis->iNrOfReceptions == 0)
      {
        bAllDone = (ptActionWalker->ptThis->lActionCounter >= 1);
      }
      else
      {
        bAllDone =
          (ptActionWalker->ptThis->lActionCounter ==
           ptActionWalker->ptThis->iNrOfReceptions);
      }
    }
    ptActionWalker = ptActionWalker->ptNext;
  }

  /**************************************************************************/
  /* If all actions are done, then clear all action counters and do a       */
  /* TransferState. The TransferState implicitly updates the overview. If   */
  /* TransferState is not executed, the overview must be updated.           */
  /**************************************************************************/
  if (bAllDone == TRUE)
  {
    ptActionWalker = ptTransition->ptActionList;
    while (ptActionWalker != NULL)
    {
      if (ptActionWalker->ptThis != NULL)
      {
        ptActionWalker->ptThis->lActionCounter = 0;
      }
      ptActionWalker = ptActionWalker->ptNext;
    }
    TransferState(ptStateMachine, ptTransition);
  }
  else
  {
    Update_Overview();
  }
}

void TransferState(
  struct TStateMachine *ptStateMachine,
  struct TTransition *ptTransition)
/****************************************************************************/
/* Transfers the current state of statemachine ptStateMachine into the      */
/* state indicated by ptAction.                                             */
/* Running timers are abandoned.                                            */
/****************************************************************************/
{
  int16     bLogThis;
  int16     iInstantTransition;
  char     *pcOldStateName;

  bLogThis = (_ptGlobals->iLogMode & BSEK_LOG_STATE_TRANSITIONS);
  bLogThis = (bLogThis == BSEK_LOG_STATE_TRANSITIONS);

  iInstantTransition = (ptTransition->iInstantNextState);

  /* If there was a timer active in this state, and the new state is a     */
  /* different state, delete the timer. If the new state is the same state */
  /* then the timer remains active.                                        */

  if ((ptStateMachine->ptRunningTimer != NULL)
      && (ptStateMachine->ptCurrentState != ptTransition->ptNextState))
  {
    DeleteTimer(&ptStateMachine->ptRunningTimer);
    ptStateMachine->ptRunningTimer = NULL;
    ptStateMachine->ptStateInTimer = NULL;
  }

  /* Increment the transaction counter for the action in transit.          */
  ptTransition->lTransitionCounter++;

  pcOldStateName = ptStateMachine->ptCurrentState->pcName;

  ptStateMachine->ptCurrentState = ptTransition->ptNextState;
  if (bLogThis == TRUE)
  {
    char     *pcBuffer;

    pcBuffer = (char *) malloc(250);
    if (ptStateMachine->ptCurrentState != NULL)
    {
      sprintf(pcBuffer, "%s:%s %s -> %s",
              ptStateMachine->pcDeviceName,
              ptStateMachine->pcName,
              pcOldStateName, ptStateMachine->ptCurrentState->pcName);
    }
    else
    {
      sprintf(pcBuffer, "%s:%s terminates",
              ptStateMachine->pcDeviceName, ptStateMachine->pcName);
    }
    LogLine(pcBuffer);

    free(pcBuffer);
  }

  if (iInstantTransition)
  {
    SingleStep(ptStateMachine, ptStateMachine->iIOChannel);
  }

  Update_Overview();
}



char     *DeviceName(
  int16 i)
{
  struct TIOList *ptIOList;

  ptIOList = _ptIOList;         /* Try to find the name of the device on      */
  /* on which the information is received...    */

  while ((ptIOList != NULL)
         && (ptIOList->ptThis != NULL)
         && (ptIOList->ptThis->iNumber != i) && (ptIOList->ptNext != NULL))
  {
    ptIOList = ptIOList->ptNext;
  }

  if (ptIOList == NULL)
  {
    return "-- empty administration --";
  }

  if (ptIOList->ptThis == NULL)
  {
    return "-- devicename not known --";
  }

  if (ptIOList->ptThis->iNumber != i)
  {
    return "-- devicename not found --";
  }

  return ptIOList->ptThis->pcName;

}
