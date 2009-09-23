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
// 28/02/2001 E.A. Nijboer            Initial release
//
//=========================================================================
*/


/****************************************************************************/
/*                                                                          */
/* Exp0 = Exp1 Op1 Exp1                                                     */
/* Exp0 = Exp1                                                              */
/*                                                                          */
/* Exp1 = Exp2 Op2 Exp2                                                     */
/* Exp1 = Exp2                                                              */
/*                                                                          */
/* Exp2 = Exp3 Op3 Exp3                                                     */
/* Exp2 = Exp3                                                              */
/*                                                                          */
/* Exp3 = Exp4 Op4 Exp4                                                     */
/* Exp3 = Exp4                                                              */
/*                                                                          */
/* Exp4 = Identifier                                                        */
/* Exp4 = Constant                                                          */
/* Exp4 = ( Exp0 )                                                          */
/*                                                                          */
/* Op1 = '=' ASSIGNMENT                                                     */
/*                                                                          */
/* Op2 = '*' MUL                                                            */
/* Op2 = '/' DIV                                                            */
/* Op2 = '|' OR                                                             */
/* Op2 = '!=' UNEQUAL                                                       */
/* Op2 = '<>' UNEQUAL                                                       */
/* Op2 = '<=' LESSEQUAL                                                     */
/* Op2 = '>=' MOREEQUAL                                                     */
/* Op2 = '>'  MORE                                                          */
/* Op2 = '<'  LESS                                                          */
/*                                                                          */
/* Op3 = '+' PLUS                                                           */
/* Op3 = '-' MINUS                                                          */
/* Op3 = '&' AND                                                            */
/*                                                                          */
/* Op4 = '!' NOT                                                            */
/* Op4 = '~' NOT                                                            */
/*                                                                          */



#define TO_HEX(a)   (((a)>'9')?((a)-'A'+10):((a)-'0'))
#define TO_ASCII(a) (((a)>9)?((a)-10+'A'):((a)+'0'))

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
/* Local data.                                                              */
/****************************************************************************/

char      Operator[TOKENSIZE];

#define LEVELCOUNT 3

char      OperatorSet[LEVELCOUNT + 1][50] = {
  " = ",
  " + - & ",
  " * / | == != <> <= >= < > ",
  " ! ~ "
};

static char *pcNull = "0x00";

/****************************************************************************/
/* Local function headers.                                                  */
/****************************************************************************/

int16     Parse_TExpressionX(
  char *pcToken,
  struct TExpression *ptExpr,
  struct TVariableList *ptVarList,
  int16 iLevel);

int16     Parse_TLeave(
  char *pcToken,
  struct TExpression *ptExpr,
  struct TVariableList *ptVarList);

char     *OperatorString(
  int iOperator);

char     *a_equal(
  char *pcL,
  char *pcR);
char     *a_nonequal(
  char *pcL,
  char *pcR);
char     *a_less(
  char *pcL,
  char *pcR);
char     *a_lessequal(
  char *pcL,
  char *pcR);
char     *a_more(
  char *pcL,
  char *pcR);
char     *a_moreequal(
  char *pcL,
  char *pcR);
char     *a_assignment(
  struct TExpression *ptL,
  char *pcR);
char     *a_plus(
  char *pcL,
  char *pcR);
char     *a_minus(
  char *pcL,
  char *pcR);
char     *a_div(
  char *pcL,
  char *pcR);
char     *a_mul(
  char *pcL,
  char *pcR);
char     *a_and(
  char *pcL,
  char *pcR);
char     *a_or(
  char *pcL,
  char *pcR);
char     *a_not(
  char *pcL,
  char *pcR);
char     *a_value(
  struct TVariable *ptV);

struct TExpression *newExpression(
  void)
{
  struct TExpression *n;
  n = (struct TExpression *) malloc(sizeof(struct TExpression));
  n->ptVariable = NULL;
  n->ptLeftExpression = NULL;
  n->ptRightExpression = NULL;
  n->iOperator = UNDEFINED;
  return n;
}

struct TExpression *dupExpression(
  struct TExpression *ptOld,
  struct TVariableList *ptVarList)
{
  struct TExpression *n;

  if (ptOld == NULL)
  {
    return NULL;
  }
  else
  {
    n = newExpression();

    if ((ptVarList != NULL)
        && (ptOld->ptVariable != NULL) && (ptOld->ptVariable->pcName != NULL))
    {
      n->ptVariable = FindVar(ptOld->ptVariable->pcName, ptVarList);
    }
    else
    {
      n->ptVariable = ptOld->ptVariable;
      if (n->ptVariable != NULL)
      {
        Increment(&n->ptVariable->iRefCount);
      }
    }
    n->ptLeftExpression = dupExpression(ptOld->ptLeftExpression, ptVarList);
    n->ptRightExpression = dupExpression(ptOld->ptRightExpression, ptVarList);
    n->iOperator = ptOld->iOperator;
    n->iOperatorLevel = ptOld->iOperatorLevel;

    return n;
  }
}

void delExpression(
  struct TExpression **p)
{
  if ((*p) != NULL)
  {
    if ((*p)->ptVariable != NULL)
      delVariable(&((*p)->ptVariable));
    if ((*p)->ptLeftExpression != NULL)
      delExpression(&((*p)->ptLeftExpression));
    if ((*p)->ptRightExpression != NULL)
      delExpression(&((*p)->ptRightExpression));
    free(*p);
    *p = NULL;
  }
}

int16 OperatorId(
  char *Operator)
{
  if (strcmp(Operator, "==") == 0)
    return EQUAL;
  if (strcmp(Operator, "<>") == 0)
    return NONEQUAL;
  if (strcmp(Operator, "!=") == 0)
    return NONEQUAL;
  if (strcmp(Operator, "<") == 0)
    return LESS;
  if (strcmp(Operator, "<=") == 0)
    return LESSEQUAL;
  if (strcmp(Operator, ">") == 0)
    return MORE;
  if (strcmp(Operator, ">=") == 0)
    return MOREEQUAL;
  if (strcmp(Operator, "=") == 0)
    return ASSIGNMENT;
  if (strcmp(Operator, "+") == 0)
    return PLUS;
  if (strcmp(Operator, "-") == 0)
    return MINUS;
  if (strcmp(Operator, "/") == 0)
    return DIV;
  if (strcmp(Operator, "*") == 0)
    return MUL;
  if (strcmp(Operator, "&") == 0)
    return AND;
  if (strcmp(Operator, "|") == 0)
    return OR;
  if (strcmp(Operator, "!") == 0)
    return NOT;
  if (strcmp(Operator, "~") == 0)
    return NOT;
  return UNDEFINED;
}

char     *OperatorString(
  int iOperator)
{
  switch (iOperator)
  {
    case EQUAL:
      return " == ";
    case NONEQUAL:
      return " <> ";
    case LESS:
      return " < ";
    case LESSEQUAL:
      return " <= ";
    case MORE:
      return " > ";
    case MOREEQUAL:
      return " >= ";
    case ASSIGNMENT:
      return " = ";
    case PLUS:
      return " + ";
    case MINUS:
      return " - ";
    case MUL:
      return " * ";
    case DIV:
      return " / ";
    case AND:
      return " & ";
    case OR:
      return " | ";
    default:
      return " *** internal arithmetric error *** ";
  }
}


int16 Parse_TExpression(
  char *pcToken,
  struct TAction * ptAction,
  struct TVariableList * ptReferenceList)
{
  if (ptAction->ptExpression == NULL)
  {
    ptAction->ptExpression = newExpression();
  }

  return Parse_TExpressionX(pcToken, ptAction->ptExpression, ptReferenceList,
                            0);
}

int16 Parse_TExpressionX(
  char *pcToken,
  struct TExpression * ptExpr,
  struct TVariableList * ptVarList,
  int16 iLevel)
{
  struct TExpression *ptNew;

  if (iLevel <= LEVELCOUNT)
  {
    if (!Parse_TExpressionX(pcToken, ptExpr, ptVarList, iLevel + 1))
    {
      return FALSE;
    }

    *Operator = 0;
    GetToken(pcToken);
    strcat(Operator, " ");
    strcat(Operator, pcToken);
    strcat(Operator, " ");

    if (strpos(Operator, OperatorSet[iLevel]) != NULL)
    {
      ptNew = newExpression();
      ptNew->iOperator = ptExpr->iOperator;
      ptNew->iOperatorLevel = ptExpr->iOperatorLevel;
      ptNew->ptVariable = ptExpr->ptVariable;   /* NO refcnt update!! */
      ptNew->ptLeftExpression = ptExpr->ptLeftExpression;
      ptNew->ptRightExpression = ptExpr->ptRightExpression;

      ptExpr->ptVariable = NULL;        /* again, NO refcnt update!! */

      ptExpr->iOperatorLevel = iLevel;
      ptExpr->ptLeftExpression = ptNew;
      ptExpr->ptRightExpression = newExpression();
      ptExpr->iOperator = OperatorId(pcToken);

      GetToken(pcToken);
      if (!Parse_TExpressionX
          (pcToken, ptExpr->ptRightExpression, ptVarList, iLevel))
      {
        return FALSE;
      }
    }
    else
    {
      UngetToken();
    }
  }
  else
  {
    if (!Parse_TLeave(pcToken, ptExpr, ptVarList))
    {
      return FALSE;
    }
  }
  return TRUE;
}

int16 Parse_TLeave(
  char *pcToken,
  struct TExpression * ptExpr,
  struct TVariableList * ptVarList)
{
  int16     iTokenType;

  UngetToken();
  iTokenType = GetToken(pcToken);
  /* That was just to find out the type of the last scanned token.. :-) */

  if (strcmp(pcToken, "(") == 0)
  {
    Parse(pcToken);
    Parse_TExpressionX(pcToken, ptExpr, ptVarList, 0);
    Parse(pcToken);
    if (strcmp(pcToken, ")") != 0)
    {
      AddError1("Close parentheses forgotten? ");
      return FALSE;
    }
  }
  else if (iTokenType == IDENTIFIER_TOKEN)
  {
    ptExpr->ptVariable = FindVar(pcToken, ptVarList);
    if (ptExpr->ptVariable == NULL)
    {
      AddError2("Variable %s not found", pcToken);
      return FALSE;
    }
    ptExpr->iOperator = NUMBER;
    ptExpr->iOperatorLevel = LEVELCOUNT + 1;
  }
  else if ((iTokenType == DEC_NUMBER_TOKEN)
           || (iTokenType == BIN_NUMBER_TOKEN)
           || (iTokenType == HEX_NUMBER_TOKEN))
  {
    ptExpr->ptVariable = newVariable(NULL);
    ptExpr->ptVariable->ptValue = newValue();
    ptExpr->ptVariable->ptValue->pcValue = my_strdup(pcToken);
    ptExpr->iOperator = NUMBER;
    ptExpr->iOperatorLevel = LEVELCOUNT + 1;
  }
  else if (OperatorId(pcToken) == NOT)
  {
    ptExpr->iOperator = NOT;
    ptExpr->ptRightExpression = newExpression();
    GetToken(pcToken);
    Parse_TExpressionX(pcToken, ptExpr->ptRightExpression, ptVarList,
                       LEVELCOUNT);
  }
  else
  {
    AddError2("Token %s not expected. Allowed tokens: ( identifier constant. ",
              pcToken);
    return FALSE;
  }
  return TRUE;
}

/************************************************************************/


void WriteExpression(
  char *pcBuffer,
  struct TExpression *ptExpr,
  int16 iLevel)
{
  if (ptExpr->iOperatorLevel < iLevel)
  {
    strcat(pcBuffer, "(");
  }

  if (ptExpr->iOperator == NUMBER)
  {
    WriteVar(pcBuffer, ptExpr->ptVariable);
  }
  else if (ptExpr->iOperator == NOT)
  {
    strcat(pcBuffer, "!");
    WriteExpression(pcBuffer, ptExpr->ptRightExpression,
                    ptExpr->iOperatorLevel);
  }
  else if ((ptExpr->ptLeftExpression != NULL)
           && (ptExpr->ptRightExpression != NULL))
  {
    WriteExpression(pcBuffer, ptExpr->ptLeftExpression,
                    ptExpr->iOperatorLevel + 1);
    strcat(pcBuffer, OperatorString(ptExpr->iOperator));
    WriteExpression(pcBuffer, ptExpr->ptRightExpression,
                    ptExpr->iOperatorLevel);
  }
  else
  {
    strcat(pcBuffer, "** Internal arithmetric error **");
  }

  if (ptExpr->iOperatorLevel < iLevel)
  {
    strcat(pcBuffer, ")");
  }
}

void SetType(
  struct TExpression *ptExpr,
  struct TType *ptType)
{
  if (ptExpr == NULL)
    return;
  if ((ptExpr->iOperator == ASSIGNMENT)
      || (ptExpr->iOperator == PLUS)
      || (ptExpr->iOperator == MINUS)
      || (ptExpr->iOperator == DIV) || (ptExpr->iOperator == MUL))
  {
    SetType(ptExpr->ptLeftExpression, ptType);
    SetType(ptExpr->ptRightExpression, ptType);
  }

  if ((ptExpr->ptVariable != NULL) && (ptExpr->ptVariable->ptItsType == NULL))
  {
    ptExpr->ptVariable->ptItsType = ptType;
    Increment(&(ptType->iRefCount));
  }
}

struct TType *TypeCheck(
  struct TExpression *ptExpr)
{
  struct TType *ptLeftType;
  struct TType *ptRightType;
  char      pcErrorLine1[250];
  char      pcErrorLine2[250];
  char      pcErrorLine3[250];
  char      pcErrorLine[1024];

  if (ptExpr == NULL)
    return NULL;

  ptLeftType = TypeCheck(ptExpr->ptLeftExpression);
  ptRightType = TypeCheck(ptExpr->ptRightExpression);

  if ((ptLeftType == ptRightType) && (ptLeftType != NULL))
  {
    if (ptExpr->iOperator == EQUAL)
      return _ptBooleanType;
    if (ptExpr->iOperator == NONEQUAL)
      return _ptBooleanType;
    if (ptExpr->iOperator == LESS)
      return _ptBooleanType;
    if (ptExpr->iOperator == LESSEQUAL)
      return _ptBooleanType;
    if (ptExpr->iOperator == MORE)
      return _ptBooleanType;
    if (ptExpr->iOperator == MOREEQUAL)
      return _ptBooleanType;
    if (ptExpr->iOperator == ASSIGNMENT)
      return ptLeftType;
    if (ptExpr->iOperator == PLUS)
      return ptLeftType;
    if (ptExpr->iOperator == MINUS)
      return ptLeftType;
    if (ptExpr->iOperator == DIV)
      return ptLeftType;
    if (ptExpr->iOperator == MUL)
      return ptLeftType;
    if (ptExpr->iOperator == AND)
      return _ptBooleanType;
    if (ptExpr->iOperator == OR)
      return _ptBooleanType;
    FatalError1("** internal arithmetric error.. **");
  }

  if ((ptLeftType != NULL) && (ptRightType != NULL))
  {
    sprintf(pcErrorLine1, "Arithmetric error: Type Conflict\n");

    pcErrorLine[0] = 0;
    WriteExpression(pcErrorLine, ptExpr->ptLeftExpression, 0);
    sprintf(pcErrorLine2, "%s has type %s\n", pcErrorLine, ptLeftType->pcName);

    pcErrorLine[0] = 0;
    WriteExpression(pcErrorLine, ptExpr->ptRightExpression, 0);
    sprintf(pcErrorLine3, "%s has type %s\n", pcErrorLine, ptRightType->pcName);

    strcpy(pcErrorLine, pcErrorLine1);
    strcat(pcErrorLine, pcErrorLine2);
    strcat(pcErrorLine, pcErrorLine3);
    AddError1(pcErrorLine);

    return NULL;
  }

  if (ptExpr->iOperator == NOT)
  {
    if (ptRightType != _ptBooleanType)
    {
      AddError1("Cannot perform a NOT operation on a non-boolean");
      return NULL;
    }
    else
    {
      return _ptBooleanType;
    }
  }

  if ((ptExpr->iOperator == EQUAL)
      || (ptExpr->iOperator == NONEQUAL)
      || (ptExpr->iOperator == LESS)
      || (ptExpr->iOperator == LESSEQUAL)
      || (ptExpr->iOperator == MOREEQUAL) || (ptExpr->iOperator == MORE))
  {
    if ( (ptRightType != ptLeftType) &&
         (ptLeftType  != NULL) &&
         (ptRightType != NULL) )
    {
      AddError1("Cannot perform a compare on different types");
      return NULL;
    }
    else
    {
      return _ptBooleanType;
    }
  }

  if ((ptExpr->iOperator == AND) || (ptExpr->iOperator == OR))
  {
    if (ptRightType != _ptBooleanType)
    {
      AddError1("Boolean operator: Right operand is not a boolean");
      return NULL;
    }
    if (ptLeftType != _ptBooleanType)
    {
      AddError1("Boolean operator: Left operand is not a boolean");
      return NULL;
    }

    return _ptBooleanType;

  }

  if (ptLeftType != NULL)
  {
    SetType(ptExpr->ptRightExpression, ptLeftType);
    return ptLeftType;
  }

  if (ptRightType != NULL)
  {
    SetType(ptExpr->ptLeftExpression, ptRightType);
    return ptRightType;
  }

  if (ptExpr->iOperator == NUMBER)
  {
    if ((ptExpr->ptVariable != NULL) && (ptExpr->ptVariable->ptItsType != NULL))
    {
      return ptExpr->ptVariable->ptItsType;
    }
  }
  return NULL;
}


char     *EvaluateExpression(
  struct TExpression *ptExpr)
{
  char     *pcLeftValue, *pcRightValue;
  char     *pcResult;

  if (ptExpr == NULL)
    return NULL;

  pcLeftValue = EvaluateExpression(ptExpr->ptLeftExpression);
  pcRightValue = EvaluateExpression(ptExpr->ptRightExpression);

  switch (ptExpr->iOperator)
  {
    case EQUAL:
      pcResult = a_equal(pcLeftValue, pcRightValue);
      break;
    case NONEQUAL:
      pcResult = a_nonequal(pcLeftValue, pcRightValue);
      break;
    case LESS:
      pcResult = a_less(pcLeftValue, pcRightValue);
      break;
    case LESSEQUAL:
      pcResult = a_lessequal(pcLeftValue, pcRightValue);
      break;
    case MORE:
      pcResult = a_more(pcLeftValue, pcRightValue);
      break;
    case MOREEQUAL:
      pcResult = a_moreequal(pcLeftValue, pcRightValue);
      break;
    case ASSIGNMENT:
      pcResult = a_assignment(ptExpr->ptLeftExpression, pcRightValue);
      break;
    case PLUS:
      pcResult = a_plus(pcLeftValue, pcRightValue);
      break;
    case MINUS:
      pcResult = a_minus(pcLeftValue, pcRightValue);
      break;
    case DIV:
      pcResult = a_div(pcLeftValue, pcRightValue);
      break;
    case MUL:
      pcResult = a_mul(pcLeftValue, pcRightValue);
      break;
    case AND:
      pcResult = a_and(pcLeftValue, pcRightValue);
      break;
    case OR:
      pcResult = a_or(pcLeftValue, pcRightValue);
      break;
    case NOT:
      pcResult = a_not(pcLeftValue, pcRightValue);
      break;
    case NUMBER:
      pcResult = a_value(ptExpr->ptVariable);
      break;
    default:
      pcResult = NULL;
      break;
  }

  if (pcLeftValue != NULL)
    free(pcLeftValue);
  if (pcRightValue != NULL)
    free(pcRightValue);

  return pcResult;
}


void Equalize(
  char *pcA,
  char *pcB,
  char **ppcResultA,
  char **ppcResultB)
/****************************************************************************/
/* Makes a copy of both strings. The strings are expected to contain hexa-  */
/* decimal numbers, e.g. 0x01 and 0x0004.  The new pointers will point to   */
/* (in this case) 0x0001 and 0x0004. After use, the new pointers must be    */
/* freed!!!!                                                                */
/* If one of the pointers *ppcA or *ppcB is NULL, an uninitialised variable */
/* is used in an expression. In that case, the variable will be treated as  */
/* zero. The NULL pointer is then replaced by a pointer to 0x00             */
/****************************************************************************/
{
  int16     iA, iB, iMax;

  iA = strlen(pcA);
  iB = strlen(pcB);

  iMax = iA;
  if (iMax < iB)
    iMax = iB;

  iA = iMax - iA;
  iB = iMax - iB;

  *ppcResultA = (char *) malloc(iMax + 1);
  *ppcResultB = (char *) malloc(iMax + 1);

  strcpy(*ppcResultA, "0x");
  while (iA-- > 0)
    strcat(*ppcResultA, "0");

  strcpy(*ppcResultB, "0x");
  while (iB-- > 0)
    strcat(*ppcResultB, "0");

  strcat(*ppcResultA, (pcA) + 2);
  strcat(*ppcResultB, (pcB) + 2);

}

char     *a_equal(
  char *pcL,
  char *pcR)
{
  char     *pcNewLeft, *pcNewRight;

  Equalize(pcL, pcR, &pcNewLeft, &pcNewRight);

  if (strcmp(pcNewLeft, pcNewRight) == 0)
  {
    free(pcNewLeft);
    free(pcNewRight);
    return my_strdup("TRUE");
  }
  else
  {
    free(pcNewLeft);
    free(pcNewRight);
    return my_strdup("FALSE");
  }
}


char     *a_nonequal(
  char *pcL,
  char *pcR)
{
  char     *pcNewLeft, *pcNewRight;

  Equalize(pcL, pcR, &pcNewLeft, &pcNewRight);

  if (strcmp(pcNewLeft, pcNewRight) != 0)
   {
    free(pcNewLeft);
    free(pcNewRight);
    return my_strdup("TRUE");
  }
  else
  {
    free(pcNewLeft);
    free(pcNewRight);
    return my_strdup("FALSE");
  }
}

char     *a_less(
  char *pcL,
  char *pcR)
{
  char     *pcNewLeft, *pcNewRight;

  Equalize(pcL, pcR, &pcNewLeft, &pcNewRight);

  if (strcmp(pcNewLeft, pcNewRight) < 0)
  {
    free(pcNewLeft);
    free(pcNewRight);
    return my_strdup("TRUE");
  }
  else
  {
    free(pcNewLeft);
    free(pcNewRight);
    return my_strdup("FALSE");
  }
}

char     *a_lessequal(
  char *pcL,
  char *pcR)
{
  char     *pcNewLeft, *pcNewRight;

  Equalize(pcL, pcR, &pcNewLeft, &pcNewRight);

  if (strcmp(pcNewLeft, pcNewRight) <= 0)
  {
    free(pcNewLeft);
    free(pcNewRight);
    return my_strdup("TRUE");
  }
  else
  {
    free(pcNewLeft);
    free(pcNewRight);
    return my_strdup("FALSE");
  }
}

char     *a_more(
  char *pcL,
  char *pcR)
{
  char     *pcNewLeft, *pcNewRight;

  Equalize(pcL, pcR, &pcNewLeft, &pcNewRight);

  if (strcmp(pcNewLeft, pcNewRight) > 0)
  {
    free(pcNewLeft);
    free(pcNewRight);
    return my_strdup("TRUE");
  }
  else
  {
    free(pcNewLeft);
    free(pcNewRight);
    return my_strdup("FALSE");
  }
}

char     *a_moreequal(
  char *pcL,
  char *pcR)
{
  char     *pcNewLeft, *pcNewRight;

  Equalize(pcL, pcR, &pcNewLeft, &pcNewRight);

  if (strcmp(pcNewLeft, pcNewRight) >= 0)
  {
    free(pcNewLeft);
    free(pcNewRight);
    return my_strdup("TRUE");
  }
  else
  {
    free(pcNewLeft);
    free(pcNewRight);
    return my_strdup("FALSE");
  }
}

char     *a_assignment(
  struct TExpression *pcL,
  char *pcR)
{

  if (pcL->ptVariable == NULL)
    return my_strdup(pcR);
  if (pcL->ptVariable->ptValue == NULL)
  {
    pcL->ptVariable->ptValue = newValue();
  }

  if (pcL->ptVariable->ptValue->pcValue != NULL)
  {
    free(pcL->ptVariable->ptValue->pcValue);
    pcL->ptVariable->ptValue->pcValue = NULL;
  }

  pcL->ptVariable->ptValue->pcValue = my_strdup(pcR);

  return my_strdup(pcR);
}

char     *a_plus(
  char *pcL,
  char *pcR)
{
  char     *pcResult;
  char     *pcTempResult;
  char     *pcNewLeft, *pcNewRight;
  int       iDigitSum;
  int       iEnd, iCounter, iCarry;

  Equalize(pcL, pcR, &pcNewLeft, &pcNewRight);

  iCarry = 0;
  
  iEnd = strlen(pcNewLeft);              /* or pcR, both strings are expected have equal length   */
  pcResult = (char*) calloc(iEnd+2, sizeof(char)); /*leave space for carrier */
  pcTempResult = (char*) calloc(iEnd+2, sizeof(char)); /*leave space for carrier */
  
  if ((NULL != pcResult) && (NULL != pcTempResult))
  {
    pcResult[0] = 0;
    if (pcNewLeft[1] == 'x') 
    {
      /* processing data from right to left */
      iCounter = iEnd-1;
      while (pcNewLeft[iCounter] != 'x')
      {
        iDigitSum = TO_HEX(pcNewLeft[iCounter]) + TO_HEX(pcNewRight[iCounter]) + iCarry;
        iCarry = iDigitSum / 0x10;
        iDigitSum = iDigitSum % 0x10;
    
        pcTempResult[iCounter] = TO_ASCII(iDigitSum);
    
        iCounter--;
      }
      if (iCarry != 0)
      {
        pcTempResult[0] = '0';
        pcTempResult[iCounter] = TO_ASCII(iCarry);
      }
      else
      {
        /*Prevents valgrind from creating an error */
        pcTempResult[0] = '0';
        pcTempResult[1] = 'x';
        iCounter++;
      }
      if (iCounter != (iEnd-1))
      {
        strcpy(pcResult, "0x");
        strcat(pcResult, &pcTempResult[iCounter]);
      }
    }
    else
    {
      iCounter = iEnd-1;
      while (iCounter !=0)
      {
        iDigitSum = TO_HEX(pcNewLeft[iCounter]) + TO_HEX(pcNewRight[iCounter]) + iCarry;
        iCarry = iDigitSum / 0x0A;
        iDigitSum = iDigitSum % 0x0A;
    
        pcTempResult[iCounter] = TO_ASCII(iDigitSum);
    
        iCounter--;
      }
      if (iCarry != 0)
      {
        pcTempResult[iCounter] = TO_ASCII(iCarry);
      }
      else
      {
        iCounter++;
      }
      strcpy(pcResult, &pcTempResult[iCounter]);
    }
    free(pcTempResult);
  }

  free(pcNewLeft);
  free(pcNewRight);
  return(pcResult);
}


char     *a_minus(
  char *pcL,
  char *pcR)
{
  char     *pcResult;
  char     *pcNewLeft, *pcNewRight;
  int       iDigitDiff;
  int       i, iCarry;

  Equalize(pcL, pcR, &pcNewLeft, &pcNewRight);

  pcResult = my_strdup(pcNewLeft);
  i = strlen(pcNewLeft);              /* or pcR, both strings are expected have equal length   */

  i--;

  iCarry = 0;

  while ((i != 0) && (pcL[i] != 'x'))
  {

    iDigitDiff = TO_HEX(pcNewLeft[i]) - TO_HEX(pcNewRight[i]);       /* range [ -0x0F , 0x0F ]       */
    iDigitDiff = iDigitDiff + iCarry;   /* range [ -0x10 , 0x0F ]       */

    /* range shift is needed, because both / and % are undefined when one   */
    /* of the operands is negative. By adding 0x10 we're sure we're ope-    */
    /* rating in the positive range and have predictable results            */

    iDigitDiff += 0x10;         /* range [  0x00 , 0x1F ]       */
    iCarry = iDigitDiff / 0x10; /* range [  0    , 1    ]       */
    iCarry--;                   /* range [  -1   , 0    ]       */
    iDigitDiff = iDigitDiff % 0x10;     /* range [  0x00 , 0x0F ]       */

    pcResult[i] = TO_ASCII(iDigitDiff);

    i--;
  }

  free(pcNewLeft);
  free(pcNewRight);

  return pcResult;
}

char     *a_div(
  char *pcL,
  char *pcR)
{
  char     *pcTafeltje[17];

  char     *pcResult;
  char     *pcCounter;
  char     *pcNewCounter;
  char     *pcMult;

  uint16    i, j, iDigit, iDivverLength;

  /* use the ptr pcResult as temporary pointer. Otherwise I'm using a lot   */
  /* of pointers. The first thing to do is to change pcR. The format of     */
  /* pcR is 0xXXXX... This has to be changed into 0x0XXXX....: there has to */
  /* be an extra prefix zero. We need this because the entries in the table */
  /* pcTafeltje have the same lenght as pcR, and these entries can be one   */
  /* significant digit longer. Because we don't want to change the value of */
  /* pcR each time this function is invoked, this additional zero is inser  */
  /* ted in a local copy of the string.                                     */

  pcResult = malloc(strlen(pcR) + 2);
  strcpy(pcResult + 1, pcR);
  pcResult[0] = '0';
  pcResult[1] = 'x';
  pcResult[2] = '0';

  pcR = pcResult;
  pcResult = NULL;

  pcTafeltje[0] = malloc(strlen(pcR) + 1);
  strcpy(pcTafeltje[0], "0x");
  i = 2;
  while (pcR[i] != 0)
  {
    pcTafeltje[0][i] = '0';
    i++;
  }
  pcTafeltje[0][i] = 0;

  for (i = 1; i < 17; i++)
    pcTafeltje[i] = a_plus(pcTafeltje[i - 1], pcR);

  pcMult = malloc(strlen(pcL) + strlen(pcR) - 1);
  *pcMult = 0;

  pcResult = my_strdup(pcL);
  pcResult[2] = 0;

  /* Use expression 0x1000 / 0x02 as example. The pcCounter 0x1000 needs    */
  /* to be preceeded by 2-1 zeroes, the number of digits in 0x02 minus 1.   */

  pcCounter = malloc(strlen(pcL) + strlen(pcR) - 1);
  strcpy(pcCounter, "0x");
  i = strlen(pcR) - 3;
  while (i-- > 0)
    strcat(pcCounter, "0");
  strcat(pcCounter, pcL + 2);


  /* Ready. pcCounter is now 0x001000, pcDivver is now 0x020000.            */
  /* Now the preparation is ready, and the real dividing can start.         */

  i = strlen(pcL) - 2;          /* This is the number of digits to be generated.     */
  iDivverLength = strlen(pcR);

  while (i > 0)
  {
    iDigit = 0;
    while (strncmp(pcTafeltje[iDigit], pcCounter, iDivverLength) <= 0)
      iDigit++;

    /* Now I'm sure that pcTafeltje[iDigit] > pcCounter                     */
    /* so: iDigit * pcR > pcCounter                                         */

    iDigit--;

    /* Now I'm sure that iDigit is the largest digit that matches the expr. */
    /* iDigit * pcR <= pcCounter                                            */

    pcResult[strlen(pcResult) + 1] = 0;
    pcResult[strlen(pcResult)] = TO_ASCII(iDigit);

    strcpy(pcMult, pcTafeltje[iDigit]);
    while (strlen(pcMult) < strlen(pcCounter))
      strcat(pcMult, "0");

    pcNewCounter = a_minus(pcCounter, pcMult);
    free(pcCounter);
    pcCounter = pcNewCounter;

    j = 2;
    while (j < strlen(pcCounter) - 1)
    {
      pcCounter[j] = pcCounter[j + 1];
      j++;
    }
    pcCounter[j] = '0';

    i--;
  }

  for (i = 0; i < 17; i++)
    free(pcTafeltje[i]);

  free(pcCounter);
  free(pcMult);
  free(pcR);
  /* This is allowed, because pcR is not pointing to the original string of */
  /* the pcR value.                                                         */

  return pcResult;
}


char     *a_mul16x(
  char *pcL,
  int16 iX)
{
  int       i, iStrLen;
  char     *pcResult;

  iStrLen = strlen(pcL);
  pcResult = malloc(iStrLen + 1);
  strcpy(pcResult, "0x");
  i = 2;

  while (i < iStrLen - iX)
  {
    pcResult[i] = pcL[i + iX];
    i++;
  }

  while (i < iStrLen)
  {
    pcResult[i] = '0';
    i++;
  }
  pcResult[i++] = 0;

  return pcResult;
}


char     *a_mul(
  char *pcL,
  char *pcR)
{
  char     *pcTafeltje[16];
  int16     i;
  char     *pcLine;
  char     *pcGrandTotal;
  char     *pcInterTotal;
  char     *pcNewLeft, *pcNewRight;
  int16     iFactor;
  int16     iPwr;

  Equalize(pcL, pcR, &pcNewLeft, &pcNewRight);

  pcGrandTotal = NULL;

  pcTafeltje[0] = malloc(strlen(pcR) + 1);
  strcpy(pcTafeltje[0], "0x");
  i = 2;
  while (pcR[i] != 0)
  {
    pcTafeltje[0][i] = '0';
    i++;
  }
  pcTafeltje[0][i] = 0;

  for (i = 1; i < 16; i++)
    pcTafeltje[i] = a_plus(pcTafeltje[i - 1], pcR);

  /* At this point, pcTafeltje[x] == x * pcR                                */

  iPwr = strlen(pcL) - 3;       /* weight of first digit after 0x                */
  i = 0;
  while (pcL[i + 2] != 0)
  {
    iFactor = TO_HEX(pcL[i + 2]);
    pcLine = a_mul16x(pcTafeltje[iFactor], iPwr);
    if (pcGrandTotal == NULL)
    {
      pcGrandTotal = my_strdup(pcLine);
    }
    else
    {
      pcInterTotal = a_plus(pcGrandTotal, pcLine);
      free(pcGrandTotal);
      pcGrandTotal = pcInterTotal;
    }
    i++;
    iPwr--;
    free(pcLine);
  }

  for (i = 0; i < 16; i++)
    free(pcTafeltje[i]);
  free(pcNewLeft);
  free(pcNewRight);

  return pcGrandTotal;
}


char     *a_and(
  char *pcL,
  char *pcR)
{
  if (strcmp(pcL, "TRUE") == 0)
    return my_strdup(pcR);
  else
    return my_strdup("FALSE");
}

char     *a_or(
  char *pcL,
  char *pcR)
{
  if (strcmp(pcL, "TRUE") == 0)
    return my_strdup("TRUE");
  else
    return my_strdup(pcR);
}

char     *a_not(
  char *pcL,
  char *pcR)
{
  if (strcmp(pcR, "TRUE") == 0)
    return my_strdup("FALSE");
  else
    return my_strdup("TRUE");
}

char     *a_value(
  struct TVariable *ptV)
{
  char     *pcResult;
  int16     i;

  if (ptV == NULL)
    return my_strdup(pcNull);
  if (ptV->ptValue == NULL)
    return my_strdup(pcNull);
  if (ptV->ptValue->pcValue == NULL)
    return my_strdup(pcNull);

  if (ptV->ptValue->pcValue[1] == 'x')
  {
    return my_strdup(ptV->ptValue->pcValue);
  }
  else if ((strcmp(ptV->ptValue->pcValue, "TRUE") == 0)
           || (strcmp(ptV->ptValue->pcValue, "FALSE") == 0))
  {
    return my_strdup(ptV->ptValue->pcValue);
  }
  else
  {
    pcResult = (char *) malloc(strlen(ptV->ptValue->pcValue) + 3);
    i = GetInteger(ptV->ptValue->pcValue);
    sprintf(pcResult, "0x%X", i);
    return pcResult;
  }
}
