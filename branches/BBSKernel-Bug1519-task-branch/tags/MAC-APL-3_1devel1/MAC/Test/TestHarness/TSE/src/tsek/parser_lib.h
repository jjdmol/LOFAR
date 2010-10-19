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


#define SUBFIELD(x)     (((x)>>10) & 0x3F)
#define OCF(x)          ((x)&0x3FF)


struct TValue *newValue(
  void);
struct TValueDef *newValueDef(
  void);
struct TValueDefList *newValueDefList(
  void);
struct TType *newType(
  void);
struct TTypeList *newTypeList(
  void);
struct TParameter *newParameter(
  void);
struct TParameterList *newParameterList(
  void);
struct TRepetition *newRepetition(
  void);
struct TFunction *newFunction(
  void);
struct TFunctionList *newFunctionList(
  void);
struct TEvent *newEvent(
  void);
struct TEventList *newEventList(
  void);
struct TVariable *newVariable(
  struct TType *ptThisType);
struct TVariableList *newVariableList(
  void);
struct TAction *newAction(
  void);
struct TActionList *newActionList(
  void);
struct TTransition *newTransition(
  void);
struct TTransitionList *newTransitionList(
  void);
struct TState *newState(
  void);
struct TStateList *newStateList(
  void);
struct TStateMachine *newStateMachine(
  void);
struct TStateMachineList *newStateMachineList(
  void);
struct TData *newData(
  void);
struct TDataList *newDataList(
  void);
struct TTimerRecord *newTimerRecord(
  void);
struct TTimerRecordList *newTimerRecordList(
  void);
struct TBoard *newBoard(
  void);
struct TBoardList *newBoardList(
  void);
struct TIO *newIO(
  void);
struct TIOList *newIOList(
  void);



void      delValue(
  struct TValue **ppt);
void      delValueDef(
  struct TValueDef **ppt);
void      delValueDefList(
  struct TValueDefList **ppt);
void      delType(
  struct TType **ppt);
void      delTypeList(
  struct TTypeList **ppt);
void      delParameter(
  struct TParameter **ppt);
void      delParameterList(
  struct TParameterList **ppt);
void      delFunction(
  struct TFunction **ppt);
void      delFunctionList(
  struct TFunctionList **ppt);
void      delEvent(
  struct TEvent **ppt);
void      delEventList(
  struct TEventList **ppt);
void      delVariable(
  struct TVariable **ppt);
void      delVariableList(
  struct TVariableList **ppt);
void      delAction(
  struct TAction **ppt);
void      delActionList(
  struct TActionList **ppt);
void      delState(
  struct TState **ppt);
void      delStateList(
  struct TStateList **ppt);
void      delStateMachine(
  struct TStateMachine **ppt);
void      delStateMachineList(
  struct TStateMachineList **ppt);
void      delData(
  struct TData **ppt);
void      delDataList(
  struct TDataList **ppt);
void      delTimerRecord(
  struct TTimerRecord **ppt);
void      delTimerRecordList(
  struct TTimerRecordList **ppt);
void      delBoard(
  struct TBoard **ppt);
void      delBoardList(
  struct TBoardList **ppt);
void      delIO(
  struct TIO **ppt);
void      delIOList(
  struct TIOList **ppt);


extern struct TTypeList *_ptTypeList;
extern struct TFunctionList *_ptFunctionList;
extern struct TEventList *_ptEventList;
extern struct TStateMachineList *_ptStateMachineList;
extern struct TDataList *_ptDataList;
extern struct TBoardList *_ptBoardList;
extern struct TIOList *_ptIOList;
extern struct TVariableList *_ptGlobalVars;
extern struct TType *_ptDataType;
extern struct TType *_ptBooleanType;
extern struct TType CommSpecType;

extern char *_pcWarningText;
extern char *_pcErrorText;
extern char *_pcFatalText;
extern char *pcProtocol;
extern char *pcDevice[16];

extern struct TParameterList TimerParamList;
extern struct TParameterList OneParam;
extern struct TParameterList TwoParam;
extern struct TParameterList ThreeParam;
extern struct TParameterList FourParam;
extern struct TParameterList SignalParam;
extern int iWarningCounter;
extern int iErrorCounter;
extern int *piChannels;         /* REMOVE IF NOT WORKING */

void      CloseObsoleteIO(
  struct TIOList *ptOldIOList,
  struct TIOList *ptNewIOList);
void      LinkAllBranchesToOne(
  struct TParameterList *ptRoot,
  struct TParameterList *ptOne);
void      ConnectVarToVarList(
  struct TVariable **pptVar,
  struct TVariableList *ptReferenceList,
  struct TDataList *ptDataList,
  struct TParameterList *ptDefList);
int       Connect_Invocations(
  struct TVariableList *ptParams,
  struct TVariableList *ptInvokes);

struct TType *FindType(
  char *pcTypeName);
struct TFunction *FindFunction(
  char *pcFunctionName);
struct TFunction *FindFunctionOpcode(
  int iOpcode);
struct TEvent *FindEvent(
  char *pcEventName);
struct TEvent *FindEventOpcode(
  char cOpcode);
int       FindInteraction(
  char *pcToken);
struct TVariable *FindVar(
  char *pcVarName,
  struct TVariableList *ptReference);
struct TData *FindData(
  char *pcVarName,
  struct TDataList *ptReference);
struct TType *FindType(
  char *pcTypeName);
struct TStateMachine *FindStateMachine(
  char *pcName);
int       FindBoardNumber(
  char *pcName);
struct TParameter *FindParameterBackwards(
  char *pcName,
  struct TParameterList *ptList);
struct TParameter *FindParameter(
  char *pcName,
  struct TParameterList *ptList);
struct TEvent *FindMatchingEvent(
  char *pcBuffer,
  int iLength);

struct TStateMachine *Duplicate_StateMachine(
  struct TStateMachine *ptM);

void      InitParser(
  void);
void      ShutDownParser(
  void);

void      LogError(
  char *pcError);
void      LogMsg(
  char *pcMsg);
void      LogWarning(
  char *pcWarning);

void      Increment(
  int *i);
void      Decrement(
  int *i);

/****************************************************************************/
/* The following definitions are used for reporting warnings and errors     */
/* during parsing the input files.                                          */
/****************************************************************************/

extern char *_pcLine;
void      Warn(
  char *_pcLine);
void      Error(
  char *_pcLine);
void      Fatal(
  char *_pcLine);
void      ResetErrorLogging(
  void);


#define AddWarning1(a)        { sprintf(_pcLine,a);      LogWarning(_pcLine);}
#define AddWarning2(a,b)      { sprintf(_pcLine,a,b);    LogWarning(_pcLine);}
#define AddWarning3(a,b,c)    { sprintf(_pcLine,a,b,c);  LogWarning(_pcLine);}
#define AddWarning4(a,b,c,d)  { sprintf(_pcLine,a,b,c,d);LogWarning(_pcLine);}
#define AddWarning5(a,b,c,d,e){ sprintf(_pcLine,a,b,c,d,e);LogWarning(_pcLine);}

#define AddError1(a)          { sprintf(_pcLine,a);        LogError(_pcLine);}
#define AddError2(a,b)        { sprintf(_pcLine,a,b);      LogError(_pcLine);}
#define AddError3(a,b,c)      { sprintf(_pcLine,a,b,c);    LogError(_pcLine);}
#define AddError4(a,b,c,d)    { sprintf(_pcLine,a,b,c,d);  LogError(_pcLine);}
#define AddError5(a,b,c,d,e)  { sprintf(_pcLine,a,b,c,d,e);LogError(_pcLine);}

#define AddMsg1(a)            { sprintf(_pcLine,a);          LogMsg(_pcLine);}
#define AddMsg2(a,b)          { sprintf(_pcLine,a,b);        LogMsg(_pcLine);}
#define AddMsg3(a,b,c)        { sprintf(_pcLine,a,b,c);      LogMsg(_pcLine);}
#define AddMsg4(a,b,c,d)      { sprintf(_pcLine,a,b,c,d);    LogMsg(_pcLine);}
#define AddMsg5(a,b,c,d,e)    { sprintf(_pcLine,a,b,c,d,e);  LogMsg(_pcLine);}

#define FatalError1(a)         { sprintf(_pcLine,a);          Fatal(_pcLine);}
#define FatalError2(a,b)       { sprintf(_pcLine,a,b);        Fatal(_pcLine);}
#define FatalError3(a,b,c)     { sprintf(_pcLine,a,b,c);      Fatal(_pcLine);}
#define FatalError4(a,b,c,d)   { sprintf(_pcLine,a,b,c,d);    Fatal(_pcLine);}
#define FatalError5(a,b,c,d,e) { sprintf(_pcLine,a,b,c,d,e);  Fatal(_pcLine);}



void      StoreFileVariableList(
  struct TVariableList *ptList);

void      GenerateDebugOutput(
  char *pcFile);

void      WriteStateMachineList(
  char *pBuffer,
  struct TStateMachineList *ptStateMachineList,
  int iMode);

void      WriteBoardList(
  char *pcBuffer,
  struct TBoardList *ptBoardList);


void      WriteVar(
  char *pcBuffer,
  struct TVariable *ptVar);
