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
// Description   : This module is the header file for the Receive Buffer
//
// Revisions:
//
// Date       Author                  Changes
// 31/10/2000 Dennis Olausson         Initial release
//
//=========================================================================
*/


#ifndef _RECEIVEBUFF_H_
#define _RECEIVEBUFF_H_

#ifdef  __cplusplus
extern    "C"
{
#endif

#ifdef WIN32
#include "windows.h"            /* For window defines */
#endif
#include "codingstandard.h"     /*For common project defines */
#include "protocollist.h"       /* For Protocol access */


/************************************************************************
 * DEFINITIONS                                                          *
 ************************************************************************/

/* Return Messages */
#define BUFFER_OK                    ((int8)    1)
#define BUFFER_NOT_FOUND            ((int8)    0)
#define BUFFER_ALREADY_EXCIST        ((int8)-1)
#define BUFFER_OUT_OF_MEMORY        ((int8)-2)
#define BUFFER_MISSING_ARGUMENT        ((int8)-3)
#define BUFFER_BUSY                    ((int8)-4)

/************************************************************************
 * STRUCTURES/TYPES                                                     *
 ************************************************************************/
  struct TRBuffer
  {
    BOOL      bBusy;
    struct TProtocol *ptProtocol;
    int16     iBufferIndex;
    /* HANDLE                                hPort; */ /* Not needed in the implementation of BSE 2.4 */
    uint8    *pBuffer;
    int16     iDeviceNr;
  };

  struct TRBufferList
  {
    struct TRBuffer tBuffer;
    struct TRBufferList *ptNext;
  };


/************************************************************************
 * FUNCTIONS                                                            *
 ************************************************************************/
  int8      IO_BufferCreate(
  /* HANDLE hPort,*/
  int16 iBufferSize,
  struct TProtocol *ptProtocol,
  int16 iDeviceNr);
  int8      IO_BufferDeleteEntry(
  int16 iDeviceNumber /*HANDLE hPort */ );
  void      IO_BufferDeleteAll(
  void);
  int8      IO_BufferAdd(
  int16 iDeviceNumber /*HANDLE hPort */ ,
  uint8 * pAddStr,
  int16 iCharCount,
  int *OSCrashDetected);
  int8      IO_BufferFlush(
  int16 iDeviceNumber /*HANDLE hPort */ );





#ifdef __cplusplus
}
#endif                          /*  __cplusplus */

#endif
