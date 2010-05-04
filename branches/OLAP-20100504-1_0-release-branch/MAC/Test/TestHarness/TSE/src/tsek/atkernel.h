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
// 29/08/2000 E.A. Nijboer            Initial release
// 12/01/2001 E.A. Nijboer            platform test.
// 09/02/2001 E.A. Nijboer            
//
//=========================================================================
*/

#ifndef _ATKERNEL_H_
#define _ATKERNEL_H_

#ifdef __cplusplus
extern    "C"
{
#endif


#include "codingstandard.h"

/* ------------------------------------------------------------------------ */
/* DEFINE macro's                                                           */

/* return values for open file functions                                    */
#define BSEK_OK                    ((int16)   0)
#define BSEK_FILE_CANNOT_BE_OPENED ((int16)  -1)
#define BSEK_SCRIPT_ERROR          ((int16)  -2)
#define BSEK_NO_SPECIFICATION      ((int16)  -3)
#define BSEK_KERNEL_BUSY           ((int16)  -4)

/* return values for reset script function                                  */
#define BSEK_SCRIPT_RUNNING        ((int16)  -5)
#define BSEK_BATCHFILE_RUNNING     ((int16)  -6)
#define BSEK_NO_SCRIPT_LOADED      ((int16)  -7)
#define BSEK_NO_BATCH_LOADED       ((int16)  -8)

/* return values for start script/batch function                            */
#define BSEK_SCRIPT_ALREADY_RUNS   ((int16)   9)        /* positive ! */
#define BSEK_BATCH_ALREADY_RUNS    ((int16)  10)        /* positive ! */
#define BSEK_NO_SCRIPT_TO_RUN      ((int16) -11)
#define BSEK_NO_BATCH_TO_RUN       ((int16) -12)

#define BSEK_OTHER_ERROR           ((int16) -99)

/* logging modes:                                                           */
#define BSEK_NO_LOGGING            ((int16) 0x0000 )
#define BSEK_DEFAULT_LOGGING       ((int16) 0x0000 )
#define BSEK_LOG_STATE_TRANSITIONS ((int16) 0x0001 )
#define BSEK_LOG_TIME_STAMPS       ((int16) 0x0002 )    /* obsolete !! */
#define BSEK_LOG_EXTENSIVE         ((int16) 0x0004 )
#define BSEK_LOG_STRING            ((int16) 0x0008 )
#define BSEK_TIMESTAMPS_MASK       ((int16) 0x0030 )
#define BSEK_LOG_SHORT_TIMESTAMPS  ((int16) 0x0010 )
#define BSEK_LOG_FULL_TIMESTAMPS   ((int16) 0x0020 )

/* overview modes                                                           */
#define BSEK_PROCESSED_FILES       ((int16) 0)
#define BSEK_STATEMACHINES_HEADERS ((int16) 1)
#define BSEK_STATEMACHINES_FULL    ((int16) 2)

/* step mode                                                                */
#define STEP_UNINITIALISED         ((int16) -1)
#define STEP_FINISH_OK             ((int16)  0)
#define STEP_FINISH_ERROR          ((int16)  1)
#define STEP_FINISH_WARNING        ((int16)  2)



/* ------------------------------------------------------------------------ */
/* TYPEDEF definitions. Used structures in exchange buffers.                */


/* object name list                                                         */
  struct BSEK_ObjectName
  {
    char     *pcObjectName;
    struct BSEK_ObjectName *ptNext;
  };


/* ------------------------------------------------------------------------ */
/* FUNCTION prototypes (in alphabetical order)                              */

  char     *BSEK_CreateHTML(
  char *pcFileName,
  struct BSEK_ObjectName *ptObjects,
  int16 bWithData,
  int16 iCharacterSize);
/* BSEK_CreateHTML will create an HTML file, based on the given logfile and  */
/* the given sequence of objects, and other parameters.						 */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* pFileName               : ptr to string containing the log-file from     */
/*                           which the object names are retrieved.          */
/* *pObjects               : ptr to list of object names, defining the de-  */
/*                           sired sequence of objects in the to be gene-   */
/*                           rated MSC.                                     */
/* bWithData               : TRUE : The to be generated MSC will also con-  */
/*                                  tain the data in the body of each log-  */
/*                                  ged message.                            */
/*                           FALSE: The to be generated MSC will contain    */
/*                                  no additional data.                     */
/* iCharacterSize          : integer indicating the size of the characters  */
/*                           in the to be generated MSC.                    */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* char *                  : ptr to string containing the name of the       */
/*                           generated file.                                */
/* Remarks:                                                                 */
/* The GUI might want to start the default HTML viewer to display the       */
/* generated file?                                                          */
/* ------------------------------------------------------------------------ */



  void      BSEK_ErrorPosition(
  char **ppcError);
/* BSEK_ErrorPosition describes the position where an error is found during  */
/* reading an input file.                                                   */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* ppError                 : ptr to ptr to a string containing an easy      */
/*                           readable string. The kernel will allocate      */
/*                           memory for this string. The caller of this     */
/*                           function is responsible for freeing the memory */
/*                           block.                                         */
/* Return value:                                                            */
/* --                                                                       */
/* Remarks:                                                                 */
/* When during reading an input file, whether this is a specification file, */
/* a script file or a batch file, errors can occur. An error will be        */
/* reported by a returned error code, a detailled description of this error */
/* can be obtained using this function. This description is formatted with  */
/* cr/lf characters.                                                        */
/* An example of such a string can be:                                      */
/*                                                                          */
/*      Error reading file D:\scripts\file
/*      Included by file D:\scripts\test_1.ats, line 112                    */
/*      Included by file D:\scripts\big_test.ats, line 15                   */
/*                                                                          */
/*      Found token : (                                                     */
/*      Expected token : {                                                  */
/*                                                                          */
/* ------------------------------------------------------------------------ */



  void      BSEK_Init(
  void);
/* BSEK_Init intialises the KERNEL. This function must be called before any  */
/* function can be called.                                                  */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return parameters:                                                       */
/* ------------------------------------------------------------------------ */



  struct BSEK_ObjectName *BSEK_GetObjectNames(
  char *pcFileName);
/* BSEK_GetObjectNames returns a list of object names from a log-file.       */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* char *pFileName         : ptr to a zero-terminated string, containing    */
/*                           a filename. This string must meet the same     */
/*                           requirements as the filename string used in    */
/*                           the standard fopen function. This file will    */
/*                           be opened to read a log file from.             */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* struct BSEK_ObjectName * : ptr to a list of object names. If the log file */
/*                           contains no object names, a NULL pointer will  */
/*                           be returned.                                   */
/*                           The kernel will allocate memory for the        */
/*                           records, the caller of this function will be   */
/*                           responsible for freeing the list.              */
/* ------------------------------------------------------------------------ */



  int16     BSEK_LoadBatch(
  char *pcFileName);
/* BSEK_LoadBatch loads a test batch file.                                   */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* char *pFileName         : ptr to a zero-terminated string, containing    */
/*                           a filename. This string must meet the same     */
/*                           requirements as the filename string used in    */
/*                           the standard fopen function. This file will    */
/*                           be opened to read a batch file from.           */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for open file functions"                       */
/* ------------------------------------------------------------------------ */

  int16 BSEK_LoadIOFile       ( char                          *pcFileName);
/* BSEK_LoadIOFile loads the IO file. This file configures                  */
/* the communication module between AutoTest and the Software Under Test.   */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* char *pcFileName        : ptr to a zero-terminated string, containing    */
/*                           a filename. This string must meet the same     */
/*                           requirements as the filename string used in    */
/*                           the standard fopen function.                   */
/* Output parameters       : None                                           */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for open file functions"                       */
/* ------------------------------------------------------------------------ */

  int16     BSEK_LoadScript(
  char *pcFileName);
/* BSEK_LoadScript loads a test script. This testscript can contain a list   */
/* of device names which the user can map to communication ports.           */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* char *pFileName         : ptr to a zero-terminated string, containing    */
/*                           a filename. This string must meet the same     */
/*                           requirements as the filename string used in    */
/*                           the standard fopen function. This file will    */
/*                           be opened to read a test script from.          */
/* Output parameters:                                                       */
/* ppDDL                   : ptr to ptr to a (list of) device descriptors.  */
/*                           If the script does not contain a list of       */
/*                           devices, a ptr to a NULL ptr will be returned. */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for open file functions"                       */
/* ------------------------------------------------------------------------ */



  int16     BSEK_LoadSpecification(
  char *pcFileName);
/* BSEK_LoadSpecification loads the specification file. This file configures */
/* the communication module, and defines all possible events, commands      */
/* and parameters possible between AutoTest and the Software Under Test.    */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* char *pcFileName        : ptr to a zero-terminated string, containing    */
/*                           a filename. This string must meet the same     */
/*                           requirements as the filename string used in    */
/*                           the standard fopen function.                   */
/* Output parameters:                                                       */
/* **ppCDL                 : ptr to a list of IO Channel names. The list of */
/*                           channel names is found in the specification    */
/*                           file.                                          */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for open file functions"                       */
/* ------------------------------------------------------------------------ */



  int16     BSEK_OpenLogFile(
  char *pcFileName,
  int16 iReplay);
/* BSEK_OpenLogFile opens a logfile, to which exactly the same logging as    */
/* sent to the GUI by the BSEG_LogLine function.                             */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/*               Replay - indicates if a scripy file is to be replayed(1) or*/ 
/*               not(0).                                                    */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for reset script/batch function"               */
/* ------------------------------------------------------------------------ */



  void      BSEK_ReceiveString(
  int16 iDeviceNumber,
  int16 iLength,
  char *pcData);
/* BSEK_ReceiveString will enable the kernel to react on incoming strings.   */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* iDeviceNumber           : The device number from which a string is re-   */
/*                           ceived.                                        */
/* iLength                 : The amount of bytes received.                  */
/* pData                   : ptr to the received data. The kernel will free */
/*                           this memory block after processing.            */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* --                                                                       */
/* Remarks:                                                                 */
/* This function should not be called from the GUI. The I/O driver can use  */
/* this function to pass received data to the kernel.                       */
/* ------------------------------------------------------------------------ */

  void      BSEK_InternalEvent(
  int16 iDeviceNumber,
  char *pcData);
/* BSEK_InternalEvent acts as an interface to trigger internal events.      */
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
/*																		    */
/* ------------------------------------------------------------------------ */

  int16     BSEK_ResetScript(
  void);
/* BSEK_ResetScript sets back a script in its initial state.                 */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for reset script function"                     */
/* Remarks:                                                                 */
/* Resetting means that all variables are set back to uninitialized, and    */
/* that the "current state" is reinitialised to the first state.            */
/* Resetting is only possible if a script is loaded and not running.        */
/* ------------------------------------------------------------------------ */



  int16     BSEK_RunBatch(
    int16 iReplay);
/* BSEK_RunBatch will run a batch file.                                      */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/*               Replay - indicates if a batch file is to be replayed(1) or */ 
/*               not(0).                                                    */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for run script/batch function"                 */
/* ------------------------------------------------------------------------ */



  int16     BSEK_RunScript(
    int16 iReplay);
/* BSEK_RunScript starts/continues a test script.                            */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/*               Replay - indicates if a scripy file is to be replayed(1) or*/ 
/*               not(0).                                                    */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for run script/batch function"                 */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */


  int16     BSEK_Shutdown(
  void);
/* BSEK_Shutdown stops all scripts and frees all allocated memory.           */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for shutdown"                                  */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */


  int16     BSEK_StepScript(
  void);
/* BSEK_StepScript steps through a script.                                   */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for run script/batch function"                 */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */



  int16     BSEK_SetLoggingMode(
  int16 iLoggingMode);
/* BSEK_SetLoggingMode will set the logging mode.                            */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* iLoggingMode            : enumerated value as defined in "logging modes" */
/*                           A bitwise OR can be used to combine different  */
/*                           values.                                        */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* --                                                                       */
/* Remarks:                                                                 */
/* The kernel has full control over the contents of the log-window. The     */
/* GUI shouldn't add extra functionality to this log window, because        */
/* this log-window and the contents of a log-file should be identical.      */
/* Using this function, the user can influence the level of logging of both */
/* output devices (window and file)                                         */
/* ------------------------------------------------------------------------ */


  int16     BSEK_SetViewMode(
  int16 iViewMode);
/* BSEK_SetViewMode will set the view mode, which is the contents of the     */
/* right window in the GUI.                                                 */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* iViewMode               : enumerated value as defiend in "view modes"    */
/*                                                                          */
/* Output parameters:      :                                                */
/* Return value:                                                            */
/* --                                                                       */
/* Remarks:                                                                 */
/* The kernel has full control over the contents of the view-window. The    */
/* GUI shoudn't add extra functionality to this view window.                */
/* ------------------------------------------------------------------------ */


  int16     BSEK_StopBatch(
  char *pcReason);
/* BSEK_StopBatch will stop a batch file.                                    */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for stop script/batch function"                */
/* ------------------------------------------------------------------------ */



  int16     BSEK_StopScript(
  char *pcReason);
/* BSEK_StopScript stops a test script.                                      */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* int16                   : enumerated values as defined in "return values */
/*                           for stop script/batch function"                */
/* Remarks:                                                                 */
/* Stopping means that the current state of the testscript will be "frozen" */
/* i.e. the testscript will not generate test commands any more, and in-    */
/* coming events will not influence the state of the test-script. However,  */
/* these events will be logged using the BSEG_LogLine function.              */
/* ------------------------------------------------------------------------ */



  char     *BSEK_Version(
  int16 * piVersionNumber);
/* BSEK_Version provides version information.                                */
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* piVersionNumber         : div 100 : major version number                 */
/*                           mod 100 : minor version number (major.minor)   */
/* Return value:                                                            */
/* char *                  : Ptr to three lines of ASCII text in the format */
/*                              AutoTest version 0.60                       */
/*                              sept 25, 2000                               */
/*                              Copyright (c) ...                           */
/* ------------------------------------------------------------------------ */

  void      BSEK_ProcessIncomingBuffer(
  void);
/* BSEK_ProcessIncomingBuffer gives us an interface for handling the buffer.*/
/* ------------------------------------------------------------------------ */
/* Input parameters:                                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/* Info:                   : check eventprocessor.h/c file                  */
/*                              mars 14, 2003                               */
/* author                  : Reza Gh (epkragi)                              */
/*                              Copyright (c) ...                           */
/* ------------------------------------------------------------------------ */

  void      SetEnabledChannels(
  int *iEnabledChannel,
  char *pcEnabledDevice[16],
  char *pcProtocolName);

  void      CloseLogFile(
  void);

#ifdef __cplusplus
}
#endif

#endif /* _ATKERNEL_H_ */
