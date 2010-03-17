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


#ifndef _executor_master_h
#define _executor_master_h

#ifdef WIN32
#include "windows.h"

#include <process.h>
#include "winbase.h"
#else
#include <pthread.h>
#endif

#define EX_OK          ((int16) (  0))
#define EX_THREAD_BUSY ((int16) ( -1))
#define EX_NOT_OK      ((int16) (-99))

#define   HALTED      ((int16)(10))
#define   STEP        ((int16)(11))
#define   RUNNING     ((int16)(12))


/* Pending codes. These codes are used to control which statemachines are    */
/* 'active' and which statemachines have to wait for other statemachines.    */
/* Normal statemachines are just running. When a statemachine invokes (a     */
/* list of) other statemachines, the invoking statemachine becomes sleeping  */
/* New statemachines become just born, and will be made running the next     */
/* process tick. Ended statemachines have executed the END transition, and   */
/* will be deleted before the next process tick.                             */
/* When one of the child statemachines of a statemachine is ending, it's     */
/* parent becomes 'waking up'. When in waking up mode, it it checked before  */
/* the next process tick whether there are more childs active or not. If not */
/* the process becomes running.                                              */

/* When overview is in body mode, only running statemachines are displayed   */
/* in full. Just born and sleeping statemachines only have a header with     */
/* sleeping/born information.                                                */

#define   RUNNING_SM ((int16)0)
#define   SLEEPING   ((int16)1)
#define   JUST_BORN  ((int16)2)
#define   WAKING_UP  ((int16)3)
#define   ENDING     ((int16)4)

extern pthread_mutex_t RawEventBufferSemaphore;


extern int       InitExecutor(
  void);
extern int       StartExecutor(
  void);
extern int       StopExecutor(
  char *pcReason);
extern void WaitForEndExecutor(
  void );
extern int ExecutorStopped(
  void );  
extern int       PauseExecutor(
  void);
extern int       StepExecutor(
  void);


#endif
