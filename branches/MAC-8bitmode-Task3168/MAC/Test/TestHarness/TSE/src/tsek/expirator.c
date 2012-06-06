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


#include "codingstandard.h"
#include "expirator.h"
#include "bsegui.h"
#include "general_lib.h"
#include "executor_lib.h"
#include "atkernel_intern.h"

#include <time.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

struct TTimerList *_ptTimerList = NULL;


/****************************************************************************/
/* local function headers.                                                  */
/****************************************************************************/
struct TTimer *newTimer(
  void);
struct TTimerList *newTimerList(
  void);


int16 InitTimer(
  void)
/****************************************************************************/
/* Initialises the expirator module.                                        */
/****************************************************************************/
{
  /* Define a dummy record at the head of the timer list. Such a dummy      */
  /* records simplifies a lot of algorithms. This dummy record will never   */
  /* be deleted!                                                            */
  /* (It's expirationmoment is set to zero; which will be earlier than all  */
  /*  other expiration moments.)                                            */

  _ptTimerList = newTimerList();
  _ptTimerList->ptThis = newTimer();
  return 0;
}


int16 ClosedownTimer(
  void)
/****************************************************************************/
/* Deletes all memory occupied by the timer module                          */
/****************************************************************************/
{
  struct TTimerList *ptTimerList;

  while (_ptTimerList != NULL)
  {
    ptTimerList = _ptTimerList->ptNext;
    free(_ptTimerList->ptThis);
    free(_ptTimerList);
    _ptTimerList = ptTimerList;
  }

  return 0;
}

void MakeTimerTick(
  void)
/****************************************************************************/
/* Test if a timer expires. If so, make the statemachine do the correspon-  */
/* ding state transition.                                                   */
/****************************************************************************/
{
  struct TTimer   *ptTimer;
  char            *pLogLine;
  struct timeval   tNow;
  int16            iTimerElapsed;
  
  /* Test only the first one. (The second one. The first one is a dummy.    */
  if (_ptTimerList->ptNext != NULL)
  {
    iTimerElapsed = FALSE;
    gettimeofday(&tNow, NULL);
    if (_ptTimerList->ptNext->ptThis->tExpirationMoment.tv_sec == tNow.tv_sec)
    {
      if (_ptTimerList->ptNext->ptThis->tExpirationMoment.tv_usec <= tNow.tv_usec)
      {
        iTimerElapsed = TRUE;
      }
      else
      {
        iTimerElapsed = FALSE;
      }
    }
    else if (_ptTimerList->ptNext->ptThis->tExpirationMoment.tv_sec < tNow.tv_sec)
    {
      iTimerElapsed = TRUE;
    }
    else
    {
      iTimerElapsed = FALSE;
    }
    if (TRUE == iTimerElapsed)
    {
      ptTimer = _ptTimerList->ptNext->ptThis;
      pLogLine = (char *) malloc(256);
      sprintf(pLogLine, "%s:%s TIMER expires",
              ptTimer->ptStateMachine->pcName,
              ptTimer->ptStateMachine->ptStateInTimer->pcName);
      LogLine(pLogLine);
      free(pLogLine);

      TransferState(ptTimer->ptStateMachine,
                    ptTimer->ptStateMachine->ptRunningTimerTransition);
    }
  }
}


struct TTimer *AddTimer(
  struct timeval        tExpirationMoment,
  struct TStateMachine *ptStateMachine)
{
  BOOL               bFound;
  struct TTimerList *ptTimerWalker;
  struct TTimerList *ptNewTimer;

  ptNewTimer = newTimerList();
  ptNewTimer->ptThis = newTimer();

  ptNewTimer->ptThis->ptStateMachine = ptStateMachine;
  memcpy( &ptNewTimer->ptThis->tExpirationMoment, 
          &tExpirationMoment,
          sizeof( struct timeval));

  /* New timer record is ready now. Now insert it in the right position in  */
  /* the timer list. (In such a way that the first record in the list expi- */
  /* res the first.                                                         */


  ptTimerWalker = _ptTimerList;
  
  bFound = FALSE;
  /* Go to the next timer in the list when 
   * 1) the seconds of the timer in the list is less than the seconds of 
   *    the new timer.
   * 2) The seconds of the two timers are equal and the microsecond value 
   *    of the timer in the list is less than the microseconds of the new 
   *    timer
   */
  while ( (ptTimerWalker->ptNext != NULL) && (bFound == FALSE) )
  {
    if ( (ptTimerWalker->ptNext->ptThis->tExpirationMoment.tv_sec < 
             tExpirationMoment.tv_sec) ||
         ( (ptTimerWalker->ptNext->ptThis->tExpirationMoment.tv_sec ==  
              tExpirationMoment.tv_sec) &&
           (ptTimerWalker->ptNext->ptThis->tExpirationMoment.tv_usec <
               tExpirationMoment.tv_sec) ) ) 
    {
      ptTimerWalker = ptTimerWalker->ptNext;
    }
    else
    {
      bFound = TRUE;
    }
  }

  /* Found. The new record has to be added just after ptTimerWalker. We     */
  /* don't have to take care of the heading of the list: records will never */
  /* be added to the beginning of this list due to a dummy record over      */
  /* there.                                                                 */

  ptNewTimer->ptNext = ptTimerWalker->ptNext;
  ptTimerWalker->ptNext = ptNewTimer;

  return ptNewTimer->ptThis;

}


void DeleteTimer(
  struct TTimer **ppt)
/****************************************************************************/
/* Delete the indicated timer from the timer list .                         */
/****************************************************************************/
{
  struct TTimerList *ptWalker;
  struct TTimerList *ptObsolete;

  ptWalker = _ptTimerList;
  while ((ptWalker->ptNext != NULL) && (ptWalker->ptNext->ptThis != *ppt))
  {
    ptWalker = ptWalker->ptNext;
  }

  if (ptWalker != NULL)
  {
    /* found. Make a bypass */

    ptObsolete = ptWalker->ptNext;
    ptWalker->ptNext = ptWalker->ptNext->ptNext;

    /* Now ptObsolete->ptThis == *ppt. Both ptObsolete and *ppt can be */
    /* removed now.                                                    */

    free(*ppt);
    free(ptObsolete);

    *ppt = NULL;
  }
  else
  {
    /* not found. */
  }
}

void CurrentTime(
 struct timeval *tTime)
/****************************************************************************/
/* Return the amount of milliseconds passed by since the start of this      */
/* program.                                                                 */
/****************************************************************************/
{
 
  gettimeofday(tTime, NULL);
}



struct TTimer *newTimer(
  void)
{
  struct TTimer *n;

  n = (struct TTimer *) malloc(sizeof(struct TTimer));

  n->tExpirationMoment.tv_sec = 0;
  n->tExpirationMoment.tv_usec = 0;
  n->ptStateMachine = NULL;

  return n;
}

struct TTimerList *newTimerList(
  void)
{
  struct TTimerList *n;

  n = (struct TTimerList *) malloc(sizeof(struct TTimer));
  n->ptThis = NULL;
  n->ptNext = NULL;

  return n;
}

void delTimerList(
  void)
{
  struct TTimerList *n = NULL;
  struct TTimerList *p = NULL;

  p = _ptTimerList;

  while (p != NULL)
  {
    n = p->ptNext;
    if (p->ptThis != NULL)
    {
      /* First prevent the statemachine from referring to a non-existent  */
      /* timer. This also makes the timer re-initialising when the script */
      /* is restarted. (The old timers are removed, when a script is      */
      /* stopped. This implies that it is impossible to singlestep through */
      /* a timer.                                                         */
      if (p->ptThis->ptStateMachine != NULL)
      {
        p->ptThis->ptStateMachine->ptStateInTimer = NULL;
        p->ptThis->ptStateMachine->ptRunningTimer = NULL;
      }

      free(p->ptThis);
    }
    free(p);
    p = n;
  }

  _ptTimerList = newTimerList();
}

void WriteTimerList(
  char *pcText)
{
  char      line[250];

  struct TTimerList *ptTimerListRecord;

  ptTimerListRecord = _ptTimerList;

  while (ptTimerListRecord != NULL)
  {
    if (ptTimerListRecord->ptThis != NULL)
    {
      if ( (ptTimerListRecord->ptThis->tExpirationMoment.tv_sec > 0) ||
           (ptTimerListRecord->ptThis->tExpirationMoment.tv_usec > 0) )
      {
        sprintf(line, "\n\n");
        strcat(pcText, line);
        sprintf(line, "Timer in %s\n",
                ptTimerListRecord->ptThis->ptStateMachine->pcName);
        strcat(pcText, line);
      }
    }
    ptTimerListRecord = ptTimerListRecord->ptNext;
  }

}
