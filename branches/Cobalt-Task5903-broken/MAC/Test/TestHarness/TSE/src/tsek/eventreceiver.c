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
// 16/10/2000 E.A. Nijboer            Initial release
// 12/01/2001 E.A. Nijboer            platform test.
// 09/02/2001 E.A. Nijboer            
//
//=========================================================================
*/


/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/

#include <pthread.h>
#include <time.h>

#include "general_lib.h"
#include "eventreceiver.h"
#include "atkernel.h"
#include "bsegui.h"
#include "string.h"
#include "malloc.h"
#include "atkernel_intern.h"
#include "stores.h"
#include "atdriver.h"

#include "executor_master.h"

#define MAX_NO_EVENTS               ((int16) 10000)

/*****************************************************************************/
/* Local variables and stores                                                */
/*****************************************************************************/
struct TRawEventList *_ptRawEventList;
struct TRawEventList *ptLastRawEvent;


static int16     bStore = FALSE;
static int16     bWarnedForUnprocessedData = FALSE;
static int16     iBufferSem = 1;
static int16     iNumberOfEvents=0;
/*****************************************************************************/
/* Local function headers                                                    */
/*****************************************************************************/
void      StoreThisEvent(
  int16 iDeviceNumber,
  int16 iLength,
  char *pcData);
struct TRawEventList *newRawEventList(
  void);
struct TRawEvent *newRawEvent(
  void);

void      buffer_P(
  int16 * i);
void      buffer_V(
  int16 * i);

/*****************************************************************************/
/* Exported function bodies                                                  */
/*****************************************************************************/

int16 ResetEventReceiver(
  void)
{
  _ptRawEventList = NULL;
  ptLastRawEvent = NULL;
  return 0;
}

int16 StoreIncomingEvents(
  void)
{
  bStore = TRUE;
  return 0;
}

int16 FlushIncomingEvents(
  void)
{
  struct TIOList *ptWalker;

  ptWalker = _ptIOList;

  while (ptWalker != NULL)
  {
    if (ptWalker->ptThis != NULL)
    {
      BSED_FlushBuffer(ptWalker->ptThis->iNumber);
    }
    ptWalker = ptWalker->ptNext;
  }

  bStore = FALSE;
  bWarnedForUnprocessedData = FALSE;
  return 0;
}


void BSEK_ReceiveString(
  int16 iDeviceNumber,
  int16 iLength,
  char *pcData)
{
  StoreThisEvent(iDeviceNumber, iLength, pcData);
}


struct TRawEvent *GetNextEvent(
  void)
{
  char                 *pcErrorString;
  struct TRawEventList *ptEventListHead;
  struct TRawEvent     *ptFirstEvent;
  struct TRawEvent     *ptReturnValue;

  if (pthread_mutex_lock(&RawEventBufferSemaphore) == -1)
  {
    return NULL;
  }

  if (_ptRawEventList == NULL)
  {
    /* This is simple.                                                      */
    ptReturnValue = NULL;
  }
  else
  {
    /* This is slightly more complex. Prepare the deletion of the first     */
    /* record.                                                              */

    ptEventListHead = _ptRawEventList;
    _ptRawEventList = _ptRawEventList->ptNext;
    if (_ptRawEventList == NULL)
    {
      /* If this was the one and only event in the buffer, the              */
      /* ptLastRawEvent pointer becomes invalid too. Therefore, remove it.  */
      if (iNumberOfEvents != 1)
      {
        iNumberOfEvents = 1;
      }          
      ptLastRawEvent = NULL;
    }
    ptFirstEvent = ptEventListHead->ptThis;
    free(ptEventListHead);

    ptReturnValue = (ptFirstEvent);     /* The invoker should free this record. */
    iNumberOfEvents--;
  }

  if (pthread_mutex_unlock(&RawEventBufferSemaphore) == -1)
  {
    pcErrorString = (char*) malloc( 
      (strlen("internal error 1 releasing semaphores. ")+1) * sizeof(char));
    strcpy( pcErrorString,"internal error 1 releasing semaphores. ");
    /* weird. don't know what to do here... */
    BSEG_LogLine(pcErrorString);
  }


  return ptReturnValue;
}

/*****************************************************************************/
/* local function bodies                                                     */
/*****************************************************************************/

void StoreThisEvent(
  int16 iDeviceNumber,
  int16 iLength,
  char *pcData)
{
  struct TRawEvent *ptNewEvent;
  char             *pcErrorString;

  if (pthread_mutex_lock(&RawEventBufferSemaphore) == -1)
  {
    pcErrorString = (char*) malloc( 
      (strlen("Failed to get mutex lock for event")+1) * sizeof(char));
    strcpy( pcErrorString,"Failed to get mutex lock for event");
    BSEG_LogLine(pcErrorString);
    return;
  }

  if (iNumberOfEvents < MAX_NO_EVENTS)
  { 
    iNumberOfEvents++;
	  if (_ptRawEventList == NULL)
	  {
	    _ptRawEventList = newRawEventList();
	    _ptRawEventList->ptThis = newRawEvent();
	    ptNewEvent = _ptRawEventList->ptThis;
	    ptLastRawEvent = _ptRawEventList;
	  }
	  else
	  {
	    ptLastRawEvent->ptNext = newRawEventList();
	    ptLastRawEvent->ptNext->ptThis = newRawEvent();
	    ptLastRawEvent = ptLastRawEvent->ptNext;
	    ptNewEvent = ptLastRawEvent->ptThis;
	  }
	  ptNewEvent->pcEvent = pcData;
	  ptNewEvent->iEventLength = iLength;
	  ptNewEvent->iDeviceNumber = iDeviceNumber;
	
	  if (pthread_mutex_unlock(&RawEventBufferSemaphore) == -1)
	  {
	    pcErrorString = (char*) malloc( 
	      (strlen("internal error 2 releasing semaphores. ")+1) * sizeof(char));
	    strcpy( pcErrorString,"internal error 2 releasing semaphores. ");
	    /* weird. don't know what to do here... */
	    BSEG_LogLine(pcErrorString);
	  }
  }
  else
  {
    pcErrorString = (char*) malloc( 
      (strlen("The event queue is full dropping the event.")+1) * sizeof(char));
    strcpy( pcErrorString,"The event queue is full dropping the event.");
    BSEG_LogLine(pcErrorString);
  }
}

struct TRawEventList *newRawEventList(
  void)
{
  struct TRawEventList *n;
  n = (struct TRawEventList *) malloc(sizeof(struct TRawEventList));

  n->ptThis = NULL;
  n->ptNext = NULL;

  return n;
}

struct TRawEvent *newRawEvent(
  void)
{
  struct TRawEvent *n;
  n = (struct TRawEvent *) malloc(sizeof(struct TRawEvent));

  n->pcEvent = NULL;
  n->iEventLength = 0;
  n->iDeviceNumber = 0;

  return n;
}
