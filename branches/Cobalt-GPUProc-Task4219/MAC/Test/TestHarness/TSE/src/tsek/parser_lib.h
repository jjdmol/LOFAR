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


extern struct TValue *newValue(
  void);
extern struct TValueDef *newValueDef(
  void);
extern struct TValueDefList *newValueDefList(
  void);
extern struct TType *newType(
  void);
extern struct TTypeList *newTypeList(
  void);
extern struct TParameter *newParameter(
  void);
extern struct TParameterList *newParameterList(
  void);
extern struct TRepetition *newRepetition(
  void);
extern struct TFunction *newFunction(
  void);
extern struct TFunctionList *newFunctionList(
  void);
extern struct TEvent *newEvent(
  void);
extern struct TEventList *newEventList(
  void);
extern struct TVariable *newVariable(
  struct TType *ptThisType);
extern struct TVariableList *newVariableList(
  void);
extern struct TAction *newAction(
  void);
extern struct TActionList *newActionList(
  void);
extern struct TTransition *newTransition(
  void);
extern struct TTransitionList *newTransitionList(
  void);
extern struct TState *newState(
  void);
extern struct TStateList *newStateList(
  void);
extern struct TStateMachine *newStateMachine(
  void);
extern struct TStateMachineList *newStateMachineList(
  void);
extern struct TData *newData(
  void);
extern struct TDataList *newDataList(
  void);
extern struct TTimerRecord *newTimerRecord(
  void);
extern struct TTimerRecordList *newTimerRecordList(
  void);
extern struct TBoard *newBoard(
  void);
extern struct TBoardList *newBoardList(
  void);
extern struct TIO *newIO(
  void);
extern struct TIOList *newIOList(
  void);



extern void      delValue(
  struct TValue **ppt);
extern void      delValueDef(
  struct TValueDef **ppt);
extern void      delValueDefList(
  struct TValueDefList **ppt);
extern void      delType(
  struct TType **ppt);
extern void      delTypeList(
  struct TTypeList **ppt);
extern void      delParameter(
  struct TParameter **ppt);
extern void      delParameterList(
  struct TParameterList **ppt);
extern void      delFunction(
  struct TFunction **ppt);
extern void      delFunctionList(
  struct TFunctionList **ppt);
extern void      delEvent(
  struct TEvent **ppt);
extern void      delEventList(
  struct TEventList **ppt);
extern void      delVariable(
  struct TVariable **ppt);
extern void      delVariableList(
  struct TVariableList **ppt);
extern void      delAction(
  struct TAction **ppt);
extern void      delActionList(
  struct TActionList **ppt);
extern void      delState(
  struct TState **ppt);
extern void      delStateList(
  struct TStateList **ppt);
extern void      delStateMachine(
  struct TStateMachine **ppt);
extern void      delStateMachineList(
  struct TStateMachineList **ppt);
extern void      delData(
  struct TData **ppt);
extern void      delDataList(
  struct TDataList **ppt);
extern void      delTimerRecord(
  struct TTimerRecord **ppt);
extern void      delTimerRecordList(
  struct TTimerRecordList **ppt);
extern void      delBoard(
  struct TBoard **ppt);
extern void      delBoardList(
  struct TBoardList **ppt);
extern void      delIO(
  struct TIO **ppt);
extern void      delIOList(
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
extern struct TParameterList TimerTwoParametersList;
extern struct TParameterList OneParam;
extern struct TParameterList TwoParam;
extern struct TParameterList ThreeParam;
extern struct TParameterList FourParam;
extern struct TParameterList SignalParam;
extern int iWarningCounter;
extern int iErrorCounter;
extern int *piChannels;         /* REMOVE IF NOT WORKING */

extern void      CloseObsoleteIO(
  struct TIOList *ptOldIOList,
  struct TIOList *ptNewIOList);
extern void      LinkAllBranchesToOne(
  struct TParameterList *ptRoot,
  struct TParameterList *ptOne);
extern void      ConnectVarToVarList(
  struct TVariable **pptVar,
  struct TVariableList *ptReferenceList,
  struct TDataList *ptDataList,
  struct TParameterList *ptDefList);
extern int       Connect_Invocations(
  struct TVariableList *ptParams,
  struct TVariableList *ptInvokes);

extern int16 CheckType( 
  struct TType *ptType );
extern struct TType *FindType(
  char *pcTypeName);
extern int16 CheckFunction( 
  struct TFunction *ptFunction );
extern struct TFunction *FindFunction(
  char *pcFunctionName);
extern struct TFunction *FindFunctionOpcode(
  int iOpcode);
extern int16 CheckEvent( 
  struct TEvent *ptEvent );
extern struct TEvent *FindEvent(
  char *pcEventName);
extern struct TEvent *FindEventOpcode(
  char cOpcode);
extern int       FindInteraction(
  char *pcToken);
extern struct TVariable *FindVar(
  char *pcVarName,
  struct TVariableList *ptReference);
extern struct TData *FindData(
  char *pcVarName,
  struct TDataList *ptReference);
extern struct TType *FindType(
  char *pcTypeName);
extern struct TStateMachine *FindStateMachine(
  char *pcName);
extern int       FindBoardNumber(
  char *pcName);
extern struct TParameter *FindParameterBackwards(
  char *pcName,
  struct TParameterList *ptList);
extern struct TParameter *FindParameter(
  char *pcName,
  struct TParameterList *ptList);
extern struct TEvent *FindMatchingEvent(
  char *pcBuffer,
  int iLength);

extern struct TStateMachine *Duplicate_StateMachine(
  struct TStateMachine *ptM);

extern void      InitParser(
  void);
extern void      ShutDownParser(
  void);

extern void      LogError(
  char *pcError);
extern void      LogMsg(
  char *pcMsg);
extern void      LogWarning(
  char *pcWarning);

extern void      Increment(
  int *i);
extern void      Decrement(
  int *i);

/****************************************************************************/
/* The following definitions are used for reporting warnings and errors     */
/* during parsing the input files.                                          */
/****************************************************************************/

extern char *_pcLine;
extern void      Warn(
  char *_pcLine);
extern void      Error(
  char *_pcLine);
extern void      Fatal(
  char *_pcLine);
extern void      ResetErrorLogging(
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



extern void      StoreFileVariableList(
  struct TVariableList *ptList);

extern void      GenerateDebugOutput(
  char *pcFile);

extern void      WriteStateMachineList(
  char *pBuffer,
  struct TStateMachineList *ptStateMachineList,
  int iMode);

extern void      WriteBoardList(
  char *pcBuffer,
  struct TBoardList *ptBoardList);


extern void      WriteVar(
  char *pcBuffer,
  struct TVariable *ptVar);
