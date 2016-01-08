/*****************************************************************************/
/*                                                                           */
/* File       : atd_pipe.h                                                   */
/*                                                                           */
/* Start date : 2000-10-18                                                   */
/*                                                                           */
/* Author     : AU-System Lars Eberson (lars.eberson@ausys.se)               */
/*                                                                           */
/* Version    : 0.0                                                          */
/*                                                                           */
/* Description: Header file for AutoTest DRIVER SOCKET server                */
/*                                                                           */
/*****************************************************************************/


#ifndef _ATD_SOCKET_H_
#define _ATD_SOCKET_H_

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/* Type definitions                                                          */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/* Constants                                                                 */
/*                                                                           */
/*****************************************************************************/
#define ATD_SOCKET_OK         ((int16)  0)
#define ATD_SOCKET_ERROR      ((int16) -1)


/*****************************************************************************/
/*                                                                           */
/* External references                                                       */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/* Variables                                                                 */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/* Functions                                                                 */
/*                                                                           */
/*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/* SOCKET_Init, creates the Mutex needed for the administration and starts  */
/*   Windows Sockets.                                                       */
/* ------------------------------------------------------------------------ */
/* Input       :                                                            */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value:                                                            */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
extern void SOCKET_Init( void );

/* SOCKET_Open Updates the administration and when the socket is a client   */
/*               it will try to connect.                                    */
/* ------------------------------------------------------------------------ */
/* Input       :                                                            */
/*   DeviceNumber - The number of the device provided by the                */
/*               Kernel.                                                    */
/*   Settings - Settings of the socket format:                              */
/*              <client/server> <ip address/hostname> <port number>         */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value:                                                            */
/*   int16 - ATD_SOCKET_OK, when successfull, ATD_SOCKET_ERROR otherwise    */
/* --                                                                       */
/* Remarks:                                                                 */
/*   Server sockets are administrated but not started. The server is        */ 
/*   started after ConfigurationDone is invoked, because the server does not*/
/*   know how many connections is must accept at this point.                */
/* ------------------------------------------------------------------------ */
extern int16 SOCKET_Open( int16 iDeviceNumber, char *pcSettings );


/* SOCKET_Close closes the socket corresponding to the DeviceNumber         */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*   DeviceNumber - the device to which the data is sent.                   */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value:                                                            */
/*   int16 - ATD_SOCKET_OK, when successfull, ATD_SOCKET_ERROR otherwise    */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
extern int16 SOCKET_Close( int16 iDeviceNumber );


/* SOCKET_SendData sends data to the specified DeviceNumber.                */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*   DeviceNumber - the device to which the data is sent.                   */
/*   ByteCount    - Number of bytes to send                                 */
/*   Data         - Data to transmit                                        */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value:                                                            */
/*   int16 - ATD_SOCKET_OK, when successfull, ATD_SOCKET_ERROR otherwise    */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
extern int16 SOCKET_SendData( int16  iDeviceNumber,
							                int16	 iByteCount,
                              uint8 *pData );


/* SOCKET_StartConfiguration starts all of the TCP/IP servers sepcified in  */
/* the IO file	                                                            */
/* ------------------------------------------------------------------------ */
/* Input parameters : None                                                  */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value:                                                            */
/* None                                                                     */
/* --                                                                       */
/* Remarks:                                                                 */
/* This function is intended to be called from the kernel, to start the     */
/* TCP/IP servers specified in the IO file                                  */
/* ------------------------------------------------------------------------ */
extern int16 SOCKET_StartConfiguration( void );

#ifdef __cplusplus
extern "C"
}
#endif

#endif /* _ATD_SOCKET_H_ */
