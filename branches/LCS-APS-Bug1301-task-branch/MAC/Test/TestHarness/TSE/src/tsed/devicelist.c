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
// Description   : This module is the implementation file for the Device List
//
// Revisions:
//
// Date       Author                  Changes
// 25/10/2000 Dennis Olausson         Initial release
//
//=========================================================================
*/


#ifdef __cplusplus
extern    "C"
{
#endif
#include <stdio.h>              /* For debug formatting */
#include <stdlib.h>

#ifdef WIN32
#include "windows.h"            /* For windows defines */
#include "atd_pipe.h"
#include "atd_rs232.h"
#include "atd_usb.h"
#endif

#include "codingstandard.h"     /* For common project defines */
#include "protocollist.h"       /* For Protocol functions */
#include "devicelist.h"

/************************************************************/
/* Local Variables                                          */
/************************************************************/
  static struct TDeviceList *ptListHead = NULL;

#ifdef _DEBUGGING_
  char      dbgstr[80];         /* string for debug messages */
#endif

/************************************************************
 * Local Function Declaration                               */
  void      LOC_DeleteDeviceList(
  struct TDeviceList *ptDeviceList);
  struct TDeviceList *LOC_DeleteDevice(
  struct TDeviceList *ptDeviceList,
  int16 iDeviceNumber);

/************************************************************
 * Exported Functions                                       *
 ************************************************************/

/************************************************************
 * Function: IO_AddToDeviceList                             *
 * Description:                                             *
 * -Adds an entry to the Device List.                       *
 * -Further AddOns appends to the end of the list.          *
 * -If the same Device number is added an error is returned *
 *                                                          *
 * Return value:                                            *
 * The funtion returns one of the defined return codes      *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-10-25    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  int16     IO_AddToDeviceList(
  int16 iDeviceNumber,
  int16 iProtocol,
  /* HANDLE hDevice, */
  enum EDeviceType eDevType)
  {
    struct TDeviceList *ptPresent;
    struct TDeviceList tNewEntry;
    struct TDeviceList *ptAppender;

    /*      Fill a new Entry structure */
              tNewEntry.tDevice.iDeviceNumber = iDeviceNumber;
    /* tNewEntry.tDevice.hDevice = hDevice;*/
              tNewEntry.tDevice.ptProtocol = IO_ProtocolListFind(iProtocol);
              tNewEntry.tDevice.eDevType = eDevType;
              tNewEntry.ptNext = NULL;

    if        (
  tNewEntry.tDevice.ptProtocol == NULL)
      return DEVICE_ERROR_NO_PROTOCOL;

/*        Get hold of the begining of the list entry        */
    if        (
  ptListHead != NULL)
    {
      ptPresent = ptListHead;

      /* If the Device is already defined, generate error! */
      if (
  ptPresent->tDevice.iDeviceNumber == tNewEntry.tDevice.iDeviceNumber)
        return DEVICE_ERROR_DEVICE_EXCIST;
      else
      {                         /* Find the last entry */
        while (ptPresent->ptNext != NULL)
        {
          ptPresent = ptPresent->ptNext;
          /* If the Device is already defined, generate error! */
          if (
  ptPresent->tDevice.iDeviceNumber == tNewEntry.tDevice.iDeviceNumber)
            return DEVICE_ERROR_DEVICE_EXCIST;
        }
        /* The end is found! */
        /* New entry, append it to the end of the list */
        ptAppender = (struct TDeviceList *) malloc(sizeof(struct TDeviceList));

        if (ptAppender != NULL)
        {
          ptAppender->tDevice = tNewEntry.tDevice;
          ptAppender->ptNext = tNewEntry.ptNext;
          ptPresent->ptNext = ptAppender;
          return DEVICE_OK;     /* Protocol Successfully added. */
        }
        else
          return DEVICE_ERROR_OUT_OF_MEMORY;    /* malloc failed */
      }
    }
    /* The list is empty append the new entry to the head */
    else
    {
      ptAppender = (struct TDeviceList *) malloc(sizeof(struct TDeviceList));
      if (ptAppender != NULL)
      {
        ptAppender->tDevice = tNewEntry.tDevice;
        ptAppender->ptNext = tNewEntry.ptNext;
        ptListHead = ptAppender;
        return DEVICE_OK;       /* Protocol Successfully added. */
      }
      else
        return DEVICE_ERROR_OUT_OF_MEMORY;      /* malloc failed. */
        
    }
  }


/************************************************************
 * Function: IO_UpdateDeviceList                            *
 * Description:                                             *
 * -Change the Protocol on an excisting Device.             *
 * -Further AddOns appends to the end of the list.          *
 * -If the Device isn't found an error is returned          *
 *                                                          *
 * Return value:                                            *
 * The funtion returns one of the defined return codes      *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-10-25    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  int16     IO_UpdateDeviceList(
  int16 iDeviceNr,
  int16 iProtocolNr)
  {
    struct TDeviceList *ptPresent;
    struct TProtocol *ptProtocol;

    ptPresent = ptListHead;
    if (ptPresent != NULL)
    {                           /* Find the Device */
      while ((ptPresent->tDevice.iDeviceNumber != iDeviceNr) &&
             (ptPresent != NULL))
        ptPresent = ptPresent->ptNext;

      if (ptPresent == NULL)    /* No matching Devices */
        return DEVICE_ERROR_NO_DEVICE;

      ptProtocol = IO_ProtocolListFind(iProtocolNr);
      if (ptProtocol == NULL)   /* No matching Protocol */
        return DEVICE_ERROR_NO_PROTOCOL;

      ptPresent->tDevice.ptProtocol = ptProtocol;
      return DEVICE_OK;         /* Device uppdated! */
    }
    else
      return DEVICE_ERROR_NO_DEVICE;
  }


/************************************************************
 * Function: IO_DeviceListPurge                             *
 * Description:                                             *
 * Deletes and clean up the entire list                     *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-10-25    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  void      IO_DeviceListPurge(
  void)
  {
    /* Remove every object until the end of the list */
    LOC_DeleteDeviceList(ptListHead);
    ptListHead = NULL;
  }


/************************************************************
 * Function: IO_DeviceListCloseDevice                       *
 * Description:                                             *
 * Close the given device and free its buffer               *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-12-06    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  int16     IO_DeviceListCloseDevice(
  int16 iDeviceNumber)
  {
    if (ptListHead != NULL)
    {
#ifdef _DEBUGGING_
      OutputDebugString("IO_DeviceList: Close Device!\n");
#endif
      ptListHead = LOC_DeleteDevice(ptListHead, iDeviceNumber);
      return DEVICE_OK;
    }
    else
    {
#ifdef _DEBUGGING_
      OutputDebugString("IO_DeviceList: Device List Empty!\n");
#endif
      return DEVICE_ERROR;
    }
  }






/************************************************************
 * Function: IO_DeviceListNrGetDev                          *
 * Description:                                             *
 * Search for the Device with the given Device Number.      *
 *                                                          *
 * Return value:                                            *
 * The funtion returns a pointer to the resulting           *
 * TDevice structure.                                       *
 * If the list is empty, there is no match or any other     *
 * error occured, NULL is returned.                         *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-10-26    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  struct TDevice *IO_DeviceListNrGetDev(
  int16 iDeviceNr)
  {
    struct TDeviceList *ptPresent;

    if (ptListHead != NULL)     /* Empty list? */
    {
      ptPresent = ptListHead;
      /* Search for the corresponding Device Number   */
      while ((iDeviceNr != ptPresent->tDevice.iDeviceNumber) &&
             (ptPresent->ptNext != NULL))
        ptPresent = ptPresent->ptNext;
      if (iDeviceNr != ptPresent->tDevice.iDeviceNumber)
        return NULL;            /* Didn't find the Device Number, return NULL */
      else
        return &ptPresent->tDevice;     /* Found the DeviceNumber, return the adress */
    }                           /* of the Device structure */
    else
      return NULL;
  }


/************************************************************
 * Function: IO_DeviceListDevGetDev                         *
 * Description:                                             *
 * Search for the Device with the given Device Handle.      *
 *                                                          *
 * Return value:                                            *
 * The funtion returns a pointer to the resulting           *
 * TDevice structure.                                       *
 * If the list is empty, there is no match or any other     *
 * error occured, NULL is returned.                         *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-10-26    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  struct TDevice *IO_DeviceListPortDevGetDev(
  int16 iDeviceNumber /*HANDLE hDevice */ )
  {
    struct TDeviceList *ptPresent;

    ptPresent = ptListHead;
    if (ptPresent != NULL)      /* Empty list? */
    {
      /* Search for the corresponding Device Handle   */
      while ((iDeviceNumber != ptPresent->tDevice.iDeviceNumber) &&
             (ptPresent->ptNext != NULL))
        ptPresent = ptPresent->ptNext;
      if (iDeviceNumber != ptPresent->tDevice.iDeviceNumber)
        return NULL;            /* Didn't find the Device Handle return NULL */
      else
        return &ptPresent->tDevice; /* Found the Device Handle, return the adress */
    }                           /* of the Device structure */
    else
      return NULL;
  }


/************************************************************
 * Local Functions                                          *
 ************************************************************/

/************************************************************
 * Local Function: LOC_DeleteDeviceList                     *
 * Description:                                             *
 * Recursive Function that deletes and clean up an entry    *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-10-25    Created                  AUS/Dennis Olausson *
 * 00-12-06 Changed name from:                              *
 *            LOC_DeleteDeviceEntry                         *
 *            To:                                           *
 *            LOC_DeleteDeviceList    AUS/Dennis Olausson   *
 *                                                          *
 ************************************************************/
  void      LOC_DeleteDeviceList(
  struct TDeviceList *ptDeviceList)
  {
    if (ptDeviceList != NULL)
    {
      LOC_DeleteDeviceList(ptDeviceList->ptNext);
      /* Close the corresponding port */
      switch (ptDeviceList->tDevice.eDevType)
      {
        case PIPE:
          /* PIPE_Close(ptDeviceList->tDevice.iDeviceNumber); */
          break;
        case RS232:
          /* RS232_ClosePort(ptDeviceList->tDevice.iDeviceNumber); */
          break;
        case USB:
          /* USB_CloseDevice(ptDeviceList->tDevice.iDeviceNumber); */
          break;
        default:
          break;
      };
      free(ptDeviceList);
      ptDeviceList = NULL;
    }
  }


/************************************************************
 * Local Function: LOC_DeleteDevice                         *
 * Description:                                             *
 * Recursive Function that finds and deletes a device       *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-12-06    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  struct TDeviceList *LOC_DeleteDevice(
  struct TDeviceList *ptDeviceList,
  int16 iDeviceNumber)
  {
    struct TDeviceList *tReturnList;

    /* Has the list ended */
    if (ptDeviceList != NULL)
    {
      /* Search for the correct device        */
      if (ptDeviceList->tDevice.iDeviceNumber == iDeviceNumber)
      {
#ifdef _DEBUGGING_
        sprintf(dbgstr, "IO_DeviceList: LOC_DD: Device: %d found\n",
                iDeviceNumber) OutputDebugString(dbgstr);
#endif
        /* Close the correct type of port       */
        switch (ptDeviceList->tDevice.eDevType)
        {
          case PIPE:
            /* PIPE_Close(ptDeviceList->tDevice.iDeviceNumber); */
            break;
          case RS232:
            /*RS232_ClosePort(ptDeviceList->tDevice.iDeviceNumber);*/
            break;
          default:
#ifdef _DEBUGGING_
            OutputDebugString("IO_DeviceList: LOC_DD: Unknown Device!\n");
#endif
            break;
        };

        /* Return the next pointer to tie the list together     */
        tReturnList = ptDeviceList->ptNext;
        /* Free the Device      */
        free(ptDeviceList);
#ifdef _DEBUGGING_
        OutputDebugString("IO_DeviceList: LOC_DD: Device deleted!\n");
#endif
      }
      else
      {
#ifdef _DEBUGGING_
        OutputDebugString("IO_DeviceList: LOC_DD: Device not found!\n");
#endif
        /* Device not found, check next device  */
        ptDeviceList->ptNext =
          LOC_DeleteDevice(ptDeviceList->ptNext, iDeviceNumber);
        tReturnList = ptDeviceList;
      }
      return tReturnList;
    }
#ifdef _DEBUGGING_
    OutputDebugString("IO_DeviceList: LOC_DD: End of List!\n");
#endif
    return NULL;
  }


#ifdef __cplusplus
}
#endif /* __cplusplus */
