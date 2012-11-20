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


#ifndef GENERAL_LIB
#define GENERAL_LIB

#include "codingstandard.h"
#ifndef WIN32
#include <sys/time.h>
#endif
/****************************************************************************/
/* The macro's                                                              */
/****************************************************************************/

#ifndef TRUE
#define TRUE  (0==0)
#endif

#ifndef FALSE
#define FALSE (!TRUE)
#endif

#define TO_HEX(a)   (((a)>'9')?((a)-'A'+10):((a)-'0'))
#define TO_ASCII(a) (((a)>9)?((a)-10+'A'):((a)+'0'))

#define TO_BYTE(a,b) (TO_HEX(a) << 4 | TO_HEX(b))

#define TOKENSIZE      (1024)

#define UNIONARRAYSIZE 10

#define TIMEKIND       (0)
#define ENUMKIND       (1)
#define BITFIELDKIND   (2)
#define COMMANDKIND    (3)
#define ASCIIKIND      (4)
#define ASCII0KIND     (5)
#define UNICODEKIND    (6)
#define UNICODE0KIND   (7)
#define DECIMALKIND    (8)
#define FILEDATAKIND   (9)
#define ARRAYKIND      (10)

#define TIMER_ACTION            (100)
#define WAIT_ACTION             (101)
#define CLEAR_ACTION            (102)
#define TERMINATE_ACTION        (103)
#define R_SIG_ACTION            (104)
#define S_SIG_ACTION            (105)
#define PRINT_ACTION            (106)
#define COMPARE_ACTION          (107)
#define PROT_PROTOCOL_ACTION    (108)
#define ASCII_PROTOCOL_ACTION   (109)
#define TCI_PROTOCOL_ACTION     (110)
#define PROT_WRITEDATA_ACTION   (111)
#define PROT_READDATA_ACTION    (112)
#define BREAK_ACTION            (113)
#define FUNCTION_ACTION         (114)
#define EVENT_ACTION            (115)
#define ARITHMETRIC_ACTION      (116)
#define RESCUE_ACTION           (117)
#define IF_ACTION               (118)
#define STATEMACHINE_ACTION     (119)
#define END_ACTION              (120)
#define CTS_HIGH_ACTION         (121)
#define CTS_LOW_ACTION          (122)
#define DSR_HIGH_ACTION         (123)
#define DSR_LOW_ACTION          (124)
#define DTR_HIGH_ACTION         (125)
#define DTR_LOW_ACTION          (126)
#define SET_COMM_BREAK_ACTION   (127)
#define CLEAR_COMM_BREAK_ACTION (128)
#define SOCKET_GENERAL_ERROR    (129)
#define SOCKET_SEND_ERROR       (130)
#define SOCKET_RECEIVE_ERROR    (131)
#define SOCKET_CONNECT_ERROR    (132)
#define SOCKET_DISCONNECT_ERROR (133)
#define SOCKET_ACCEPT_ERROR     (134)
#define SOCKET_UNKNOWN_ERROR    (135)
#define CONNECTION_LOST         (136)
#define CONNECTION_ESTABLISHED  (137)
#define MTIMER_ACTION           (138)
#define RTIMER_ACTION           (139)
#define RMTIMER_ACTION          (140)

/****************************************************************************/
/* The struct's                                                             */
/****************************************************************************/


struct TValueDef
{
  char     *pcMask;
  char     *pcValue;
  char     *pcDescription;
  int       iRefCount;
};

struct TValueDefList
{
  struct TValueDef *ptThis;
  struct TValueDefList *ptNext;
  int       iRefCount;
};

struct TType
{
  char     *pcName;
  int       iEndianess;         /* 0 : most cases (Intel way   )   */
  /* 1 : strings    (Motorola way)   */
  unsigned int iSizeInBytes;
  unsigned int uiNumberOfElements;
  unsigned int uiSizeOfElement;
  int       iLessAllowed;       /* 1 : Type can be less bytes ..   */
  char     *pcLowerLimit;       /* ptr to (SizeInBytes) bytes, de- */
  char     *pcUpperLimit;       /* fining lower and upper limit    */

  int       iKind;              /* 0 : undefined                   */
  /* 1 : TIME                        */
  /* 2 : ENUM                        */
  /* 3 : BITFIELD                    */
  float     fTimeScaling;
  char     *pcDecimalDescription;
  struct TValueDefList *ptDefinition;
  int       iRefCount;
};

struct TTypeList
{
  struct TType *ptThis;
  struct TTypeList *ptNext;
  int       iRefCount;
};

struct TParameter
{
  char     *pcName;
  int       iArray;             /* TRUE if array parameter.        */
  int       iOffset;
  struct TType *ptTypeDef;
  int       iRefCount;
  struct TValue *ptValue;       /* Ptr if parameter is predefined const */
  int       iLoopCount;         /* used runtime for repsets... */
  char     *pcLengthIndicator;  /* used if length of a field is indicated by another field */
  char     *pcLengthFromField;
  char     *pcLengthToField;
  struct TParameter *ptLengthFromField;
  struct TParameter *ptLengthToField;
  struct TParameter *ptLengthIndicator;
};

struct TRepetition
{
  int       iBegin;             /* TRUE if begin, FALSE if end */
  struct TParameter *ptParameter;       /* The par containing repcount */
  int       iCounter;           /* Used as runtime loopcount   */
};

/* The next structure requires some explanation. A field 'FollowedByUnion'  */
/* is inserted, because of the following reason:                            */
/* If this field is followed by a union, the type of the following field    */
/* depends on the type of this field. The actual value of this field is     */
/* used as an index in the array of next pointers.                          */
/* If the field is not followed by a union, only the first entry of the     */
/* array of pointers is valid.                                              */
/* An "else" entry is implemented, the last element in the aptNext array is */
/* used for this.                                                           */
/* ptRepRec might point to a record that is used for repeated fragments of  */
/* parameter lists. In the spec file this looks like:                       */
/* a : ta,                                                                  */
/* {                                                                        */
/*   b : tb,                                                                */
/*   c : tc                                                                 */
/* } [a]                                                                    */
/*                                                                          */
/* The list will look like a , reprec, b, c, reprec.                        */

struct TParameterList
{
  struct TParameter *ptThis;
  int       iFollowedByUnion;   /* boolean..                   */
  struct TParameterList *aptNext[UNIONARRAYSIZE + 1];
  struct TRepetition *ptRepRec; /* added 2004                  */
  struct TParameterList *ptBackTrack;   /* added 2004                  */
  int       iRefCount;
};

/* Two variables fulfill a rather complex function in the following struc-  */
/* ture: SizeOfSendArrayParameters and SizeOfReceiveArrayParameters. These  */
/* values are used to calculate the offset of elements of an array, in case */
/* one or more of the parameters is an array. See the PROT specification for*/
/* more information. Array parameters are never seperated by normal para-   */
/* meters in the current spec. If this changes, we have a serious problem.  */
/* Call the n parameters of the function p(0) .. p(n). The parameters       */
/* p1..p(m-1) are normal parameters, parameter p(m) indicates the size of   */
/* the array, parameter p(m+1) .. p(n) are array parameters. In the spec of */
/* PROT, the parameters will be sent in the following order: (example m=3,  */
/* n=6) p0 p1 p2 p3 p4 p5 p6 p4 p5 p6 p4 p5 p6 p4 p5 p6 ...                 */
/* Let's call the size of each parameter s(x). This results in s0 s1 s2 s3  */
/* s4 s5 and s6. Now we can calculate the offset of each parameter in the   */
/* PROT-string:                                                             */
/* Offset       p0 = 0   (its the first one, no offset.)                    */
/* Offset       p1 = s0                                                     */
/* Offset       p2 = s0 + s1                                                */
/* Offset       p3 = s0 + s1 + s2                                           */
/* Offset first p4 = s0 + s1 + s2 + s3                                      */
/* Offset first p5 = s0 + s1 + s2 + s3 + s4                                 */
/* Offset first p6 = s0 + s1 + s2 + s3 + s4 + s5                            */
/* Offset 2nd   p4 = s0 + s1 + s2 + s3 + s4 + s5 + s6                       */
/* Offset 2nd   p5 = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s4                  */
/* Offset 2nd   p6 = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s4 + s5             */
/* Offset 3nd   p4 = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s4 + s5 + p6        */
/* ect.                                                                     */
/* The offset of the x-th p4, p5, p6 can be denoted as:                     */
/* Offset x-th  p4 = s0 + s1 + s2 + s3 +           (x-1)*(s4 + s5 + s6)     */
/* Offset x-th  p5 = s0 + s1 + s2 + s3 + s4 +      (x-1)*(s4 + s5 + s6)     */
/* Offset x-th  p6 = s0 + s1 + s2 + s3 + s4 + s5 + (x-1)*(s4 + s5 + s6)     */
/* While walking through the list of parameters it is easy to summarise     */
/* the size of the passed parameters. While walking through the arrays it   */
/* is easy to know the sum of the size of all array parameters. That's the  */
/* reason of these two variables.                                           */

struct TFunction
{
  char     *pcName;
  int       iSizeOfSendArrayParameters;
  int       iSizeOfReceiveArrayParameters;
  struct TParameterList *ptSendParameters;
  struct TParameterList *ptReceiveParameters;
  int       iRefCount;
};

struct TFunctionList
{
  struct TFunction *ptThis;
  struct TFunctionList *ptNext;
  int       iRefCount;
};

struct TEvent
{
  char     *pcName;
  int       iSizeOfArrayParameters;
  struct TParameterList *ptParameters;
  int       iRefCount;
};

struct TEventList
{
  struct TEvent *ptThis;
  struct TEventList *ptNext;
  int       iRefCount;
};

struct TVariable
{
  char     *pcName;
  struct TType *ptItsType;
  struct TValue *ptValue;
  int       iRefCount;
};


struct TVariableList
{
  struct TVariable *ptThis;
  struct TVariableList *ptNext;
  int       iRefCount;
};

struct TValue
{
  char          *pcValue;
  char          *pcFileName;
  char          *pcFileMode;    
  int            iRefCount;
  unsigned int   uiLength;
  int            bFixed;             /* if true, a reset script won't influence this val. */

};

struct TAction
{
  long      lActionCounter;
  int       iActionType;
  int       iNrOfReceptions;
  struct TFunction *ptFunction; /* only defined if ActionType == 0   */
  struct TEvent *ptEvent;       /* only defined if ActionType == 1   */
  struct TStateMachine *ptStateMachine; /*  defined if ActionType == 2   */
  struct TExpression *ptExpression;
  struct TVariableList *ptVarList;      /* Not used for expressions.         */
  int       iSourceLineNumber;  /* Can be used in error-msg's */
  int       iRefCount;
};

struct TActionList
{
  struct TAction *ptThis;
  struct TActionList *ptNext;
  int       iRefCount;
};

struct TTransition
{
  struct TActionList *ptActionList;
  char     *pcNextStateName;
  struct TState *ptNextState;
  long      lTransitionCounter;
  int       iInstantNextState;
  int       iAlternative;       /* 0 : primary statemachine        */
  /* 1 : alternative statemachine    */
  int       iRefCount;
};

struct TTransitionList
{
  struct TTransition *ptThis;
  struct TTransitionList *ptNext;
  int       iRefCount;
};

struct TState
{
  char     *pcName;
  struct TTransitionList *ptTransitionList;
  int       iRefCount;
};

struct TStateList
{
  struct TState *ptThis;
  struct TStateList *ptNext;
  int       iRefCount;
};

struct TTimer
{
  struct timeval        tExpirationMoment;
  struct TStateMachine *ptStateMachine;
};

struct TTimerList
{
  struct TTimer *ptThis;
  struct TTimerList *ptNext;
};

struct TStateMachine
{
  char     *pcName;
  char     *pcDeviceName;
  struct TVariableList *ptParameters;
  struct TVariableList *ptLocalVars;
  struct TStateList *ptStates;
  struct TState *ptCurrentState;
  struct TState *ptRescueState;
  char     *pcRescueStateName;
  struct TAction *ptActionInProgress;
  struct TTransition *ptRunningTimerTransition;
  struct TState *ptStateInTimer;
  struct TTimer *ptRunningTimer;
  struct TStateMachine *ptMaster;
  int       iPending;
  int       iRefCount;
  int       iIOChannel;
  int       iAlternative;
  int       iHidden;
};

struct TStateMachineList
{
  struct TStateMachine *ptThis;
  struct TStateMachineList *ptNext;
  int       iRefCount;
};


struct TData
{
  char     *pcName;
  struct TType *ptItsType;      /* should be NULL always. */
  char     *pcValue;
  int       iRefCount;
  int       bFixed;
};


struct TDataList
{
  struct TData *ptThis;
  struct TDataList *ptNext;
  int       iRefCount;
};

struct TBoard
{
  char     *pcName;
  int       iNumber;
  struct TStateMachineList *ptStateMachines;
  int       iRefCount;
};

struct TBoardList
{
  struct TBoard *ptThis;
  struct TBoardList *ptNext;
  int       iRefCount;
};

struct TGlobals
{
  int       iOverviewMode;
  int       iLogMode;
  int       iDebugLevel;
};

struct TIO
{
  int16     iConfigured;
  int16     iNeeded;
  char     *pcName;
  int16     iNumber;
  char     *pcIOPortName;
  int16     iProtocol;
  int16     iTerminateTokenLength;
  int16     iMaxTokenLength;
  char     *pTerminateToken;
  char     *pComConfig;
};

struct TIOList
{
  struct TIO *ptThis;
  struct TIOList *ptNext;
  int       iRefCount;
};


/****************************************************************************/
/* Exported data                                                       */
/****************************************************************************/
extern long      lSeedValue;

/****************************************************************************/
/* Exported functions                                                       */
/****************************************************************************/

/* Reads the seed value from given filename */
void GetSeedValue(char * pcFilename);

void      MallocText(
  void);
void      FreeText(
  void);

int16     GetInteger(
  char *pcToken);
void CurrentTime(
  struct timeval *);
struct TParameterList *NextqRepParameter(
  struct TParameterList *ptParameterList);
struct TParameterList *NextParameter(
  struct TParameterList *ptParameterList,
  char *pcValue);
void      InitBoards(
  struct TBoardList *ptBoardList);
void      InitStateMachine(
  struct TStateMachine *ptMachine);
struct TState *FindState(
  char *pcName,
  struct TStateList *ptStateList);
void      Update_Overview(
  void);
char     *strpos(
  char *pcShortString,
  char *pcLongString);
char     *my_strdup(
  const char *p);
void      asciicat(
  char *pcBuffer,
  char *pcString);

#endif
