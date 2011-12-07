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
// 05/03/2001 E.A. Nijboer            
//
//=========================================================================
*/


#include <stdio.h>
#include <string.h>

#include "bsegui.h"
#include "stores.h"
#include "driver_configuration.h"
#include "atdriver.h"
#include "general_lib.h"
#include "parser_lib.h"

/****************************************************************************/
/* Local function headers .                                                 */
/****************************************************************************/
int16     ConfigureChannel(
  struct TIO *ptIO);
int16     ConfigureChannelToPort(
  struct TIO *ptIO);




/****************************************************************************/
/* Exported functions                                                       */
/****************************************************************************/
int16 ConfigureDriver(
  struct TIOList *ptIOList)
{
  struct TIOList *ptWalker;

  /* First configure all channel protocols                                 */
  ptWalker = ptIOList;
  while ((ptWalker != NULL) && (ptWalker->ptThis != NULL))
  {
    if (ptWalker->ptThis->iConfigured == FALSE)
    {
      if (!ConfigureChannel(ptWalker->ptThis))
        return FALSE;
    }
    ptWalker = ptWalker->ptNext;
  }

  /* When all channel protocols are defined, assign an IO port to them.    */
  ptWalker = ptIOList;
  while ((ptWalker != NULL) && (ptWalker->ptThis != NULL))
  {
    if (ptWalker->ptThis->iConfigured == FALSE)
    {
      if (!ConfigureChannelToPort(ptWalker->ptThis))
        return FALSE;
    }
    ptWalker = ptWalker->ptNext;
  }
  return TRUE;
}

/****************************************************************************/
/* Local functions                                                          */
/****************************************************************************/
int16 ConfigureChannel(
  struct TIO * ptIO)
{
  BSED_SetProtocol(ptIO->iNumber,
                   ptIO->iProtocol,
                   ptIO->iTerminateTokenLength,
                   ptIO->pTerminateToken, ptIO->iMaxTokenLength);
  return TRUE;
}


int16 ConfigureChannelToPort(
  struct TIO * ptIO)
{
  int16     iComPort;
  int16     iDevice;
  int16     iResult;

/*  pcLogLine = (char*)malloc(250); */

  if (strncmp(ptIO->pcIOPortName, "COM", 3) == 0)
  {
    /* It's a comport. Which comport is intended??                         */
    iComPort = GetInteger(ptIO->pcIOPortName + 3);

    if ((iComPort <= 0) || (iComPort > 16))
    {
      AddError2("Error: Comport %s not supported by BSE", ptIO->pcIOPortName);
      return FALSE;
    }
    else
    {
      iResult = BSED_RS232_SetDevice(ptIO->iNumber, ptIO->iNumber, iComPort);
      if (iResult == DEVICE_COULD_NOT_BE_OPENED)
      {
/*         sprintf(pcLogLine,"Warning: Comport %s could not be opened",ptIO->pcIOPortName); */
/*         BSEG_LogLine(pcLogLine); */
      }
    }
  }
  else if (strncmp(ptIO->pcIOPortName, "USB", 3) == 0)
  {

    /* It's an USB device. Which instance is intended ?? */
    iDevice = GetInteger(ptIO->pcIOPortName + 3);

    if ((iDevice < 1) || (iDevice > 127))
    {
      AddError2("Error: USB device %s not supported", ptIO->pcIOPortName);
      return FALSE;
    }
    else
    {
      BSED_USB_SetDevice(ptIO->iNumber, ptIO->iNumber, iDevice);
    }
  }
  else if (strncmp(ptIO->pcIOPortName, "PIPE", 3) == 0)
  {
    /* It's a named pipe.                                                   */
    BSED_PIPE_SetDevice(ptIO->iNumber, ptIO->iNumber, ptIO->pcIOPortName);
  }
  else if (strncmp(ptIO->pcIOPortName, "SOCKET", 3) == 0)
  {
    /* It's a socket.                                                   */
    BSED_SOCKET_SetDevice(ptIO->iNumber, ptIO->iNumber, ptIO->pComConfig);
  }

  return TRUE;
}
