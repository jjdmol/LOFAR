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
// Description   : This module is the header file of the AutoTest driver module. Subject to
//                 change with notice
//
// Revisions:
//
// Date       Author                  Changes
// 10/08/2000 Eilko Nijboer           Initial release
//
//=========================================================================
*/


#include "codingstandard.h"
#ifdef WIN32
#include "windows.h"
#endif

    /* ------------------------------------------------------------------------ */
    /* DEFINE macro's                                                           */

    /* result codes :                                                           */
#define  OK                           ((int16)  0 )
#define  DEVICE_COULD_NOT_BE_OPENED   ((int16) -1 )
#define  DRIVER_NOT_AVAILABLE         ((int16) -2 )
#define  OTHER_PROBLEMS               ((int16) -3 )
#define  DEVICE_BUSY                  ((int16) -4 )

    /* protocol result codes :                                                  */
#define  IMPOSSIBLE_PROTOCOL      ((int16) -4 )

    /* protocol codes :                                                         */
#define  TOKEN_TERMINATED         ((int16)  0 )
#define  ONE_LENGTH_BYTE          ((int16)  1 )
#define  TWO_LENGTH_BYTES_LO_HI   ((int16)  2 )
#define  TWO_LENGTH_BYTES_HI_LO   ((int16)  3 )
#define  FIXED_LENGTH             ((int16)  4 )
#define  PROT_PROTOCOL            ((int16)  5 )
#define  LOFAR                    ((int16)  6 )

#ifdef __cplusplus
extern    "C"
{
#endif

  /* ------------------------------------------------------------------------ */
  /* FUNCTION prototypes (in alphabetical order)                              */

  int16     BSED_PIPE_SetDevice(
  int16 iDeviceNumber,
  int16 iProtocolNumber,
  char *pPipeName);
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



  int16     BSED_RS232_SetDevice(
  int16 iDeviceNumber,
  int16 iProtocolNumber,
  int16 iPortNumber);
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
  /* An enumerated value as defined in "result codes".                        */
  /* ------------------------------------------------------------------------ */



  int16     BSED_ShutDown(
  void);
  /* BSED_ShutDown will close all open ports and free all allocated memory    */
  /* ------------------------------------------------------------------------ */
  /* Input parameters:                                                        */
  /*                                                                          */
  /* Output parameters:                                                       */
  /* --                                                                       */
  /* Return value:                                                            */
  /* An enumerated value as defined in "result codes".                        */
  /* ------------------------------------------------------------------------ */


  void      BSED_Init(
  void);
  /* BSED_Init will initialize the driver component.                          */
  /* ------------------------------------------------------------------------ */
  /* Input parameters:                                                        */
  /* --                                                                       */
  /* Output parameters:                                                       */
  /* --                                                                       */
  /* Return value:                                                            */
  /* --                                                                       */
  /* ------------------------------------------------------------------------ */


  int16     BSED_CloseDevice(
  int16 iDeviceNumber);
  /* BSED_CloseDevice will close the specific port and free allocated memory  */
  /* ------------------------------------------------------------------------ */
  /* Input parameters:                                                        */
  /* iDeviceNumber           : Integer defining the device which will be      */
  /*                           closed                                         */
  /*                                                                          */
  /* Output parameters:                                                       */
  /* --                                                                       */
  /* Return value:                                                            */
  /* An enumerated value as defined in "result codes".                        */
  /* ------------------------------------------------------------------------ */


  extern int16     BSED_FlushBuffer(
  int16 iDeviceNumber);
  /* BSED_FlushBuffer will flush the device buffer                            */
  /* ------------------------------------------------------------------------ */
  /* Input parameters:                                                        */
  /* iDeviceNumber           : Integer defining which device buffer will be   */
  /*                           flushed                                        */
  /*                                                                          */
  /* Output parameters:                                                       */
  /* --                                                                       */
  /* Return value:                                                            */
  /* An enumerated value as defined in "result codes".                        */
  /* ------------------------------------------------------------------------ */

extern void      BSED_InternalEvent(
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
  int16 iDeviceNumber,
  char *pcData);

#ifdef _NO_CONTROL_PANEL_
  /* BSED_SOCKET_SetDevice will initialize a socket and give it				*/
  /* the given number.														                    */
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
  extern int16 BSED_SOCKET_SetDevice(	int16  iDeviceNumber,
        					                    int16  iProtocolNumber,
        					                    char  *pcSocketSettings );
#endif /* _NO_CONTROL_PANEL_ */

  int16     BSED_USB_SetDevice(
  int16 iDeviceNumber,
  int16 iProtocolNumber,
  int16 iInstanceNumber);
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


  int16     BSED_SetProtocol(
  int16 iProtocolNumber,
  int16 iProtocol,
  int16 iTokenLength,
  char *pTerminateToken,
  int16 iMaxLength);
  /* BSEG_SetProtocol will set the (basic) protocol type for the given device */
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



  int16     BSED_SendString(
  int16 iDeviceNumber,
  int16 iByteCount,
  uint8 * pData);
  /* BSEG_SendString will send out the given data to the indicated device     */
  /* ------------------------------------------------------------------------ */
  /* Input parameters:                                                        */
  /* iDeviceNumber           : The device to which the data must be sent to.  */
  /* iByteCount              : The number of bytes that have to be sent.      */
  /* pData                   : ptr to the data to be sent.                    */
  /* --                                                                       */
  /* Output parameters:                                                       */
  /* --                                                                       */
  /* Return value:                                                            */
  /* int16                   : enumerated type as defined in "result codes"   */
  /* --                                                                       */
  /* Remarks:                                                                 */
  /* This function is intended to be called from the kernel. The io driver    */
  /* will add one or two bytes indicating the length, or add a terminating    */
  /* token to the string, dependent on the initialized protocol for the given */
  /* device.                                                                  */
  /* ------------------------------------------------------------------------ */



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
  extern int16 BSED_StartConfiguration( void );

  void      IO_ReceiveData(
  int iChannel /*HANDLE hComChannel */ ,
  int16 iByteCount,
  uint8 * pData,
  int OSCrashDetected);
  /* IO_ReceiveData will accept the given data from the indicated device      */
  /* ------------------------------------------------------------------------ */
  /* Input parameters:                                                        */
  /* hComChannel             : Handle to the communication channel (device).  */
  /* iByteCount              : The number of bytes that have to be received.  */
  /* OSCrashDetected         : To detect the MagicWord that is send when an   */
  /*                           OSECrash occour.                               */
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



#ifdef __cplusplus
}
#endif
