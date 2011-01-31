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
// 08/08/2000 E.A. Nijboer            Initial release
// 12/01/2001 E.A. Nijboer            platform test.
// 09/02/2001 E.A. Nijboer            
//
//=========================================================================
*/


#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "general_lib.h"

#include "scanner.h"
#include "basic_parser.h"
#include "parser_lib.h"
#include "parser.h"
#include "bsegui.h"
#include "atdriver.h"
#include "atkernel_intern.h"
#include "driver_configuration.h"

#include "arithmetric.h"

/****************************************************************************/
/* Local function headers.                                                  */
/****************************************************************************/

int       Parse_TValueDef(
  char *pcToken,
  struct TValueDef *ptValueDef,
  int iFieldSize);
int       Parse_TValueDefList(
  char *pcToken,
  struct TValueDefList *ptValueDefList,
  int iFieldSize);
int       Parse_TType(
  char *pcToken,
  struct TType *ptType);
int       Parse_TTypeList(
  char *pcToken,
  struct TTypeList **ptTypeList);
int       Parse_TParameter(
  char *pcToken,
  struct TParameter *ptThis,
  int iOffset,
  int *piArrayParameterSize);
int       Parse_OptParameterList(
  char *pcToken,
  struct TParameterList *ptList,
  int iOffset,
  int *ptArrayParameterSize);
int       Parse_TParameterList(
  char *pcToken,
  struct TParameterList *ptList,
  int iOffset,
  int *ptArrayParameterSize);
int       Parse_TFunction(
  char *pcToken,
  struct TFunction *ptThis);
int       Parse_TFunctionList(
  char *pcToken,
  struct TFunctionList **ptList);
int       Parse_TEvent(
  char *pcToken,
  struct TEvent *ptThis);
int       Parse_TEventList(
  char *pcToken,
  struct TEventList **ptList);
void      Find_StateName(
  char *pcName,
  struct TStateList *ptStateList,
  struct TState **pptState);
int       Parse_QVariableVar(
  char *pcToken,
  struct TVariableList *ptList);
int       Parse_QFixedVar(
  char *pcToken,
  struct TVariableList *ptList);
int       Parse_QStringVar(
  char *pcToken,
  struct TVariableList *ptList,
  int    bNoLengthIndicator);
int       Parse_TVarDeclarationList(
  char *pcToken,
  struct TVariableList *ptList);
int       Parse_TParameterInvokeList(
  char *pcToken,
  struct TVariableList *ptList,
  struct TParameterList *ptDefList,
  struct TParameterList *ptAlternateDefList,
  struct TVariableList *ptReference,
  int16 bCheckDefList);
int       Parse_TAction(
  char *pcToken,
  struct TAction *ptAction,
  struct TVariableList *ptReference);
int       Parse_TStateTransition(
  char *pcToken,
  struct TStateList **pptList,
  struct TVariableList *ptReference,
  int *piTransitionType);
int       Parse_TStateList(
  char *pcToken,
  struct TStateList **pptList,
  struct TVariableList *ptReference,
  int *piTransitionType);
int       Parse_TStateMachine(
  char *pcToken,
  struct TStateMachine *ptThis);
int       Parse_TStateMachineList(
  char *pcToken,
  struct TStateMachineList *ptList);
int       Parse_TStateMachineInvokings(
  char *pcToken,
  struct TStateMachineList *ptList,
  struct TVariableList *ptRefVars,
  char *pcDeviceName,
  int iAlternative);
int       Parse_TDataList(
  char *pcToken,
  struct TDataList *ptDataList);
int       Parse_TData(
  char *pcToken,
  struct TData *ptData);
int       Parse_TIOList(
  char *pcToken,
  struct TIOList *ptIOList);
int       Parse_TIO(
  char *pcToken,
  struct TIO *ptTIO);
int       Parse_TestScript(
  char *pcToken);
int       Parse_Specification_File(
  char *pcFileName);
int       Parse_Script_File(
  char *pcFileName);
int       LOC_Add_To_IOList(
  struct TIOList *ptIOList);
int       LOC_Add_To_IO(
  char *pcDevice,
  int iChannel,
  struct TIO *ptIO);


/****************************************************************************/
/* Local function bodies.                                                   */
/****************************************************************************/

int Parse_TValueDef(
  char *pcToken,
  struct TValueDef *ptValueDef,
  int iFieldSize)
/****************************************************************************/
/* input  : FieldSize  : the expected length of pMask and pValue            */
/*          pcToken    : The first token of the to be read value            */
/*                       definition. This token must have been              */
/*                       scanned before invoking this function.             */
/*          ptValueDef : pointer to allocated place for the result.         */
/* output : ptValueDef : pointer to the resulting datastructure.            */
/*          pcToken    : pointer to first token after the ValueDef          */
/*          (on failure: undefined)                                         */
/* globals: --                                                              */
/* result : true : a ValueDef is scanned properly. The read ValueDef is     */
/*                 substituted in *ptValueDef.                              */
/*          false: an error is found. The value of *ptValueDef is undefined.*/
/*                                                                          */
/* Comment: a *ptValueDef has three elements in it's data structure: pMask, */
/*          pValue, pDescription. On the input file, a ValueDefinition has  */
/*          the following possible structures:                              */
/*                                                                          */
/* VALUEDEF = <NUMBERTOKEN> ':' <STRING_TOKEN>                              */
/* VALUEDEF = '(' <NUMBERTOKEN> ',' <NUMBERTOKEN> ')' ':' <STRING_TOKEN>    */
/*                                                                          */
/*          If the first definition is parsed, the pcValue and the          */
/*          pcDefinition are allocated and filled, pcMask is set to NULL.   */
/*          If the second  definition is parsed, all three fields are       */
/*          allocated and filled. The length (in chars) of the NUMBER       */
/*          tokens must match the iFieldSize parameter.                     */
/****************************************************************************/
{
  int       iSize;

  if (*pcToken == '(')
  {
    /* VALUEDEF = '(' <NUMBERTOKEN> ',' <NUMBERTOKEN> ')' ':'              */
    /*            <STRING_TOKEN>                                           */

    if (!ParseHexNumber(pcToken))
      return FALSE;

    iSize = strlen(pcToken);
    if (iFieldSize != iSize)
    {
      AddError1("Size of mask should match size of type");
      return FALSE;
    }

    ptValueDef->pcMask = my_strdup(pcToken);

    if (!ParseChar(','))
      return FALSE;

    if (!ParseNumber(pcToken))
      return FALSE;

    iSize = strlen(pcToken);
    if (iFieldSize != iSize)
    {
      AddError1("Size of value should match size of type");
      return FALSE;
    }

    ptValueDef->pcValue = my_strdup(pcToken);

    if (!ParseChar(')'))
      return FALSE;
  }
  else if (iLastParsedTokenType == HEX_NUMBER_TOKEN)
  {
    /* VALUEDEF = <NUMBERTOKEN> ':' <STRING_TOKEN>                         */
    iSize = strlen(pcToken);
    if (iFieldSize != iSize)
    {
      AddError1("Size of value should match size of type");
      return FALSE;
    }
    ptValueDef->pcValue = my_strdup(pcToken);
  }
  else
  {
    AddError1("Hexdecimal number expected");
    return FALSE;
  }
  if (!ParseChar(':'))
    return FALSE;
  if (!ParseString(pcToken))
    return FALSE;

  ptValueDef->pcDescription = my_strdup(pcToken);

  Parse(pcToken);
  return TRUE;
}

int Parse_TValueDefList(
  char *pcToken,
  struct TValueDefList *ptValueDefList,
  int iFieldSize)
/*****************************************************************************
   input  : iFieldSize     : the expected length of pMask and pValue
            pcToken        : The first token of the to be read value 
                             definition list. This token must have been 
                             scanned before invoking this function.
            ptValueDefList : pointer to allocated place for the result.
   output : ptValueDefList : pointer to the resulting datastructure.
            pcToken        : first token after this element.
            (on failure : undefined)
   globals: --
   result : true : a ValueDefinitionList is scanned properly.
            false: an error is found.
   Comment: a ValueDefinitionList has two elements in it's data
            structure: ptThis and ptNext. This record is actually
            a list node, not a list. But in this parser, list
            nodes are considered to be lists.
            A ValueDefinitionList has the following structure:

   VALUEDEFLIST = VALUEDEF
   VALUEDEFLIST = VALUEDEF ',' VALUEDEFLIST

*****************************************************************************/
{

  ptValueDefList->ptThis = newValueDef();
  if (!Parse_TValueDef(pcToken, ptValueDefList->ptThis, iFieldSize))
    return FALSE;

  while (*pcToken == ',')
  {
    ptValueDefList->ptNext = newValueDefList();
    ptValueDefList = ptValueDefList->ptNext;
    ptValueDefList->ptThis = newValueDef();
    Parse(pcToken);
    if (!Parse_TValueDef(pcToken, ptValueDefList->ptThis, iFieldSize))
      return FALSE;
  }
  return TRUE;
}

int Parse_TType(
  char *pcToken,
  struct TType *ptType)
/*****************************************************************************
   Comment: a Type has seven elements in it's data
            structure: pName, SizeInBytes, pLowerLimit,
            pUpperLimit, Kind, TimeScaling, Definition.

   TYPE = <IDENTIFIER_TOKEN> '=' '{' <NUMBERTOKEN> '}'
   TYPE = <IDENTIFIER_TOKEN> '=' '{' <NUMBERTOKEN> ',' <NUMBERTOKEN> ',' 
          <NUMBERTOKEN> '}'
   TYPE = <IDENTIFIER_TOKEN> '=' '{' <NUMBERTOKEN> ',' <NUMBERTOKEN> ',' 
          <NUMBERTOKEN> <"TIME">     <NUMBERTOKEN> '}'
   TYPE = <IDENTIFIER_TOKEN> '=' '{' <NUMBERTOKEN> ',' <NUMBERTOKEN> ',' 
          <NUMBERTOKEN> <"ENUM">     <VALUEDEFLIST> '}'
   TYPE = <IDENTIFIER_TOKEN> '=' '{' <NUMBERTOKEN> ',' <NUMBERTOKEN> ',' 
          <NUMBERTOKEN> <"BITFIELD"> <VALUEDEFLIST> '}'
   TYPE = <IDENTIFIER_TOKEN> '=' '{' <NUMBERTOKEN> ',' <NUMBERTOKEN> ',' 
          <NUMBERTOKEN> <"COMMAND">  <VALUEDEFLIST> '}'
**********/
{
  int16     iTokenType;

  ptType->pcName = my_strdup(pcToken);

  if (!ParseChar('='))
    return FALSE;
  Parse(pcToken);
  if ((*pcToken) == '-')
  {
    ptType->iEndianess = TRUE;
    Parse(pcToken);
  }

  if ((*pcToken) != '{')
  {
    AddError2("{ expected, %s read.", pcToken);
    return FALSE;
  }

  if (!ParseNumber(pcToken))
  {
    /* Failed to retreive the value of the type */
    return FALSE;
  }
  ptType->iSizeInBytes = GetInteger(pcToken);

  Parse(pcToken);

  if (strcmp(pcToken, "-") == 0)
  {
    /* Field is up to n bytes; i.e. n bytes or less.. */
    ptType->iLessAllowed = 1;
    Parse(pcToken);
  }

  if ((*pcToken) == ',')
  {
    /* This must be an array of some kind...*/
    /* read minimum and maximum                                            */

    iTokenType = Parse(pcToken);
    if (iTokenType == IDENTIFIER_TOKEN)
    {
      if (strcmp(pcToken, "ASCII") == 0)
        ptType->iKind = ASCIIKIND;
      else if (strcmp(pcToken, "ASCII_0") == 0)
        ptType->iKind = ASCII0KIND;
      else if (strcmp(pcToken, "UNICODE") == 0)
        ptType->iKind = UNICODEKIND;
      else if (strcmp(pcToken, "UNICODE_0") == 0)
        ptType->iKind = UNICODE0KIND;
      else if (strcmp(pcToken, "FILEDATA") == 0)
        ptType->iKind = FILEDATAKIND;
      else if (strcmp(pcToken, "ARRAY") == 0)
      {
        /* Array has the following format:
         * { NumberOfElements, ARRAY, sizeofElement } */
        ptType->iKind = ARRAYKIND;
        ptType->uiNumberOfElements = ptType->iSizeInBytes;
        if (!ParseChar(','))
          return FALSE;
        if (!ParseNumber(pcToken))
        {
          return FALSE;
        }
        ptType->uiSizeOfElement = GetInteger(pcToken);
        ptType->iSizeInBytes = ptType->uiNumberOfElements * ptType->uiSizeOfElement;
      }
      else if (strcmp(pcToken, "DECIMAL") == 0)
      {
        if (!ParseChar(','))
          return FALSE;
        Parse(pcToken);
        ptType->pcDecimalDescription = strdup(pcToken);
      }
      Parse(pcToken);
    }
    else
    {
      if ((iTokenType == DEC_NUMBER_TOKEN)
          || (iTokenType == HEX_NUMBER_TOKEN)
          || (iTokenType == BIN_NUMBER_TOKEN))
      {
        if (strlen(pcToken) != ptType->iSizeInBytes * 2 + 2)
        {
          AddError2("Definition of minimum must be %d bytes",
                    ptType->iSizeInBytes);
          return FALSE;
        }
        ptType->pcLowerLimit = my_strdup(pcToken);
      }
      else
        UngetToken();

      if (!ParseChar(','))
        return FALSE;

      iTokenType = Parse(pcToken);
      if ((iTokenType == DEC_NUMBER_TOKEN)
          || (iTokenType == HEX_NUMBER_TOKEN)
          || (iTokenType == BIN_NUMBER_TOKEN))
      {
        if (strlen(pcToken) != ptType->iSizeInBytes * 2 + 2)
        {
          AddError2("Definition of maximum must be %d bytes",
                    ptType->iSizeInBytes);
          return FALSE;
        }
        ptType->pcUpperLimit = my_strdup(pcToken);
      }
      else
        UngetToken();

      /* The Type of the data */
      Parse(pcToken);
      if (*pcToken == ',')
      {
        if (!ParseIdentifier(pcToken))
          return FALSE;
        if (strcmp(pcToken, "TIME") == 0)
          ptType->iKind = TIMEKIND;
        else if (strcmp(pcToken, "ENUM") == 0)
          ptType->iKind = ENUMKIND;
        else if (strcmp(pcToken, "BITFIELD") == 0)
          ptType->iKind = BITFIELDKIND;
        else if (strcmp(pcToken, "COMMAND") == 0)
          ptType->iKind = COMMANDKIND;
        else if (strcmp(pcToken, "ASCII") == 0)
          ptType->iKind = ASCIIKIND;
        else if (strcmp(pcToken, "ASCII_0") == 0)
          ptType->iKind = ASCII0KIND;
        else if (strcmp(pcToken, "UNICODE") == 0)
          ptType->iKind = UNICODEKIND;
        else if (strcmp(pcToken, "UNICODE_0") == 0)
          ptType->iKind = UNICODE0KIND;
        else if (strcmp(pcToken, "FILEDATA") == 0)
          ptType->iKind = FILEDATAKIND;
        else if (strcmp(pcToken, "ARRAY") == 0)
        {
          ptType->iKind = ARRAYKIND;
        }
        else
        {
          AddError2("Unknown type kind %s", pcToken);
          return FALSE;
        }

        if ((ptType->iKind == COMMANDKIND)
            || (ptType->iKind == ASCIIKIND)
            || (ptType->iKind == ASCII0KIND)
            || (ptType->iKind == UNICODEKIND)
            || (ptType->iKind == UNICODE0KIND))
        {
          Parse(pcToken);
        }
        else
        {
          if (!ParseChar(','))
            return FALSE;

          if (ptType->iKind == TIMEKIND)
          {
            if (!ParseFloat(pcToken))
              return FALSE;
            sscanf(pcToken, "%f", &(ptType->fTimeScaling));
            Parse(pcToken);
          }
          else if (ptType->iKind == ARRAYKIND)
          {
            ptType->uiNumberOfElements = ptType->iSizeInBytes;
            /* For now only accepting number of bytes */
            if (!ParseNumber(pcToken))
            {
              return FALSE;
            }
            ptType->uiSizeOfElement = GetInteger(pcToken);
            ptType->iSizeInBytes = ptType->uiNumberOfElements * ptType->uiSizeOfElement;
            /* Finished with array reading the next token for test at end of function*/
            Parse(pcToken);
          }
          else
          {
            Parse(pcToken);
            ptType->ptDefinition = newValueDefList();
            if (!Parse_TValueDefList(pcToken,
                                     ptType->ptDefinition,
                                     ptType->iSizeInBytes * 2 + 2) != 0)
            {
              return FALSE;
            }
          }
        }
      }
    }
  }
  if (*pcToken != '}')
  {
    AddError2("} expected, %s found", pcToken);
    return FALSE;
  }

  Parse(pcToken);
  return TRUE;
}



int Parse_TTypeList(
  char *pcToken,
  struct TTypeList **ptTypeList)
/****************************************************************************/
/* TYPELIST  = TYPE                                                         */
/* TYPELIST  = TYPE TYPELIST                                                */
/****************************************************************************/
{
  struct TTypeList *ptNewTypeList;
  struct TTypeList *ptWalker;
  
  ptNewTypeList = newTypeList();
  ptNewTypeList->ptThis = newType();

  Parse(pcToken);
  if (iLastParsedTokenType == IDENTIFIER_TOKEN) /* type name?               */
  {
    if (!Parse_TType(pcToken, ptNewTypeList->ptThis))
    {
      free(ptNewTypeList->ptThis);
      free(ptNewTypeList);
      return FALSE;
    }
    else
    {
      if (NULL == *ptTypeList)
      {
        *ptTypeList = ptNewTypeList;
        ptWalker = ptNewTypeList;
      }
      else
      {
        if (CheckType(ptNewTypeList->ptThis) == TRUE)
        {
          ptWalker = *ptTypeList;
          while (NULL != ptWalker->ptNext)
          {
            ptWalker = ptWalker->ptNext;
          }
          ptWalker->ptNext = ptNewTypeList;
          ptWalker         = ptWalker->ptNext;
        }
        else
        {
          free(ptNewTypeList->ptThis);
          free(ptNewTypeList);
          return(FALSE);
        }
      }
    }
    while (iLastParsedTokenType == IDENTIFIER_TOKEN)      /* while type name parsd */
    {
      /* Allocate space for a new record                                      */
      ptNewTypeList         = newTypeList();
      ptNewTypeList->ptThis = newType();
  
      /* parse inputfile to fill new record.                                  */
      if (!Parse_TType(pcToken, ptNewTypeList->ptThis))
      {
        free(ptNewTypeList->ptThis);
        free(ptNewTypeList);
        return FALSE;
      }
      else
      {
        if (CheckType(ptNewTypeList->ptThis) == TRUE)
        {
          ptWalker->ptNext = ptNewTypeList;
          ptWalker         = ptWalker->ptNext;
        }
        else
        {
          free(ptNewTypeList->ptThis);
          free(ptNewTypeList);
          return FALSE;
        }
      }
    }
  }
  else
  {
    free(ptNewTypeList->ptThis);
    free(ptNewTypeList);
  }


  UngetToken();

  return TRUE;
}


int Parse_TParameter(
  char *pcToken,
  struct TParameter *ptThis,
  int iOffset,
  int *piArrayParameterSize)
/****************************************************************************
                pName                    TypeDef->pName
   PARAMETER  = HEX_NUMBER_TOKEN
   PARAMETER  = IDENTIFIER_TOKEN     ':' IDENTIFIER_TOKEN

   PARAMETER  = IDENTIFIER_TOKEN     ':' IDENTIFIER_TOKEN '(' IDENTIFIER_TOKEN '..' ')'
   PARAMETER  = IDENTIFIER_TOKEN     ':' IDENTIFIER_TOKEN '[' IDENTIFIER_TOKEN ']'

*****************************************************************************/
{
  /* The token containing the parameter is already beiing read and can be   */
  /* substituted.                                                           */
  ptThis->pcName = my_strdup(pcToken);

  ptThis->iOffset = iOffset;

  if (iLastParsedTokenType == HEX_NUMBER_TOKEN)
  {
    ptThis->ptValue = newValue();
    ptThis->ptValue->pcValue = my_strdup(pcToken);
    ptThis->ptValue->bFixed = TRUE;
    return TRUE;
  }
  else
  {
    Parse(pcToken);

    if (*pcToken == '*')
    {
      /* Array parameter read. (Obsolete functionality...                     */
      ptThis->iArray = TRUE;
      Parse(pcToken);
    }

    /* syntax check. A colon is expected.                                     */
    if (*pcToken != ':')
    {
      AddError2(": expected, %s found", pcToken);
      return FALSE;
    }

    /* Try to read the type identifier.                                       */
    if (Parse(pcToken) != IDENTIFIER_TOKEN)
    {
      AddError1("Type identifier expected");
      return FALSE;
    }

    /* Type identifier read. Try to find it in the type administration.       */
    ptThis->ptTypeDef = FindType(pcToken);

    /* Found?                                                                 */
    if (ptThis->ptTypeDef == NULL)
    {
      AddError2("Type [%s] not found", pcToken);
      /* no need to quit, parser is still in sync.                          */
    }


    Parse(pcToken);

    /* Array Definition???                                                    */
    if (strcmp(pcToken, "[]") == 0)
    {
      /* Yes. But accept it only for non-predefined types...                  */
      if (ptThis->ptTypeDef != NULL)
      {
        (*piArrayParameterSize) += ptThis->ptTypeDef->iSizeInBytes;
      }
      else
      {
        AddError1("Array of predefined types not supported");
        /* no need to quit, parser is still in sync.                          */
      }
    }

    else if (strcmp(pcToken, "(") == 0)
    {
      if (!ParseIdentifier(pcToken))
        return FALSE;
      ptThis->pcLengthFromField = strdup(pcToken);

      Parse(pcToken);
      if (strcmp(pcToken, "..") != 0)
      {
        AddError1(".. expected");
        return FALSE;
      }

      Parse(pcToken);

      if (iLastParsedTokenType == IDENTIFIER_TOKEN)
      {
        ptThis->pcLengthToField = strdup(pcToken);
        Parse(pcToken);
      }

      if (strcmp(pcToken, ")") != 0)
      {
        AddError1(") expected");
        return FALSE;
      }

    }
    else if (strcmp(pcToken, "[") == 0)
    {
      if (!ParseIdentifier(pcToken))
        return FALSE;
      ptThis->pcLengthIndicator = strdup(pcToken);
      Parse(pcToken);
      if (strcmp(pcToken, "]") != 0)
      {
        AddError1("] expected");
        return FALSE;
      }
    }
    else
      UngetToken();

    /* ready. The token following the Parameter Declaration is not parsed yet. */
    /* An accidentical parsed [] token is put back on the input stream.       */

    return TRUE;
  }
}

int Parse_OptParameterList(
  char *pcToken,
  struct TParameterList *ptList,
  int iOffset,
  int *piArrayParameterSize)
/* OPT_PARAMETER_LIST = '(' IDENTIFIER_TOKEN '=' NUMBERTOKEN ')' 
                        PARAMETERLIST                                       */
/* OPT_PARAMETER_LIST = '(' 'ELSE' ')' PARAMETERLIST                        */
{
  int       i;

  if (Parse(pcToken) != IDENTIFIER_TOKEN)
  {
    AddError3("'%s' expected, %s found", ptList->ptThis->pcName, pcToken);
    return FALSE;
  }

  /* Check if union variable is based on previous parameter.                */
  if (strcmp(pcToken, ptList->ptThis->pcName) != 0)
  {
    /* Union variable is NOT based on previous parameter. This is proba-    */
    /* bly the ELSE branch. Parse the ELSE branch.                          */
    if (strcmp(pcToken, "ELSE") != 0)
    {
      AddError2("Conditions must be based on %s", ptList->ptThis->pcName);
      /* no need to exit, parser is still in sync.                          */
    }
    else
    {
      /* ELSE branch read. Complete parsing the ELSE branch.                */
      if (!ParseChar(')'))
        return FALSE;

      if (Parse(pcToken) != IDENTIFIER_TOKEN)
      {
        AddError1("Parameter expected");
        return FALSE;
      }

      /* Check if the UNION already has an else branch? Always be prepared  */
      /* on funny programmers.                                              */

      if (ptList->aptNext[UNIONARRAYSIZE] != NULL)
      {
        AddError1("More than one ELSE branche is superfluous");
        /* Don't exit, parser is still in sync. Maybe the programmer pre-   */
        /* pared more jokes...                                              */
      }
      else
      {
        ptList->aptNext[UNIONARRAYSIZE] = newParameterList();
        ptList->aptNext[UNIONARRAYSIZE]->ptBackTrack = ptList;
      }

      /* and read the parameter list in the ELSE branch. Even if the pro-   */
      /* grammer is joking, we have to keep in sync...                      */
      /* Note: By re-using the "pcList->ptNext[UNIONARRAYSIZE]" entry,      */
      /* we might introduce a memory leak. Because this memory leak is      */
      /* small and will probably only happen during developement of a       */
      /* specification file, ( a one time job for a test environment)       */
      /* this memory leak will not be solved.                               */

      if (!Parse_TParameterList(pcToken,
                                ptList->aptNext[UNIONARRAYSIZE],
                                iOffset, piArrayParameterSize))
      {
        return FALSE;
      }
    }
  }
  else
  {
    /* Union variable is based on previous parameter. This is the most      */
    /* common case. Complete parsing this CASE branch.                      */

    if (!ParseChar('='))
      return FALSE;

    if (!ParseNumber(pcToken))
      return FALSE;

    i = GetInteger(pcToken);

    if (i >= (UNIONARRAYSIZE - 1))      /* minus one, because the last one is        */
      /* reserved for the ELSE branche.               */
    {
      /* This is a limitation of AutoTest, introduced by the decision to    */
      /* use an array for implementing the UNION. A fundamental better      */
      /* choice would have been to use a linked list, but for the expected  */
      /* application area this was the quickest and easiest solution.       */
      AddError2("Condition values may not exceed 0x%X", UNIONARRAYSIZE - 1);

      /* Parsing can continue, the parser is still in sync. But we must     */
      /* prevent runtime errors caused by the detected violation. The       */
      /* index i must be set to a value in valid range.                     */
      i = 0;
    }
    else
    {
      /* you never know...                                                  */
      if (ptList->aptNext[i] != NULL)
      {
        AddError2("Duplicate entry for 0x%X", i);
        /* but continue parsing. See also comment ca. 20 lines above,       */
        /* about multiple ELSE branches.                                    */
      }
    }

    if (!ParseChar(')'))
      return FALSE;

    if (!ParseIdentifier(pcToken))
      return FALSE;

    ptList->aptNext[i] = newParameterList();
    ptList->aptNext[i]->ptBackTrack = ptList;

    if (!Parse_TParameterList(pcToken,
                              ptList->aptNext[i],
                              iOffset, piArrayParameterSize))
      return FALSE;
  }

  Parse(pcToken);
  return TRUE;
}


int Parse_TParameterList(
  char *pcToken,
  struct TParameterList *ptList,
  int iOffset,
  int *piArrayParameterSize)
/*****************************************************************************

   PARAMETERLIST = PARAMETER
   PARAMETERLIST = PARAMETER ',' PARAMETERLIST
   PARAMETERLIST = PARAMETER ',' 'UNION' '{' OPT_PARAMETER_LIST_LIST '}'
   PARAMETERLIST = PARAMETER ',' '{' PARAMETERLIST '}' '[' PARAMETERNAME ']'

(and implicit coded:)

   OPT_PARAMETER_LIST_LIST = OPT_PARAMETER_LIST
   OPT_PARAMETER_LIST_LIST = OPT_PARAMETER_LIST ',' OPT_PARAMETER_LIST_LIST

   OPT_PARAMETER_LIST = '(' IDENTIFIER_TOKEN '=' NUMBERTOKEN ')' 
                        PARAMETERLIST

*****************************************************************************/
{
  int       i;
  struct TParameterList *ptNewEntry;
  struct TParameterList *ptLastEntry;

  ptList->ptThis = newParameter();
  if (!Parse_TParameter(pcToken, ptList->ptThis, iOffset, piArrayParameterSize))
    return FALSE;

  if (ptList->ptThis->pcLengthIndicator != NULL)
  {
    ptList->ptThis->ptLengthIndicator =
      FindParameterBackwards(ptList->ptThis->pcLengthIndicator, ptList);
    if (ptList->ptThis->ptLengthIndicator == NULL)
    {
      AddError2("%s is not a known parameter",
                ptList->ptThis->pcLengthIndicator);
    }

    if (!ptList->ptThis->ptTypeDef->iLessAllowed)
    {
      AddError2
        ("%s is not a sizeable parameter. (Size should be preceeded by a minus sign)",
         ptList->ptThis->pcName);
    }
  }

  /* iOffset must be updated, but only if a non-predefined typedefinition   */
  /* is found. If a predefined typedefinition is found, we cannot refer to  */
  /* properties of this predefined typedefinition, because it's pointer     */
  /* is NULL.                                                               */
  if (ptList->ptThis->ptTypeDef)
  {
    iOffset += ptList->ptThis->ptTypeDef->iSizeInBytes;
  }

  Parse(pcToken);
  if (*pcToken == ',')
  {
    Parse(pcToken);
    if (strcmp(pcToken, "UNION") == 0)
    {
      ptList->iFollowedByUnion = TRUE;

      if (!ParseChar('{'))
        return FALSE;

      /* Now start reading the implicit coded OPT_PARAMETER_LIST_LIST      */
      Parse(pcToken);
      while (*pcToken == '(')
      {
        /* There is a new OPT_PARAMETER_LIST. Parse it.                    */
        Parse_OptParameterList(pcToken, ptList, iOffset, piArrayParameterSize);
      }

      /* All the OPT_PARAMETER_LIST's are parsed. If there are gap's in    */
      /* this list, and there is an ELSE entry, those gap's have to be     */
      /* replaced by the ELSE entry.                                       */
      for (i = 0; i < UNIONARRAYSIZE; i++)
      {
        if (ptList->aptNext[i] == NULL)
        {
          ptList->aptNext[i] = ptList->aptNext[UNIONARRAYSIZE];
        }
      }

      /* pcToken != '(' is checked, but != '}' not yet.                    */
      if ((*pcToken) != '}')
      {
        AddError2("} expected, %c found", *pcToken);
        return FALSE;
      }
      Parse(pcToken);

      /* Continue parsing further parameterdefinitions after the UNION, if */
      /* any. The length of all possible union branches should be equal,   */
      /* but this is not tested.                                           */
      if ((*pcToken) == ',')
      {
        /* there _are_ parameterdefinitions after the UNION. All (nested)  */
        /* UNION branches must be linked to one new entry. First make that */
        /* new entry.                                                      */

        ptNewEntry = newParameterList();
        ptNewEntry->ptBackTrack = ptList;

        /* Connect the ends of all (unionarraysize) UNION branches to the  */
        /* next parameter. The easiest way to do this is recursive. There- */
        /* fore a dedicated, recursive function is used.                   */

        LinkAllBranchesToOne(ptList, ptNewEntry);

        /* The reference counter is one too high now. It was initialised   */
        /* on one, and incremented for each branch linked to it.           */

        Decrement(&(ptNewEntry->iRefCount));

        if (Parse(pcToken) == IDENTIFIER_TOKEN)
        {
          if (strcmp(pcToken, "UNION") == 0)
          {
            /* We don't accept this. A union immedeately following a union */
            /* is ridigiolous.                                             */
            AddError1("Consecutive UNIONS are not allowed.");

            /* we don't even try to keep in sync.                          */
            return FALSE;
          }
          else
          {
            ptNewEntry->iFollowedByUnion = FALSE;
            if (!Parse_TParameterList(pcToken,
                                      ptNewEntry,
                                      iOffset, piArrayParameterSize))
              return FALSE;
          }
        }
        else
        {
          AddError1("Due to comma after union, new parameter expected");
          return FALSE;
        }
      }
    }
    else if (strcmp(pcToken, "{") == 0)
    {                           /* Parse Repetition set .. */
      ptList->iFollowedByUnion = FALSE;
      ptList->aptNext[0] = newParameterList();
      ptList->aptNext[0]->aptNext[0] = newParameterList();
      ptList->aptNext[0]->aptNext[0]->ptBackTrack = ptList;
      ptList->aptNext[0]->ptThis = NULL;
      ptList->aptNext[0]->ptRepRec = newRepetition();
      ptList->aptNext[0]->ptRepRec->iBegin = TRUE;
      Parse(pcToken);

      if (!Parse_TParameterList(pcToken,
                                ptList->aptNext[0]->aptNext[0],
                                iOffset, piArrayParameterSize))
        return FALSE;
      if (!ParseChar('}'))
        return FALSE;
      if (!ParseChar('['))
        return FALSE;
      if (Parse(pcToken) != IDENTIFIER_TOKEN)
      {
        AddError2("Identifier expected, %s found", pcToken);
        return FALSE;
      }
      ptList->aptNext[0]->ptRepRec->ptParameter =
        FindParameterBackwards(pcToken, ptList);
      if (ptList->aptNext[0]->ptRepRec->ptParameter == NULL)
      {
        AddError2("Parameter %s not defined yet", pcToken);
      }

      /* Parsing is ready. Add some administrative stuff:                  */
      /* ptList-> is pointing to a Repetition indicator record.            */
      /* ptList->aptNext[1] should point to the matching Repetition        */
      /* indicator (Repetition indicator and matching indicator indicate   */
      /* the begin and end of a repeated part of parameters. This can be   */
      /* nested.                                                           */

      ptLastEntry = newParameterList();
      LinkAllBranchesToOne(ptList, ptLastEntry);

      ptLastEntry->ptRepRec = newRepetition();
      ptLastEntry->ptBackTrack = ptList;
      ptLastEntry->ptRepRec->iBegin = FALSE;
      ptLastEntry->aptNext[1] = ptList->aptNext[0];
      ptList->aptNext[0]->aptNext[1] = ptLastEntry;

      if (!ParseChar(']'))
        return FALSE;
      Parse(pcToken);

      if ((*pcToken) == ',')
      {
        /* there _are_ parameterdefinitions after the replist.             */

        ptNewEntry = newParameterList();
        ptNewEntry->ptBackTrack = ptList;
        ptLastEntry->aptNext[0] = ptNewEntry;

        if (Parse(pcToken) == IDENTIFIER_TOKEN)
        {
          if (strcmp(pcToken, "UNION") == 0)
          {
            AddError1("UNIONS immedeately following a RepList is not allowed.");

            /* we don't even try to keep in sync.                          */
            return FALSE;
          }
          else
          {
            ptNewEntry->iFollowedByUnion = FALSE;
            if (!Parse_TParameterList(pcToken,
                                      ptNewEntry,
                                      iOffset, piArrayParameterSize))
              return FALSE;
          }
        }
        else
        {
          AddError1("Due to comma after RepList, new parameter expected");
          return FALSE;
        }
      }
    }
    else
    {
      /* no union and no '{'. Just a new parameter.                        */
      ptList->iFollowedByUnion = FALSE;
      ptList->aptNext[0] = newParameterList();
      ptList->aptNext[0]->ptBackTrack = ptList;

      if (!Parse_TParameterList(pcToken,
                                ptList->aptNext[0],
                                iOffset, piArrayParameterSize))
        return FALSE;

    }
  }

  UngetToken();                 /* Put back the } or the ( token.                           */
  return TRUE;
}


void SubstituteLengthFields(
  struct TParameterList *ptList)
{
  struct TParameterList *ptWalker;

  ptWalker = ptList;

  while (ptWalker != NULL)
  {
    if (ptWalker->ptThis->pcLengthFromField != NULL)
    {
      ptWalker->ptThis->ptLengthFromField =
        FindParameter(ptWalker->ptThis->pcLengthFromField, ptList);
      if (ptWalker->ptThis->ptLengthFromField == NULL)
      {
        AddError2("Parameter %s in length definition does not exist",
                  ptWalker->ptThis->pcLengthFromField);
      }
    }
    if (ptWalker->ptThis->pcLengthToField != NULL)
    {
      ptWalker->ptThis->ptLengthToField =
        FindParameter(ptWalker->ptThis->pcLengthToField, ptList);
      if (ptWalker->ptThis->ptLengthToField == NULL)
      {
        AddError2("Parameter %s in length definition does not exist",
                  ptWalker->ptThis->pcLengthToField);
      }
    }
    ptWalker = ptWalker->aptNext[0];    /* Formally not correct, but in 99.9999% of the cases it is good enough */
    while ((ptWalker != NULL) && (ptWalker->ptThis == NULL))
      ptWalker = ptWalker->aptNext[0];
  }
}

int Parse_TFunction(
  char *pcToken,
  struct TFunction *ptThis)
/****************************************************************************/
/*                                                                          */
/* FUNCTION = IDENTIFIER_TOKEN '=' '{' PARAMETERLIST '}'                    */
/* FUNCTION = IDENTIFIER_TOKEN                                              */
/*                                                                          */
/****************************************************************************/
{
  /* copy name into structure.                                              */
  ptThis->pcName = my_strdup(pcToken);

  if (strcmp(pcToken, "GetLoginRsp") == 0)
  {
    pcToken = pcToken;
  }

  Parse(pcToken);
  if (strcmp(pcToken, "=") == 0)
  {
    if (!ParseChar('{'))
      return FALSE;

    Parse(pcToken);
    if ((iLastParsedTokenType == IDENTIFIER_TOKEN)
        || (iLastParsedTokenType == HEX_NUMBER_TOKEN))
    {
      ptThis->ptSendParameters = newParameterList();
      if (!Parse_TParameterList(pcToken,
                                ptThis->ptSendParameters,
                                0, &ptThis->iSizeOfSendArrayParameters))
      {
        return FALSE;
      }
      Parse(pcToken);
      if (strcmp(pcToken, "}") != 0)
      {
        AddError2("} expected, %s found\n", pcToken);
        return FALSE;
      }
    }
    else
    {
      AddError2("Identifier expected, %s found\n", pcToken);
      return FALSE;
    }
  }
  else
  {
    AddError2("'=' expected, %s found\n", pcToken);
    return FALSE;
  }
  SubstituteLengthFields(ptThis->ptSendParameters);
  return TRUE;
}



int Parse_TFunctionList(
  char *pcToken,
  struct TFunctionList **ptList)
/****************************************************************************/
/*                                                                          */
/* FUNCTIONLIST = FUNCTION                                                  */
/* FUNCTIONLIST = FUNCTION FUNCTIONLIST                                     */
/*                                                                          */
/****************************************************************************/
{
  struct TFunctionList *ptNewFunctionList;
  struct TFunctionList *ptWalker;
   
  ptNewFunctionList = newFunctionList();
  ptNewFunctionList->ptThis = newFunction();
  
  Parse(pcToken);
  if (iLastParsedTokenType == IDENTIFIER_TOKEN)
  {
    if (!Parse_TFunction(pcToken, ptNewFunctionList->ptThis))
    {
      free(ptNewFunctionList->ptThis);
      free(ptNewFunctionList);
      return(FALSE);
    } 
    else
    {
      if (NULL == *ptList)
      {
        *ptList  = ptNewFunctionList;
        ptWalker = *ptList;
      }
      else
      {
        if (TRUE == CheckFunction(ptNewFunctionList->ptThis))
        {
          ptWalker = *ptList;
          while (NULL != ptWalker->ptNext)
          {
            ptWalker = ptWalker->ptNext;
          }
          
          ptWalker->ptNext = ptNewFunctionList;
          ptWalker = ptWalker->ptNext;
        }
        else
        {
          free(ptNewFunctionList->ptThis);
          free(ptNewFunctionList);
          return(FALSE);
        }
      }
    }
    
    /* while a function-name is parsed                                        */
    while (Parse(pcToken) == IDENTIFIER_TOKEN)
    {
      /* make space for new function                                          */
      ptNewFunctionList = newFunctionList();
      ptNewFunctionList->ptThis = newFunction();
    
      /* and try to parse that new function.                                  */
      if (!Parse_TFunction(pcToken, ptNewFunctionList->ptThis))
      {
        free(ptNewFunctionList->ptThis);
        free(ptNewFunctionList);
        return FALSE;
      }
      else
      {
        if (TRUE == CheckFunction(ptNewFunctionList->ptThis))
        {
          ptWalker->ptNext = ptNewFunctionList;
          ptWalker         = ptWalker->ptNext;
        }
        else
        {
          free(ptNewFunctionList->ptThis);
          free(ptNewFunctionList);
          return(FALSE);
        }
      }
    }
  }
  else
  {
    free(ptNewFunctionList->ptThis);
    free(ptNewFunctionList);
  }
  UngetToken();

  return TRUE;
}

int Parse_TEvent(
  char *pcToken,
  struct TEvent *ptThis)
/****************************************************************************/
/*                                                                          */
/* EVENT = IDENTIFIER_TOKEN '=' '{' PARAMETERLIST '}'                       */
/* EVENT = IDENTIFIER_TOKEN                                                 */
/*                                                                          */
/****************************************************************************/
{
  /* copy name into structure.                                              */
  ptThis->pcName = my_strdup(pcToken);

  Parse(pcToken);
  if (strcmp(pcToken, "=") == 0)
  {
    if (!ParseChar('{'))
      return FALSE;

    Parse(pcToken);
    if ((iLastParsedTokenType == IDENTIFIER_TOKEN)
        || (iLastParsedTokenType == HEX_NUMBER_TOKEN))
    {
      ptThis->ptParameters = newParameterList();
      if (!Parse_TParameterList(pcToken,
                                ptThis->ptParameters,
                                0, &ptThis->iSizeOfArrayParameters))
      {
        return FALSE;
      }
      Parse(pcToken);
      if (strcmp(pcToken, "}") != 0)
      {
        AddError2("} expected, %s found\n", pcToken);
        return FALSE;
      }

    }
  }
  else
  {
    AddError2("'=' expected, %s found", pcToken);
    return FALSE;
  }

  SubstituteLengthFields(ptThis->ptParameters);
  return TRUE;
}


int       CountTypeRefs(
  struct TTypeList *ptTypeList);


int Parse_TEventList(
  char *pcToken,
  struct TEventList **ptList)
/****************************************************************************/
/*                                                                          */
/* EVENTLIST = EVENT                                                        */
/* EVENTLIST = EVENT EVENTLIST                                              */
/*                                                                          */
/****************************************************************************/
{
  struct TEventList *ptNewEventList;
  struct TEventList *ptWalker;
  
  ptNewEventList         = newEventList();
  ptNewEventList->ptThis = newEvent();

  Parse(pcToken);
  if (iLastParsedTokenType == IDENTIFIER_TOKEN)
  {
    if (!Parse_TEvent(pcToken, ptNewEventList->ptThis))
    {
      free(ptNewEventList->ptThis);
      free(ptNewEventList);
      return(FALSE);
    } 
    else
    {
      if (NULL == (*ptList))
      {
        *ptList  =  ptNewEventList;
        ptWalker = *ptList;
      }
      else
      {
        ptWalker = *ptList;
        if (TRUE == CheckEvent(ptNewEventList->ptThis))
        {
          while (NULL != ptWalker->ptNext)
          {
            ptWalker = ptWalker->ptNext;
          }
          
          ptWalker->ptNext = ptNewEventList;
          ptWalker         = ptWalker->ptNext;
        }
        else
        {
          free(ptNewEventList->ptThis);
          free(ptNewEventList);
          return(FALSE);
        }
      }
    }
    
    /* while a function-name is parsed                                        */
    while (Parse(pcToken) == IDENTIFIER_TOKEN)
    {
      /* make space for new function                                          */
      ptNewEventList = newEventList();
      ptNewEventList->ptThis = newEvent();
  
      /* and try to parse that new function.                                  */
      if (!Parse_TEvent(pcToken, ptNewEventList->ptThis))
      {
        free(ptNewEventList->ptThis);
        free(ptNewEventList);
        return FALSE;
      }
      else
      {
        if (TRUE == CheckEvent(ptNewEventList->ptThis))
        {
          ptWalker->ptNext = ptNewEventList;
          ptWalker = ptWalker->ptNext;
        }
        else
        {
          free(ptNewEventList->ptThis);
          free(ptNewEventList);
          return(FALSE);
        }
      }
    }
  }
  else
  {
    free(ptNewEventList->ptThis);
    free(ptNewEventList);
  }

  UngetToken();

  return TRUE;

}

/****************************************************************************/
/*  First two functions are defined which are usefull in building           */
/*  the state-machine structures: Find_StateName() and                      */
/*  Add_ActionEntry(). Followed by (again) parsing routines                 */
/*  Actually, these functions belong in parse_lib.c...                      */
/****************************************************************************/

void Find_StateName(
  char *pcName,
  struct TStateList *ptStateList,
  struct TState **pptState)
/****************************************************************************/
/* input  : Name       : ptr to name of searched state.                     */
/*          tStateList : The list which is searched trough.                 */
/* output : State      : ptr to ptr to struct describing searched state     */
/* globals: --                                                              */
/* result : --                                                              */
/* comment: If the (name of the) searched state is not found, a record      */
/*          is added to the list of states. This new record represents      */
/*          the searched state.                                             */
/****************************************************************************/
{
  /* While 'this' state record has the wrong name,                          */
  while (strcmp(ptStateList->ptThis->pcName, pcName) != 0)
  {
    /* Are there are more state records available?                          */
    if (ptStateList->ptNext != NULL)
    {
      /* Yes. Investigate them.                                             */
      ptStateList = ptStateList->ptNext;
    }
    else
    {
      /* No. No state record found. Make a new one.                         */
      ptStateList->ptNext = newStateList();
      ptStateList = ptStateList->ptNext;
      ptStateList->ptThis = newState();
      ptStateList->ptThis->pcName = my_strdup(pcName);
    }
  }
  *pptState = ptStateList->ptThis;
}


void Add_TransitionEntry(
  struct TState *ptThis,
  struct TTransition **pptTransition)
/****************************************************************************/
/* input  : This       : The state which gets a new transition entry        */
/* output : Transition : ptr to ptr to struct describing added trans. entry */
/* globals: --                                                              */
/* result : --                                                              */
/* comment: If the mentioned state has no transition list yet, a new list   */
/*          is created. Otherwise, a record is added to this list.          */
/*          A pointer to the free record is returned.                       */
/****************************************************************************/
{
  struct TTransitionList *ptList;

  if (ptThis->ptTransitionList == NULL)
  {
    /* There are no records yet. Make a first TransitionList record.        */
    ptThis->ptTransitionList = newTransitionList();
    ptThis->ptTransitionList->ptThis = newTransition();
    *pptTransition = ptThis->ptTransitionList->ptThis;
  }
  else
  {
    /* go to the end of the existing TransitionList                         */
    ptList = ptThis->ptTransitionList;
    while (ptList->ptNext != NULL)
      ptList = ptList->ptNext;

    /* Add a new TransitionList record at the end, and go to that new rec.  */
    ptList->ptNext = newTransitionList();
    ptList = ptList->ptNext;

    /* Create a Transition record.                                          */
    ptList->ptThis = newTransition();
    *pptTransition = ptList->ptThis;
  }
}

#ifdef NIET
void Add_ActionEntry(
  struct TState *ptThis,
  struct TAction **pptAction)
/****************************************************************************/
/* input  : This   : The state which gets a new action entry                */
/* output : Action : ptr to ptr to struct describing added action entry     */
/* globals: --                                                              */
/* result : --                                                              */
/* comment: If the mentioned state has no action list yet, a new list       */
/*          is created. Otherwise, a record is added to this list.          */
/*          A pointer to the free record is returned.                       */
/****************************************************************************/
{
  struct TActionList *ptList;

  if (ptThis->ptActionList == NULL)
  {
    /* There are no records yet. Make a first ActionList record.            */
    ptThis->ptActionList = newActionList();
    ptThis->ptActionList->ptThis = newAction();
    *pptAction = ptThis->ptActionList->ptThis;
  }
  else
  {
    /* go to the end of the existing ActionList                             */
    ptList = ptThis->ptActionList;
    while (ptList->ptNext != NULL)
      ptList = ptList->ptNext;

    /* Add a new ActionList record at the end and go to that new record     */
    ptList->ptNext = newActionList();
    ptList = ptList->ptNext;

    /* Create an Action record for the tail ActionList record.              */
    ptList->ptThis = newAction();
    *pptAction = ptList->ptThis;
  }
}
#endif


int Parse_QVariableVar(
  char *pcToken,
  struct TVariableList *ptList)
/****************************************************************************/
/* VARIABLEVAR = IDENTIFIER_TOKEN                                            */
/****************************************************************************/
{
  ptList->ptThis->pcName = my_strdup(pcToken);

  Parse(pcToken);
  return TRUE;
}

int Parse_QFixedVar(
  char *pcToken,
  struct TVariableList *ptList)
/***************************************************************************
   FIXEDVAR = NUMBERTOKEN
***************************************************************************/
{
  int16     iScannedLength;
  int16     iExpectedLength;
  char      pcErrorLine[2500];

  if (ptList->ptThis->ptValue == NULL)
  {
    ptList->ptThis->ptValue = newValue();
  }

  ptList->ptThis->ptValue->bFixed = TRUE;

  if (ptList->ptThis->ptValue->pcValue != NULL)
  {
    free(ptList->ptThis->ptValue->pcValue);
    ptList->ptThis->ptValue->pcValue = NULL;
  }

  ptList->ptThis->ptValue->pcValue = my_strdup(pcToken);

  if (ptList->ptThis->ptItsType != NULL) 
  {
    if (pcToken[1] == 'x')
    {
      /* Only check the length when the script contains a hexadecimal constant */
  
      iExpectedLength = ptList->ptThis->ptItsType->iSizeInBytes;
      iScannedLength = (strlen(pcToken) - 2) / 2;
      if (ptList->ptThis->ptItsType->iLessAllowed == 0)
      {
        if ((iScannedLength != iExpectedLength) && (iExpectedLength != 0))
        {
          sprintf(pcErrorLine,
                  "Expected parameter-type %s must be %d bytes. %s is %d%s bytes",
                  ptList->ptThis->ptItsType->pcName,
                  ptList->ptThis->ptItsType->iSizeInBytes, pcToken, iScannedLength,
                  (strlen(pcToken) % 2 == 0) ? "" : ".5");
          AddError1(pcErrorLine);
        }
      }
      else
      {
        if ((iScannedLength > iExpectedLength) && (iExpectedLength != 0))
        {
          sprintf(pcErrorLine,
                  "Expected parameter-type %s must be %d bytes. %s is %d%s bytes",
                  ptList->ptThis->ptItsType->pcName,
                  ptList->ptThis->ptItsType->iSizeInBytes, pcToken, iScannedLength,
                  (strlen(pcToken) % 2 == 0) ? "" : ".5");
          AddError1(pcErrorLine);
        }
      }
    }
    else
    {
      /* Only hexadecimal values are allowed in array */
      /* Otherwise it is impossible to determine the size of each array element */
      if (ptList->ptThis->ptItsType->iKind == ARRAYKIND)
      {
        sprintf(pcErrorLine,
                "Array data (%s) must be hexadecimal",
                pcToken);
        AddError1(pcErrorLine);
      }
    }

  }

  Parse(pcToken);
  return TRUE;
}

int Parse_QStringVar(
  char *pcToken,
  struct TVariableList *ptList,
  int   bNoLengthIndicator)
/***************************************************************************/
/* FIXEDVAR = STRINGTOKEN                                                  */
/*                                                                         */
/* precondition : ptList->pcVarName == NULL                                */
/*                ptList->pcValue   == NULL                                */
/*                                                                         */
/*           e.g. pcToken[0] = 0x41 'A'                                    */
/*                pcToken[1] = 0x42 'B'                                    */
/*                pcToken[2] = 0x00                                        */
/*                                                                         */
/* postcondition : ptList->pcVarName[0] = 0x24 '"'                         */
/*                                  [1] = 0x41 'A'                         */
/*                                  [2] = 0x42 'B'                         */
/*                                  [3] = 0x24 '"'                         */
/*                                  [4] = 0x00                             */
/*                                                                         */
/*                       ->ptValue  [0] = 0x40 '0'                         */
/*                       ->ptValue  [1] = 0x78 'x'                         */
/*                       ->ptValue  [2] = 0x44 '4'                         */
/*                       ->ptValue  [3] = 0x41 '1'                         */
/*                       ->ptValue  [4] = 0x44 '4'                         */
/*                       ->ptValue  [5] = 0x42 '2'                         */
/*                       ->ptValue  [6] = 0x00                             */
/*                                                                         */
/***************************************************************************/
{
  char     *pcChar;
  uint16    i;

  if ((ptList == NULL)
      || (ptList->ptThis == NULL) || (ptList->ptThis->ptItsType == NULL))

  {
    /* Type cannot be evaluated, an error has been logged already */
    /* Do as little as possible to keep in sync, and continue.    */

    Parse(pcToken);
    return TRUE;
  }

  i = strlen(pcToken);

  if (ptList->ptThis->ptItsType->iKind == ASCIIKIND)
  {
    if (ptList->ptThis->ptItsType->iLessAllowed == 0)
    {
      if (i != ptList->ptThis->ptItsType->iSizeInBytes)
      {
        AddError5
          ("Found token %s has length %d. Expected ascii type %s should be %d bytes.",
           pcToken, i, ptList->ptThis->ptItsType->pcName,
           (ptList->ptThis->ptItsType->iSizeInBytes)) Parse(pcToken);
        return TRUE;
      }
    }
    else
    {
      if (i > ptList->ptThis->ptItsType->iSizeInBytes)
      {
        AddError5
          ("Found token %s has length %d. Expected ascii type %s should be at most %d bytes.",
           pcToken, i, ptList->ptThis->ptItsType->pcName,
           (ptList->ptThis->ptItsType->iSizeInBytes)) Parse(pcToken);
        return TRUE;
      }
    }
  }
  else if (ptList->ptThis->ptItsType->iKind == ASCII0KIND)
  {
    {
      if (i >= ptList->ptThis->ptItsType->iSizeInBytes)
      {
        AddError5
          ("Found token %s has length %d. Expected ascii type %s should be at most %d bytes. (One byte reserved for \\0)",
           pcToken, i, ptList->ptThis->ptItsType->pcName,
           (ptList->ptThis->ptItsType->iSizeInBytes) - 1) Parse(pcToken);
        return TRUE;
      }
    }
  }
  else if (ptList->ptThis->ptItsType->iKind == UNICODEKIND)
  {
    if (ptList->ptThis->ptItsType->iLessAllowed == 0)
    {
      if (i != ptList->ptThis->ptItsType->iSizeInBytes / 2)
      {
        AddError5
          ("Found token %s has length %d. Expected unicode type %s should be %d characters.",
           pcToken, i, ptList->ptThis->ptItsType->pcName,
           (ptList->ptThis->ptItsType->iSizeInBytes) / 2) Parse(pcToken);
        return TRUE;
      }
    }
    else
    {
      if (i > ptList->ptThis->ptItsType->iSizeInBytes / 2)
      {
        AddError5
          ("Found token %s has length %d. Expected unicode type %s should be at most %d characters.",
           pcToken, i, ptList->ptThis->ptItsType->pcName,
           (ptList->ptThis->ptItsType->iSizeInBytes) / 2) Parse(pcToken);
        return TRUE;
      }
    }
  }
  else if (ptList->ptThis->ptItsType->iKind == UNICODE0KIND)
  {
    {
      if (i >= ptList->ptThis->ptItsType->iSizeInBytes)
      {
        AddError5
          ("Found token %s has length %d. Expected unicode type %s should be at most %d characters. (Two bytes reserved for \\0)",
           pcToken, i, ptList->ptThis->ptItsType->pcName,
           (ptList->ptThis->ptItsType->iSizeInBytes) - 1) Parse(pcToken);
        return TRUE;
      }
    }
  }
  else if (i != ptList->ptThis->ptItsType->iSizeInBytes)
  {
    AddError5
      ("Found token %s has length %d. Expected type %s should be %d bytes",
       pcToken, i, ptList->ptThis->ptItsType->pcName,
       ptList->ptThis->ptItsType->iSizeInBytes) Parse(pcToken);
    return TRUE;
  }

  pcChar = pcToken;

  ptList->ptThis->pcName = (char *) malloc(strlen(pcToken) + 3);
  strcpy(ptList->ptThis->pcName, "\"");
  strcat(ptList->ptThis->pcName, pcToken);
  strcat(ptList->ptThis->pcName, "\"");

  if (ptList->ptThis->ptValue == NULL)
  {
    ptList->ptThis->ptValue = newValue();
  }

  if (ptList->ptThis->ptValue->pcValue != NULL)
  {
    free(ptList->ptThis->ptValue->pcValue);
  }

  ptList->ptThis->ptValue->pcValue =
    (char *) malloc(ptList->ptThis->ptItsType->iSizeInBytes * 2 + 3);
  i = 0;

  ptList->ptThis->ptValue->pcValue[i++] = '0';
  ptList->ptThis->ptValue->pcValue[i++] = 'x';
  while (*pcChar)
  {
    if ((ptList->ptThis->ptItsType->iKind == UNICODEKIND)
        || (ptList->ptThis->ptItsType->iKind == UNICODE0KIND))
    {
      ptList->ptThis->ptValue->pcValue[i++] = '0';
      ptList->ptThis->ptValue->pcValue[i++] = '0';
    }

    ptList->ptThis->ptValue->pcValue[i++] =
      (char) TO_ASCII(((*pcChar) & 0xF0) >> 4);
    ptList->ptThis->ptValue->pcValue[i++] = (char) TO_ASCII(((*pcChar) & 0x0F));

    pcChar++;
  }

  if (ptList->ptThis->ptItsType->iKind == UNICODE0KIND)
  {
    ptList->ptThis->ptValue->pcValue[i++] = '0';
    ptList->ptThis->ptValue->pcValue[i++] = '0';
    ptList->ptThis->ptValue->pcValue[i++] = '0';
    ptList->ptThis->ptValue->pcValue[i++] = '0';
  }
  if (ptList->ptThis->ptItsType->iKind == ASCII0KIND)
  {
    ptList->ptThis->ptValue->pcValue[i++] = '0';
    ptList->ptThis->ptValue->pcValue[i++] = '0';
  }

  if (  (!ptList->ptThis->ptItsType->iLessAllowed)
      ||(bNoLengthIndicator == TRUE)
     )
  {
    /* If not less allowed, then fill with zero's                         */
    /* If No length indicator specified, and a shorter string is accepted */
    /* then also fill with zero's                                         */
    while (i < ptList->ptThis->ptItsType->iSizeInBytes * 2 + 2)
    {
      ptList->ptThis->ptValue->pcValue[i++] = '0';
      ptList->ptThis->ptValue->pcValue[i++] = '0';
    }
  }

  ptList->ptThis->ptValue->pcValue[i] = 0;
  ptList->ptThis->ptValue->bFixed = TRUE;

  Parse(pcToken);

  return TRUE;
}

int Parse_TVarDeclarationList(
  char *pcToken,
  struct TVariableList *ptList)
/****************************************************************************/
/* VARLIST = VARIABLEVAR                                                    */
/* VARLIST = VARIABLEVAR ',' VARLIST                                        */
/*                                                                          */
/* and implicitly coded:                                                    */
/*                                                                          */
/****************************************************************************/
{
  int       iTokenType;

  iTokenType = Parse(pcToken);

  ptList->ptThis = newVariable(NULL);

  if (iTokenType == IDENTIFIER_TOKEN)
  {
    ptList->ptThis->pcName = my_strdup(pcToken);
  }
  else
  {
    AddError2("Identifier expected, %s found", pcToken);
    return FALSE;
  }

  Parse(pcToken);

  if (*pcToken == '=')
  {
    /* The script writer is a comedian, or (s)he knows about the new faci-  */
    /* lities of BSE.                                                       */
    /* One of the new facilities is that the following construction is      */
    /* allowed now:                                                         */
    /* VAR a = 0x00, b.                                                     */

    Parse(pcToken);             /* We allow any token as a value.                       */

    ptList->ptThis->ptValue = newValue();
    ptList->ptThis->ptValue->pcValue = my_strdup(pcToken);
    ptList->ptThis->ptValue->bFixed = TRUE;

    if (strcmp(pcToken, "FILE") == 0)
    {
      iTokenType = Parse(pcToken);
      if (iTokenType != STRING_TOKEN)
      {
        AddError1("Filename expected (must be surrounded with \") after FILE ");
        return FALSE;
      }
      ptList->ptThis->ptValue->pcFileName = my_strdup(pcToken);
      iTokenType = Parse(pcToken);
      if (iTokenType != IDENTIFIER_TOKEN)
      {
        AddError1("File Operation missing");
        return FALSE;
      }
      ptList->ptThis->ptValue->pcFileMode = my_strdup(pcToken);
      free(ptList->ptThis->ptValue->pcValue);
      ptList->ptThis->ptValue->pcValue = NULL;

    }


    Parse(pcToken);
  }

  if (*pcToken == ',')
  {
    /* There are more variable definitions. Let's do it recursive, just     */
    /* for fun.                                                             */
    ptList->ptNext = newVariableList();
    if (!Parse_TVarDeclarationList(pcToken, ptList->ptNext))
      return FALSE;
  }
  else
  {
    /* There are no more variable definitions. Oops. We read one token      */
    /* too much. Put it back in the input stream.                           */
    UngetToken();
  }
  return TRUE;
}

int Parse_TParameterInvokeList(
  char *pcToken,
  struct TVariableList *ptList,
  struct TParameterList *ptDefList,
  struct TParameterList *pAlternateDefList,
  struct TVariableList *ptReference,
  int16 bCheckDefList)
/****************************************************************************/
/* VARLIST = VAR                                                            */
/* VARLIST = VAR ',' VARLIST                                                */
/*                                                                          */
/* and implicitly coded:                                                    */
/*                                                                          */
/* VAR = (empty)                                                            */
/* VAR = VARIABLEVAR                                                        */
/* VAR = FIXEDVAR                                                           */
/****************************************************************************/
{
  int       iTokenType;
  struct TParameterList *ptNextDefList = NULL;
  struct TParameterList *ptX;
  struct TFunction *ptNextFunction = NULL;
  struct TType *ptVarType = NULL;
  int       i;

  iTokenType = Parse(pcToken);

  if ((bCheckDefList == TRUE) && (ptDefList == NULL))
  {
    AddError1("Too many parameters in function or event");
    bCheckDefList = FALSE;      /* to prevent this warning multiple times pop up */
    ptList->ptThis = newVariable(NULL);
  }
  else if (bCheckDefList == FALSE)
  {
    ptList->ptThis = newVariable(NULL);
  }
  else
  {
    /* Find out the expected type of the variable. This can be done only by */
    /* referring to the DefList, which is a list of parameters referring to */
    /* records in the TType administration.                                 */
    /* First process eventually repeat records in this list...              */
    /* and eventually contant records in this list...                       */

    while ((ptDefList != NULL)
           &&
           ((ptDefList->ptRepRec != NULL)
            || (ptDefList->ptThis->ptTypeDef == NULL)
            || (ptDefList->ptThis->ptLengthFromField != NULL)
            || (ptDefList->ptThis->ptLengthToField != NULL)))
    {
      if (ptDefList->ptRepRec != NULL)
      {
        if (ptDefList->ptRepRec->iBegin)
        {
          /* We found the begin of a repeated block. Now find out how     */
          /* many times we can expect the referenced block..              */
          if (ptDefList->ptRepRec->ptParameter == NULL)
          {
            AddError2("Parameter %s should be eigther constant or unspecified",
                      ptDefList->ptRepRec->ptParameter->pcName);
            i = 0;
          }
          else
          {
            if (ptDefList->ptRepRec->ptParameter->ptValue == NULL)
            {
              AddError2("Parameter %s must be a constant! ",
                        ptDefList->ptRepRec->ptParameter->pcName);
              i = 0;
            }

            else
              i =
                GetInteger(ptDefList->ptRepRec->ptParameter->ptValue->pcValue);
          }

          if (i == 0)
            ptDefList = ptDefList->aptNext[1];
          else
            ptDefList->ptRepRec->iCounter = i;

          ptDefList = ptDefList->aptNext[0];
        }
        else
        {
          /* We found the end of a repeated block. Jump to the beginning */
          /* Decrement counter and continue..                            */

          ptDefList = ptDefList->aptNext[1];
          ptDefList->ptRepRec->iCounter--;

          if (ptDefList->ptRepRec->iCounter == 0)
            ptDefList = ptDefList->aptNext[1];

          ptDefList = ptDefList->aptNext[0];
        }
      }
      else if ((ptDefList->ptThis->ptLengthFromField != NULL)
               || (ptDefList->ptThis->ptLengthToField != NULL))
      {
        /* We found a length indicating field. Skip this record.           */
        ptDefList = ptDefList->aptNext[0];
      }
      else
      {
        /* We found a constant record. Skip this record...                 */
        ptDefList = ptDefList->aptNext[0];
      }
    }
    if (ptDefList == NULL)
    {
      AddError1("Too many parameters");
      return FALSE;
    }

    ptVarType = ptDefList->ptThis->ptTypeDef;

    /* ptVarType might point to the internal "Command Specific" type. If so, */

    /* Now we know the type of the to be read variable. Allocate space for  */
    /* that variable, and assign the expected type to it.                   */
    if (iTokenType == COMMA_TOKEN)
      ptList->ptThis = newVariable(ptVarType);
    else if (iTokenType == HEX_NUMBER_TOKEN)
      ptList->ptThis = newVariable(ptVarType);
    else if (iTokenType == IDENTIFIER_TOKEN)
      ptList->ptThis = newVariable(ptVarType);
    else if (iTokenType == STRING_TOKEN)
      ptList->ptThis = newVariable(ptVarType);
    else
      ptList->ptThis = newVariable(ptVarType);

    if ((iTokenType == HEX_NUMBER_TOKEN)
        || (iTokenType == DEC_NUMBER_TOKEN) || (iTokenType == BIN_NUMBER_TOKEN))
    {
      ptDefList->ptThis->ptValue = ptList->ptThis->ptValue;
    }
    /* Prepare reading the next parameter, by shifting ptDefList to the     */
    /* next record. The next record can depend on the just read parameter.  */
    if (ptDefList->iFollowedByUnion)
    {
      /* Followed by a union. Find out the value determining which branch   */
      /* to choose.                                                         */

      if ((iTokenType != BIN_NUMBER_TOKEN)
          && (iTokenType != HEX_NUMBER_TOKEN)
          && (iTokenType != DEC_NUMBER_TOKEN))
      {
        if (iTokenType == COMMA_TOKEN)
        {
          /* the field is empty. Use the else-branch of the union.          */
          ptNextDefList = ptDefList->aptNext[UNIONARRAYSIZE];
          UngetToken();         /* The comma will be read again.            */
        }
        else
        {
          AddError2("Constant expected, %s found", pcToken);
          return FALSE;
        }
      }
      else
      {
        i = GetInteger(pcToken);
        if ((i < 0) || (i > UNIONARRAYSIZE))
        {
          AddError2("Warning. Constant %s out of range.", pcToken);
          i = UNIONARRAYSIZE + 1;       /* use else-branche, if available..       */
        }
        ptNextDefList = ptDefList->aptNext[i];
      }
    }
    else
    {
      /* Not followed by a union. The first element [0] indicates the next  */
      /* typedefinition record.                                             */
      ptNextDefList = ptDefList->aptNext[0];
    }
  }

  if (iTokenType == IDENTIFIER_TOKEN)
    Parse_QVariableVar(pcToken, ptList);
  else if (iTokenType == HEX_NUMBER_TOKEN)
    Parse_QFixedVar(pcToken, ptList);
  else if (iTokenType == DEC_NUMBER_TOKEN)
    Parse_QFixedVar(pcToken, ptList);
  else if (iTokenType == BIN_NUMBER_TOKEN)
    Parse_QFixedVar(pcToken, ptList);
  else if (iTokenType == STRING_TOKEN)
  {
    if (ptDefList->ptThis->ptLengthIndicator == NULL)
    {
      Parse_QStringVar(pcToken, ptList,TRUE);
    }
    else
    {
      Parse_QStringVar(pcToken, ptList,FALSE);
    }
  }
  else if (iTokenType == COMMA_TOKEN)   /* nothing.. Empty parameter..   */
  {
    if (bCheckDefList == FALSE)
    {
      AddError1("Empty parameters not allowed in statemachine invokings");
    }
  }
  else if (iTokenType == CLOSE_PARENTHESES_TOKEN) /* last empty paramete..  */ ;
  else
  {
    AddError2("Unexpected token %s", pcToken);
    return FALSE;
  }

  if (*pcToken == ',')
  {
    ptList->ptNext = newVariableList();
    if (!Parse_TParameterInvokeList(pcToken,
                                    ptList->ptNext,
                                    ptNextDefList,
                                    NULL, ptReference, bCheckDefList))
    {
      return FALSE;
    }
  }
  else
  {
    UngetToken();
    if (ptNextDefList != NULL)
    {
      /* Find out whether there are really no more DefList records          */
      /* by walking to the end of the list but stop when a ptX->ptThis      */
      /* is found.                                                          */
      ptX = ptNextDefList;
      while ((ptX != NULL)
             && (((ptX->ptThis == NULL)
                  && (ptX->aptNext[0] != NULL))
                 || ((ptX->ptRepRec != NULL) && (ptX->aptNext[1] != NULL))))
      {
        if (ptX->ptRepRec != NULL)
        {
          ptX = ptX->aptNext[1];
          if (ptX->ptRepRec->iCounter > 1)
          {
            AddError1("Event or Command has too few parameters ");
            ptX = NULL;
          }
          else
          {
            ptX = ptX->aptNext[1]->aptNext[0];
          }
        }
        else
        {
          ptX = ptX->aptNext[0];
        }
      }

      if (   (ptX != NULL)
          && (ptX->ptThis != NULL)
          && (ptX->ptThis->ptTypeDef != NULL)
         )
      {
        AddError1("Event or Command has too few parameters ");
      }
      /* We expect that the parser is still in sync. Therefore stopping     */
      /* the parser (return FALSE) is not needed.                           */
    }
  }

  /* The information is read without failures. Now try to find the variable */
  /* in the reference list and use this reference if available.             */
  /*                                                                        */
  /* Extra explanation : When the parameter list is written as:             */
  /* (first), (rest)                                                        */
  /*                                                                        */
  /* with (first) the first parameter; and (rest) the zero or more next     */
  /* parameters; (first) is only scanned and checked; and (rest) is scan-   */
  /* ned, checked and linked to the reference list. This is because this    */
  /* function is recursive; and (rest) is processed by recursive calls of   */
  /* this function.                                                         */

  /* If there is a VarName, and the VarName is not a printable version of   */
  /* a string variable, and the VarName is not a function name              */

  if ((ptList->ptThis->pcName != NULL)
      && (ptList->ptThis->pcName[0] != '\"') && (ptNextFunction == NULL))
  {
    ConnectVarToVarList(&(ptList->ptThis),      /* The variable just read..      */
                        ptReference,    /* The common VarList            */
                        _ptDataList,    /* globally defined datalist     */
                        ptDefList       /* checklist for variable type   */
      );
  }

  return TRUE;
}



int Parse_TAction(
  char *pcToken,
  struct TAction *ptAction,
  struct TVariableList *ptReference)
/****************************************************************************/
/*                                                                          */
/* ACTION = STRINGTOKEN PARAMETERS                                          */
/* ACTION = EXPRESSION                                                      */
/*                                                                          */
/* The first token of an ACTION must have been parsed and stored in pcToken */
/* The token following the ACTION is parsed.                                */
/****************************************************************************/
{
  struct TParameterList *ptDefList = NULL;
  int       iAction;
  int       iTokenType;

  iAction = 0;
  ptAction->iSourceLineNumber = CurrentLine();  /* usefull in error-msgs     */

  /* First try if the just parsed token is the identifier of an event.      */
  ptAction->ptEvent = FindEvent(pcToken);
  ptAction->ptFunction = NULL;
  ptAction->iActionType = EVENT_ACTION;

  /* If it was not an event,                                                */
  if (ptAction->ptEvent == NULL)
  {
    /* Then try if the just parsed token is the identifier of a function    */
    ptAction->ptFunction = FindFunction(pcToken);
    ptAction->iActionType = FUNCTION_ACTION;

    /* If it wasn't a function too,                                         */
    if (ptAction->ptFunction == NULL)
    {
      /* Then it can be an internal event.                                    */
      ptAction->iActionType = FindInteraction(pcToken);

      if (ptAction->iActionType == 0)
      {
        ptAction->ptStateMachine = FindStateMachine(pcToken);
        if (ptAction->ptStateMachine != NULL)
        {
          ptAction->iActionType = STATEMACHINE_ACTION;
        }
        else
        {
          if (!Parse_TExpression(pcToken, ptAction, ptReference))
          {
            return FALSE;
          }
          TypeCheck(ptAction->ptExpression);
          ptAction->iActionType = ARITHMETRIC_ACTION;
        }
      }
    }
  }

  /* Function, Event, arithmetric or internal action read successfully.      */

  if (ptAction->ptEvent != NULL)
  {
    ptDefList = ptAction->ptEvent->ptParameters;
  }
  else if (ptAction->ptFunction != NULL)
  {
    ptDefList = ptAction->ptFunction->ptSendParameters;
  }
  else if (ptAction->ptStateMachine != NULL)
  {
    /************************************************************************/
    /* The parameter list of a statemachine invoking here will not be       */
    /* parsed using the normal Parse_TParameterInvokeList invoke as it is   */
    /* being done for functions and events. It cannot be done because the   */
    /* Parse_TParameterInvokeList expects a list of parameters rather than  */
    /* a list of variables to compare the types.. Unfortunately..           */
    /************************************************************************/

    if (ptAction->ptStateMachine->ptParameters != NULL)
    {
      if (!ParseChar('('))
        return FALSE;

      ptAction->ptVarList = newVariableList();
      if (!Parse_TParameterInvokeList(pcToken,
                                      ptAction->ptVarList,
                                      NULL, NULL, ptReference, FALSE))
      {
        return FALSE;
      }

      if (!ParseChar(')'))
        return FALSE;
    }
    ptDefList = NULL;           /* Because we don't expect more parameters..          */

  }
  else
  {
    iAction = ptAction->iActionType;
    if (iAction == TIMER_ACTION)
      ptDefList = &TimerParamList;
    else if (iAction == MTIMER_ACTION)
      ptDefList = &TimerParamList;
    else if (iAction == RTIMER_ACTION)
      ptDefList = &TimerTwoParametersList;
    else if (iAction == RMTIMER_ACTION)
      ptDefList = &TimerTwoParametersList;
    else if (iAction == WAIT_ACTION)
      ptDefList = &OneParam;
    else if (iAction == CLEAR_ACTION)
      ptDefList = &OneParam;
    else if (iAction == R_SIG_ACTION)
      ptDefList = &SignalParam;
    else if (iAction == S_SIG_ACTION)
      ptDefList = &SignalParam;
    else if (iAction == TERMINATE_ACTION)
      ptDefList = NULL;
    else if (iAction == COMPARE_ACTION)
      ptDefList = &TwoParam;
    else if (iAction == PROT_PROTOCOL_ACTION)
      ptDefList = NULL;
    else if (iAction == ASCII_PROTOCOL_ACTION)
      ptDefList = NULL;
    else if (iAction == TCI_PROTOCOL_ACTION)
      ptDefList = NULL;
    else if (iAction == PROT_WRITEDATA_ACTION)
      ptDefList = &FourParam;
    else if (iAction == PROT_READDATA_ACTION)
      ptDefList = &FourParam;
    else if (iAction == BREAK_ACTION)
      ptDefList = NULL;
    else if (iAction == PRINT_ACTION)
      ptDefList = &OneParam;
    else if (iAction == RESCUE_ACTION)
      ptDefList = NULL;
    else if (iAction == ARITHMETRIC_ACTION)
      ptDefList = NULL;
    else if (iAction == IF_ACTION)
      ptDefList = NULL;
    else if (iAction == END_ACTION)
      ptDefList = NULL;
    else if (iAction == CTS_LOW_ACTION)
      ptDefList = NULL;
    else if (iAction == DSR_LOW_ACTION)
      ptDefList = NULL;
    else if (iAction == DSR_HIGH_ACTION)
      ptDefList = NULL;
    else if (iAction == CTS_HIGH_ACTION)
      ptDefList = NULL;
    else if (iAction == SOCKET_GENERAL_ERROR)
      ptDefList = NULL;
    else if (iAction == SOCKET_SEND_ERROR)
      ptDefList = NULL;
    else if (iAction == SOCKET_RECEIVE_ERROR)
      ptDefList = NULL;
    else if (iAction == SOCKET_CONNECT_ERROR)
      ptDefList = NULL;
    else if (iAction == SOCKET_DISCONNECT_ERROR)
      ptDefList = NULL;
    else if (iAction == SOCKET_ACCEPT_ERROR)
      ptDefList = NULL;
    else if (iAction == SOCKET_UNKNOWN_ERROR)
      ptDefList = NULL;
    else if (iAction == CONNECTION_LOST)
      ptDefList = NULL;
    else if (iAction == CONNECTION_ESTABLISHED)
      ptDefList = NULL;
    else
      FatalError1("Internal error: unknown iAction");
  }

  Parse(pcToken);

  if (*pcToken == '(')
  {
    if (iAction != IF_ACTION)
    {
      ptAction->ptVarList = newVariableList();

      if (!Parse_TParameterInvokeList(pcToken,
                                      ptAction->ptVarList,
                                      ptDefList, NULL, ptReference, TRUE))
      {
        return FALSE;
      }
    }
    else                        /* iAction == IF_ACTION */
    {
      Parse(pcToken);           /* First token of expression must have been parsed.   */
      if (!Parse_TExpression(pcToken, ptAction, ptReference))
      {
        return FALSE;
      }
      TypeCheck(ptAction->ptExpression);

    }
    if (!ParseChar(')'))
      return FALSE;
    Parse(pcToken);
  }
  else
  {
    /* parameterless event. The ptVarList remains NULL                      */

    while (ptDefList != NULL)
    {
      if (ptDefList->ptThis != NULL)
      {
        if (ptDefList->ptThis->ptValue == NULL)
        {
          AddError1("Parameters expected ");
          ptDefList = NULL;
        }
        else
          ptDefList = ptDefList->aptNext[0];
      }
    }
  }

  if (*pcToken == '[')
  {
    iTokenType = Parse(pcToken);
    if ((iTokenType != DEC_NUMBER_TOKEN)
        && (iTokenType != BIN_NUMBER_TOKEN) && (iTokenType != HEX_NUMBER_TOKEN))
    {
      AddError2("Number expected, %s found", pcToken);
      return FALSE;
    }
    ptAction->iNrOfReceptions = GetInteger(pcToken);
    if (!ParseChar(']'))
      return FALSE;
    Parse(pcToken);
  }
  return TRUE;

}

int Parse_TActionList(
  char *pcToken,
  struct TActionList *ptActionList,
  struct TVariableList *ptReference,
  int iAlternative)
/****************************************************************************/
/*                                                                          */
/* ACTIONLIST = ACTION                                                      */
/* ACTIONLIST = ACTION ACTIONLIST                                           */
/*                                                                          */
/* The first token of an ACTIONLIST must have been parsed. The token        */
/* following an ACTIONLIST will be parsed and stored in pcToken.            */
/****************************************************************************/
{
  if (ptActionList->ptThis == NULL)
  {
    ptActionList->ptThis = newAction();
  }

  while (ptActionList->ptThis->iActionType == 0)
  {
    if (!Parse_TAction(pcToken, ptActionList->ptThis, ptReference))
    {
      return FALSE;
    }
    if (iAlternative == 1)
    {
      /* The action must be performed on an alternative board. This only    */
      /* works for internal transitions and for commands! Check this...     */
      if (ptActionList->ptThis->iActionType == EVENT_ACTION)
        AddError1("Alternative statemachine doesn't work for events");
      if (ptActionList->ptThis->iActionType == STATEMACHINE_ACTION)
        AddError1
          ("Alternative statemachine doesn't work for statemachine invokings");
      if (ptActionList->ptThis->iActionType == ARITHMETRIC_ACTION)
        AddError1("Alternative statemachine doesn't work for arithmetric");
      if (ptActionList->ptThis->iActionType == TIMER_ACTION)
        AddError1("Alternative statemachine doesn't work for timers");
      if (ptActionList->ptThis->iActionType == MTIMER_ACTION)
        AddError1("Alternative statemachine doesn't work for mtimers");
      if (ptActionList->ptThis->iActionType == RTIMER_ACTION)
        AddError1("Alternative statemachine doesn't work for rtimers");
      if (ptActionList->ptThis->iActionType == RMTIMER_ACTION)
        AddError1("Alternative statemachine doesn't work for rtimers");
      if (ptActionList->ptThis->iActionType == WAIT_ACTION)
        AddError1("Alternative statemachine doesn't work for wait");
      if (ptActionList->ptThis->iActionType == CLEAR_ACTION)
        AddError1("Alternative statemachine doesn't work for clear");
      if (ptActionList->ptThis->iActionType == R_SIG_ACTION)
        AddError1("Alternative statemachine doesn't work for r_sig");
      if (ptActionList->ptThis->iActionType == S_SIG_ACTION)
        AddError1("Alternative statemachine doesn't work for s_sig");
      if (ptActionList->ptThis->iActionType == TERMINATE_ACTION)
        AddError1("Alternative statemachine doesn't work for terminate");

      if (ptActionList->ptThis->iActionType == COMPARE_ACTION)
        AddError1("Alternative statemachine doesn't work for comparisations");
      if (ptActionList->ptThis->iActionType == PROT_WRITEDATA_ACTION)
        AddError1("Alternative statemachine doesn't work for WriteData");
      if (ptActionList->ptThis->iActionType == PROT_READDATA_ACTION)
        AddError1("Alternative statemachine doesn't work for ReadData");
      if (ptActionList->ptThis->iActionType == BREAK_ACTION)
        AddError1("Alternative statemachine doesn't work for Break");
      if (ptActionList->ptThis->iActionType == PRINT_ACTION)
        AddError1("Alternative statemachine doesn't work for Print");
      if (ptActionList->ptThis->iActionType == RESCUE_ACTION)
        AddError1("Alternative statemachine doesn't work for Rescue");
      if (ptActionList->ptThis->iActionType == IF_ACTION)
        AddError1("Alternative statemachine doesn't work for If");
      if (ptActionList->ptThis->iActionType == END_ACTION)
        AddError1("Alternative statemachine doesn't work for End");
    }
    if ((*pcToken != ';')
        && (*pcToken != ',')
        && (*pcToken != '.') && (strcmp(pcToken, "->") != 0))
    {
      /* Another Action can be parsed...                                    */
      ptActionList->ptNext = newActionList();
      ptActionList = ptActionList->ptNext;
      ptActionList->ptThis = newAction();
    }
  }
  return TRUE;
}


int Parse_TStateTransition(
  char *pcToken,
  struct TStateList **pptList,
  struct TVariableList *ptReference,
  int *piTransitionType)
/****************************************************************************/
/*                                                                          */
/* STATETRANSITION = IDENTIFIER_TOKEN ':' ACTIONLIST NEXTSTATE              */
/* STATETRANSITION = IDENTIFIER_TOKEN ':' ':' ACTIONLIST NEXTSTATE          */
/*                                                                          */
/* NEXTSTATE = ','                                                          */
/* NEXTSTATE = '.'                                                          */
/* NEXTSTATE = ';' IDENTIFIER_TOKEN '.'                                     */
/*                                                                          */
/****************************************************************************/
/* *piTransitionType = 0x00 : Normal transition.                            */
/*                   = 0x01 : Alternative board transition.                 */
{
  struct TState *ptState;
  struct TTransition *ptTransition;

  *piTransitionType = 0x00;

  /* The IDENTIFIER_TOKEN has already been read. Start reading the :        */

  if (!ParseChar(':'))
    return FALSE;

  if (*pptList == NULL)
  {
    /* No states are defined yet for this state machine                     */
    *pptList = newStateList();
    (*pptList)->ptThis = newState();
    (*pptList)->ptThis->pcName = my_strdup(pcToken);
    ptState = (*pptList)->ptThis;
  }
  else
  {
    /* if states are defined, find the mentioned state.                     */
    /* This function makes a new state if the mentioned state is not found  */
    Find_StateName(pcToken, *pptList, &ptState);
  }

  /* Make a new action entry in the action list.                            */
  Add_TransitionEntry(ptState, &ptTransition);
  ptTransition->ptActionList = newActionList();

  /* Check whether it's intended for the alternative statemachine.          */
  Parse(pcToken);
  if (strcmp(pcToken, ":") == 0)
  {
    *piTransitionType |= 0x01;

    ptTransition->iAlternative = 1;
    Parse(pcToken);
  }

  /* and fill it while reading the action.                                  */
  if (!Parse_TActionList
      (pcToken, ptTransition->ptActionList, ptReference,
       ptTransition->iAlternative))
    return FALSE;

  switch (*pcToken)
  {
    case ',':
      ptTransition->pcNextStateName = strdup("");
      break;
    case '.':
      ptTransition->pcNextStateName = NULL;
      break;
    case ';':
      if (!ParseIdentifier(pcToken))
        return FALSE;
      ptTransition->pcNextStateName = strdup(pcToken);
      if (!ParseChar('.'))
        return FALSE;
      break;
    case '-':                  /* Actually, this is the first character of the -> token     */
      /* Earlier has been made sure that a -> token is parsed iso  */
      /* another token starting with a - character.                */
      if (!ParseIdentifier(pcToken))
        return FALSE;
      ptTransition->pcNextStateName = strdup(pcToken);
      if (!ParseChar('.'))
        return FALSE;
      ptTransition->iInstantNextState = TRUE;
      break;
    default:
      AddError2("Unexpected token %s", pcToken);
      break;
  }

  return TRUE;
}


int Parse_TStateList(
  char *pcToken,
  struct TStateList **pptList,
  struct TVariableList *ptReference,
  int *piTransitionType)
/****************************************************************************/
/*                                                                          */
/* STATELIST = STATETRANSITION                                              */
/* STATELIST = STATETRANSITION STATELIST                                    */
/*                                                                          */
/****************************************************************************/
{
  int       i;

  *piTransitionType = 0;

  if (!Parse_TStateTransition(pcToken, pptList, ptReference, &i))
    return FALSE;
  *piTransitionType |= i;

  while (Parse(pcToken) == IDENTIFIER_TOKEN)
  {
    if (!Parse_TStateTransition(pcToken, pptList, ptReference, &i))
      return FALSE;
    *piTransitionType |= i;
  }
  UngetToken();
  return TRUE;
}


int Parse_TStateMachine(
  char *pcToken,
  struct TStateMachine *ptThis)
/****************************************************************************/
/*                                                                          */
/* STATEMACHINE = IDENTIFIERTOKEN '(' VARLIST ')' '='                       */
/*                 '{' STATETRANSITIONLIST '}'                              */
/*                                                                          */
/****************************************************************************/
{
  struct TVariableList *ptWalker;
  int       iTransitionType;

  ptThis->pcName = my_strdup(pcToken);

  Parse(pcToken);
  if (*pcToken == '(')
  {
    ptThis->ptParameters = newVariableList();
    if (!Parse_TVarDeclarationList(pcToken, ptThis->ptParameters))
    {
      return FALSE;
    }
    if (!ParseChar(')'))
      return FALSE;
    Parse(pcToken);
  }
  if (*pcToken != '=')
  {
    AddError2("Error: reading token %s. Expected token: =", pcToken);
    return FALSE;
  }

  Parse(pcToken);

  if (strcmp(pcToken, "hidden") == 0)
  {
    ptThis->iHidden = TRUE;
    Parse(pcToken);
  }

  if (*pcToken != '{')
  {
    AddError2("Error: reading token %s. Expected token: {", pcToken);
    return FALSE;
  }

  Parse(pcToken);

  if (strcmp(pcToken, "VAR") == 0)
  {
    ptThis->ptLocalVars = newVariableList();

    if (!Parse_TVarDeclarationList(pcToken, ptThis->ptLocalVars))
    {
      return FALSE;
    }

    if (!ParseChar('.'))
      return FALSE;

    /* both the parameter list and the local var list are read now. To      */
    /* simplify variable references in the state machine, the parameter     */
    /* list is connected to the tail of the local var list. The reference   */
    /* to the start of the parameter list remains valid, so external        */
    /* reference to the paramaters remains possible.                        */

    ptWalker = ptThis->ptLocalVars;
    while (ptWalker->ptNext != NULL)
      ptWalker = ptWalker->ptNext;
    ptWalker->ptNext = ptThis->ptParameters;

    Parse(pcToken);
  }
  else
  {
    /* See comment block in then-part, ten lines above...                   */
    ptThis->ptLocalVars = ptThis->ptParameters;
  }

  if (strcmp(pcToken, "RESCUE") == 0)
  {
    if (Parse(pcToken) != IDENTIFIER_TOKEN)
    {
      AddError2("RESCUE: statename expected, %s found", pcToken);
      return FALSE;
    }
    ptThis->pcRescueStateName = strdup(pcToken);

    if (!ParseChar('.'))
      return FALSE;

    Parse(pcToken);

  }

  if (ptThis->ptParameters != NULL)
  {
    /* The head of the ptParameter list is (if existing) also referred to   */
    /* by the tail of the ptLocalVars list.                                 */

    ptThis->ptParameters->iRefCount++;
  }

  if (iLastParsedTokenType != IDENTIFIER_TOKEN)
  {
    AddError1("State name expected");
    return FALSE;
  }

  if (!Parse_TStateList
      (pcToken, &ptThis->ptStates, ptThis->ptLocalVars, &iTransitionType))
  {
    return FALSE;
  }

  if ((iTransitionType & 0x01) == 0x01)
  {
    ptThis->iAlternative = 0x01;
  }

  if (!ParseChar('}'))
    return FALSE;

  /* The real work has been done now. To check if all declared variables    */
  /* are actually used, we do a final check. This check will not result in  */
  /* error discovery, only warnings can be generated.                       */

  ptWalker = ptThis->ptLocalVars;

  while (ptWalker != NULL)
  {
    if (ptWalker->ptThis->ptItsType == NULL)
    {
      AddWarning3("In statemachine %s, variable %s is never used",
                  ptThis->pcName, ptWalker->ptThis->pcName);
    }
    ptWalker = ptWalker->ptNext;
  }

  return TRUE;
}


int Parse_TStateMachineList(
  char *pcToken,
  struct TStateMachineList *ptList)
/****************************************************************************/
/*                                                                          */
/* STATEMACHINELIST = STATEMACHINE                                          */
/* STATEMACHINELIST = STATEMACHINE STATEMACHINELIST                         */
/*                                                                          */
/****************************************************************************/
{
  /* Create space for the first statemachine                                */
  ptList->ptThis = newStateMachine();
  Parse(pcToken);

  /* And try to fill it.                                                    */
  if (!Parse_TStateMachine(pcToken, ptList->ptThis))
    return FALSE;
  Parse(pcToken);

  /* while more statemachines can be parsed                                 */
  while (iLastParsedTokenType == IDENTIFIER_TOKEN)
  {
    /* allocate space for one more statemachine                             */
    ptList->ptNext = newStateMachineList();
    ptList = ptList->ptNext;
    ptList->ptThis = newStateMachine();

    /* and try to fill it.                                                  */
    if (!Parse_TStateMachine(pcToken, ptList->ptThis))
      return FALSE;

    Parse(pcToken);
  }

  /* Read one token too much. Put last read token back.                     */
  UngetToken();

  return TRUE;
}


int Parse_TStateMachineInvokings(
  char *pcToken,
  struct TStateMachineList *ptList,
  struct TVariableList *ptRefVars,
  char *pcDeviceName,
  int iAlternative)
/****************************************************************************/
/* Parse a statemachine invoking. The structure looks like:                 */
/* Master(a,b,c,d)                                                          */
/* Slave1(a,b)                                                              */
/* Slave2(c,d).                                                             */
/****************************************************************************/
{
  int       iTokenType;
  struct TStateMachine *ptStateMachine;
  struct TVariableList *ptInvokeVars;

  iTokenType = Parse(pcToken);  /* The statemachine name to be invoked.      */

  while (iTokenType == IDENTIFIER_TOKEN)
  {
    ptStateMachine = FindStateMachine(pcToken);
    if (ptStateMachine == NULL)
    {
      AddError2("State machine %s not found", pcToken);
      return FALSE;
    }

    /* Found. Make a copy of that statemachine.                             */
    ptList->ptThis = Duplicate_StateMachine(ptStateMachine);

    /* re-nitialise some fields in this statemachine.                       */
    ptList->ptThis->pcDeviceName = my_strdup(pcDeviceName);

    if ((ptStateMachine->iAlternative == 1) && (iAlternative == -1))
    {
      AddError2("Statemachine %s requires an additional device",
                ptStateMachine->pcName);
    }

    ptList->ptThis->iAlternative = iAlternative;

    iTokenType = Parse(pcToken);
    if (iTokenType == OPEN_PARENTHESES_TOKEN)
    {
      ptInvokeVars = newVariableList();
      if (!Parse_TParameterInvokeList(pcToken,
                                      ptInvokeVars,
                                      NULL, NULL, ptRefVars, FALSE))
      {
        return FALSE;
      }

      if (!ParseChar(')'))
        return FALSE;

      Connect_Invocations(ptList->ptThis->ptParameters, ptInvokeVars);

      delVariableList(&(ptInvokeVars));
      iTokenType = Parse(pcToken);
    }
    else
    {
      /* no parameters provided. Check if parameters were needed..          */
      if (ptList->ptThis->ptParameters != NULL)
      {
        AddError2("Statemachine invoking %s requires parameters",
                  ptList->ptThis->pcName);
      }
    }

    if (iTokenType == IDENTIFIER_TOKEN)
    {
      ptList->ptNext = newStateMachineList();
      ptList = ptList->ptNext;
    }
  }
  if (strcmp(pcToken, ".") != 0)
  {
    AddError1("A device definition must terminate with a period.");
    return FALSE;
  }

  return TRUE;
}

int Parse_TDataList(
  char *pcToken,
  struct TDataList *ptDataList)
{
  int       iTokenType;

  ptDataList->ptThis = newData();
  if (Parse_TData(pcToken, ptDataList->ptThis) == FALSE)
    return FALSE;

  iTokenType = Parse(pcToken);
  if (iTokenType == IDENTIFIER_TOKEN)
  {
    UngetToken();
    ptDataList->ptNext = newDataList();
    return Parse_TDataList(pcToken, ptDataList->ptNext);
  }
  else
  {
    UngetToken();
    return TRUE;
  }
}


int Parse_TData(
  char *pcToken,
  struct TData *ptData)
{
  int       iTokenType;

  iTokenType = Parse(pcToken);
  ptData->ptItsType = NULL;
  if (iTokenType == IDENTIFIER_TOKEN)
  {
    ptData->pcName = my_strdup(pcToken);
    ptData->ptItsType = _ptDataType;
    iTokenType = Parse(pcToken);
    if (*pcToken == '=')
    {
      /* An initialised databuffer found. Parse the initialisation.         */
      if (ParseString(pcToken) == FALSE)
        return FALSE;

      ptData->pcValue = my_strdup(pcToken);
      ptData->bFixed = TRUE;

      return TRUE;

    }
    else
    {
      /* An uninitialised databuffer found. The name is already parsed, so  */
      /* we are ready and can return.                                       */
      UngetToken();
      return TRUE;
    }
  }
  else
  {
    AddError1("Data-variable name expected");
    return FALSE;
  }
}

int Parse_TIOList(
  char *pcToken,
  struct TIOList *ptIOList)
{
  int       iTokenType;

  iTokenType = Parse(pcToken);
  while (iTokenType == IDENTIFIER_TOKEN)
  {
    ptIOList->ptThis = newIO();
    if (!Parse_TIO(pcToken, ptIOList->ptThis))
      return FALSE;
    ptIOList->ptNext = newIOList();
    ptIOList = ptIOList->ptNext;
    iTokenType = Parse(pcToken);
  }

  UngetToken();
  return TRUE;
}

int Parse_TIO(
  char *pcToken,
  struct TIO *ptIO)
{
  int       iTokenType;
  static int iSeqNr = 256;
  int       iNumber;

  ptIO->pcName = my_strdup(pcToken);
  if (!ParseChar('='))
    return FALSE;
  if (!ParseChar('{'))
    return FALSE;

  iTokenType = Parse(pcToken);
  if (iTokenType != IDENTIFIER_TOKEN)
  {
    AddError2("IO Channel name expected, %s found", pcToken);
    return FALSE;
  }
  ptIO->iNumber = iSeqNr++;
  ptIO->pcIOPortName = my_strdup(pcToken);

  if (!ParseChar(','))
    return FALSE;

  iTokenType = Parse(pcToken);
  if (iTokenType != IDENTIFIER_TOKEN)
  {
    AddError2("Protocol name expected, %s found", pcToken);
    return FALSE;
  }
  if (strcmp(pcToken, "PROT") == 0)
  {
    /* Max parameter lenght of a prot event is 255; because of the single byte */
    /* length field. The PROT event header has 2 bytes: an event code and the  */
    /* length field itself. The Uart protocol adds 1 byte to indicate the     */
    /* type of message (event, command, acl data, sco data, ect)              */
    /* That makes a total of 258 bytes for incoming prot events.               */
    ptIO->iProtocol = PROT_PROTOCOL;
    ptIO->iTerminateTokenLength = 0;
    ptIO->iMaxTokenLength = 258;
    ptIO->pTerminateToken = NULL;

    if (!ParseChar('}'))
      return FALSE;
  }
  else if (strcmp(pcToken, "PASCAL") == 0)
  {
    if (!ParseChar(','))
      return FALSE;
    Parse(pcToken);
    iNumber = GetInteger(pcToken);

    ptIO->iProtocol = ONE_LENGTH_BYTE;
    ptIO->iTerminateTokenLength = 0;
    ptIO->iMaxTokenLength = iNumber;
    ptIO->pTerminateToken = NULL;

    if (!ParseChar('}'))
      return FALSE;

  }
  else if (strcmp(pcToken, "PASCAL_HI_LO") == 0)
  {
    if (!ParseChar(','))
      return FALSE;
    Parse(pcToken);
    iNumber = GetInteger(pcToken);

    ptIO->iProtocol = TWO_LENGTH_BYTES_HI_LO;
    ptIO->iTerminateTokenLength = 0;
    ptIO->iMaxTokenLength = iNumber;
    ptIO->pTerminateToken = NULL;

    if (!ParseChar('}'))
      return FALSE;

  }
  else if (strcmp(pcToken, "PASCAL_LO_HI") == 0)
  {
    if (!ParseChar(','))
      return FALSE;
    Parse(pcToken);
    iNumber = GetInteger(pcToken);

    ptIO->iProtocol = TWO_LENGTH_BYTES_LO_HI;
    ptIO->iTerminateTokenLength = 0;
    ptIO->iMaxTokenLength = iNumber;
    ptIO->pTerminateToken = NULL;

    if (!ParseChar('}'))
      return FALSE;

  }
  else if (strcmp(pcToken, "TOKENTERMINATED") == 0)
  {
    if (!ParseChar(','))
      return FALSE;

    Parse(pcToken);
    iNumber = strlen(pcToken);
    if (iNumber == 0)
      iNumber = 1;              /* For the '\0' string. */

    ptIO->iProtocol = TOKEN_TERMINATED;
    ptIO->iTerminateTokenLength = iNumber;
    ptIO->pTerminateToken = my_strdup(pcToken);

    if (!ParseChar(','))
      return FALSE;
    Parse(pcToken);
    iNumber = GetInteger(pcToken);
    ptIO->iMaxTokenLength = iNumber;

    if (!ParseChar('}'))
      return FALSE;

  }
  else if (strcmp(pcToken,"LOFAR")==0)
  {
     if (!ParseChar(',')) 
     {
       return FALSE;
     }

     ptIO->iProtocol             = LOFAR;
     ptIO->iTerminateTokenLength = 0;
     ptIO->pTerminateToken       = NULL;

     Parse(pcToken);
     iNumber               = GetInteger(pcToken);
     ptIO->iMaxTokenLength = iNumber;

     if (!ParseChar(',')) 
     {
       return FALSE;
     }

	   Parse(pcToken);
	   ptIO->pComConfig = strdup(pcToken);

     if (!ParseChar('}')) 
     {
       return FALSE;
     }

  }
  else
  {
    AddError2("Unknown protocol: %s", pcToken);
    while (strcmp(pcToken, "}") != 0)
    {
      Parse(pcToken);
    }
  }
  return TRUE;
}


int Parse_TestScript(
  char *pcToken)
{
  int       iTokenType;
  int       iAlternativeBoard;

  struct TBoardList *ptBoardListWalker;
  struct TBoard *ptBoard;

  _ptBoardList = newBoardList();
  ptBoardListWalker = _ptBoardList;

  delVariableList(&_ptGlobalVars);

  iTokenType = Parse(pcToken);

  if (strcmp(pcToken, "VAR") == 0)
  {
    _ptGlobalVars = newVariableList();
    if (!Parse_TVarDeclarationList(pcToken, _ptGlobalVars))
      return FALSE;
    if (!ParseChar('.'))
      return FALSE;

    iTokenType = Parse(pcToken);
  }
  while (iTokenType == IDENTIFIER_TOKEN)
  {
    ptBoardListWalker->ptThis = newBoard();
    ptBoardListWalker->ptNext = newBoardList();

    ptBoard = ptBoardListWalker->ptThis;

    ptBoard->pcName = my_strdup(pcToken);

    /* Now try to find out the corresponding protocol number, find it in    */
    /* the TIOList structure.                                               */

    ptBoard->iNumber = FindBoardNumber(pcToken);

    ptBoard->ptStateMachines = newStateMachineList();

    iTokenType = Parse(pcToken);
    if (iTokenType == IDENTIFIER_TOKEN)
    {
      iAlternativeBoard = FindBoardNumber(pcToken);
      iTokenType = Parse(pcToken);
    }
    else
    {
      iAlternativeBoard = -1;
    }

    if (strcmp(pcToken, ":") != 0)
      return FALSE;

    if (!Parse_TStateMachineInvokings(pcToken,
                                      ptBoard->ptStateMachines,
                                      _ptGlobalVars,
                                      ptBoard->pcName, iAlternativeBoard))
    {
      return FALSE;
    }
    iTokenType = Parse(pcToken);

    if (iTokenType == IDENTIFIER_TOKEN)
    {
      ptBoardListWalker = ptBoardListWalker->ptNext;
    }
    else
    {
      delBoardList(&(ptBoardListWalker->ptNext));
    }
  }

  UngetToken();

  return TRUE;
}

int Parse_IO_File(char *pcFileName)
/****************************************************************************/
/* Opens an input file, expects it to be a specification file and reads the */
/* contents. The contents are stored in the IO administration.              */
/* After reading, the file is closed.                                       */
/* The number of found errors is returned as an integer.                    */
/****************************************************************************/
{
  char             pcToken[TOKENSIZE];
  int              iReady;
  int              iTokenType;
  char             pcLogLine[80];
  struct TIOList  *ptNewIOList = NULL;

  InitScanner();

  ResetErrorLogging();

  if (OpenInputFile(pcFileName) != NO_SCANNER_ERROR)
  {
    return P_FILE_NOT_FOUND;
  }

  iReady = 0;
  
  while (iReady==0)
  {
    iTokenType = Parse(pcToken);
    if (iTokenType == END_OF_FILE) iReady = 1;

    if (iReady==0) if (*pcToken!='[')             iReady = 1;
    if (iReady==0) if (!ParseIdentifier(pcToken)) iReady = 1;
    if (iReady==0) if (!ParseChar  (']'))         iReady = 1;

    if (iReady==0)
    {
      if (strcmp(pcToken,"io")==0)
      {

        ptNewIOList = newIOList();
        if (Parse_TIOList(pcToken,ptNewIOList))
        {
          CloseObsoleteIO(_ptIOList, ptNewIOList);
          delIOList(&_ptIOList);
          _ptIOList = ptNewIOList;
          /* io section read successfully. Now configure the driver module */
          if (!ConfigureDriver(_ptIOList)) iReady = 1;
        }
        else
        {
          iReady = 1;
        }
      } 
      else
      {
        /* An unknown section is found. Ignore this section, probably the   */
        /* input file is written originally for a later version of this     */
        /* script engine. Probably the file cannot be executed by this ver- */
        /* sion of the script engine, however in that case an other error   */
        /* will probably give more information. A warning that this section */
        /* is ignored, will be logged in the logging window.                */

        sprintf(pcLogLine,"Warning: ignoring section [%s]",pcToken);
        LogLine(pcLogLine);
        
        /* Try to find the beginning of a new section.                      */

        iTokenType = Parse(pcToken);
        while ((iTokenType != END_OF_FILE)
            && (pcToken[0] != '[')
              )        {
          iTokenType = Parse(pcToken);
        }
        if (pcToken[0]=='[')
        {
          /* Begin of new section found. Put back the parsed [ token        */
          UngetToken();
        }
      }
    }
  }
  CloseInputFile();

  if (iErrorCounter == 0)
  {
    return P_OK;
  }
  else
  {
    return (iErrorCounter);
  }

}




int Parse_Specification_File(
  char *pcFileName)
/****************************************************************************/
/* Opens an input file, expects it to be a specification file and reads the */
/* contents. The contents are stored in the _ptTypeList, _ptFunctionList    */
/* and the _ptEventList store. After reading, the file is closed.           */
/* The number of found errors is returned as an integer.                    */
/****************************************************************************/
{
  char      pcToken[TOKENSIZE];
  int       iReady;
  int       iTokenType;
  char      pcLogLine[80];

  delEventList(&_ptEventList);
  delFunctionList(&_ptFunctionList);
  delTypeList(&_ptTypeList);

  InitScanner();

  ResetErrorLogging();

  if (OpenInputFile(pcFileName) != NO_SCANNER_ERROR)
  {
    return P_FILE_NOT_FOUND;
  }

  iReady = 0;

  while (iReady == 0)
  {
    iTokenType = Parse(pcToken);
    if (iTokenType == END_OF_FILE)
      iReady = 1;

    if (iReady == 0)
      if (*pcToken != '[')
        iReady = 1;
    if (iReady == 0)
      if (!ParseIdentifier(pcToken))
        iReady = 1;
    if (iReady == 0)
      if (!ParseChar(']'))
        iReady = 1;

    if (iReady == 0)
    {
      if (strcmp(pcToken, "type") == 0)
      {
        if (!Parse_TTypeList(pcToken, &_ptTypeList))
        {
          iErrorCounter++;
          iReady = 1;
        }
      }
      else if (strcmp(pcToken, "functions") == 0)
      {
        if (!Parse_TFunctionList(pcToken, &_ptFunctionList))
        {
          iErrorCounter++;
          iReady = 1;
        }
      }
      else if (strcmp(pcToken, "events") == 0)
      {
        if (!Parse_TEventList(pcToken, &_ptEventList))
        {
          iErrorCounter++;
          iReady = 1;
        }
      }
      else
      {
        /* An unknown section is found. Ignore this section, probably the   */
        /* input file is written originally for a later version of this     */
        /* script engine. Probably the file cannot be executed by this ver- */
        /* sion of the script engine, however in that case an other error   */
        /* will probably give more information. A warning that this section */
        /* is ignored, will be logged in the logging window.                */

        sprintf(pcLogLine, "Warning: ignoring section [%s]", pcToken);
        LogLine(pcLogLine);

        /* Try to find the beginning of a new section.                      */

        iTokenType = Parse(pcToken);
        while ((iTokenType != END_OF_FILE) && (pcToken[0] != '['))
        {
          iTokenType = Parse(pcToken);
        }
        if (pcToken[0] == '[')
        {
          /* Begin of new section found. Put back the parsed [ token        */
          UngetToken();
        }
      }
    }
  }
  CloseInputFile();

  if (iErrorCounter == 0)
  {
    sprintf(pcLogLine, "Successfully loaded %s", pcFileName);
    LogLine(pcLogLine);

    return P_OK;
  }
  else
  {
    return (iErrorCounter);
  }

}

int Parse_Script_File(
  char *pcFileName)
/****************************************************************************/
/* Opens an input file, expects it to be a script file and reads the        */
/* contents. The contents are stored in the _ptStatemachineList and the     */
/* _ptBoardList. After reading, the file is closed. The number of errors is */
/* returned as an integer.                                                  */
/****************************************************************************/
{
  char      pcToken[TOKENSIZE];
  int       bReady;
  int       iTokenType;
  char      pcLogLine[80];

  ResetErrorLogging();

  InitScanner();

  delDataList(&_ptDataList);
  delStateMachineList(&_ptStateMachineList);
  delBoardList(&_ptBoardList);
  delVariableList(&_ptGlobalVars);

  if (OpenInputFile(pcFileName) != NO_SCANNER_ERROR)
  {
    return P_FILE_NOT_FOUND;
  }

  bReady = FALSE;

  while (!bReady)
  {
    iTokenType = Parse(pcToken);

    if (iTokenType == END_OF_FILE)
      bReady = TRUE;

    else
    {
      if (!bReady)
        if (*pcToken != '[')
          bReady = TRUE;
      if (!bReady)
        if (!ParseIdentifier(pcToken))
          bReady = TRUE;
      if (!bReady)
        if (!ParseChar(']'))
          bReady = TRUE;

      if (bReady == TRUE)
      {
        AddError1("Section expected");
      }
    }

    if (!bReady)
    {
      if (strcmp(pcToken, "data") == 0)
      {
        _ptDataList = newDataList();
        if (!Parse_TDataList(pcToken, _ptDataList))
          bReady = TRUE;
      }
      else if (strcmp(pcToken, "statemachines") == 0)
      {
        _ptStateMachineList = newStateMachineList();
        if (!Parse_TStateMachineList(pcToken, _ptStateMachineList))
          bReady = TRUE;
      }
      else if ((strcmp(pcToken, "testscript") == 0)
               || (strcmp(pcToken, "devices") == 0))
      {
        if (!Parse_TestScript(pcToken))
          bReady = TRUE;
      }
      else
      {
        sprintf(pcLogLine, "Warning: ignoring section [%s]", pcToken);
        LogLine(pcLogLine);
        iTokenType = Parse(pcToken);
        while ((iTokenType != END_OF_FILE) && (pcToken[0] != '['))
        {
          iTokenType = Parse(pcToken);
        }
        if (pcToken[0] == '[')
        {
          /* Begin of new section found. Put back the parsed [ token        */
          UngetToken();
        }
      }
    }
  }
  CloseInputFile();

  /* Parsing the file is ended now. One reason for ending is not logged     */
  /* yet, this is the include file problem. If including a file was the     */
  /* reason for ending the parse process, log this error now.               */

  if (iTokenType == INCLUDE_FILE_PROBLEM)
  {
    AddError2("Problems including file %s", pcToken);
  }

  /* Ready. If parsing completed successfully, initialise the statemachi-   */
  /* nes. If parsing didn't complete successfully, delete the boardlist and */
  /* the statemachine list. (The statemachine list contains the original    */
  /* statemachines. The boardlist contains list of statemachines, but these */
  /* statemachines are physical copies of the statemachinelist.             */

  /* The information found in the .prot file, stored in the type list, the   */
  /* function list and the event list remains intact: these stores are not  */
  /* deleted.                                                               */

  if (iErrorCounter == 0)
  {
    InitBoards(_ptBoardList);
  }
  else
  {
    delBoardList(&_ptBoardList);
    delStateMachineList(&_ptStateMachineList);
  }

  if (iErrorCounter == 0)
  {
    return P_OK;
  }
  else
  {
    return (iErrorCounter);
  }
}

void Add_IO(
  void)
{
  struct TIOList *ptNewIOList = NULL;

  ptNewIOList = newIOList();

  delIOList(&_ptIOList);

  /*delBoardList(&_ptBoardList);   Added */
  InitBoards(_ptBoardList);     /* Added */

  LOC_Add_To_IOList(ptNewIOList);

  CloseObsoleteIO(_ptIOList, ptNewIOList);
  delIOList(&_ptIOList);
  _ptIOList = ptNewIOList;
  /* io section read successfully. Now configure the driver module */
  ConfigureDriver(_ptIOList);
}

int LOC_Add_To_IOList(
  struct TIOList *ptIOList)
{
  int       i;

  for (i = 0; i < 16; i++)
  {
    if (piChannels[i] != 0)
    {
      ptIOList->ptThis = newIO();
      LOC_Add_To_IO(pcDevice[i], piChannels[i], ptIOList->ptThis);
      /*LOC_Add_To_IO(i+1, piChannels[i], ptIOList->ptThis); */
      ptIOList->ptNext = newIOList();
      ptIOList = ptIOList->ptNext;
    }
  }

  return TRUE;
}

int LOC_Add_To_IO(
  char *pcDevice,
  int iChannel,
  struct TIO *ptIO)
{
  int       iNumber;

  ptIO->pcName = my_strdup(pcDevice);

  ptIO->iNumber = iChannel;
  ptIO->pcIOPortName = my_strdup(pcDevice);

  if (strcmp(pcProtocol, "PROT") == 0)
  {
    /* Max parameter lenght of a prot event is 255; because of the single byte */
    /* length field. The PROT event header has 2 bytes: an event code and the  */
    /* length field itself. The Uart protocol adds 1 byte to indicate the     */
    /* type of message (event, command, acl data, sco data, ect)              */
    /* That makes a total of 258 bytes for incoming prot events.               */
    ptIO->iProtocol = PROT_PROTOCOL;
    ptIO->iTerminateTokenLength = 0;
    ptIO->iMaxTokenLength = 258;
    ptIO->pTerminateToken = NULL;
  }
  else if (strcmp(pcProtocol, "PASCAL") == 0)
  {
    iNumber = GetInteger(pcProtocol);

    ptIO->iProtocol = ONE_LENGTH_BYTE;
    ptIO->iTerminateTokenLength = 0;
    ptIO->iMaxTokenLength = iNumber;
    ptIO->pTerminateToken = NULL;
  }
  else if (strcmp(pcProtocol, "PASCAL_HI_LO") == 0)
  {
    iNumber = GetInteger(pcProtocol);

    ptIO->iProtocol = TWO_LENGTH_BYTES_HI_LO;
    ptIO->iTerminateTokenLength = 0;
    ptIO->iMaxTokenLength = iNumber;
    ptIO->pTerminateToken = NULL;
  }
  else if (strcmp(pcProtocol, "PASCAL_LO_HI") == 0)
  {
    iNumber = GetInteger(pcProtocol);

    ptIO->iProtocol = TWO_LENGTH_BYTES_LO_HI;
    ptIO->iTerminateTokenLength = 0;
    ptIO->iMaxTokenLength = iNumber;
    ptIO->pTerminateToken = NULL;
  }
  else if (strcmp(pcProtocol, "TOKENTERMINATED") == 0)
  {
    iNumber = strlen(pcProtocol);
    if (iNumber == 0)
      iNumber = 1;              /* For the '\0' string. */

    ptIO->iProtocol = TOKEN_TERMINATED;
    ptIO->iTerminateTokenLength = iNumber;
    ptIO->pTerminateToken = my_strdup(pcProtocol);

    iNumber = GetInteger(pcProtocol);
    ptIO->iMaxTokenLength = iNumber;
  }
  else
  {
    AddError2("Unknown protocol: %s", pcProtocol);
  }
  return TRUE;
}
