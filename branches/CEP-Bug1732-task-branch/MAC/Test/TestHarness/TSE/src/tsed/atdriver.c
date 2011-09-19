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
// Description   : This module is the main module of the AutoTest driver. Subject to
//                 change without notice
//
// Revisions:
//
// Date       Author                  Changes
// 03/10/2000 Lars Eberson            Initial release
//
//=========================================================================
*/


/* ------------------------------------------------------------------------ */
/* INCLUDE FILES                                                            */
/* ------------------------------------------------------------------------ */

#include <stdio.h>

#include "codingstandard.h"
#include "atdriver.h"
#include "protocollist.h"
#include "devicelist.h"
#include "receivebuffer.h"
#ifdef WIN32
#include "atd_pipe.h"
#include "atd_rs232.h"
#include "atd_usb.h"
#endif
#include "atd_socket.h"
#include "atkernel.h"
#include "bsegui.h"


        /* ------------------------------------------------------------------------ */
        /* LOCAL VARIABLES                                                          */

        /* ------------------------------------------------------------------------ */
        /* LOCAL FUNCTIONS HEADERS                                                  */
int16     LOC_AddDeviceToList(
  int16 iDeviceNumber,
  int16 iProtocolNumber,
  /*HANDLE hDevice, */
  enum EDeviceType eDevType);


    /* ------------------------------------------------------------------------ */
    /* FUNCTIONS                                                                */


    /* BSED_ShutDown will free all allocated memory and close all ports.        */
    /* ------------------------------------------------------------------------ */
    /* Input parameters:                                                        */
    /* Description:        Clears the Device List and the Protocol List         */
    /*                     and closes all ports.                                */
    /* Output parameters:                                                       */
    /* --                                                                       */
    /* Return value:                                                            */
    /* An enumerated value as defined in "result codes".                        */
    /* ------------------------------------------------------------------------ */
int16 BSED_ShutDown(
  void)
{
  /* Remove and free all Devices  */
  IO_DeviceListPurge();

  /* Remove and free all Protocols        */
  IO_ProtocolListPurge();

  /* No result from functions is returned and none is wanted      */
  /* so we asume that all went well.                              */
  return OK;
}


    /* BSED_Init will initialize the driver component.                          */
    /* ------------------------------------------------------------------------ */
    /* Input parameters:                                                        */
    /* --                                                                       */
    /* Output parameters:                                                       */
    /* --                                                                       */
    /* Return value:                                                            */
    /* --                                                                       */
    /* ------------------------------------------------------------------------ */
void BSED_Init(
  void)
{
#ifdef WIN32  
  USB_Init();
#endif  
  SOCKET_Init();
  /* BSED_InitSelf(); */
  /* BSED_ConnectToServer(); */
}                               /* BSED_Init */

    /* BSED_CloseDevice will close the divice and free allocated memory         */
    /* ------------------------------------------------------------------------ */
    /* Input parameters:                                                        */
    /* iDeviceNumber           : An integer that describes the the device that  */
    /*                           will be closed                                 */
    /* Description:            : The given device will be closed and its buffer */
    /*                           will be freed.                                 */
    /* Output parameters:                                                       */
    /* --                                                                       */
    /* Return value:                                                            */
    /* An enumerated value as defined in "result codes".                        */
    /* ------------------------------------------------------------------------ */
int16 BSED_CloseDevice(
  int16 iDeviceNumber)
{
  int16     iResult;
  struct TDevice *ptDevice;

  ptDevice = IO_DeviceListNrGetDev(iDeviceNumber);
  if (ptDevice != NULL)
  {
    if (IO_BufferDeleteEntry(ptDevice->iDeviceNumber /*hDevice */ ) !=
        BUFFER_OK)
      iResult = OTHER_PROBLEMS;
    else
      switch (IO_DeviceListCloseDevice(ptDevice->iDeviceNumber))
      {
        case DEVICE_OK:
          iResult = OK;
          break;
        default:
          iResult = OTHER_PROBLEMS;
          break;
      };
  }
  else
    iResult = OTHER_PROBLEMS;

  return iResult;
}


    /* BSED_FlushBuffer will flush the buffer of the divice                     */
    /* ------------------------------------------------------------------------ */
    /* Input parameters:                                                        */
    /* iDeviceNumber           : An integer that describes the the device that  */
    /*                           will be flushed                                */
    /* Description:            : The buffer of the given device will be flushed */
    /* Output parameters:                                                       */
    /* --                                                                       */
    /* Return value:                                                            */
    /* An enumerated value as defined in "result codes".                        */
    /* ------------------------------------------------------------------------ */
int16 BSED_FlushBuffer(
  int16 iDeviceNumber)
{
  int16     iResult;
  struct TDevice *ptDevice;

  ptDevice = IO_DeviceListNrGetDev(iDeviceNumber);
  if (ptDevice != NULL)
  {
    switch (IO_BufferFlush(ptDevice->iDeviceNumber /*hDevice */ ))
    {
      case BUFFER_OK:
        iResult = OK;
        break;
      default:
        iResult = OTHER_PROBLEMS;
        break;
    };
  }
  else
    iResult = OTHER_PROBLEMS;
  return iResult;
}


  /* BSED_PIPE_SetDevice will initialize a pipe and give it the given number. */
  /* ------------------------------------------------------------------------ */
  /* Input parameters:                                                        */
  /* iDeviceNumber           : The number the pipe will get. This number is   */
  /*                           in the kernel known as a specific device. The  */
  /*                           GUI got this number from the kernel, the user  */
  /*                           linked a pipe to it, and this function will    */
  /*                           open the pipe for the given device number.     */
  /* ... ???                 : To be defined parameters, indicating which     */
  /*                           pipe is going to be used. I don't have a clue  */
  /*                           about how to address a pipe.                   */
  /* Output parameters:                                                       */
  /* --                                                                       */
  /* Return value:                                                            */
  /* An enumerated value as defined in "result codes".                        */
  /* ------------------------------------------------------------------------ */
  int16 BSED_PIPE_SetDevice(
    int16 iDeviceNumber,
    int16 iProtocolNumber,
    char *pPipeName)
  {
#ifdef WIN32  
    HANDLE    hPort = NULL;
  
    hPort = PIPE_Open(pPipeName);
    if (hPort == NULL)
      return DEVICE_COULD_NOT_BE_OPENED;
  
    return LOC_AddDeviceToList(iDeviceNumber, iProtocolNumber, /*hPort, */ PIPE);
#else    
    return DEVICE_COULD_NOT_BE_OPENED;
#endif
  }

  /* BSED_RS232_SetDevice will initialize a comport and give it the given     */
  /* number.                                                                  */
  /* ------------------------------------------------------------------------ */
  /* Input parameters:                                                        */
  /* iDeviceNumber           : The number the comport will get. This number   */
  /*                           is in the kernel known as a specific device.   */
  /*                           The GUI got this number from the kernel, the   */
  /*                           user linked a comport to it, and this function */
  /*                           will open the comport for the given device     */
  /*                           number.                                        */
  /* iPortNumber             : Integer defining which comport is going to be  */
  /*                           used. (COM1 .. COMxx, how many comports are    */
  /*                           available.                                     */
  /* Output parameters:                                                       */
  /* --                                                                       */
  /* Return value:                                                            */
  /* An enumerated value as defined in "Result Codes".                        */
  /* ------------------------------------------------------------------------ */
  int16 BSED_RS232_SetDevice(
    int16 iDeviceNumber,
    int16 iProtocolNumber,
    int16 iPortNumber)
  {
#ifdef WIN32  
    HANDLE    hPort = NULL;
  
    /* hPort = RS232_OpenPort(iPortNumber);
       if(hPort == NULL)
       return DEVICE_COULD_NOT_BE_OPENED; */
  
  
    return LOC_AddDeviceToList(iDeviceNumber, iProtocolNumber, /*hPort, */ RS232);
#else    
       return DEVICE_COULD_NOT_BE_OPENED;
#endif
  }

  /* BSED_USB_SetDevice will open a specified driver instance and give it the */
  /* given device number.                                                     */
  /* ------------------------------------------------------------------------ */
  /* Input parameters:                                                        */
  /* iDeviceNumber           : The number the USB device will get. This       */
  /*                           number is in the kernel known as a specific    */
  /*                           device. The GUI got this number from the       */
  /*                           kernel, the user linked a USB device to it.    */
  /*                           This function will open the USB driver for the */
  /*                           given device number.                           */
  /* iInstanceNumber         : The instance of the USB driver to open.        */
  /* Output parameters:                                                       */
  /* --                                                                       */
  /* Return value:                                                            */
  /* An enumerated value as defined in "result codes".                        */
  /* ------------------------------------------------------------------------ */
  int16 BSED_USB_SetDevice(
    int16 iDeviceNumber,
    int16 iProtocolNumber,
    int16 iInstanceNumber)
  {
#ifdef WIN32
    HANDLE    hUsbDevice;
  
    hUsbDevice = USB_OpenDevice(iInstanceNumber);
    if (hUsbDevice == NULL)
      return DEVICE_COULD_NOT_BE_OPENED;
  
    return LOC_AddDeviceToList(iDeviceNumber, iProtocolNumber,    /*hUsbDevice, */
                               USB);
#else                               
    return DEVICE_COULD_NOT_BE_OPENED;
#endif
  }                               /* BSED_USB_SetDevice */

#ifdef _NO_CONTROL_PANEL_
/* BSED_SOCKET_SetDevice will initialize a socket and give it the given     */
/*  number.														                                      */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* iDeviceNumber           : The number that the socket will get.           */
/*                           This number is in the kernel known as a        */
/*                           specific device. The GUI got this number from  */ 
/*                           the kernel, the user linked a socket to it,    */
/*                           and this function will open the socket for the */
/*                           given device number.                           */
/* ... ???                 : To be defined parameters, indicating which     */
/*                           pipe is going to be used. I don't have a clue  */
/*                           about how to address a socket.                 */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* An enumerated value as defined in "result codes".                        */
/* ------------------------------------------------------------------------ */
  int16 BSED_SOCKET_SetDevice(	int16  iDeviceNumber,
        					            int16  iProtocolNumber,
        					            char  *pcSocketSettings )
  {
    int16	iResult;

    iResult = SOCKET_Open(iDeviceNumber,pcSocketSettings);

    if(ATD_SOCKET_ERROR == iResult)
	  {
        return DEVICE_COULD_NOT_BE_OPENED;
	  }
	  else
	  {
    	  return LOC_AddDeviceToList(iDeviceNumber, iProtocolNumber, SOCKETS);
	  }
  }
#endif

    /* BSED_SetProtocol will set the (basic) protocol type for the given device */
    /* ------------------------------------------------------------------------ */
    /* Input parameters:                                                        */
    /* iDeviceNumber           : The device number the protocol is meant for.   */
    /* iProtocol               : enumerated type as defined in "protocol codes" */
    /* iTokenLength            : The length of a pTerminateToken, if used.      */
    /* *pTerminateToken        : ptr to a string used as terminator in received */
    /*                           events. Only meaningfull if iProtocol is set   */
    /*                           to TOKEN_TERMINATED.                           */
    /* iMaxLength              : The maximum string length an incoming event    */
    /*                           will ever be. Needed to know in case of TOKEN_ */
    /*                           TERMINATED events.                             */
    /* Output parameters:                                                       */
    /* --                                                                       */
    /* Return value:                                                            */
    /* int16                   : enumerated type as defined in "protocol return */
    /*                           codes"                                         */
    /* Remarks:                                                                 */
    /* This function is intended to be called from the kernel, which retrieves  */
    /* the nessecary information from the specification file.                   */
    /* ------------------------------------------------------------------------ */
int16 BSED_SetProtocol(
  int16 iProtocolNumber,
  int16 iProtocol,
  int16 iTokenLength,
  char *pTerminateToken,
  int16 iMaxLength)
{
  if (IO_AddToProtocolList
      (iProtocolNumber, iProtocol, iTokenLength, pTerminateToken, iMaxLength))
    return OK;
  else
    return OTHER_PROBLEMS;
}

#ifdef _NO_CONTROL_PANEL_
    /* BSED_StartConfiguration notifies the socket driver that it can start the */
    /* TCP/IP servers                                                           */
    /* ------------------------------------------------------------------------ */
    /* Input parameters:  None                                                  */
    /* --                                                                       */
    /* Output parameters: None                                                  */
    /* --                                                                       */
    /* Return value:                                                            */
    /* int16                   : enumerated type as defined in "protocol return */
    /*                           codes"                                         */
    /* Remarks:                                                                 */
    /* This function is intended to be called from the kernel, to start the     */
    /* TCP/IP servers specified in the IO file                                  */
    /* ------------------------------------------------------------------------ */
    int16 BSED_StartConfiguration( void )
    {
      if (ATD_SOCKET_OK == SOCKET_StartConfiguration())
      {
        return OK;
      }
      else
      {
        return OTHER_PROBLEMS;
      }
    }
#endif /* _NO_CONTROL_PANEL_ */
       


    /* BSEG_SendString will send out the given data to the indicated device     */
    /* ------------------------------------------------------------------------ */
    /* Input parameters:                                                        */
    /* iDeviceNumber           : The device to which the data must be sent to.  */
    /* iByteCount              : The number of bytes that have to be sent.      */
    /* pData                   : ptr to the data to be sent.                    */
    /* Output parameters:                                                       */
    /* --                                                                       */
    /* Return value:                                                            */
    /* int16                   : enumerated type as defined in "result codes"   */
    /* Remarks:                                                                 */
    /* This function is intended to be called from the kernel. The io driver    */
    /* will add one or two bytes indicating the length, or add a terminating    */
    /* token to the string, dependent on the initialized protocol for the given */
    /* device.                                                                  */
    /* ------------------------------------------------------------------------ */
int16 BSED_SendString(
  int16 iDeviceNumber,
  int16 iByteCount,
  uint8 * pData)
{
#ifdef _NO_CONTROL_PANEL_
          int16	          iResult;
	        struct TDevice *ptDevice;

          ptDevice = IO_DeviceListNrGetDev(iDeviceNumber);
	        if (NULL != ptDevice)
	        {
		        switch (ptDevice->eDevType)
		        {
		        case PIPE:
			        return DRIVER_NOT_AVAILABLE;
              /*
			        iResult = PIPE_SendData(iDeviceNumber, iByteCount, pData);
			        if(iResult == ATD_PIPE_ERROR)
        		    return OTHER_PROBLEMS;
			        else
				        return OK;
                */
			        break;
		        case RS232:
			        return DRIVER_NOT_AVAILABLE;
              /*
			        iResult = RS232_SendData(iDeviceNumber, iByteCount, pData);
			        if(iResult == ATD_SERIAL_ERROR)
				        return OTHER_PROBLEMS;
			        else
				        return OK;
                */
			        break;
            case USB:
			        return DRIVER_NOT_AVAILABLE;
              /*
			        iResult = USB_SendData(iDeviceNumber, iByteCount, pData);
			        if(iResult != ATD_USB_OK)
				        return OTHER_PROBLEMS;
			        else
				        return OK;
			        break;
              */
            case SOCKETS:
			        iResult = SOCKET_SendData(iDeviceNumber, iByteCount, pData);
			        if(iResult != ATD_SOCKET_OK)
				        return OTHER_PROBLEMS;
			        else
				        return OK;
			        break;
		        default:
			        return DRIVER_NOT_AVAILABLE;
                break;
		        }
		        return OK;
	        }
	        else
          {
		        return DEVICE_COULD_NOT_BE_OPENED;
          }
#else
  BSEG_WriteData(iDeviceNumber, iByteCount, pData);
  return OK;
#endif /* _NO_CONTROL_PANEL_ */
}

  /* BSED_InternalEvent acts as an interface to trigger internal events.      */
  /* ------------------------------------------------------------------------ */
  /* Input parameters:                                                        */
  /* iDeviceNumber           : The device number for which a string is in-    */
  /*                           tended.                                        */
  /* pcData                  : ptr to the ascii string. The following ascii   */
  /*                           strings are currently defined:                 */
  /*                           SOCKET_GENERAL_ERROR                           */
  /*                           SOCKET_SEND_ERROR                              */
  /*                           SOCKET_RECEIVE_ERROR                           */
  /*                           SOCKET_CONNECT_ERROR                           */
  /*                           SOCKET_DISCONNECT_ERROR                        */
  /*                           SOCKET_ACCEPT_ERROR                            */
  /*                           SOCKET_UNKNOWN_ERROR                           */
  /*                           CONNECTION_LOST                                */
  /*                           CONNECTION_ESTABLISHED                         */
  /*                                                                          */
  /* --                                                                       */
  /* Remarks:                                                                 */
  /* This function just calls the corresponding atkernel function and passes  */
  /* the parameters.                                                          */
  /* ------------------------------------------------------------------------ */
  void      BSED_InternalEvent(
    int16 iDeviceNumber,
    char *pcData)
  {

    BSEK_InternalEvent( iDeviceNumber, pcData);
  }


    /* BSED_ReceiveData will accept the given data from the indicated device    */
    /* ------------------------------------------------------------------------ */
    /* Input parameters:                                                        */
    /* hComChannel             : Handle to the communication channel (device).  */
    /* iByteCount              : The number of bytes that have to be received.  */
    /* pData                   : ptr to the data to be received.                */
    /* Output parameters:                                                       */
    /* --                                                                       */
    /* Return value:                                                            */
    /* void                                                                     */
    /* Remarks:                                                                 */
    /* This function is intended to be called from the different driver         */
    /* components. The drivers call the function to send data received on the   */
    /* ports                                                                    */
    /*                                                                          */
    /* ------------------------------------------------------------------------ */
void IO_ReceiveData(
  int iDeviceNumber /*HANDLE hComChannel */ ,
  int16 iByteCount,
  uint8 * pData,
  int OSCrashDetected)
{
  int8      iResult;

  do
  {
    iResult = IO_BufferAdd(iDeviceNumber /*hComChannel */ , pData, iByteCount,
                           &OSCrashDetected);
  } while (iResult == BUFFER_BUSY);
}

    /************************************************************
     * Function: LOC_AddDeviceToList                            *
     * Description:                                             *
     * Local function that translates to IO_AddToDeviceList     *
     * -Adds an entry to the Device List.                       *
     * -Further AddOns appends to the end of the list.          *
     * -If the same Device number is added an error is returned *
     *                                                          *
     * Return value:                                            *
     * The funtion returns one of the defined return codes      *
     *                                                          *
     * Revision:                                                *
     * Date      Comment          Author                        *
     * 00-11-06  Created          AUS/Dennis Olausson           *
     *                                                          *
     ************************************************************/
int16 LOC_AddDeviceToList(
  int16 iDeviceNumber,
  int16 iProtocolNumber,
  /*HANDLE hDevice, */
  enum EDeviceType eDevType)
{
  int16     iResult, i2Result, i3Result;
  struct TDevice *tDevice;

  iResult =
    IO_AddToDeviceList(iDeviceNumber, iProtocolNumber, /*hDevice, */ eDevType);
  switch (iResult)
  {
      /* No matching ProtocolNumber */
    case DEVICE_ERROR_NO_PROTOCOL:
      return IMPOSSIBLE_PROTOCOL;
      break;
      /* A Device with that number is already registered */
    case DEVICE_ERROR_DEVICE_EXCIST:   /*  Change the protocol on that Device */
      i2Result = IO_UpdateDeviceList(iDeviceNumber, iProtocolNumber);
      switch (i2Result)
      {
          /* Device Excist -> No Device = Fatal Error! */
        case DEVICE_ERROR_NO_DEVICE:
          return OTHER_PROBLEMS;
          break;
          /* Tried to change to a nonexcisting Protocol */
        case DEVICE_ERROR_NO_PROTOCOL:
          return IMPOSSIBLE_PROTOCOL;
          break;
          /* The Protocol change succeded */
          /* Remove the old buffer and create a new one */
        case DEVICE_OK:
          tDevice = IO_DeviceListNrGetDev(iDeviceNumber);
          if (tDevice != NULL)
          {
            i3Result =
              IO_BufferDeleteEntry(tDevice->iDeviceNumber /*hDevice */ );
            if (i3Result <= 0)
            {
              i3Result = IO_BufferCreate(       /*tDevice->hDevice, */
                                          tDevice->ptProtocol->iMaxLength,
                                          tDevice->ptProtocol,
                                          tDevice->iDeviceNumber);
              if (i2Result <= 0)
                return OTHER_PROBLEMS;
              else
                return OK;
            }
            return OTHER_PROBLEMS;
          }
          break;
          /* Unknown Result Code = Fatal Error! */
        default:
          return OTHER_PROBLEMS;
          break;
      }
      /* Memory allocation failed = Fatal Error! */
    case DEVICE_ERROR_OUT_OF_MEMORY:
      return OTHER_PROBLEMS;
      break;
      /* Device successfully added */
      /* Create a buffer */
    case DEVICE_OK:
      tDevice = IO_DeviceListNrGetDev(iDeviceNumber);
      if (tDevice == NULL)
        return OTHER_PROBLEMS;
      i2Result = (int16) IO_BufferCreate(       /*tDevice->hDevice,*/
                                          tDevice->ptProtocol->iMaxLength,
                                          tDevice->ptProtocol,
                                          tDevice->iDeviceNumber);
      if (i2Result <= 0)
        return OTHER_PROBLEMS;
      return OK;
      break;
      /* Unknown Result Code = Fatal Error! */
    default:
      return OTHER_PROBLEMS;
      break;
  }

  return DRIVER_NOT_AVAILABLE;
}
