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

#ifdef WIN_32
#include "windows.h"
#include <process.h>
#endif 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "codingstandard.h"
#include "executor_master.h"
#include "stepper.h"
#include "expirator.h"
#include "eventprocessor.h"
#include "eventreceiver.h"
#include "bsegui.h"
#include "atkernel_intern.h"
#include "parser_lib.h"


/*****************************************************************************/
/* Local variables.                                                          */
/*****************************************************************************/

pthread_t  tProcessHandle = 0;
int        iRunningMode = HALTED;
char      *pcStopReason = NULL;
char       pDummy;
int        iRunningState = HALTED;

pthread_mutex_t    RawEventBufferSemaphore = PTHREAD_MUTEX_INITIALIZER;

int       bThreadActive = FALSE;

/*****************************************************************************/
/* Local function headers.                                                   */
/*****************************************************************************/
void      *Dispatcher(
  void *dummy);
void      Process_Pending_SM(
  );




/*****************************************************************************/
/* Exported functions.                                                       */
/* These functions are not commented, because they are extremely short and   */
/* predictable.                                                              */
/*****************************************************************************/



/*****************************************************************************/
int InitExecutor(
  void)
{

  InitTimer();
  ResetEventReceiver();
  bThreadActive = FALSE;

  return EX_OK;
}


/*****************************************************************************/
int StartExecutor(
  void)
{
  pthread_attr_t tThreadAttributes;
  
  StoreIncomingEvents();

  iRunningMode  = RUNNING;
  iRunningState = RUNNING;
  
  pthread_attr_init( &tThreadAttributes );
  pthread_attr_setstacksize( &tThreadAttributes, sizeof(char) * 1024);
  pthread_attr_setdetachstate( &tThreadAttributes, PTHREAD_CREATE_JOINABLE);
  
  pthread_create( &tProcessHandle, 
                  &tThreadAttributes, 
                  Dispatcher, 
                  NULL);

  return EX_OK;
}

/*****************************************************************************/
int StopExecutor(
  char *pcReason)
{
  if (iRunningMode != HALTED)
  {
    pcStopReason = my_strdup(pcReason);
    iRunningMode = HALTED;
    
  }
  return EX_OK;
}

/*****************************************************************************/
int ExecutorStopped(
  void )
{
  int iResult;
  
  if( iRunningState == HALTED)
  {
    iResult = EX_OK;
  }
  else
  {
    iResult = EX_THREAD_BUSY;
  }
  return( iResult );
}

/*****************************************************************************/
void WaitForEndExecutor()
{
  pthread_join(tProcessHandle, NULL);
}
/*****************************************************************************/
int PauseExecutor(
  void)
{

  pcStopReason = my_strdup("user interruption");
  iRunningMode = HALTED;
  return EX_OK;
}

/*****************************************************************************/
int StepExecutor(
  void)
{
  StoreIncomingEvents();
  iRunningMode = STEP;
  Dispatcher((void *) &pDummy);
  return EX_OK;
}

/*****************************************************************************/
/* Local functions.                                                          */
/*****************************************************************************/
void *Dispatcher(
  void *dummy)
/* This function is the scheduler, which makes the scripts executing.        */
/* The one and only parameter of this function is not used, but the os       */
/* function _beginthread() requires a function with a void pointer...        */
{
  int16     bAnyModified;

  dummy = NULL;                 /* Just to prevent warning that dummy is unused.             */

  while (bThreadActive == TRUE)
    usleep(10000);
  bThreadActive = TRUE;


  while (iRunningMode != HALTED)
  {
    /* Hit the three sub-processes                                           */
    bAnyModified = MakeStep();

#ifdef GERRIT_SLOTS_METHOD
    /* Choosing 'this' method results in a different way of scrip executing. */
    /* The difference is that the default method is to do one single step on */
    /* each statemachine, and then process the input buffer. Gerrit's method */
    /* is to execute statemachines until all statemachines are waiting on an */
    /* event.                                                                */
    /* The latter method is unusable when statemachines have infinite loops  */
    /* without event transitions.                                            */
    /* Therefore this method is not preferred.                               */
    while (bAnyModified == TRUE)
    {
      bAnyModified = MakeStep();
    }
#endif

    MakeTimerTick();
    ProcessIncomingBuffer();
    usleep(10000);


    /* And do some internal things.                                          */
    Process_Pending_SM();

    if (iRunningMode == STEP)
    {
      iRunningMode = HALTED;
    }
  }

  FlushIncomingEvents();
  delTimerList();

  StoreFileVariableList(_ptGlobalVars);

  bThreadActive = FALSE;
  if (pcStopReason != NULL)
  {
    internal_ScriptStopped(pcStopReason);       /* Notify internal state machine */
  }
  else
  {
    pcStopReason = (char*) malloc( (strlen("")+1)* sizeof(char));
    strcpy(pcStopReason,"");
    internal_ScriptStopped(pcStopReason); /* Notify internal state machine */
  }
  iRunningState = HALTED;

  return(NULL);
}





void Process_Pending_SM(
  )
{
  struct TBoardList *ptBoards;
  struct TStateMachineList *ptSMs;
  struct TStateMachineList *ptSMwalker;
  int16     iAllChildsEnded;
  int16     iUpdateNeeded;

  iUpdateNeeded = FALSE;

  for (ptBoards = _ptBoardList; ptBoards != NULL; ptBoards = ptBoards->ptNext)
  {
    if (ptBoards->ptThis != NULL)
    {
      for (ptSMs = ptBoards->ptThis->ptStateMachines; ptSMs != NULL;
           ptSMs = ptSMs->ptNext)
      {
        if (ptSMs->ptThis != NULL)
        {
          switch (ptSMs->ptThis->iPending)
          {
            case RUNNING_SM:
              break;

            case SLEEPING:
              break;

            case JUST_BORN:
              ptSMs->ptThis->iPending = RUNNING_SM;
              iUpdateNeeded = TRUE;
              break;

            case WAKING_UP:
              iAllChildsEnded = TRUE;
              for (ptSMwalker = ptBoards->ptThis->ptStateMachines;
                   ptSMwalker != NULL; ptSMwalker = ptSMwalker->ptNext)
              {
                if ((ptSMwalker->ptThis != NULL)
                    && (ptSMwalker->ptThis->ptMaster == ptSMs->ptThis)
                    && (ptSMwalker->ptThis->iPending != ENDING))
                {
                  iAllChildsEnded = FALSE;
                }
              }

              if (iAllChildsEnded == TRUE)
              {
                ptSMs->ptThis->iPending = RUNNING_SM;
              }
              iUpdateNeeded = TRUE;

              break;

            case ENDING:
              delStateMachine(&(ptSMs->ptThis));
              iUpdateNeeded = TRUE;
              break;

            default:
              break;

          }
        }
      }
    }
  }

  if (iUpdateNeeded)
  {
    Update_Overview();
  }
}
