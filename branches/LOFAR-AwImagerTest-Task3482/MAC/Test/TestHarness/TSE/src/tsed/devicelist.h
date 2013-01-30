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
// Description   : This module is the header file for the Device List
//
// Revisions:
//
// Date       Author                  Changes
// 24/10/2000 Dennis Olausson         Initial release
//
//=========================================================================
*/


#ifndef _DEVICELIST_H_
#define _DEVICELIST_H_

#ifdef  __cplusplus
extern    "C"
{
#endif

#ifdef WIN32
#include "windows.h"            /* For window defines */
#endif
#include "codingstandard.h"     /* For common project defines */

/************************************************************************
 * DEFINITIONS                                                          *
 ************************************************************************/

/* Return Messages */
#define DEVICE_OK                     ((int16)  1)
#define DEVICE_ERROR                  ((int16)  0)
#define DEVICE_ERROR_NO_PROTOCOL      ((int16) -1)
#define DEVICE_ERROR_DEVICE_EXCIST    ((int16) -2)
#define DEVICE_ERROR_OUT_OF_MEMORY    ((int16) -3)
#define DEVICE_ERROR_NO_DEVICE        ((int16) -4)


/************************************************************************
 * STRUCTURES/TYPES                                                     *
 ************************************************************************/

  enum EDeviceType
  { PIPE, RS232, USB, SOCKETS };

  struct TDevice
  {
    int16     iDeviceNumber;
    /*HANDLE                                hDevice; */
    enum EDeviceType eDevType;
    struct TProtocol *ptProtocol;
  };

  struct TDeviceList
  {
    struct TDevice tDevice;
    struct TDeviceList *ptNext;
  };


/************************************************************************
 * FUNCTIONS                                                            *
 ************************************************************************/

  int16     IO_AddToDeviceList(
  int16 iDeviceNumber,
  int16 iProtocol,
  /* HANDLE hDevice,*/
  enum EDeviceType eDevType);

  int16     IO_UpdateDeviceList(
  int16 iDeviceNumber,
  int16 ProtocolNr);

  void      IO_DeviceListPurge(
  void);

  int16     IO_DeviceListCloseDevice(
  int16 iDeviceNumber);

  struct TDevice *IO_DeviceListNrGetDev(
  int16 iDeviceNumber);

  struct TDevice *IO_DeviceListPortGetDev(
  int16 iDeviceNumber /*HANDLE hPort */ );



#ifdef __cplusplus
}
#endif                          /* __cplusplus */


#endif
