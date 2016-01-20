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

#ifndef _EVENTRECEIVER_H_
#define _EVENTRECEIVER_H_

#include "codingstandard.h"

struct TRawEvent
{
  char     *pcEvent;
  int16     iEventLength;
  int16     iDeviceNumber;
};

struct TRawEventList
{
  struct TRawEvent *ptThis;
  struct TRawEventList *ptNext;
};



struct TRawEvent *GetNextEvent(
  void);

extern void      StoreThisEvent(
  int16 iDeviceNumber,
  int16 iLength,
  char *pcData);

extern int16     ResetEventReceiver(
  void);
extern int16     StoreIncomingEvents(
  void);
extern int16     FlushIncomingEvents(
  void);

#endif /* _EVENTRECEIVER_H_ */
