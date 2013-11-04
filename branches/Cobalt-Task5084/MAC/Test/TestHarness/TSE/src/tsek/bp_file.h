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
// Description   : This file contains the main functionality of the batch processor
//                 the state machine.
//
// Revisions:
//
// Date       Author                  Changes
// 09/02/2001 Widjai Lila             Initial release
// 10/11/2001 Widjai Lila             
//
//=========================================================================
*/


#ifndef BP_FILE_H
#define BP_FILE_H

#include "bp_admin.h"

int16     bp_LoadBatch(
  int8 * pcString,
  TScriptFiles ** pptFiles);

void      bp_FreeMemFileList(
  TScriptFiles * ptFiles);


#endif
