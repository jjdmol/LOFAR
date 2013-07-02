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

#include "codingstandard.h"

/* Apply a Microsoft Trick.                                                 */

#define NO_SCANNER_ERROR          ((int16)  0)

#define DEC_NUMBER_TOKEN          ((int16)  1)
#define HEX_NUMBER_TOKEN          ((int16)  2)
#define BIN_NUMBER_TOKEN          ((int16)  3)
#define STRING_TOKEN              ((int16)  4)
#define OPERATOR_TOKEN            ((int16)  5)
#define PRECOMPILER_TOKEN         ((int16)  6)
#define OPEN_PARENTHESES_TOKEN    ((int16)  7)
#define CLOSE_PARENTHESES_TOKEN   ((int16)  8)
#define OPEN_BRACKETS_TOKEN       ((int16)  9)
#define CLOSE_BRACKETS_TOKEN      ((int16) 10)
#define IDENTIFIER_TOKEN          ((int16) 11)
#define DOT_TOKEN                 ((int16) 12)
#define COMMA_TOKEN               ((int16) 13)
#define SEMICOLON_TOKEN           ((int16) 14)
#define COLON_TOKEN               ((int16) 15)
#define LEFT_ARROW_TOKEN          ((int16) 16)
#define RIGHT_ARROW_TOKEN         ((int16) 17)
#define COMMENT_TOKEN             ((int16) 18)
#define TILDE_TOKEN               ((int16) 19)
#define BACKSLASH_TOKEN           ((int16) 20)
#define FLOAT_TOKEN               ((int16) 21)
#define DOTDOT_TOKEN              ((int16) 22)

#define UNEXPECTED_CHAR_READ      ((int16) -1)
#define UNTERMINATED_STRING_READ  ((int16) -2)
#define BAD_MACRO_DEFINITION      ((int16) -3)
#define INCLUDE_FILE_PROBLEM      ((int16) -4)
#define UNKNOWN_PREPROCESSOR      ((int16) -5)
#define OUT_OF_MEMORY             ((int16) -6)
#define CANNOT_OPEN_FILE          ((int16) -7)
#define END_OF_FILE               ((int16) -8)


#ifdef __cplusplus
extern    "C"
{
#endif

  extern int GetToken(
  char *pcToken);
  extern void UngetToken(
  void);
  extern char *CurrentFile(
  void);
  extern char *FileIncludedBy(
  void);
  extern int CurrentLine(
  void);
  extern int OpenInputFile(
  char *pcFileName);
  extern int InitScanner(
  void);
  extern void CloseInputFile(
  void);
  extern char *NameOf(
  int iTokenType);
  extern void ShutDownScanner(
  void);

#ifdef __cplusplus
}
#endif
