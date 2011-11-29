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

#ifndef _STEPPER_H_
#define _STEPPER_H_

#include "codingstandard.h"
#include "general_lib.h"

extern uint32 ul_SOCKET_GENERAL_ERROR;
extern uint32 ul_SOCKET_SEND_ERROR;
extern uint32 ul_SOCKET_RECEIVE_ERROR;
extern uint32 ul_SOCKET_CONNECT_ERROR;
extern uint32 ul_SOCKET_DISCONNECT_ERROR;
extern uint32 ul_SOCKET_ACCEPT_ERROR;
extern uint32 ul_SOCKET_UNKNOWN_ERROR;
extern uint32 ul_CONNECTION_LOST;
extern uint32 ul_CONNECTION_ESTABLISHED;

extern int16     MakeStep(
  void);

extern int16     SingleStep(
  struct TStateMachine *ptStateMachine,
  int16 iIOChannel);

#endif /* _STEPPER_H_ */
