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
#include "executor_lib.h"       /* for the _Signal constants. */
#include "scanner.h"
#include "atkernel.h"
#include "atdriver.h"
#include "executor_master.h"
#include "arithmetric.h"

#define LINELENGTH (136)

#ifndef max
#define max(a,b) ((a)>(b))?(a):(b)
#endif

#ifndef isascii
#define isascii(a) (((a)>=0x20)&&((a)<=0x7F))
#endif


/***************************************************************************/
/* Local headers.                                                          */
/***************************************************************************/

void      delValue(
  struct TValue **ppt);



/****************************************************************************/
/*                                                                          */
/* A few useful predefined records:                                         */
/*                                                                          */
/****************************************************************************/

struct TType TimerPar1Type = { "timer", /* pcName       */
  0,                            /* iEndianess         */
  0,                            /* iSizeInBytes       */
  0,                            /* uiNumberOfElements */
  0,                            /* uiSizeOfElement    */
  0,                            /* iLessAllowed       */
  NULL,                         /* pcLowerLimit       */
  NULL,                         /* pcUpperLimit       */
  TIMEKIND,                     /* iKind              */
  (float) 1.00,                 /* fTimeScaling       */
  NULL,                         /* ptDefinition */
  0                             /* iRefCount    */
};

struct TParameter TimerPar1 = { "Timer parameter",      /* pcName       */
  0,                            /* iArray       */
  0,                            /* iOffset      */
  &TimerPar1Type,               /* ptTypeDef    */
  0                             /* iRefCount    */
};

struct TParameterList TimerParamList = { &TimerPar1,    /* ptThis       */
  0,                            /* iFollowedB.. */
  {NULL,                        /* aptNext[0]   */
   NULL                         /* aptNext[1]   */
   },
  0                             /* iRefCount    */
};

struct TParameterList TimerTwoParametersList = { &TimerPar1, /* ptThis       */
  0,                            /* iFollowedB.. */
  {&TimerParamList,             /* aptNext[0]   */
   NULL                         /* aptNext[1]   */
   },
  0                             /* iRefCount    */
};

struct TParameter JustAParam = { "parameter",   /* pcName       */
  0,                            /* iArray       */
  0,                            /* iOffset      */
  NULL,                         /* ptTypeDef    */
  0                             /* iRefCount    */
};

struct TParameterList OneParam = { &JustAParam, /* ptThis       */
  0,                            /* iFollowedB.. */
  {NULL,                        /* aptNext[0]   */
   NULL                         /* aptNext[1]   */
   },
  0                             /* iRefCount    */
};

struct TParameterList TwoParam = { &JustAParam, /* ptThis       */
  0,                            /* iFollowedB.. */
  {&OneParam,                   /* aptNext[0]   */
   NULL                         /* aptNext[1]   */
   },
  0                             /* iRefCount    */
};

struct TParameterList ThreeParam = { &JustAParam,       /* ptThis       */
  0,                            /* iFollowedB.. */
  {&TwoParam,                   /* aptNext[0]   */
   NULL                         /* aptNext[1]   */
   },
  0                             /* iRefCount    */
};

struct TParameterList FourParam = { &JustAParam,        /* ptThis       */
  0,                            /* iFollowedB.. */
  {&ThreeParam,                 /* aptNext[0]   */
   NULL                         /* aptNext[1]   */
   },
  0                             /* iRefCount    */
};

struct TType SignalType = { "t_signal", /* pcName       */
  0,                            /* iEndianess         */
  1,                            /* iByteCount         */
  0,                            /* uiNumberOfElements */
  0,                            /* uiSizeOfElement    */
  0,                            /* iLessAllowed       */
  NULL,                         /* pcLowerLimit       */
  NULL,                         /* pcUpperLimit       */
  0,                            /* iKind        */
  0.0,                          /* fTimeScaling */
  NULL,                         /* ptTypeDef    */
  0                             /* iRefCount    */
};

struct TType CommSpecType = { "COMMAND_SPECIFIC",       /* pcName       */
  0,                            /* iEndianess   */
  1,                            /* iByteCount   */
  0,                            /* uiNumberOfElements */
  0,                            /* uiSizeOfElement    */
  0,                            /* iLessAllowed */
  NULL,                         /* pcLowerLimit */
  NULL,                         /* pcUpperLimit */
  0,                            /* iKind        */
  0.0,                          /* fTimeScaling */
  NULL,                         /* ptTypeDef    */
  0                             /* iRefCount    */
};

struct TType CommPackType = { "COMMAND_PACKET", /* pcName       */
  0,                            /* iEndianess   */
  1,                            /* iByteCount   */
  0,                            /* uiNumberOfElements */
  0,                            /* uiSizeOfElement    */
  0,                            /* iLessAllowed */
  NULL,                         /* pcLowerLimit */
  NULL,                         /* pcUpperLimit */
  0,                            /* iKind        */
  0.0,                          /* fTimeScaling */
  NULL,                         /* ptTypeDef    */
  0                             /* iRefCount    */
};

struct TTypeList PredefType_1 = { &CommPackType,        /* ptThis       */
  NULL,                         /* ptNext       */
  0                             /* iRefCount    */
};

struct TTypeList PredefTypes = { &CommSpecType, /* ptThis       */
  &PredefType_1,                /* ptNext       */
  0                             /* iRefCount    */
};


struct TParameter Signal = { "signal",  /* pcName       */
  0,                            /* iArray       */
  0,                            /* iOffset      */
  &SignalType,                  /* ptTypeDef    */
  0,                            /* iRefCount    */
};

struct TParameterList SignalParam = { &Signal,  /* ptThis       */
  0,                            /* iFollowedB.. */
  {NULL,                        /* aptNext[0]   */
   NULL                         /* aptNext[1]   */
   },
  0                             /* iRefCount    */
};

struct TType DataType = { "just data",  /* pcName       */
  0,                            /* iEndianess   */
  0,                            /* iByteCount   */
  0,                            /* uiNumberOfElements */
  0,                            /* uiSizeOfElement    */
  0,                            /* iLessAllowed */
  NULL,                         /* pcLowerLimit */
  NULL,                         /* pcUpperLimit */
  0,                            /* iKind        */
  0.0,                          /* iTimeScaling */
  NULL,                         /* pTypeDef     */
  0                             /* iRefCount    */
};

struct TType BooleanType = { "boolean", /* pcName       */
  0,                            /* iEndianess   */
  1,                            /* iByteCount   */
  0,                            /* uiNumberOfElements */
  0,                            /* uiSizeOfElement    */
  0,                            /* iLessAllowed */
  "0x00",                       /* pcLowerLimit */
  "0x01",                       /* pcUpperLimit */
  0,                            /* iKind        */
  0.0,                          /* iTimeScaling */
  0                             /* iRefCount    */
};


/****************************************************************************/
/****************************************************************************/


struct TTypeList *_ptTypeList = NULL;
struct TFunctionList *_ptFunctionList = NULL;
struct TEventList *_ptEventList = NULL;
struct TStateMachineList *_ptStateMachineList = NULL;
struct TDataList *_ptDataList = NULL;
struct TBoardList *_ptBoardList = NULL;
struct TGlobals *_ptGlobals = NULL;
struct TIOList *_ptIOList = NULL;
struct TVariableList *_ptGlobalVars = NULL;
struct TType *_ptDataType = &DataType;
struct TType *_ptBooleanType = &BooleanType;

struct BSEK_DeviceDescription *_ptDeviceList = NULL;


int       iWarningCounter;
int       iErrorCounter;
int       iFatalCounter;

char     *_pcWarningText = NULL;
char     *_pcErrorText = NULL;
char     *_pcFatalText = NULL;
char     *_pcLine = NULL;

struct BSEK_DeviceDescription *_ptDeviceDescription;
int16     iDeviceCounter;
int      *piChannels;
char     *pcDevice[16];
char     *pcProtocol;



/****************************************************************************/
/* Local function headers.                                                  */
/****************************************************************************/
void      StoreFileVariable(
  struct TVariable *ptVar);

void      WriteDataList(
  char *pcBuffer);
void      WriteBoard(
  char *pcBuffer,
  struct TBoard *ptBoard,
  int iMode);
void      WriteStateMachine(
  char *pcBuffer,
  struct TStateMachine *ptStateMachine);
void      WriteStateMachineHeader(
  int16 iMode,
  char *pcBuffer,
  struct TStateMachine *ptStateMachine);
void      WriteStateList(
  char *pcBuffer,
  struct TStateList *ptStateList);
void      WriteVarList(
  char *pcBuffer,
  struct TVariableList *ptVarList);
void      WriteVarListGlobal(
  char *pcBuffer,
  struct TVariableList *ptVarList);
void      WriteVarGlobal(
  char *pcBuffer,
  struct TVariable *ptVar);
void      WriteState(
  char *pcBuffer,
  struct TState *ptState);
void      WriteAction(
  char *pcBuffer,
  struct TAction *ptAction);

/****************************************************************************/
/****************************************************************************/

void Increment(
  int *i)
/* This function is used to increment the reference counter of records. If  */
/* the iRefCounter is zero, the record is a predefined, fixed record and    */
/* can never be deleted. To indicate this, the iRefCounter must remain zero */
/* forever. Therefore, if iRefCount is zero, it will not be incremented.    */
{
  if (*i > 0)
  {
    (*i)++;
  }
}

void Decrement(
  int *i)
/* This function is used to decrement the reference counter of records. If  */
/* the iRefCounter is zero, the record is a predefined, fixed record and    */
/* can never be deleted. if iRefCount is zero, the record is a predefined   */
/* one. Therefore, the counter may never become zero by decrementing the    */
/* link count. If this should happen, an internal error is detected.        */
{
  if (*i > 0)
  {
    (*i)--;
    if (*i == 0)
    {
      /* internal error....                                                 */
      FatalError1("INTERNAL ERROR... Decr. refererence counter to zero\n");
    }
  }
}

void ConnectVarToVarList(
  struct TVariable **pptVar,
  struct TVariableList *ptReferenceList,
  struct TDataList *ptDataList,
  struct TParameterList *ptDefList)
{
  struct TVariable *ptRefVar;
  struct TData *ptDataRecord;
  int       iSize;

  ptRefVar = FindVar((*pptVar)->pcName, ptReferenceList);
  ptDataRecord = NULL;

  if (ptRefVar == NULL)
  {
    /* Not found. Try to find the variable in the globally defined Data     */
    /* definition list.                                                     */

    ptDataRecord = FindData((*pptVar)->pcName, ptDataList);
    if (ptDataRecord == NULL)
    {
      AddError2("Error: Undefined parameter %s", (*pptVar)->pcName);
    }
    else
    {
      (*pptVar)->ptItsType = _ptDataType;
    }
  }

  if ((ptDefList != NULL)
      && (ptDefList->ptThis != NULL)
      && (ptDefList->ptThis->ptTypeDef != NULL) && (ptRefVar != NULL))
  {
    /* The expected TypeDef is reachable and known. The intended variable   */
    /* is found in the Reference List. We can check now if the types match. */

    if (ptRefVar->ptItsType == NULL)
    {
      /* The used variable has no type yet. The only thing we can do is     */
      /* assigning the expected type to the (yet) untyped variable. The     */
      /* next time this variable is used, real typechecking will be done.   */

      ptRefVar->ptItsType = ptDefList->ptThis->ptTypeDef;
      Increment(&ptRefVar->ptItsType->iRefCount);

      iSize = ptRefVar->ptItsType->iSizeInBytes * 2 + 3;
      if (ptRefVar->ptValue != NULL)
      {
        /* This line can possibly removed after testing...                  */
        /* Not true. This has to be tested in detail..                      */
        /* AddError1("***Internal error. Unexpected NON-NULL pointer.");    */
      }
      else
      {
        ptRefVar->ptValue = newValue();
        ptRefVar->ptValue->pcValue = (char *) malloc(iSize);
        ptRefVar->ptValue->pcValue[0] = 0;
      }
    }
    else
    {
      /* The used variable has a type already. Check if the types are mat-  */
      /* ching.                                                             */

      if (ptRefVar->ptItsType != ptDefList->ptThis->ptTypeDef)
      {
        AddError5("Error:type of par. %s is %s. Variable %s has type %s",
                  ptDefList->ptThis->pcName,
                  ptDefList->ptThis->ptTypeDef->pcName,
                  ptRefVar->pcName, ptRefVar->ptItsType->pcName);
      }
      /* else: the types are matching. Linking will be done anyway, so we   */
      /* don't have to do anything here.                                    */
    }
  }

  /* The types of the variables are checked now. The variable in the Refe-  */
  /* rence list is typed, so now we can really remove the locally allocated */
  /* variable, and refer to the variable in the Reference List.             */
  /* This will be done in any case, also if types do not match. The user    */
  /* won't be able to run the statemachines anyway, because an error is re- */
  /* ported and a global error flag is set already.                         */

  /* If the variable was a databuffer, no typechecking has been done. The   */
  /* variable is replaced by the global databuffer variable in that case.   */

  delVariable(pptVar);

  if (ptRefVar != NULL)
  {
    /* Normal variable.                                                     */
    (*pptVar) = ptRefVar;
  }
  else
  {
    /* Databuffer variable                                                  */
    (*pptVar) = (struct TVariable *) ptDataRecord;
  }
}

int Connect_Invocation(
  struct TVariable *ptParam,
  struct TVariable *ptInvoke)
/****************************************************************************/
/* definition of a statemachine : statemachine(a,b) = { ... }               */
/* invokation of a statemachine ; statemachine(x,y)                         */
/* a, b is Param list,                                                      */
/* x, y is Invoke list.                                                     */
/* before connecting, a physical copy of the statemachine has already been  */
/* made, therefore the Param list is a copy which should be connected to    */
/* the invoke list.                                                         */
/* In this function, only a and x are connected. In a next call, b and y    */
/* will be connected.                                                       */
/* Connection means: All references of a will be removed, and replaced by   */
/* references to x.                                                         */
/****************************************************************************/
{

  /* First check if connection is allowed.                                  */
  /* Connection is allowed if types do not conflict.                        */

  if ((ptParam == NULL) || (ptInvoke == NULL))
  {
    Fatal("Connect_Invocation, line 389");
    return FALSE;
  }

  if ((ptParam->ptItsType != NULL)
      && (ptInvoke->ptItsType != NULL)
      && (ptParam->ptItsType != ptInvoke->ptItsType))
  {
    AddError4("%s has type %s, whilst %s was expected",
              ptInvoke->pcName,
              ptInvoke->ptItsType->pcName, ptParam->ptItsType->pcName);
    return FALSE;
  }

  if ((ptParam->ptItsType != NULL) && (ptInvoke->ptItsType == NULL))
  {
    ptInvoke->ptItsType = ptParam->ptItsType;
    Increment(&(ptInvoke->ptItsType->iRefCount));
  }

  if (ptParam->ptValue != NULL)
  {
    delValue(&(ptParam->ptValue));
  }

  /* And make the real value connection now.                                */

  ptParam->ptValue = ptInvoke->ptValue;

  if (ptParam->ptValue == NULL)
  {
    ptParam->ptValue = newValue();
    ptInvoke->ptValue = ptParam->ptValue;
  }

  /* The value _was_ used in one statemachine, and is used now in two       */
  /* statemachines. Therefore, increment it's refcounter.                   */

  Increment(&ptParam->ptValue->iRefCount);

  return TRUE;

}


int Connect_Invocations(
  struct TVariableList *ptParams,
  struct TVariableList *ptInvokes)
/****************************************************************************/
/* definition of a statemachine : statemachine(a,b) = { ... }               */
/* invokation of a statemachine ; statemachine(x,y)                         */
/* a, b is Param list,                                                      */
/* x, y is Invoke list.                                                     */
/* before connecting, a physical copy of the statemachine has already been  */
/* made, therefore the Param list is a copy which should be connected to    */
/* the invoke list.                                                         */
/****************************************************************************/
{
  while (ptParams)
  {
    if (ptInvokes == NULL)
    {
      AddError1("Parameter count mismatch.");
      return FALSE;
    }

    Connect_Invocation(ptParams->ptThis, ptInvokes->ptThis);

    ptParams = ptParams->ptNext;
    ptInvokes = ptInvokes->ptNext;
  }
  if (ptInvokes != NULL)
  {
    AddError1("Parameter count mismatch.");
    return FALSE;
  }
  return TRUE;
}




void StoreFileVariableList(
  struct TVariableList *ptList)
/****************************************************************************/
/* Checks a list of variables, and writes all file-variables to a file.     */
/* (For each variable, a file will be created/changed). Non-file-variables  */
/* will of course not be written to a file.                                 */
/* Values and filenames are not changed.                                    */
/*                                                                          */
/* File variables are declared in scripts  using the keyword FILE:          */
/* VAR x = FILE "c:\file.txt"                                               */
/****************************************************************************/
{
  while (ptList != NULL)
  {
    StoreFileVariable(ptList->ptThis);
    ptList = ptList->ptNext;
  }
}



void StoreFileVariable(
  struct TVariable *ptVar)
/****************************************************************************/
/* If the variable is a file variable, the value of the variable is written */
/* into that file. Otherwise, the function has no effect. Values and file-  */
/* names are not changed.                                                   */
/*                                                                          */
/* File variables are declared in scripts  using the keyword FILE:          */
/* VAR x = FILE "c:\file.txt"                                               */
/****************************************************************************/
{
  struct TValue *ptVal;
  FILE     *f;

  if (ptVar == NULL)
    return;
  if (ptVar->ptValue == NULL)
    return;
  if (ptVar->ptValue->pcFileName == NULL)
    return;

  ptVal = ptVar->ptValue;

  f = fopen(ptVal->pcFileName, "w");
  if (f == NULL)
    return;

  if ((ptVal->pcValue != NULL) && (ptVal->pcValue[0] != '\0'))
  {
    fprintf(f, "%s", ptVal->pcValue);
  }
  fclose(f);
}




void delIO(
  struct TIO **ppt)
{
  if ((*ppt) != NULL)
  {
    if ((*ppt)->iNumber != -1)
    {
      BSED_CloseDevice((*ppt)->iNumber);
    }
    (*ppt) = NULL;
  }
}

void delIOList(
  struct TIOList **ppt)
{
  if (*ppt != NULL)
  {
    delIO(&(*ppt)->ptThis);
    delIOList(&(*ppt)->ptNext);
    free(*ppt);
    *ppt = NULL;
  }
}

void delValue(
  struct TValue **ppt)
{

  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      /* Only delete the pcValue pointer if it is referring to a normal   */
      /* value. This is not the case, if the pointer is a NULL pointer,   */
      /* or if the pointer refers to _SIGNAL or _NO_SIGNAL. These         */
      /* pointers refer to hard-coded strings, which can never be freed.  */


      if (((*ppt)->pcValue != NULL)
          && ((*ppt)->pcValue != _NO_SIGNAL) && ((*ppt)->pcValue != _SIGNAL))
      {
        free((*ppt)->pcValue);
      }
      if((*ppt)->pcFileName != NULL)
      {
        free((*ppt)->pcFileName);
      }
      if((*ppt)->pcFileMode != NULL)
      {
        free((*ppt)->pcFileMode);
      }
      free(*ppt);
      *ppt = NULL;
    }
    else
    {
      Decrement(&((*ppt)->iRefCount));
    }
  }
}

void delValueDef(
  struct TValueDef **ppt)
{
  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      free((*ppt)->pcMask);
      free((*ppt)->pcValue);
      free((*ppt)->pcDescription);

      free(*ppt);
      *ppt = NULL;
    }
    else
    {
      Decrement(&((*ppt)->iRefCount));
    }
  }
}

void delValueDefList(
  struct TValueDefList **ppt)
{
  struct TValueDefList *ptTail;

  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      delValueDef(&((*ppt)->ptThis));
      delValueDefList(&((*ppt)->ptNext));

      ptTail = ((*ppt)->ptNext);
      free(*ppt);
      *ppt = ptTail;
    }
    else
    {
      delValueDefList(&((*ppt)->ptNext));
    }
  }
}

void delType(
  struct TType **ppt)
{
  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      free((*ppt)->pcName);
      free((*ppt)->pcUpperLimit);
      free((*ppt)->pcLowerLimit);
      if ((*ppt)->pcDecimalDescription != NULL)
        free((*ppt)->pcDecimalDescription);
      delValueDefList(&((*ppt)->ptDefinition));

      free(*ppt);
      *ppt = NULL;
    }
    else
    {
      Decrement(&((*ppt)->iRefCount));
    }
  }
}

void delTypeList(
  struct TTypeList **ppt)
/****************************************************************************/
/* Try to delete the TypeList record. The idea is that the function calling */
/* this function, has access to a data structure containing a pointer to    */
/* the record which must be deleted. That data structure is refferring to   */
/* this record, and if that data structure is the only one, the reference   */
/* count is one. So if the reference count is one, and that data structure  */
/* doesn't need this record any more, the record can be deleted. However,   */
/* this "list" record provides access to individual Type records. If this   */
/* list record is the only record refferring to that type record, that type */
/* record can be deleted. If there are other references to that type record,*/
/* that type record cannot be deleted, and this typelist record can't be    */
/* deleted too. Therefore, both reference counts must be 1 before the       */
/* record can be deleted.                                                   */
/*                                                                          */
/* If iRefCount is already zero before we've done anything, the record is   */
/* a predefined record and may never be deleted. In that case, we only try  */
/* to delete the tail.                                                      */
/*                                                                          */
/* If deleting the tail fails, (because there are too much references to    */
/* records in the tail) the chain would be broken if this record was just   */
/* removed. To restore the chain, a pointer to the tail will be returned,   */
/* iso. a NULL pointer. If deleting the tail succeeded, a NULL pointer will */
/* be returned anyway.                                                      */
/****************************************************************************/
{
  struct TTypeList *ptTail;

  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 0)
    {
      delTypeList(&((*ppt)->ptNext));
    }
    else
    {
      if (((*ppt)->iRefCount == 1)      /* always evaluated.            */
          && ((*ppt)->ptThis->iRefCount == 1)   /* not always evaluated.        */
        )
      {
        delType(&((*ppt)->ptThis));
        delTypeList(&((*ppt)->ptNext));

        ptTail = (*ppt)->ptNext;

        free(*ppt);
        *ppt = ptTail;
      }
      else
      {
        delTypeList(&((*ppt)->ptNext));
      }
    }
  }
}

void delParameter(
  struct TParameter **ppt)
{
  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      free((*ppt)->pcName);
      delType(&((*ppt)->ptTypeDef));

      free(*ppt);
      *ppt = NULL;
    }
    else
    {
      Decrement(&((*ppt)->iRefCount));
    }
  }
}

void delParameterList(
  struct TParameterList **ppt)
{
  int       i;

  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      delParameter(&((*ppt)->ptThis));
      if ((*ppt)->ptRepRec != NULL)
      {
        (*ppt)->aptNext[1] = NULL;
      }
      for (i = 0; i < UNIONARRAYSIZE + 1; i++)
      {
        delParameterList(&((*ppt)->aptNext[i]));
      }
      if ((*ppt)->ptRepRec != NULL)
        free((*ppt)->ptRepRec);
      free(*ppt);
      *ppt = NULL;
    }
    else
    {
      Decrement(&((*ppt)->iRefCount));
    }
  }
}

void delFunction(
  struct TFunction **ppt)
{
  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      free((*ppt)->pcName);
      delParameterList(&((*ppt)->ptSendParameters));
      delParameterList(&((*ppt)->ptReceiveParameters));

      free(*ppt);
      *ppt = NULL;
    }
    else
    {
      Decrement(&((*ppt)->iRefCount));
    }
  }
}

int CountTypeRefs(
  struct TTypeList *pt)
{
  int       iCount = 0;

  while (pt)
  {
    iCount += pt->ptThis->iRefCount;
    pt = pt->ptNext;
  }
  return iCount;
}

void delFunctionList(
  struct TFunctionList **ppt)
{
  struct TFunctionList *ptTail;

  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      delFunction(&((*ppt)->ptThis));
      delFunctionList(&((*ppt)->ptNext));

      ptTail = (*ppt)->ptNext;
      free(*ppt);
      *ppt = ptTail;
    }
    else
    {
      delFunctionList(&((*ppt)->ptNext));
    }
  }
}

void delEvent(
  struct TEvent **ppt)
{
  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      free((*ppt)->pcName);
      delParameterList(&((*ppt)->ptParameters));

      free(*ppt);
      *ppt = NULL;
    }
    else
    {
      Decrement(&((*ppt)->iRefCount));
    }
  }
}

void delEventList(
  struct TEventList **ppt)
{
  struct TEventList *ptTail;

  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      delEvent(&((*ppt)->ptThis));
      delEventList(&((*ppt)->ptNext));

      ptTail = (*ppt)->ptNext;
      free(*ppt);
      *ppt = ptTail;
    }
    else
    {
      delEventList(&((*ppt)->ptNext));
    }
  }
}

void delVariable(
  struct TVariable **ppt)
{
  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      if ((*ppt)->ptItsType == _ptDataType)
      {
        if ((*ppt)->ptValue != NULL)
        {
          /* Workaround. There is for sure a better solution. At this point */
          /* there should be no need to refer to ptValue because the value  */
          /* field of a tDataRecord is undefined.                           */

          delData((struct TData **) ppt);

        }
      }
      else
      {
        if ((*ppt)->pcName != NULL)
          free((*ppt)->pcName);
        delValue(&((*ppt)->ptValue));
        delType(&((*ppt)->ptItsType));
        free(*ppt);
        *ppt = NULL;
      }
    }
    else
    {
      Decrement(&((*ppt)->iRefCount));
    }
  }
}

void delVariableList(
  struct TVariableList **ppt)
{
  struct TVariableList *ptTail;

  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      delVariable(&((*ppt)->ptThis));
      delVariableList(&((*ppt)->ptNext));

      ptTail = (*ppt)->ptNext;

      free(*ppt);
      *ppt = ptTail;
    }
    else
    {
      delVariableList(&((*ppt)->ptNext));
    }
  }
}


void delAction(
  struct TAction **ppt)
{
  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      delFunction(&((*ppt)->ptFunction));
      delEvent(&((*ppt)->ptEvent));
      delVariableList(&((*ppt)->ptVarList));
#ifdef ARITHMETRIC
      delExpression(&((*ppt)->ptExpression));
#endif

      free(*ppt);
      *ppt = NULL;
    }
    else
    {
      Decrement(&((*ppt)->iRefCount));
    }
  }
}

void delActionList(
  struct TActionList **ppt)
{
  struct TActionList *ptTail;

  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      delAction(&((*ppt)->ptThis));
      delActionList(&((*ppt)->ptNext));
      ptTail = (*ppt)->ptNext;

      free(*ppt);
      *ppt = ptTail;
    }
    else
    {
      delActionList(&((*ppt)->ptNext));
    }
  }
}

void delTransition(
  struct TTransition **ppt)
{
  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      delActionList(&((*ppt)->ptActionList));
      delState(&((*ppt)->ptNextState));
      if ((*ppt)->pcNextStateName != NULL)
      {
        free((*ppt)->pcNextStateName);
      }
      free(*ppt);
      *ppt = NULL;
    }
    else
    {
      Decrement(&((*ppt)->iRefCount));
    }
  }
}

void delTransitionList(
  struct TTransitionList **ppt)
{
  struct TTransitionList *ptTail;

  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      delTransition(&((*ppt)->ptThis));
      delTransitionList(&((*ppt)->ptNext));
      ptTail = (*ppt)->ptNext;

      free(*ppt);
      *ppt = ptTail;
    }
    else
    {
      delTransitionList(&((*ppt)->ptNext));
    }
  }
}

void delState(
  struct TState **ppt)
{
  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      free((*ppt)->pcName);
      delTransitionList(&((*ppt)->ptTransitionList));

      free(*ppt);
      *ppt = NULL;
    }
    else
    {
      Decrement(&((*ppt)->iRefCount));
    }
  }
}

void delStateList(
  struct TStateList **ppt)
{
  struct TStateList *ptTail;

  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      delState(&((*ppt)->ptThis));
      delStateList(&((*ppt)->ptNext));

      ptTail = (*ppt)->ptNext;
      free(*ppt);
      *ppt = ptTail;
    }
    else
    {
      delStateList(&((*ppt)->ptNext));
    }
  }
}

void delStateMachine(
  struct TStateMachine **ppt)
{
  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      free((*ppt)->pcName);
      free((*ppt)->pcRescueStateName);
      free((*ppt)->pcDeviceName);
      delVariableList(&((*ppt)->ptLocalVars));
      /* no need to del the parameterlist, because this list was linked to    */
      /* the localvarlist.  Because the local var list is already deleted,    */
      /* the parametervarlist is also deleted.                                */

      delStateList(&((*ppt)->ptStates));

      free(*ppt);
      *ppt = NULL;
    }
    else
    {
      Decrement(&((*ppt)->iRefCount));
    }
  }
}

void delStateMachineList(
  struct TStateMachineList **ppt)
{
  struct TStateMachineList *ptTail;

  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      delStateMachine(&((*ppt)->ptThis));
      delStateMachineList(&((*ppt)->ptNext));

      ptTail = (*ppt)->ptNext;

      free(*ppt);
      *ppt = NULL;
    }
    else
    {
      delStateMachineList(&((*ppt)->ptNext));
    }
  }
}

void delData(
  struct TData **ppt)
{
  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      free((*ppt)->pcName);
      free((*ppt)->pcValue);

      free(*ppt);
      *ppt = NULL;
    }
    else
    {
      Decrement(&((*ppt)->iRefCount));
    }
  }
}

void delDataList(
  struct TDataList **ppt)
{
  struct TDataList *ptTail;

  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      delData(&((*ppt)->ptThis));
      delDataList(&((*ppt)->ptNext));

      ptTail = (*ppt)->ptNext;

      free(*ppt);
      *ppt = ptTail;
    }
    else
    {
      delDataList(&((*ppt)->ptNext));
    }
  }
}


void delBoard(
  struct TBoard **ppt)
{
  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      free(((*ppt)->pcName));
      delStateMachineList(&((*ppt)->ptStateMachines));

      free(*ppt);
      *ppt = NULL;
    }
    else
    {
      Decrement(&((*ppt)->iRefCount));
    }
  }
}

void delBoardList(
  struct TBoardList **ppt)
{
  struct TBoardList *ptTail;

  if ((*ppt) != NULL)
  {
    if ((*ppt)->iRefCount == 1)
    {
      delBoard(&((*ppt)->ptThis));
      delBoardList(&((*ppt)->ptNext));

      ptTail = (*ppt)->ptNext;
      free(*ppt);
      *ppt = ptTail;
    }
    else
    {
      delBoardList(&((*ppt)->ptNext));
    }
  }
}


struct TValue *Duplicate_Value(
  struct TValue *ptV,
  int iValueLength)
/* iValueLength indicates the number of bytes to be stored in hex represen- */
/* tation. A three byte value is respresented as 0xXXXXXX, so the buffer    */
/* must be able to contain 8+1 characters, incl terminating zero.           */
/* The iValueLength indicates the number of bytes, not the number of cha-   */
/* racters needed to represent those bytes.                                 */
{
  struct TValue *ptCopy = NULL;

  if (ptV != NULL)
  {
    ptCopy = newValue();

    ptCopy->bFixed   = ptV->bFixed;
    ptCopy->uiLength = ptV->uiLength;

    if (ptV->pcValue != NULL)
    {
      if (ptV->pcValue[0] != '\0')
      {
        ptCopy->pcValue = my_strdup(ptV->pcValue);
      }
      else
      {
        ptCopy->pcValue = (char *) malloc(iValueLength * 2 + 3);
        ptCopy->pcValue[0] = '\0';
      }
    }

    ptCopy->pcFileName = my_strdup(ptV->pcFileName);
    ptCopy->pcFileMode = my_strdup(ptV->pcFileMode);
  }
  return ptCopy;
}


struct TVariable *Duplicate_Variable(
  struct TVariable *ptV,
  struct TVariableList *ptReference)
{
  struct TVariable *ptCopy;

  ptCopy = FindVar(ptV->pcName, ptReference);

  if (ptCopy == NULL)
  {
    /* Not in the provided reference variable list. Try find it in the      */
    /* global databuffer store.                                             */
    ptCopy = (struct TVariable *) FindData(ptV->pcName, _ptDataList);
  }

  if (ptCopy == NULL)
  {
    /* Variable not found in Reference list. Make a real new one.           */

    ptCopy = newVariable(NULL);
    ptCopy->pcName = my_strdup(ptV->pcName);
    ptCopy->ptItsType = ptV->ptItsType;
    if (ptCopy->ptItsType != NULL)
    {
      Increment(&(ptCopy->ptItsType->iRefCount));
    }

    if ((ptV->ptItsType != NULL) && (ptV->ptValue != NULL))
    {
      ptCopy->ptValue =
        Duplicate_Value(ptV->ptValue, ptV->ptItsType->iSizeInBytes);
    }
    else
    {
      if ((ptV->ptValue != NULL)
          && (ptV->ptValue->pcValue != NULL) && (ptV->ptValue->pcValue[0] != 0))
      {
        ptCopy->ptValue = Duplicate_Value(ptV->ptValue, 0);
      }
      else
      {
        ptCopy->ptValue = NULL;
      }
    }
  }
  return ptCopy;
}

struct TVariableList *Duplicate_VariableList(
  struct TVariableList *ptV,
  struct TVariableList *ptRef)
{
  struct TVariableList *ptCopy;

  if (ptV)
  {
    ptCopy = newVariableList();
    if (ptV->ptThis != NULL)
    {
      ptCopy->ptThis = Duplicate_Variable(ptV->ptThis, ptRef);
    }
    if (ptV->ptNext != NULL)
    {
      ptCopy->ptNext = Duplicate_VariableList(ptV->ptNext, ptRef);
    }
    return ptCopy;
  }
  else
  {
    return NULL;
  }
}


struct TAction *Duplicate_Action(
  struct TAction *ptA,
  struct TVariableList *ptRef)
{
  struct TAction *ptCopy;

  if (ptA != NULL)
  {
    ptCopy = newAction();
    ptCopy->iActionType = ptA->iActionType;
    ptCopy->iNrOfReceptions = ptA->iNrOfReceptions;
    ptCopy->ptFunction = ptA->ptFunction;
    ptCopy->ptStateMachine = ptA->ptStateMachine;
    ptCopy->ptEvent = ptA->ptEvent;
    ptCopy->ptExpression = dupExpression(ptA->ptExpression, ptRef);
    ptCopy->ptVarList = Duplicate_VariableList(ptA->ptVarList, ptRef);
    ptCopy->iSourceLineNumber = ptA->iSourceLineNumber;

    if (ptCopy->ptFunction != NULL)
      Increment(&(ptCopy->ptFunction->iRefCount));
    if (ptCopy->ptEvent != NULL)
      Increment(&(ptCopy->ptEvent->iRefCount));

    return ptCopy;
  }
  else
    return NULL;
}


struct TActionList *Duplicate_ActionList(
  struct TActionList *ptA,
  struct TVariableList *ptRef)
{
  struct TActionList *ptCopy;

  if (ptA != NULL)
  {
    ptCopy = newActionList();

    if (ptA->ptThis != NULL)
    {
      ptCopy->ptThis = Duplicate_Action(ptA->ptThis, ptRef);
    }

    if (ptA->ptNext != NULL)
    {
      ptCopy->ptNext = Duplicate_ActionList(ptA->ptNext, ptRef);
    }

    return ptCopy;
  }
  else
    return NULL;
}

struct TTransition *Duplicate_Transition(
  struct TTransition *ptOrg,
  struct TVariableList *ptRef)
{
  struct TTransition *ptCopy;

  if (ptOrg != NULL)
  {
    ptCopy = newTransition();
    ptCopy->ptActionList = Duplicate_ActionList(ptOrg->ptActionList, ptRef);
    ptCopy->pcNextStateName = my_strdup(ptOrg->pcNextStateName);
    ptCopy->iInstantNextState = ptOrg->iInstantNextState;
    ptCopy->iAlternative = ptOrg->iAlternative;
    return ptCopy;
  }
  else
  {
    return NULL;
  }
}

struct TTransitionList *Duplicate_TransitionList(
  struct TTransitionList *ptList,
  struct TVariableList *ptRef)
{
  struct TTransitionList *ptCopy;

  if (ptList != NULL)
  {
    ptCopy = newTransitionList();
    ptCopy->ptThis = Duplicate_Transition(ptList->ptThis, ptRef);
    ptCopy->ptNext = Duplicate_TransitionList(ptList->ptNext, ptRef);
    return ptCopy;
  }
  else
  {
    return NULL;
  }
}

struct TState *Duplicate_State(
  struct TState *ptS,
  struct TVariableList *ptRef)
{
  struct TState *ptCopy;

  if (ptS != NULL)
  {
    ptCopy = newState();
    ptCopy->pcName = my_strdup(ptS->pcName);
    ptCopy->ptTransitionList =
      Duplicate_TransitionList(ptS->ptTransitionList, ptRef);

    return ptCopy;
  }
  else
    return NULL;
}

struct TStateList *Duplicate_StateList(
  struct TStateList *ptS,
  struct TVariableList *ptRef)
{
  struct TStateList *ptCopy;

  if (ptS != NULL)
  {
    ptCopy = newStateList();
    if (ptS->ptThis)
      ptCopy->ptThis = Duplicate_State(ptS->ptThis, ptRef);
    if (ptS->ptNext)
      ptCopy->ptNext = Duplicate_StateList(ptS->ptNext, ptRef);
    return ptCopy;
  }
  else
    return NULL;
}

struct TStateMachine *Duplicate_StateMachine(
  struct TStateMachine *ptM)
/* Make a physical copy of a StateMachine record. One remark needs to be    */
/* made: A StateMachine record can be seen as a tree of records. To make    */
/* a physical copy, the whole tree will be copied. However, there are       */
/* references from the variable records in the action records to the Local- */
/* Var variable list. (I don't know how to say this simpler). For duplica-  */
/* ting the Local Var list, the second parameter in the Duplicate_Variable  */
/* list is NULL, to force a real duplication. For duplicating the rest of   */
/* the action record, variable records must not be duplicated, but need to  */
/* be linked to the just made physical copy of the original local variable  */
/* list.                                                                    */
{
  struct TStateMachine *ptCopy;
  struct TVariableList *ptWalker;

  if (ptM != NULL)
  {
    ptCopy = newStateMachine();

    ptCopy->pcName = my_strdup(ptM->pcName);

    ptCopy->ptLocalVars = Duplicate_VariableList(ptM->ptLocalVars, NULL);
    /* because the parameter list is connected to the LocalVar list,        */
    /* the parameter list is copied too. However, we need to find the entry */
    /* of the Parameter List in this variable list.                         */
    /* Walk through both lists, and stop if in the first list the right re- */
    /* cord is found.                                                       */

    ptCopy->ptParameters = ptCopy->ptLocalVars;
    ptWalker = ptM->ptLocalVars;        /* start looking from the start            */

    while (ptM && (ptWalker != ptM->ptParameters))
    {
      ptWalker = ptWalker->ptNext;
      ptCopy->ptParameters = ptCopy->ptParameters->ptNext;
    }

    /* Found. (Or an internal error, if ptM == NULL) Stop walking, ptCopy-> */
    /* ptParameters is pointing to the right record now.                    */

    ptCopy->ptStates = Duplicate_StateList(ptM->ptStates, ptCopy->ptLocalVars);

    ptCopy->iHidden = ptM->iHidden;
    ptCopy->pcRescueStateName = my_strdup(ptM->pcRescueStateName);

    return ptCopy;
  }
  else
    return NULL;
}



void Warn(
  char *pcLine)
{
  strcat(_pcWarningText, pcLine);
}

void Error(
  char *pcLine)
{
  strcat(_pcErrorText, pcLine);
}

void Fatal(
  char *pcLine)
{
  strcat(_pcFatalText, pcLine);
}


int FindBoardNumber(
  char *pcName)
{
  struct TIOList *ptList;

  char      pcError[250];
  int       i;

  ptList = _ptIOList;
  while (ptList != NULL)
  {
    if ((ptList != NULL)
        && (ptList->ptThis != NULL)
        && (strcmp(ptList->ptThis->pcName, pcName) == 0))
    {
      return ptList->ptThis->iNumber;
    }
    ptList = ptList->ptNext;
  }

  /* The board is not found. Generate a nice error log. */

  sprintf(pcError, "Device %s not found!\n\nAvailable devicenames:\n", pcName);
  ptList = _ptIOList;
  i = 0;
  while ((ptList != NULL) && (i < 5))
  {
    if ((ptList != NULL) && (ptList->ptThis != NULL))
    {
      strcat(pcError, ptList->ptThis->pcName);
      strcat(pcError, "\n");
      i++;
    }
    ptList = ptList->ptNext;
  }

  if (i == 0)
  {
    strcat(pcError,
           "none. You are using an old .prot file, without an [io] section\n");
  }
  else if (i == 5)
  {
    strcat(pcError, "...");
  }

  AddError1(pcError);

  return -1;

}


struct TData *FindData(
  char *pcVarName,
  struct TDataList *ptReference)
/****************************************************************************/
/* input  : pcVarName   : the name of the searched datafield                */
/*          ptReference : The datalist which must be searched in.           */
/* output : --                                                              */
/* globals: --                                                              */
/* result : ptr to the searched DataRecord                                  */
/*          NULL if not found.                                              */
/****************************************************************************/
{
  if (pcVarName == NULL)
    return NULL;
  while (ptReference != NULL)
  {
    if (ptReference->ptThis != NULL)
    {
      if (ptReference->ptThis->pcName != NULL)
      {
        if (strcmp(ptReference->ptThis->pcName, pcVarName) == 0)
        {
          Increment(&(ptReference->ptThis->iRefCount));
          return ptReference->ptThis;
        }
      }
      else
      {
        FatalError2("unexpected NULL pointer while looking for %s", pcVarName);
        return NULL;
      }


    }
    else
    {
      FatalError2("unexpected NULL pointer while looking for %s", pcVarName);
      return NULL;
    }
    ptReference = ptReference->ptNext;
  }
  return NULL;
}

char     *GetValueForMatching(
  struct TParameter *ptThis,
  char *pcBuffer)
{
  int       i;
  char     *pcReturnValue;
  char      Byte[3];

  pcReturnValue = NULL;

  i = strlen(ptThis->ptValue->pcValue);
  if (0 < i)
  {
    pcReturnValue = (char *) malloc(i + 1);
    if (NULL != pcReturnValue)
    {
      sprintf(pcReturnValue, "0x");
      i /= 2;
      i -= 1;

      while (i > 0)
      {
        sprintf(Byte, "%02X", (unsigned char) *pcBuffer);
        strcat(pcReturnValue, Byte);
        pcBuffer++;
        i--;
      }
    }
  }
  return pcReturnValue;
}

int BufferMatchesParameterList(
  char *pcBuffer,
  int iLength,
  struct TParameterList *ptParameterList)
{
  char     *pcCurrentValue;
  int       i = 0;

  while ((ptParameterList != NULL) && (i < iLength))
  {
    if (ptParameterList->ptRepRec != NULL)
    {
      /* We assume to be finished. */
      return TRUE;
    }
    else
    {
      if (ptParameterList->ptThis != NULL)
      {
        if ((ptParameterList->ptThis->ptValue != NULL)
            && (ptParameterList->ptThis->ptTypeDef == NULL))
        {
          /* We're pointing to a constant field!! This is what we need...   */

          pcCurrentValue =
            GetValueForMatching(ptParameterList->ptThis, pcBuffer + i);
          if (NULL != pcCurrentValue)
          {
            if (strcmp
                (ptParameterList->ptThis->ptValue->pcValue,
                 pcCurrentValue) == 0)
            {
              if (ptParameterList->iFollowedByUnion == TRUE)
              {
                /* Don't check further :-)) */
                free(pcCurrentValue);
                return TRUE;
              }
              else
              {
                i = i + (strlen(pcCurrentValue) - 2) / 2;
                free(pcCurrentValue);
                ptParameterList = ptParameterList->aptNext[0];
              }
            }
            else
            {
              /* A prooved mismatch :-( */
              free(pcCurrentValue);
              return FALSE;
            }
          }
          else
          {
            /* A failed to get Value for matching :-( */
            return FALSE;
          }
        }
        else
        {
          /* No constant field. Don't compare, just continue.. */
          if (ptParameterList->iFollowedByUnion == TRUE)
          {
            /* Don't check further :-)) */
            return TRUE;
          }
          else
          {
            i = i + ptParameterList->ptThis->ptTypeDef->iSizeInBytes;
            ptParameterList = ptParameterList->aptNext[0];
          }
        }
      }
    }
  }

  if (ptParameterList == NULL)
    return TRUE;
  if (ptParameterList->ptRepRec != NULL)
    return TRUE;
  if (i < iLength)
    return FALSE;

  return TRUE;
}


struct TEvent *FindMatchingEvent(
  char *pcBuffer,
  int iLength)
/****************************************************************************/
/* input  : Event : ptr to name of searched event.                          */
/* output : --                                                              */
/* globals: FunctionList                                                    */
/* result : ptr to the stuct describing the searched event,                 */
/*          NULL if searched event does not exsist.                         */
/****************************************************************************/
{
  struct TEventList *ptWalker;

  ptWalker = _ptEventList;

  while (ptWalker != NULL)
  {
    if (ptWalker->ptThis != NULL)
    {
      /* ptWalker->ptThis is event in library                               */
      /* It should be compared with event pcBuffer                          */
      if (BufferMatchesParameterList(pcBuffer,
                                     iLength, ptWalker->ptThis->ptParameters))
      {
        return ptWalker->ptThis;
      }
    }
    ptWalker = ptWalker->ptNext;
  }
  return NULL;
}

/* Checks the parsed event if it is an valid type. */
/* For now it only checks if the name already exists */
int16 CheckEvent( struct TEvent *ptEvent )
{
  int16              iResult;
  struct TEventList *ptWalker;

  iResult = TRUE;
  
  if (NULL != ptEvent)
  {
    ptWalker = _ptEventList;
    
    while ((NULL != ptWalker) && (FALSE == iResult))
    {
      /* For now the only test is to see if the name of the type is already defined */
      if (strcmp( ptWalker->ptThis->pcName, ptEvent->pcName) == 0)
      {
        AddError2("Type %s already exists", ptEvent->pcName);
        iResult = FALSE;
      }
      else
      {
        ptWalker = ptWalker->ptNext;
      }
    }
  }
  return(iResult);
}

struct TEvent *FindEvent(
  char *pcEventName)
/****************************************************************************/
/* input  : Event : ptr to name of searched event.                          */
/* output : --                                                              */
/* globals: FunctionList                                                    */
/* result : ptr to the stuct describing the searched event,                 */
/*          NULL if searched event does not exsist.                         */
/****************************************************************************/
{
  struct TEventList *ptWalker;

  ptWalker = _ptEventList;

  while (ptWalker != NULL)
  {
    if (ptWalker->ptThis != NULL)
    {
      if (ptWalker->ptThis->pcName != NULL)
      {
        if (strcmp(ptWalker->ptThis->pcName, pcEventName) == 0)
        {
          Increment(&(ptWalker->ptThis->iRefCount));
          return ptWalker->ptThis;
        }
        ptWalker = ptWalker->ptNext;
      }
      else
      {
        FatalError2("unexpected NULL pointer while looking for %s",
                    pcEventName);
        return NULL;
      }
    }
    else
    {
      FatalError2("unexpected NULL pointer while looking for %s", pcEventName);
      return NULL;
    }
  }
  return NULL;
}

struct TParameter *FindParameter(
  char *pcName,
  struct TParameterList *ptList)
{
  while ((ptList != NULL)
         && (ptList->ptThis != NULL)
         && (strcmp(ptList->ptThis->pcName, pcName) != 0)
         && (ptList->aptNext[0] != NULL))
  {
    ptList = ptList->aptNext[0];
    while ((ptList != NULL) && (ptList->ptThis == NULL))
    {
      ptList = ptList->aptNext[0];      /* skip reprec records */
    }
  }

  if (ptList == NULL)
    return NULL;
  if (ptList->ptThis == NULL)
    return NULL;

  if (strcmp(ptList->ptThis->pcName, pcName) == 0)
  {
    return ptList->ptThis;
  }
  else
  {
    return NULL;
  }

}

struct TParameter *FindParameterBackwards(
  char *pcName,
  struct TParameterList *ptList)
{
  while ((strcmp(ptList->ptThis->pcName, pcName) != 0)
         && (ptList->ptBackTrack != NULL))
  {
    ptList = ptList->ptBackTrack;
  }

  if (strcmp(ptList->ptThis->pcName, pcName) == 0)
  {
    return ptList->ptThis;
  }
  else
  {
    return NULL;
  }

}


struct TEvent *FindEventOpcode(
  char cId)
/****************************************************************************/
/* input  : cOpcode : opcode of searched event.                             */
/* output : --                                                              */
/* globals: FunctionList                                                    */
/* result : ptr to the stuct describing the searched event,                 */
/*          NULL if searched event does not exsist.                         */
/****************************************************************************/
{

  return NULL;
}

/* Checks the parsed function if it is an valid type. */
/* For now it only checks if the name already exists */
int16 CheckFunction( struct TFunction *ptFunction )
{
  int16                 iResult;
  struct TFunctionList *ptWalker;

  iResult = TRUE;
  
  if (NULL != ptFunction)
  {
    ptWalker = _ptFunctionList;
    
    while ((NULL != ptWalker) && (FALSE == iResult))
    {
      /* For now the only test is to see if the name of the type is already defined */
      if (strcmp( ptWalker->ptThis->pcName, ptFunction->pcName) == 0)
      {
        AddError2("Function %s already exists", ptFunction->pcName);
        iResult = FALSE;
      }
      else
      {
        ptWalker = ptWalker->ptNext;
      }
    }
  }
  return(iResult);
}

struct TFunction *FindFunction(
  char *pcFunctionName)
/****************************************************************************/
/* input  : Function  : Ptr to name of searched function                    */
/* output : --                                                              */
/* globals: _tFunctionList                                                  */
/* result : ptr to the stuct describing the searched function,              */
/*          NULL if searched function does not exsist.                      */
/****************************************************************************/
{
  struct TFunctionList *ptWalker;

  ptWalker = _ptFunctionList;
  while (ptWalker != NULL)
  {
    if (ptWalker->ptThis != NULL)
    {
      if (ptWalker->ptThis->pcName != NULL)
      {
        if (strcmp(ptWalker->ptThis->pcName, pcFunctionName) == 0)
        {
          Increment(&(ptWalker->ptThis->iRefCount));
          return ptWalker->ptThis;
        }
      }
      else
      {
        FatalError2("unexpected NULL pointer while looking for %s",
                    pcFunctionName);
        return NULL;
      }
    }
    else
    {
      FatalError2("unexpected NULL pointer while looking for %s",
                  pcFunctionName);
      return NULL;
    }
    ptWalker = ptWalker->ptNext;
  }
  return NULL;
}



struct TFunction *FindFunctionOpcode(
  int iOpcode)
/****************************************************************************/
/* input  : Opcode : 16-bits opcode of searched function                    */
/* output : --                                                              */
/* globals: _tFunctionList                                                  */
/* result : ptr to the stuct describing the searched function,              */
/*          NULL if searched function does not exsist.                      */
/****************************************************************************/
{

  return NULL;
}

int FindInteraction(
  char *pcToken)
/****************************************************************************/
/* input  : pcToken                                                         */
/* result : enum value, indicating the type of interaction.                 */
/*          0 if searched interaction does not exsist.                      */
/* comment: Only actions which are not events or functions                  */
/*          are detected here.                                              */
/****************************************************************************/
{
  if (strncmp(pcToken, "MTIMER", 5) == 0)
    return MTIMER_ACTION;
  if (strncmp(pcToken, "RTIMER", 5) == 0)
    return RTIMER_ACTION;
  if (strncmp(pcToken, "RMTIMER", 6) == 0)
    return RMTIMER_ACTION;
  if (strncmp(pcToken, "TIMER", 4) == 0)
    return TIMER_ACTION;
  if (strcmp(pcToken, "WAIT") == 0)
    return WAIT_ACTION;
  if (strcmp(pcToken, "CLEAR") == 0)
    return CLEAR_ACTION;
  if (strcmp(pcToken, "TERMINATE") == 0)
    return TERMINATE_ACTION;
  if (strcmp(pcToken, "R_SIG") == 0)
    return R_SIG_ACTION;
  if (strcmp(pcToken, "S_SIG") == 0)
    return S_SIG_ACTION;
  if (strcmp(pcToken, "PRINT") == 0)
    return PRINT_ACTION;
  if (strcmp(pcToken, "COMPARE") == 0)
    return COMPARE_ACTION;
  if (strcmp(pcToken, "PROT_PROTOCOL") == 0)
    return PROT_PROTOCOL_ACTION;
  if (strcmp(pcToken, "ASCII_PROTOCOL") == 0)
    return ASCII_PROTOCOL_ACTION;
  if (strcmp(pcToken, "TCI_PROTOCOL") == 0)
    return TCI_PROTOCOL_ACTION;
  if (strcmp(pcToken, "PROT_WRITEDATA") == 0)
    return PROT_WRITEDATA_ACTION;
  if (strcmp(pcToken, "PROT_READDATA") == 0)
    return PROT_READDATA_ACTION;
  if (strcmp(pcToken, "BREAK") == 0)
    return BREAK_ACTION;
  if (strcmp(pcToken, "END") == 0)
    return END_ACTION;
  if (strcmp(pcToken, "IF_CTS_ENABLED") == 0)
    return CTS_HIGH_ACTION;
  if (strcmp(pcToken, "IF_DSR_ENABLED") == 0)
    return DSR_HIGH_ACTION;
  if (strcmp(pcToken, "IF_CTS_DISABLED") == 0)
    return CTS_LOW_ACTION;
  if (strcmp(pcToken, "IF_DSR_DISABLED") == 0)
    return DSR_LOW_ACTION;
  if (strcmp(pcToken, "ENABLE_DTR") == 0)
    return DTR_HIGH_ACTION;
  if (strcmp(pcToken, "DISABLE_DTR") == 0)
    return DTR_LOW_ACTION;
  if (strcmp(pcToken, "SET_COMM_BREAK") == 0)
    return SET_COMM_BREAK_ACTION;
  if (strcmp(pcToken, "CLEAR_COMM_BREAK") == 0)
    return CLEAR_COMM_BREAK_ACTION;
  if (strcmp(pcToken, "SOCKET_GENERAL_ERROR") == 0)
    return SOCKET_GENERAL_ERROR;
  if (strcmp(pcToken, "SOCKET_SEND_ERROR") == 0)
    return SOCKET_SEND_ERROR;
  if (strcmp(pcToken, "SOCKET_RECEIVE_ERROR") == 0)
    return SOCKET_RECEIVE_ERROR;
  if (strcmp(pcToken, "SOCKET_CONNECT_ERROR") == 0)
    return SOCKET_CONNECT_ERROR;
  if (strcmp(pcToken, "SOCKET_DISCONNECT_ERROR") == 0)
    return SOCKET_DISCONNECT_ERROR;
  if (strcmp(pcToken, "SOCKET_ACCEPT_ERROR") == 0)
    return SOCKET_ACCEPT_ERROR;
  if (strcmp(pcToken, "SOCKET_UNKNOWN_ERROR") == 0)
    return SOCKET_UNKNOWN_ERROR;
  if (strcmp(pcToken, "CONNECTION_LOST") == 0)
    return CONNECTION_LOST;
  if (strcmp(pcToken, "CONNECTION_ESTABLISHED") == 0)
    return CONNECTION_ESTABLISHED;

  /* BREAK is for debug purposes. Not of any use for users.                 */

  if (strcmp(pcToken, "RESCUE") == 0)
    return RESCUE_ACTION;

#ifdef ARITHMETRIC
  if (strcmp(pcToken, "IF") == 0)
    return IF_ACTION;
#endif


  /* If not returned yet, no interaction is found. Return 0.                */
  return 0;
}

struct TStateMachine *FindStateMachine(
  char *pcName)
/****************************************************************************/
/* input  : pcName      : the name of the searched variable                 */
/* output : --                                                              */
/* globals: _ptStateMachineList                                             */
/* result : ptr to the searched state machine record.                       */
/*          NULL if searched state machine does not exist.                  */
/****************************************************************************/
{
  struct TStateMachineList *ptWalker;

  ptWalker = _ptStateMachineList;
  while (ptWalker)
  {
    if ((ptWalker->ptThis) && (strcmp(ptWalker->ptThis->pcName, pcName) == 0))
    {
      /* dont Increment(&(ptWalker->ptThis->iRefCount));                    */
      /* because the statemachine will be duplicated if needed.             */

      return ptWalker->ptThis;
    }
    ptWalker = ptWalker->ptNext;
  }
  return NULL;
}

/* Checks the parsed type if it is an valid type */
int16 CheckType( struct TType *ptType )
{
  int16             iResult;
  struct TTypeList *ptWalker;

  iResult = TRUE;
  
  if (NULL != ptType)
  {
    ptWalker = _ptTypeList;
    
    while ((NULL != ptWalker) && (FALSE == iResult))
    {
      /* For now the only test is to see if the name of the type is already defined */
      if (strcmp( ptWalker->ptThis->pcName, ptType->pcName) == 0)
      {
        AddError2("Type %s already exists", ptType->pcName);
        iResult = FALSE;
      }
      else
      {
        ptWalker = ptWalker->ptNext;
      }
    }
  }
  return(iResult);
}

struct TType *FindType(
  char *pcTypeName)
/****************************************************************************/
/* input  : Type  : Ptr to name of searched type                            */
/* output : --                                                              */
/* globals: TypeList                                                        */
/* result : ptr to the stuct describing the searched type,                  */
/*          NULL if searched type does not exsist.                          */
/****************************************************************************/
{
  struct TTypeList *ptThisOne;

  ptThisOne = _ptTypeList;      /* library entry of type administration.         */

  if (pcTypeName != NULL)
  {
    while (ptThisOne != NULL)
    {
      /* To be failsafe. All types in the adminstration should have a name. */
      if (ptThisOne->ptThis->pcName != NULL)
      {
        /* Is this the record we were looking for?                          */
        if (strcmp(ptThisOne->ptThis->pcName, pcTypeName) == 0)
        {
          /* Yes! return pointer to corresponding single record.            */
          if (strcmp(pcTypeName, "t_BD_ADDR") == 0)
          {
            /* AddWarning1("searched and found a t_BD_ADDR"); */
          }
          Increment(&(ptThisOne->ptThis->iRefCount));
          return ptThisOne->ptThis;
        }
        else
        {
          /* no.. Keep searching in the rest of the administration.         */
          ptThisOne = ptThisOne->ptNext;
        }
      }
    }
    /* Searched till the end of the administration and not found..          */
    return NULL;
  }
  else
  {
    /* Looking with a NULL-pointer? Return a NULL pointer :-)               */
    return NULL;
  }
}


struct TVariable *FindVar(
  char *pcVarName,
  struct TVariableList *ptReference)
/****************************************************************************/
/* input  : pcVarName   : the name of the searched variable                 */
/*          ptReference : The list of variabeles in which must be searched. */
/* output : --                                                              */
/* globals: --                                                              */
/* result : ptr to the searched variable record.                            */
/*          NULL if searched var does not exsist.                           */
/****************************************************************************/
{
  if (pcVarName == NULL)
    return NULL;
  /* that was the easy part.                                                */

  while (ptReference != NULL)
  {
    if (strcmp(ptReference->ptThis->pcName, pcVarName) == 0)
    {
      Increment(&(ptReference->ptThis->iRefCount));
      return ptReference->ptThis;
    }
    ptReference = ptReference->ptNext;
  }
  return NULL;
}

void ShutDownParser(
  void)
{
  if (_ptGlobals != NULL)
  {
    free(_ptGlobals);
    _ptGlobals = NULL;
  }
}


void InitParser(
  void)
{
  delStateMachineList(&_ptStateMachineList);
  delEventList(&_ptEventList);
  delFunctionList(&_ptFunctionList);
  delTypeList(&_ptTypeList);

  /* Allocate memory for warnings, errors and fatals. 10 K must be enough   */
  /* to hold all errors, warnings and fatals.                               */
  /* (Fatals will only be generated, if the parser discovers an internal    */
  /*  error. Users should not be able to generate input files that cause    */
  /*  fatal errors. Fatal Error testing is not possible with black box      */
  /*  testing.)                                                             */
  if (_pcWarningText == NULL)
    _pcWarningText = malloc(102400);
  if (_pcErrorText == NULL)
    _pcErrorText = malloc(102400);
  if (_pcFatalText == NULL)
    _pcFatalText = malloc(102400);
  if (_pcLine == NULL)
    _pcLine = malloc(10240);

  ResetErrorLogging();

  _ptGlobals = (struct TGlobals *) malloc(sizeof(struct TGlobals));
  _ptGlobals->iOverviewMode = 0;
  _ptGlobals->iLogMode = 0;
  _ptGlobals->iDebugLevel = 0;

}

void ResetErrorLogging(
  void)
{
  (*_pcWarningText) = 0;
  (*_pcErrorText) = 0;
  (*_pcFatalText) = 0;
  (*_pcLine) = 0;

  iWarningCounter = 0;
  iErrorCounter = 0;
  iFatalCounter = 0;

}

void LogMsg(
  char *pcMsg)
{
  char      pcLine[1024];

  sprintf(pcLine, "--> %s\n\n", pcMsg);
  Error(pcLine);

  iErrorCounter++;
}


void LogError(
  char *pcError)
{
  char      pcLine[1024];
  char     *pcIncluded;

  sprintf(pcLine, "Error reading file %s, line %d\n",
          CurrentFile(), CurrentLine());
  Error(pcLine);

  pcIncluded = FileIncludedBy();

  if ((*pcIncluded) != 0)
  {
    strcat(pcIncluded, "\n");
    Error(pcIncluded);
  }
  free(pcIncluded);

  sprintf(pcLine, "--> %s\n\n", pcError);
  Error(pcLine);

  iErrorCounter++;
}



void LogWarning(
  char *pcWarning)
{
  char      pcLine[1024];
  char     *pcIncluded;

  sprintf(pcLine, "Warning reading file %s, line %d\n",
          CurrentFile(), CurrentLine());
  Warn(pcLine);

  pcIncluded = FileIncludedBy();

  if ((*pcIncluded) != 0)
  {
    sprintf(pcLine, "Included by %s\n", pcIncluded);
    Warn(pcLine);
  }
  free(pcIncluded);

  sprintf(pcLine, "--> %s\n\n", pcWarning);
  Warn(pcLine);

  iWarningCounter++;
}



struct TValue *newValue(
  void)
{
  struct TValue *n;
  n = (struct TValue *) malloc(sizeof(struct TValue));
  n->pcValue    = NULL;
  n->pcFileName = NULL;
  n->pcFileMode = NULL;
  n->iRefCount  = 1;
  n->uiLength   = 0; 
  n->bFixed     = FALSE;
  return n;
}

struct TValueDef *newValueDef(
  void)
{
  struct TValueDef *n;
  n = (struct TValueDef *) malloc(sizeof(struct TValueDef));
  n->pcMask = NULL;
  n->pcValue = NULL;
  n->pcDescription = NULL;
  n->iRefCount = 1;
  return n;
}

struct TValueDefList *newValueDefList(
  void)
{
  struct TValueDefList *n;
  n = (struct TValueDefList *) malloc(sizeof(struct TValueDefList));
  n->ptThis = NULL;
  n->ptNext = NULL;
  n->iRefCount = 1;
  return n;
}

struct TType *newType(
  void)
{
  struct TType *n;
  n = (struct TType *) malloc(sizeof(struct TType));
  n->pcName = NULL;
  n->iSizeInBytes = 0;
  n->iLessAllowed = 0;
  n->pcUpperLimit = NULL;
  n->pcLowerLimit = NULL;
  n->pcDecimalDescription = NULL;
  n->iKind = 0;
  n->fTimeScaling = (float) 1.00;       /* suprise! */
  n->ptDefinition = NULL;
  n->iEndianess = 0;
  n->iRefCount = 1;
  return n;
}

struct TTypeList *newTypeList(
  void)
{
  struct TTypeList *n;
  n = (struct TTypeList *) malloc(sizeof(struct TTypeList));
  n->ptThis = NULL;
  n->ptNext = &PredefTypes;
  n->iRefCount = 1;
  return n;
}

struct TParameter *newParameter(
  void)
{
  struct TParameter *n;
  n = (struct TParameter *) malloc(sizeof(struct TParameter));
  n->pcName = NULL;
  n->iArray = 0;
  n->iOffset = 0;
  n->ptTypeDef = NULL;
  n->ptValue = NULL;
  n->iRefCount = 1;
  n->pcLengthIndicator = NULL;
  n->pcLengthFromField = NULL;
  n->pcLengthToField = NULL;
  n->ptLengthFromField = NULL;
  n->ptLengthToField = NULL;
  n->ptLengthIndicator = NULL;

  return n;
}

struct TParameterList *newParameterList(
  void)
{
  int       i;
  struct TParameterList *n;
  n = (struct TParameterList *) malloc(sizeof(struct TParameterList));
  n->ptThis = NULL;
  n->iFollowedByUnion = FALSE;
  for (i = 0; i < UNIONARRAYSIZE + 1; i++)
    n->aptNext[i] = NULL;
  n->iRefCount = 1;
  n->ptRepRec = NULL;
  n->ptBackTrack = NULL;
  return n;
}

struct TRepetition *newRepetition(
  void)
{
  struct TRepetition *n;
  n = (struct TRepetition *) malloc(sizeof(struct TRepetition));
  n->ptParameter = NULL;
  return n;
}


struct TFunction *newFunction(
  void)
{
  struct TFunction *n;
  int       iTotalTypeRefs;

  iTotalTypeRefs = CountTypeRefs(_ptTypeList);

  n = (struct TFunction *) malloc(sizeof(struct TFunction));
  n->pcName = NULL;
  n->iSizeOfSendArrayParameters = 0;
  n->iSizeOfReceiveArrayParameters = 0;
  n->ptSendParameters = NULL;
  n->ptReceiveParameters = NULL;
  n->iRefCount = 1;
  return n;
}

struct TFunctionList *newFunctionList(
  void)
{
  struct TFunctionList *n;
  n = (struct TFunctionList *) malloc(sizeof(struct TFunctionList));
  n->ptThis = NULL;
  n->ptNext = NULL;
  n->iRefCount = 1;
  return n;
}

struct TEvent *newEvent(
  void)
{
  struct TEvent *n;
  n = (struct TEvent *) malloc(sizeof(struct TEvent));
  n->pcName = NULL;
  n->iSizeOfArrayParameters = 0;
  n->ptParameters = NULL;
  n->iRefCount = 1;
  return n;
}

struct TEventList *newEventList(
  void)
{
  struct TEventList *n;
  n = (struct TEventList *) malloc(sizeof(struct TEventList));
  n->ptThis = NULL;
  n->ptNext = NULL;
  n->iRefCount = 1;
  return n;
}

struct TVariable *newVariable(
  struct TType *ptThisType)
{
  struct TVariable *n;
  n = (struct TVariable *) malloc(sizeof(struct TVariable));
  n->pcName = NULL;
  n->ptItsType = ptThisType;
  n->ptValue = NULL;
  if (ptThisType != NULL)
  {
    n->ptValue = newValue();
    n->ptValue->pcValue = (char *) malloc((ptThisType->iSizeInBytes * 2) + 3);
    n->ptValue->pcValue[0] = 0;
    n->ptValue->pcFileName = NULL;
    Increment(&(ptThisType->iRefCount));
  }
  n->iRefCount = 1;
  return n;
}

struct TVariableList *newVariableList(
  void)
{
  struct TVariableList *n;
  n = (struct TVariableList *) malloc(sizeof(struct TVariableList));
  n->ptThis = NULL;
  n->ptNext = NULL;
  n->iRefCount = 1;
  return n;
}


struct TAction *newAction(
  void)
{
  struct TAction *n;
  n = (struct TAction *) malloc(sizeof(struct TAction));
  n->lActionCounter = 0;
  n->iActionType = 0;
  n->iNrOfReceptions = 0;
  n->ptFunction = NULL;
  n->ptEvent = NULL;
  n->ptVarList = NULL;
  n->ptExpression = NULL;
  n->ptStateMachine = NULL;
  n->iSourceLineNumber = 0;
  n->iRefCount = 1;
  return n;
}

struct TActionList *newActionList(
  void)
{
  struct TActionList *n;
  n = (struct TActionList *) malloc(sizeof(struct TActionList));
  n->ptThis = NULL;
  n->ptNext = NULL;
  n->iRefCount = 1;
  return n;
}

struct TTransition *newTransition(
  void)
{
  struct TTransition *n;
  n = (struct TTransition *) malloc(sizeof(struct TTransition));
  n->iInstantNextState = FALSE;
  n->pcNextStateName = NULL;
  n->ptNextState = NULL;
  n->ptActionList = NULL;
  n->lTransitionCounter = 0;
  n->iAlternative = 0;
  n->iRefCount = 1;
  return n;
}

struct TTransitionList *newTransitionList(
  void)
{
  struct TTransitionList *n;
  n = (struct TTransitionList *) malloc(sizeof(struct TTransitionList));
  n->ptThis = NULL;
  n->ptNext = NULL;
  n->iRefCount = 1;
  return n;
}

struct TState *newState(
  void)
{
  struct TState *n;
  n = (struct TState *) malloc(sizeof(struct TState));
  n->pcName = NULL;
  n->ptTransitionList = NULL;
  n->iRefCount = 1;
  return n;
}

struct TStateList *newStateList(
  void)
{
  struct TStateList *n;
  n = (struct TStateList *) malloc(sizeof(struct TStateList));
  n->ptThis = NULL;
  n->ptNext = NULL;
  n->iRefCount = 1;
  return n;
}

struct TStateMachine *newStateMachine(
  void)
{
  struct TStateMachine *n;
  n = (struct TStateMachine *) malloc(sizeof(struct TStateMachine));
  n->pcName = NULL;
  n->pcDeviceName = NULL;
  n->pcRescueStateName = NULL;
  n->ptParameters = NULL;
  n->ptLocalVars = NULL;
  n->ptStates = NULL;
  n->ptCurrentState = NULL;
  n->ptRescueState = NULL;
  n->ptActionInProgress = NULL;
  n->ptRunningTimer = NULL;
  n->ptStateInTimer = NULL;
  n->ptRunningTimer = NULL;
  n->ptMaster = NULL;
  n->iPending = 0;
  n->iHidden = FALSE;
  n->iIOChannel = -1;           /* -1 -> undefined */
  n->iRefCount = 1;
  n->iAlternative = 0;
  return n;
}

struct TStateMachineList *newStateMachineList(
  void)
{
  struct TStateMachineList *n;
  n = (struct TStateMachineList *) malloc(sizeof(struct TStateMachineList));
  n->ptThis = NULL;
  n->ptNext = NULL;
  n->iRefCount = 1;
  return n;
}

struct TData *newData(
  void)
{
  struct TData *n;

  n = (struct TData *) malloc(sizeof(struct TData));
  n->pcName = NULL;
  n->pcValue = NULL;
  n->iRefCount = 1;
  n->bFixed = FALSE;
  return n;
}

struct TDataList *newDataList(
  void)
{
  struct TDataList *n;

  n = (struct TDataList *) malloc(sizeof(struct TDataList));
  n->ptThis = NULL;
  n->ptNext = NULL;
  n->iRefCount = 1;
  return n;
}

struct TBoard *newBoard(
  void)
{
  struct TBoard *n;

  n = (struct TBoard *) malloc(sizeof(struct TBoard));
  n->pcName = NULL;
  n->ptStateMachines = NULL;
  n->iRefCount = 1;
  return n;
}

struct TBoardList *newBoardList(
  void)
{
  struct TBoardList *n;

  n = (struct TBoardList *) malloc(sizeof(struct TBoardList));
  n->ptThis = NULL;
  n->ptNext = NULL;
  n->iRefCount = 1;
  return n;
}

struct TIO *newIO(
  void)
{
  struct TIO *n;

  n = (struct TIO *) malloc(sizeof(struct TIO));

  n->iConfigured = FALSE;

  return n;
}

struct TIOList *newIOList(
  void)
{
  struct TIOList *n;

  n = (struct TIOList *) malloc(sizeof(struct TIOList));
  n->ptThis = NULL;
  n->ptNext = NULL;
  n->iRefCount = 1;
  return n;
}


void CloseObsoleteIO(
  struct TIOList *ptOldIOList,
  struct TIOList *ptNewIOList)
/****************************************************************************/
/* This function is needed because of a small design bug in BSE. When       */
/* loading a .prot file containing a pipe, this file is created, and a pipe */
/* client can be started. When loading another prot file (or the same) also */
/* containing the pipe, the pipe would have been closed and reopened,       */
/* thereby loosing the connection with the pipe client. As a workaround the */
/* old io channels are not closed before the new ones are opened. New       */
/* channels having the same name as channels in the old list are not closed */
/* and reopened. Channels not mentioned in the new list are closed.         */
/****************************************************************************/
{
  struct TIOList *ptOldWalker;
  struct TIOList *ptNewWalker;

  ptNewWalker = ptNewIOList;

  /* First we mark all old channels as not needed.                          */
  ptOldWalker = ptOldIOList;
  while (ptOldWalker != NULL)
  {
    if (ptOldWalker->ptThis != NULL)
    {
      ptOldWalker->ptThis->iNeeded = FALSE;
    }
    ptOldWalker = ptOldWalker->ptNext;
  }

  /* next we mark all old needed channels as needed.                        */
  while ((ptNewWalker != NULL) && (ptNewWalker->ptThis != NULL))
  {
    ptOldWalker = ptOldIOList;
    while ((ptOldWalker != NULL)
           && (ptOldWalker->ptThis != NULL)
           &&
           ((strcmp(ptOldWalker->ptThis->pcName, ptNewWalker->ptThis->pcName) !=
             0) ||
            (strcmp
             (ptOldWalker->ptThis->pcIOPortName,
              ptNewWalker->ptThis->pcIOPortName) != 0)))
    {
      /* This one is not needed. Don't mark it as readed, skip it.          */
      ptOldWalker = ptOldWalker->ptNext;
    }

    if ((ptOldWalker != NULL) && (ptOldWalker->ptThis != NULL))
    {
      /* then strcmp(pcName,pcName) == 0                                    */
      /*  and strcmp(pcIOPortName,pcIOPortName)==0                          */
      /*                                                                    */
      /* new io channel found in old list. Mark it as needed. Mark the      */
      /* channel in the new list as already being configured. Use the old   */
      /* channel number because the driver still uses the old channel num-  */
      /* ber.                                                               */

      ptOldWalker->ptThis->iNeeded = TRUE;
      ptNewWalker->ptThis->iConfigured = TRUE;
      ptNewWalker->ptThis->iNumber = ptOldWalker->ptThis->iNumber;
    }
    ptNewWalker = ptNewWalker->ptNext;
  }

  /* Now close all not needed channels                                      */
  ptOldWalker = ptOldIOList;
  while (ptOldWalker != NULL)
  {
    if (ptOldWalker->ptThis != NULL)
    {
      if (ptOldWalker->ptThis->iNeeded == FALSE)
      {
        BSED_CloseDevice(ptOldWalker->ptThis->iNumber);
      }
      ptOldWalker->ptThis->iNumber = -1;
    }
    ptOldWalker = ptOldWalker->ptNext;
  }

  /* Ready.                                                                 */
}


void LinkAllBranchesToOne(
  struct TParameterList *ptRoot,
  struct TParameterList *ptOne)
/****************************************************************************/
/* This is a recursive function that links all endpoints in the TParameter- */
/* List tree to one single new entry.                                       */
/****************************************************************************/
{
  int       i;

  if (ptRoot->iFollowedByUnion == TRUE)
  {
    for (i = 0; i < UNIONARRAYSIZE + 1; i++)
    {
      /* link all _used_ entries including else branch to the new ptOne.    */
      if (ptRoot->aptNext[i] != NULL)
      {
        LinkAllBranchesToOne(ptRoot->aptNext[i], ptOne);
      }
    }
  }
  else
  {
    /* No following union. This can be an endpoint, but is not nessecary an */
    /* endpoint. Link to ptOne if it is an endpoint, walk into it if it is  */
    /* not an endpoint.                                                     */
    if (ptRoot->aptNext[0] == NULL)
    {
      if (ptRoot != ptOne)
      {
        ptRoot->aptNext[0] = ptOne;
        Increment(&(ptOne->iRefCount));
      }
    }
    else
    {
      LinkAllBranchesToOne(ptRoot->aptNext[0], ptOne);
    }
  }
}

/****************************************************************************/
/*                                                                          */
/* The following functions will not contain any comments. All Write@@@      */
/* functions print the structure to the first pcBuffer parameter. This      */
/* buffer is intended to be printed in an overview window for the user.     */
/*                                                                          */
/****************************************************************************/

void WriteBoardList(
  char *pcBuffer,
  struct TBoardList *ptBoardList)
{

  if (_ptGlobals->iOverviewMode == BSEK_STATEMACHINES_HEADERS)
  {
    WriteDataList(pcBuffer);
  }

  while (ptBoardList != NULL)
  {
    WriteBoard(pcBuffer, ptBoardList->ptThis, _ptGlobals->iOverviewMode);
    ptBoardList = ptBoardList->ptNext;
  }

}

void WriteBoard(
  char *pcBuffer,
  struct TBoard *ptBoard,
  int iMode)
{
  strcat(pcBuffer, ptBoard->pcName);
  strcat(pcBuffer, ": \r\n");
  WriteStateMachineList(pcBuffer, ptBoard->ptStateMachines, iMode);
  strcat(pcBuffer, "\r\n\r\n");
}



void WriteDataList(
  char *pcBuffer)
{
  struct TDataList *ptDataListWalker;
  struct TData *pt;
  char      Buffer[250];

  ptDataListWalker = _ptDataList;

  while (ptDataListWalker != NULL)
  {
    if (ptDataListWalker->ptThis != NULL)
    {
      pt = ptDataListWalker->ptThis;
      if (pt->pcName == NULL)
      {
        sprintf(Buffer, "<unnamed databuffer> ");
      }
      else
      {
        sprintf(Buffer, "%20s ", pt->pcName);
      }

      if (pt->pcValue == NULL)
      {
        strcat(pcBuffer, "<empty> \r\n");
      }
      else
      {
        strcat(Buffer, " = '");
        strcat(pcBuffer, Buffer);
        strncpy(Buffer, pt->pcValue, 20);
        strcat(Buffer, "'\r\n");
        strcat(pcBuffer, Buffer);
      }
    }
    ptDataListWalker = ptDataListWalker->ptNext;
  }

  strcat(pcBuffer, "\r\n\r\n");

}

void WriteStateMachineList(
  char *pBuffer,
  struct TStateMachineList *ptStateMachineList,
  int iMode)
{
  while (ptStateMachineList != NULL)
  {
    if (iMode == BSEK_STATEMACHINES_FULL)
    {
      WriteStateMachine(pBuffer, ptStateMachineList->ptThis);
    }
    else if (iMode == BSEK_STATEMACHINES_HEADERS)
    {
      WriteStateMachineHeader(iMode, pBuffer, ptStateMachineList->ptThis);
    }
    ptStateMachineList = ptStateMachineList->ptNext;
  }
}



void WriteStateMachine(
  char *pcBuffer,
  struct TStateMachine *ptStateMachine)
{
  if (ptStateMachine == NULL)
    return;

  if (ptStateMachine->iHidden == FALSE)
  {

    strcat(pcBuffer, ptStateMachine->pcName);

    if (ptStateMachine->ptParameters != NULL)
    {
      strcat(pcBuffer, "( ");
      WriteVarList(pcBuffer, ptStateMachine->ptParameters);
      strcat(pcBuffer, " )");
    }
    strcat(pcBuffer, " {\r\n");
    if (ptStateMachine->ptLocalVars != NULL)
    {
      strcat(pcBuffer, "VAR ");
      WriteVarList(pcBuffer, ptStateMachine->ptLocalVars);
      strcat(pcBuffer, ".\r\n");
    }
    if (ptStateMachine->ptRescueState != NULL)
    {
      strcat(pcBuffer, "RESCUE ");
      strcat(pcBuffer, ptStateMachine->ptRescueState->pcName);
      strcat(pcBuffer, ".\r\n");
    }
    WriteStateList(pcBuffer, ptStateMachine->ptStates);
    if (ptStateMachine->ptCurrentState)
    {
      strcat(pcBuffer, "Current state : ");
      strcat(pcBuffer, ptStateMachine->ptCurrentState->pcName);
      strcat(pcBuffer, "\r\n");
    }
    else
    {
      strcat(pcBuffer, "Current state : ***\r\n");
    }

    strcat(pcBuffer, "}\r\n\r\n");
  }
}

void WriteStateMachineHeader(
  int16 iMode,
  char *pcBuffer,
  struct TStateMachine *ptStateMachine)
{

  if (ptStateMachine == NULL)
    return;


  strcat(pcBuffer, ptStateMachine->pcName);

  if (ptStateMachine->iPending == RUNNING_SM)
  {
    if (ptStateMachine->ptParameters != NULL)
    {
      strcat(pcBuffer, "( ");
      if (iMode == BSEK_STATEMACHINES_HEADERS)
      {
        WriteVarListGlobal(pcBuffer, ptStateMachine->ptParameters);
      }
      else
      {
        WriteVarList(pcBuffer, ptStateMachine->ptParameters);
      }
      strcat(pcBuffer, " )");
    }
    if (iMode == BSEK_STATEMACHINES_HEADERS)
    {
      if (ptStateMachine->ptCurrentState == NULL)
      {
        strcat(pcBuffer, " -o-");
      }
      else
      {
        strcat(pcBuffer, " ");
        strcat(pcBuffer, ptStateMachine->ptCurrentState->pcName);
      }
    }
    strcat(pcBuffer, "\r\n");
  }
  else
  {
    switch (ptStateMachine->iPending)
    {
      case RUNNING_SM:
        strcat(pcBuffer, " RUNNING \r\n");
        break;
      case SLEEPING:
        strcat(pcBuffer, " SLEEPING \r\n");
        break;
      case JUST_BORN:
        strcat(pcBuffer, " BORN \r\n");
        break;
      case WAKING_UP:
        strcat(pcBuffer, " WAKING UP\r\n");
        break;
      case ENDING:
        strcat(pcBuffer, " ENDING\r\n");
        break;
      default:
        strcat(pcBuffer, "*** internal error ***\r\n");
        break;
    }
  }
}



void WriteStateList(
  char *pcBuffer,
  struct TStateList *ptStateList)
{
  while (ptStateList != NULL)
  {
    WriteState(pcBuffer, ptStateList->ptThis);
    ptStateList = ptStateList->ptNext;
  }
}


void WriteVarList(
  char *pcBuffer,
  struct TVariableList *ptVarList)
{
  while (ptVarList != NULL)
  {
    WriteVar(pcBuffer, ptVarList->ptThis);
    if (ptVarList->ptNext)
    {
      strcat(pcBuffer, ", ");
    }
    ptVarList = ptVarList->ptNext;
  }
}

void WriteVarListGlobal(
  char *pcBuffer,
  struct TVariableList *ptVarList)
{
  while (ptVarList != NULL)
  {
    WriteVarGlobal(pcBuffer, ptVarList->ptThis);
    if (ptVarList->ptNext)
    {
      strcat(pcBuffer, ", ");
    }
    ptVarList = ptVarList->ptNext;
  }
}


void WriteVarGlobal(
  char *pcBuffer,
  struct TVariable *ptVar)
{
  struct TVariableList *ptWalker = _ptGlobalVars;
  char      b[20];

  /* First find the variable in the _ptGlobalVars list that refers to the  */
  /* same value record as the variable to be printed.                      */

  while ((ptWalker != NULL)
         && (ptWalker->ptThis != NULL)
         && (ptWalker->ptThis->ptValue != ptVar->ptValue))
  {
    ptWalker = ptWalker->ptNext;
  }

  /* If found, print the name of the global variable, together with the    */
  /* value. The value of the global variable is of course the same...      */
  if ((ptWalker != NULL)
      && (ptWalker->ptThis != NULL)
      && (ptWalker->ptThis->ptValue == ptVar->ptValue))
  {
    strcat(pcBuffer, ptWalker->ptThis->pcName);
    strcat(pcBuffer, " ");
  }
  else
  {                             /* If not found, just print the value. This can happen when state-     */
    /* machines are invoked from other statemachines, using local var's.   */

  }

  if ((ptVar->ptValue != NULL) && (ptVar->ptValue->pcValue != NULL))
  {
    if ((ptVar->ptItsType != NULL) && (ptVar->ptItsType->iKind == ASCIIKIND))
    {
      asciicat(pcBuffer, ptVar->ptValue->pcValue);
    }
    else
    {
      strcat(pcBuffer, ptVar->ptValue->pcValue);
    }

    /* If DebugLevel is one or more, print also the address of the       */
    /* value to be printed.                                              */
    if (_ptGlobals->iDebugLevel > 0)
    {
      sprintf(b, " (0x%8p)", (void*)&(ptVar->ptValue->pcValue));
      strcat(pcBuffer, b);
    }
  }
}

void WriteVar(
  char *pcBuffer,
  struct TVariable *ptVar)
{
  char      b[250];
  int16     bDataBuffer;

  if (ptVar->pcName != NULL)
  {
    strcat(pcBuffer, ptVar->pcName);
  }

  /* In the next if statement the type is checked on availability. This    */
  /* is actually only the case is the variable is a data variable. A data  */
  /* variable has a different structure (for historic reasons) and cannot  */
  /* be printed here.                                                      */

  bDataBuffer = (ptVar->ptItsType == _ptDataType);

  if (!bDataBuffer)
  {
    if ((ptVar->pcName != NULL) && (ptVar->pcName[0] == '"'))
    {
      /* skip the value of string parameters. The value is actually a hex  */
      /* dump of the string, and this value is never interesting for the   */
      /* user.                                                             */
    }
    else if ((ptVar->ptValue != NULL)
             && (ptVar->ptValue->pcValue != NULL)
             && (ptVar->ptValue->pcValue[0] != 0) )
    {         
      if ((ptVar->ptItsType != NULL)  
           &&(ptVar->ptItsType->iKind != FILEDATAKIND) )
      {
        strcat(pcBuffer, " ");
  
        if ((ptVar->ptItsType != NULL) && (ptVar->ptItsType->iKind == ASCIIKIND))
        {
          strncpy( b, ptVar->ptValue->pcValue, 240 );
          asciicat(pcBuffer, b);
          if (strlen( ptVar->ptValue->pcValue) > 240)
          {
            strcat(pcBuffer,"...");
          }
        }
        else
        {
          strncpy( b, ptVar->ptValue->pcValue, 240 );
          strcat(pcBuffer, b);
          if (strlen( ptVar->ptValue->pcValue) > 240)
          {
            strcat(pcBuffer,"...");
          }
        }
  
  
        /* If DebugLevel is one or more, print also the address of the        */
        /* value to be printed.                                               */
        if (_ptGlobals->iDebugLevel > 0)
        {
          sprintf(b, " (0x%8p)", (void*)&(ptVar->ptValue->pcValue));
          strcat(pcBuffer, b);
        }
      }
      else if (ptVar->ptItsType == NULL) 
      {
        strncpy( b, ptVar->ptValue->pcValue, 240 );
        strcat(pcBuffer, b);
        if (strlen( ptVar->ptValue->pcValue) > 240)
        {
          strcat(pcBuffer,"...");
        }
      }
    }
  }
  strcat(pcBuffer, " ");
}

void WriteTransition(
  char *pcBuffer,
  struct TTransition *ptTransition)
{
  struct TActionList *ptActionList;
  char      pc[64];

  sprintf(pc, " %3ld ", ptTransition->lTransitionCounter);
  strcat(pcBuffer, pc);

  if (ptTransition->iAlternative != 0)
    strcat(pcBuffer, ":");

  ptActionList = ptTransition->ptActionList;
  while (ptActionList != NULL)
  {
    WriteAction(pcBuffer, ptActionList->ptThis);
    ptActionList = ptActionList->ptNext;
    if (ptActionList != NULL)
    {
      strcat(pcBuffer, "\r\n     ");
    }
  }
  if (ptTransition->pcNextStateName == NULL)
  {
    strcat(pcBuffer, ".\r\n");
  }
  else if (ptTransition->pcNextStateName[0] == '\0')
  {
    strcat(pcBuffer, ",\r\n");
  }
  else
  {
    sprintf(pc, "; %s.\r\n", ptTransition->pcNextStateName);
    strcat(pcBuffer, pc);
  }
}


void WriteState(
  char *pcBuffer,
  struct TState *ptState)
{

  struct TTransitionList *ptTransitionList;
  char      pc[64];

  sprintf(pc, " %s: \r\n", ptState->pcName);
  strcat(pcBuffer, pc);

  ptTransitionList = ptState->ptTransitionList;

  while (ptTransitionList != NULL)
  {
    WriteTransition(pcBuffer, ptTransitionList->ptThis);
    ptTransitionList = ptTransitionList->ptNext;
  }
}


void WriteAction(
  char *pcBuffer,
  struct TAction *ptAction)
{
  char      pc[102400];           /* used to print integers via sprintf, and other intermediate results */

  if (ptAction->lActionCounter > 0)
  {
    sprintf(pc, "%3ld ", ptAction->lActionCounter);
    strcat(pcBuffer, pc);
  }

  switch (ptAction->iActionType)
  {
    case TIMER_ACTION:
      strcat(pcBuffer, "TIMER");
      break;
    case MTIMER_ACTION:
      strcat(pcBuffer, "MTIMER");
      break;
    case RTIMER_ACTION:
      strcat(pcBuffer, "RTIMER");
      break;
    case RMTIMER_ACTION:
      strcat(pcBuffer, "RMTIMER");
      break;
    case CLEAR_ACTION:
      strcat(pcBuffer, "CLEAR");
      break;
    case WAIT_ACTION:
      strcat(pcBuffer, "WAIT");
      break;
    case TERMINATE_ACTION:
      strcat(pcBuffer, "TERMINATE");
      break;
    case R_SIG_ACTION:
      strcat(pcBuffer, "R_SIG");
      break;
    case S_SIG_ACTION:
      strcat(pcBuffer, "S_SIG");
      break;
    case PRINT_ACTION:
      strcat(pcBuffer, "PRINT");
      break;
    case COMPARE_ACTION:
      strcat(pcBuffer, "COMPARE");
      break;
    case PROT_PROTOCOL_ACTION:
      strcat(pcBuffer, "PROT_PROTOCOL");
      break;
    case ASCII_PROTOCOL_ACTION:
      strcat(pcBuffer, "ASCII_PROTOCOL");
      break;
    case TCI_PROTOCOL_ACTION:
      strcat(pcBuffer, "TCI_PROTOCOL");
      break;
    case PROT_WRITEDATA_ACTION:
      strcat(pcBuffer, "PROT_WRITEDATA");
      break;
    case PROT_READDATA_ACTION:
      strcat(pcBuffer, "PROT_READDATA");
      break;
    case RESCUE_ACTION:
      strcat(pcBuffer, "RESCUE");
      break;
    case END_ACTION:
      strcat(pcBuffer, "END");
      break;
    case CTS_HIGH_ACTION:
      strcat(pcBuffer, "IF_CTS_ENABLED");
      break;
    case CTS_LOW_ACTION:
      strcat(pcBuffer, "IF_CTS_DISABLED");
      break;
    case DSR_HIGH_ACTION:
      strcat(pcBuffer, "IF_DSR_ENABLED");
      break;
    case DSR_LOW_ACTION:
      strcat(pcBuffer, "IF_DSR_DISABLED");
      break;
    case DTR_HIGH_ACTION:
      strcat(pcBuffer, "ENABLE_DTR");
      break;
    case DTR_LOW_ACTION:
      strcat(pcBuffer, "DISABLE_DTR");
      break;
    case SET_COMM_BREAK_ACTION:
      strcat(pcBuffer, "SET_COMM_BREAK");
      break;
    case CLEAR_COMM_BREAK_ACTION:
      strcat(pcBuffer, "CLEAR_COMM_BREAK");
      break;
    case SOCKET_GENERAL_ERROR:
      strcat(pcBuffer, "SOCKET_GENERAL_ERROR");
      break;
    case SOCKET_SEND_ERROR:
      strcat(pcBuffer, "SOCKET_SEND_ERROR");
      break;
    case SOCKET_RECEIVE_ERROR:
      strcat(pcBuffer, "SOCKET_RECEIVE_ERROR");
      break;
    case SOCKET_CONNECT_ERROR:
      strcat(pcBuffer, "SOCKET_CONNECT_ERROR");
      break;
    case SOCKET_DISCONNECT_ERROR:
      strcat(pcBuffer, "SOCKET_DISCONNECT_ERROR");
      break;
    case SOCKET_ACCEPT_ERROR:
      strcat(pcBuffer, "SOCKET_ACCEPT_ERROR");
      break;
    case SOCKET_UNKNOWN_ERROR:
      strcat(pcBuffer, "SOCKET_UNKNOWN_ERROR");
      break;
    case CONNECTION_LOST:
      strcat(pcBuffer, "CONNECTION_LOST");
      break;
    case CONNECTION_ESTABLISHED:
      strcat(pcBuffer, "CONNECTION_ESTABLISHED");
      break;


    case FUNCTION_ACTION:
      if ((ptAction->ptFunction != NULL)
          && (ptAction->ptFunction->pcName != NULL))
      {
        strcat(pcBuffer, ptAction->ptFunction->pcName);
      }
      else
      {
        strcat(pcBuffer, " -(**)- ");
      }
      break;

    case EVENT_ACTION:
      if ((ptAction->ptEvent != NULL) && (ptAction->ptEvent->pcName != NULL))
      {
        strcat(pcBuffer, ptAction->ptEvent->pcName);
      }
      else
      {
        strcat(pcBuffer, " -(**)- ");
      }
      break;
    case ARITHMETRIC_ACTION:
      WriteExpression(pcBuffer, ptAction->ptExpression, 0);
      break;
    case IF_ACTION:
      strcat(pcBuffer, "IF ( ");
      WriteExpression(pcBuffer, ptAction->ptExpression, 0);
      strcat(pcBuffer, ")");
      break;
    case STATEMACHINE_ACTION:
      if ((ptAction->ptStateMachine != NULL)
          && (ptAction->ptStateMachine->pcName != NULL))
      {
        strcat(pcBuffer, ptAction->ptStateMachine->pcName);
      }
      else
      {
        strcat(pcBuffer, " =(**)= ");
      }
      break;
    default:
      strcat(pcBuffer, "***internal error***");
      break;
  }

  if (ptAction->ptVarList)
  {
    strcpy(pc, "(");
    WriteVarList(pc, ptAction->ptVarList);
    strcat(pc, ")");
    strcat(pcBuffer, pc);
  }

  if (ptAction->iNrOfReceptions > 0)
  {
    sprintf(pc, " [%d]", ptAction->iNrOfReceptions);
    strcat(pcBuffer, pc);
  }
}
