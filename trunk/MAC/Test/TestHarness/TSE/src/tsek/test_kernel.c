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
// 25/07/2000 E.A. Nijboer            Initial release
// 25/08/2000 E.A. Nijboer            
//
//=========================================================================
*/


#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "atkernel.h"
#include "general_lib.h"
#include "parser.h"

int       CountTypeRefs(
  struct TTypeList *pt);
extern struct TTypeList *_ptTypeList;

void Test_Kernel(
  void)
{


  int       iTellertje;
  int       iResult;

  char      pcBuffer[260000];   /* ca. 250 kb buffer */

  *pcBuffer = 0;

  iResult = BSEK_LoadSpecification("inputfile.prot");

  BSEK_Init();

  iResult = BSEK_LoadScript("testscript.btsw");

  iResult = BSEK_LoadSpecification("inputfile.prot");

  BSEK_Init();

  if (iResult == BSEK_OK)
  {

    iResult = BSEK_LoadScript("non existing file.btsw");

    iResult = BSEK_LoadScript("testscript.btsw");

    iTellertje = CountTypeRefs(_ptTypeList);

    if (iResult == BSEK_OK)
    {
      iResult = BSEK_OK;
    }

  }
}
