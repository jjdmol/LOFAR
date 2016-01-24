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
// Description   : This file contains the global component administration and
//                 definitions.
//
// Revisions:
//
// Date       Author                  Changes
// 09/02/2001 Widjai Lila             Initial release
//
//=========================================================================
*/


#ifndef BP_LOG_H
#define BP_LOG_H

#include "codingstandard.h"

/* Logging commands                                                         */
#define CHANGE_STATUS   0x01
#define NEW_ITEM        0x02

/*------------------------------------------------------------------------------*/
/* Module exported functions                                                    */
/*------------------------------------------------------------------------------*/

/* Sends the string to the GUI logging window                               */
void      bp_ScriptStatus(
  int16 iCommand,
  int8 * pcFileName,
  int8 * pcStatus);

void      bp_FreeMemScriptStatus(
  void);

/* Send a message to the GUI logging window                                 */
void      bp_Message(
  int8 * pcString);

/* Initialize the variables                                                 */
void      bp_InitScriptStatus(
  void);

/* Update batch logging                                                     */
void      bp_UpdateLogging(
  void);

/* Start the batch logging to a file                                        */
int16     bp_StartFileLogging(
  int8 * pcFileName,
  int16  iReplay);

/* Stop the batch logging                                                   */
void      bp_StopFileLogging(
  int8 * pcReason);

/* Creates the log filename */
int8     *bp_CreateLogFileName(
  int8 * pcFileName);

#endif
