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
// 16/08/2000 E.A. Nijboer            Initial release
// 12/01/2001 E.A. Nijboer            platform test.
// 09/02/2001 E.A. Nijboer            
//
//=========================================================================
*/


#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include "general_lib.h"

#include "parser_lib.h"

#include "scanner.h"

#include "atkernel.h"

#define LINELENGTH (136)
#define max(a,b) ((a)>(b))?(a):(b)
#define isascii(a) (((a)>=0x20)&&((a)<=0x7F))

/* parser_lib doesn't export this in the header file, so 'steal' it...      */

/****************************************************************************/


void debugValueDef(
  FILE * out,
  struct TValueDef *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TValueDef->pcMask %s\n", (void*)pt, pt->pcMask);
    fprintf(out, "                ->pcValue %s\n", pt->pcValue);
    fprintf(out, "                ->pcDesc %s\n", pt->pcDescription);
    fprintf(out, "                ->iRefCo %08x\n\n", pt->iRefCount);
  }
}


void debugValueDefList(
  FILE * out,
  struct TValueDefList *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TValueDefList->ptThis %8p\n", (void*)pt, (void*)pt->ptThis);
    fprintf(out, "           ->ptNext          %8p\n", (void*)pt->ptNext);
    fprintf(out, "           ->iRefCount       %08x\n\n", pt->iRefCount);

    debugValueDef(out, pt->ptThis);
    if (pt->ptNext != NULL)
      debugValueDefList(out, pt->ptNext);
  }
}

void debugType(
  FILE * out,
  struct TType *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TType->pcName    %s\n", (void*)pt, pt->pcName);
    fprintf(out, "       ->iSizeInBy %08x\n", pt->iSizeInBytes);
    fprintf(out, "       ->pcUpperLi %s\n", pt->pcUpperLimit);
    fprintf(out, "       ->pcLowerLi %s\n", pt->pcLowerLimit);
    fprintf(out, "       ->iKind     %08x\n", pt->iKind);
    fprintf(out, "       ->fTimeScal %08f\n", pt->fTimeScaling);
    fprintf(out, "       ->ptDefinit %8p\n", (void*)pt->ptDefinition);
    fprintf(out, "       ->iEndinaes %08x\n", pt->iEndianess);
    fprintf(out, "       ->iRefCount %08x\n\n", pt->iRefCount);

    if (pt->ptDefinition != NULL)
    {
      debugValueDefList(out, pt->ptDefinition);
    }
  }
}

void debugTypeList(FILE * out, struct TTypeList *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TTypeList->ptThis %8p\n", (void*)pt, (void*)pt->ptThis);
    fprintf(out, "           ->ptNext      %8p\n", (void*)pt->ptNext);
    fprintf(out, "           ->iRefCount   %08x\n\n", pt->iRefCount);

    debugType(out, pt->ptThis);
    if (pt->ptNext != NULL)
      debugTypeList(out, pt->ptNext);
  }
}


void debugParameter(FILE * out, struct TParameter *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TParameter->pcName %20s\n", (void*)pt, pt->pcName);
    fprintf(out, "           ->iArray  %08i\n", pt->iArray);
    fprintf(out, "           ->iOffset %08i\n", pt->iOffset);
    fprintf(out, "           ->ptTyped %8p\n", (void*)pt->ptTypeDef);
    fprintf(out, "           ->iRefCou %08i\n\n", pt->iRefCount);
  }
}

void debugParameterList(FILE * out, struct TParameterList *pt)
{
  int       i;

  if (pt != NULL)
  {
    fprintf(out, "(%8p) TParameterList->ptThis %8p\n", (void*)pt, (void*)pt->ptThis);
    fprintf(out, "           ->iFollowedByUnion %08x\n", pt->iFollowedByUnion);
    fprintf(out, "           ->iRefCount %08x\n\n", pt->iRefCount);

    debugParameter(out, pt->ptThis);

    for (i = 0; i < UNIONARRAYSIZE + 1; i++)
    {
      fprintf(out, "           ->aptNext[%02d]    %8p\n", i, (void*)pt->aptNext[i]);
    }
    for (i = 0; i < UNIONARRAYSIZE + 1; i++)
    {
      if (pt->aptNext[i] != NULL)
        debugParameterList(out, pt->aptNext[i]);
    }
  }
}

void debugFunction(FILE * out, struct TFunction *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TFunction->pcName %s\n", (void*)pt, pt->pcName);
    fprintf(out, "           ->iSizeofSend %08x\n",
            pt->iSizeOfSendArrayParameters);
    fprintf(out, "           ->iSizeofRece %08x\n",
            pt->iSizeOfReceiveArrayParameters);
    fprintf(out, "           ->ptSendParam %8p\n", (void*)pt->ptSendParameters);
    fprintf(out, "           ->ptReceivePa %8p\n", (void*)pt->ptReceiveParameters);
    fprintf(out, "           ->iRefCount   %08x\n\n", pt->iRefCount);
  }
}

void debugFunctionList(FILE * out, struct TFunctionList *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TFunctionList->ptThis %8p\n", (void*)pt, (void*)pt->ptThis);
    fprintf(out, "           ->ptNext      %8p\n", (void*)pt->ptNext);
    fprintf(out, "           ->iRefCount   %08x\n\n", pt->iRefCount);

    debugFunction(out, pt->ptThis);
    if (pt->ptNext != NULL)
      debugFunctionList(out, pt->ptNext);
  }
}


void debugEvent(FILE * out, struct TEvent *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TEvent->pcName                 %s\n", (void*)pt, pt->pcName);
    fprintf(out, "           ->iSizeOfArrayParameters %08x\n",
            pt->iSizeOfArrayParameters);
    fprintf(out, "           ->ptParameters           %8p\n",
            (void*)pt->ptParameters);
    fprintf(out, "           ->iRefCount              %08x\n\n", pt->iRefCount);
  }
}

void debugEventList(FILE * out, struct TEventList *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TEventList->ptThis %8p\n", (void*)pt, (void*)pt->ptThis);
    fprintf(out, "           ->ptNext       %8p\n", (void*)pt->ptNext);
    fprintf(out, "           ->iRefCount    %08x\n\n", pt->iRefCount);

    debugEvent(out, pt->ptThis);
    if (pt->ptNext != NULL)
      debugEventList(out, pt->ptNext);
  }
}

void debugValue(FILE * out, struct TValue *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TValue->pcValue %s\n", (void*)pt, pt->pcValue);
    fprintf(out, "            ->iRefCount %d\n", pt->iRefCount);
  }
}

void debugVariable(FILE * out, struct TVariable *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TVariable->pcVarName    %s\n", (void*)pt, pt->pcName);
    fprintf(out, "           ->ptItsType         %8p", (void*)pt->ptItsType);
    if (pt->ptItsType != NULL)
      fprintf(out, "(a %s)\n", pt->ptItsType->pcName);
    else
      fprintf(out, "\n");
    fprintf(out, "           ->ptValue           %s\n", (char*) pt->ptValue);
    fprintf(out, "           ->iRefCount         %08x\n\n", pt->iRefCount);

    if (pt->ptValue != NULL)
      debugValue(out, pt->ptValue);
  }
}

void debugVariableList(FILE * out, struct TVariableList *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TVariableList->ptThis %8p\n", (void*)pt, (void*)pt->ptThis);
    fprintf(out, "           ->ptNext          %8p\n", (void*)pt->ptNext);
    fprintf(out, "           ->iRefCount       %08x\n\n", pt->iRefCount);

    debugVariable(out, pt->ptThis);
    if (pt->ptNext != NULL)
      debugVariableList(out, pt->ptNext);
  }
}


void debugAction(FILE * out, struct TAction *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TAction->lActionCounter %08lx\n", (void*)pt,
            pt->lActionCounter);
    fprintf(out, "              ->iActionType    %08x\n", pt->iActionType);

    fprintf(out, "              ->ptFunction     %8p", (void*)pt->ptFunction);
    if (pt->ptFunction != NULL)
      fprintf(out, " (%s)\n", pt->ptFunction->pcName);
    else
      fprintf(out, "\n");

    fprintf(out, "              ->ptEvent        %8p", (void*)pt->ptEvent);
    if (pt->ptEvent != NULL)
      fprintf(out, " (%s)\n", pt->ptEvent->pcName);
    else
      fprintf(out, "\n");

    fprintf(out, "              ->ptVarList %8p", (void*)pt->ptVarList);
    fprintf(out, "              ->iSourceLineNum %08x", pt->iSourceLineNumber);
    fprintf(out, "              ->iRefCount      %08x\n\n", pt->iRefCount);
  }
}

void debugActionList(FILE * out, struct TActionList *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TActionList->ptThis %8p\n", (void*)pt, (void*)pt->ptThis);
    fprintf(out, "           ->ptNext        %8p\n", (void*)pt->ptNext);
    fprintf(out, "           ->iRefCount     %08x\n\n", pt->iRefCount);

    debugAction(out, pt->ptThis);
    if (pt->ptNext != NULL)
      debugActionList(out, pt->ptNext);
  }
}

void debugState(FILE * out, struct TState *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TState->pcName %s\n", (void*)pt, pt->pcName);
    fprintf(out, "           ->ptTransiList %8p\n", (void*)pt->ptTransitionList);
    fprintf(out, "           ->iRefCount    %08x\n\n", pt->iRefCount);
  }
}

void debugStateList(FILE * out, struct TStateList *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TStateList->ptThis %8p\n", (void*)pt, (void*)pt->ptThis);
    fprintf(out, "           ->ptNext       %8p\n", (void*)pt->ptNext);
    fprintf(out, "           ->iRefCount    %08x\n\n", pt->iRefCount);

    debugState(out, pt->ptThis);
    if (pt->ptNext != NULL)
      debugStateList(out, pt->ptNext);
  }
}

void debugStateMachine(FILE * out, struct TStateMachine *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TStateMachine->pcName %s\n", (void*)pt, pt->pcName);
    fprintf(out, "           ->ptParameters    %8p\n", (void*)pt->ptParameters);
    fprintf(out, "           ->ptLocalVars     %8p\n", (void*)pt->ptLocalVars);
    fprintf(out, "           ->ptStates        %8p\n", (void*)pt->ptStates);
    fprintf(out, "           ->ptCurrentState  %8p\n", (void*)pt->ptCurrentState);
    fprintf(out, "           ->ptActionInProg  %8p\n", (void*)pt->ptActionInProgress);
    fprintf(out, "           ->ptRunningTimer  %8p\n", (void*)pt->ptRunningTimer);
    fprintf(out, "           ->iPending        %08x\n", pt->iPending);
    fprintf(out, "           ->iIOChannel      %08x\n", pt->iIOChannel);
    fprintf(out, "           ->iRefCount       %08x\n\n", pt->iRefCount);
  }
}

void debugStateMachineList(FILE * out, struct TStateMachineList *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TStateMachineList->ptThis %8p\n", (void*)pt, (void*)pt->ptThis);
    fprintf(out, "           ->ptNext       %8p\n", (void*)pt->ptNext);
    fprintf(out, "           ->iRefCount    %08x\n\n", pt->iRefCount);

    debugStateMachine(out, pt->ptThis);
    if (pt->ptNext != NULL)
      debugStateMachineList(out, pt->ptNext);
  }
}

void debugData(FILE * out, struct TData *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TData->pcName %s\n", (void*)pt, pt->pcName);
    fprintf(out, "           ->pcValue %s\n", pt->pcValue);
    fprintf(out, "           ->iRefCou %08x\n\n", pt->iRefCount);
  }
}

void debugDataList(FILE * out, struct TDataList *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TDataList->ptThis  %8p\n", (void*)pt, (void*)pt->ptThis);
    fprintf(out, "           ->ptNext       %8p\n", (void*)pt->ptNext);
    fprintf(out, "           ->iRefCount    %08x\n\n", pt->iRefCount);

    debugData(out, pt->ptThis);
    if (pt->ptNext != NULL)
      debugDataList(out, pt->ptNext);
  }
}

void debugBoard(FILE * out, struct TBoard *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TBoard->pcName %s\n", (void*)pt, pt->pcName);
    fprintf(out, "           ->ptStateMa %8p\n", (void*)pt->ptStateMachines);
    fprintf(out, "           ->iRefCount %08x\n", pt->iRefCount);

    if (pt->ptStateMachines != NULL)
    {
      debugStateMachineList(out, pt->ptStateMachines);
    }
  }
}

void debugBoardList(FILE * out, struct TBoardList *pt)
{
  if (pt != NULL)
  {
    fprintf(out, "(%8p) TBoardList->ptThis %8p\n", (void*)pt, (void*)pt->ptThis);
    fprintf(out, "           ->ptNext        %8p\n", (void*)pt->ptNext);
    fprintf(out, "           ->iRefCount     %08x\n\n", pt->iRefCount);

    debugBoard(out, pt->ptThis);
    if (pt->ptNext != NULL)
      debugBoardList(out, pt->ptNext);
  }
}


void GenerateDebugOutput(
  char *pcFile)
{
  FILE     *output;

  output = fopen(pcFile, "w");
  if (output == NULL)
    return;

  if (_ptTypeList)
    debugTypeList(output, _ptTypeList);
  if (_ptEventList)
    debugEventList(output, _ptEventList);
  if (_ptFunctionList)
    debugFunctionList(output, _ptFunctionList);
  if (_ptStateMachineList)
    debugStateMachineList(output, _ptStateMachineList);
  if (_ptDataList)
    debugDataList(output, _ptDataList);
  if (_ptBoardList)
    debugBoardList(output, _ptBoardList);

  fclose(output);
}
