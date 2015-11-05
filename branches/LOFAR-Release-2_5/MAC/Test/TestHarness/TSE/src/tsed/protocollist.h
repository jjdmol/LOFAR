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
// Description   : This module is the header file for the ProtocolList Class
//
// Revisions:
//
// Date       Author                  Changes
// 18/10/2000 Dennis Olausson         Initial release
//
//=========================================================================
*/


#ifndef _PROTOCOLLIST_H_
#define _PROTOCOLLIST_H_

#ifdef  __cplusplus
extern    "C"
{
#endif

#include "codingstandard.h"     /* For common project defines */


/****************************************************************************/
/* STRUCTURES                                                               */
/****************************************************************************/

  struct TProtocol
  {
    int16     iProtocolNumber;
    int16     iProtocol;
    int16     iTokenLength;
    char     *pTerminateToken;
    int16     iMaxLength;
  };

  struct TProtocolList
  {
    struct TProtocol tProtocol;
    struct TProtocolList *pNext;
  };


/****************************************************************************/
/* FUNCTIONS                                                                */

  int16     IO_AddToProtocolList(
  int16 iProtocolNumber,
  int16 iProtocol,
  int16 iTokenLength,
  char *pTerminateToken,
  int16 iMaxLength);

  struct TProtocol *IO_ProtocolListFind(
  int16 iProtocolNumber);

  void      IO_ProtocolListPurge(
  void);

#ifdef __cplusplus
}
#endif

#endif /* ProtocolList.h */
