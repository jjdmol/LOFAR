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


#ifndef BP_H
#define BP_H

#include "codingstandard.h"
#ifdef WIN32
#include <windows.h>
#endif

/* Module definitions */
#define BP_OK                            0
#define BP_ALL_SCRIPTS_PROCESSED         1
/* Errors             */
#define BP_FILE_CANNOT_BE_OPENED        -1
#define BP_FILE_READ_FAILURE            -2
#define BP_LOG_FILE_NOT_CREATED         -3
#define BP_BATCH_NO_END                 -4
#define BP_NO_SCRIPTS_IN_BATCH          -5
#define BP_SCRIPT_ERROR                 -6
#define BP_NO_SPECIFICATION             -7
#define BP_KERNEL_BUSY                  -8
#define BP_UNDEFINED                    -9

#define USER_INTERRUPTION               "User Interruption"
#define BP_RUNNING                      "RUNNING"

/*---------------------------------------------------------------------------*/
/* External accessible functions											 */
/*---------------------------------------------------------------------------*/

/* Initialization the administration of the Batch Processor */
void      BP_Init(
  void);

/* Load the batch file                                      */
int16     BP_LoadBatch(
  int8 * pcFileName);

/* Run the batch file                                       */
int16     BP_RunBatch(
  int16 iReplay);

/* Stop the batch file                                      */
void      BP_StopBatch(
  int8 * pcReason);

/* Script stopped                                           */
void      BP_ScriptStopped(
  int8 * pcReason);

/* Update batch logging                                     */
void      BP_UpdateLogging(
  void);

/* Clean up the administration                              */
void      BP_Finalize(
  void);

/*-----------------------------------------------------------------------------*/
/* External definitions														   */
/*-----------------------------------------------------------------------------*/


#endif
