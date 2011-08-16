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

#include "general_lib.h"

#include "scanner.h"
#include "parser.h"
#include "test_parser.h"
#include "atkernel.h"
#include "parser_lib.h"

typedef int bool;
int       CountTypeRefs(
  struct TTypeList *ptTypeList);

int Test_Parser(
  void)
{

  int       iSpecLoaded = FALSE;
  int       iTestLoaded = FALSE;
  int       iTellertje;
  char      pcBuffer[260000];   /* ca. 250 kb buffer */

  *pcBuffer = 0;

  if (InitScanner() == NO_SCANNER_ERROR)
  {
    iSpecLoaded = (Parse_Specification_File("inputfile.prot") == 0);
    iTellertje = CountTypeRefs(_ptTypeList);

    if (iSpecLoaded)
    {

      GenerateDebugOutput("inputfile.result");
      iTestLoaded = (Parse_Script_File("testscript.btsw"));

      WriteStateMachineList(pcBuffer, _ptStateMachineList,
                            BSEK_STATEMACHINES_FULL);

    }

    iTellertje = CountTypeRefs(_ptTypeList);
    delStateMachineList(&_ptStateMachineList);

    iTellertje = CountTypeRefs(_ptTypeList);
    delEventList(&_ptEventList);

    iTellertje = CountTypeRefs(_ptTypeList);
    delFunctionList(&_ptFunctionList);

    iTellertje = CountTypeRefs(_ptTypeList);

    delTypeList(&_ptTypeList);

    iTellertje = CountTypeRefs(_ptTypeList);

  }
  return OUT_OF_MEMORY;

}
