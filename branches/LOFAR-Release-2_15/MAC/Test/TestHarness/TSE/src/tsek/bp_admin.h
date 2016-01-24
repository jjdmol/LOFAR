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
// 09/02/2001                         Initial release
//
//=========================================================================
*/


#ifndef BP_ADMIN_H
#define BP_ADMIN_H

#include "codingstandard.h"

typedef struct TScriptFiles
{
  int8     *pcName;
  struct TScriptFiles *ptNext;
} TScriptFiles;

typedef struct
{
  int8     *pcBatchFile;        /* Batch file name                       */
  int8     *pcSpecFile;         /* Specification file name               */
  int8      bRunning;           /* Batch file is running                 */
  int8      bStopBatch;         /* Stop batch button is pressed          */
  struct TScriptFiles *ptStrScript;     /* Start location of the script files    */
  struct TScriptFiles *ptNxtScript;     /* Next script file                      */
} TAdmin;

#define MAX_FILE_NAME   255     /* Max 255 characters for storing path   */
                                                   /* and filename                          */

#endif
