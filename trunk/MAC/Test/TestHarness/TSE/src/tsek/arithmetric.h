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


#ifdef __cplusplus
extern    "C"
{
#endif

#include "codingstandard.h"

  struct TExpression
  {
    int16     iOperator;
    int16     iOperatorLevel;
    struct TVariable *ptVariable;
    struct TExpression *ptLeftExpression;
    struct TExpression *ptRightExpression;
  };


/*                         ptValue   ptLeftExpr ptRightExpr */
#define NUMBER     (0 )         /* defined   NULL       NULL        */
#define EQUAL      (1 )         /* NULL      defined    defined     */
#define NONEQUAL   (2 )         /* NULL      defined    defined     */
#define LESS       (3 )         /* NULL      defined    defined     */
#define LESSEQUAL  (4 )         /* NULL      defined    defined     */
#define MORE       (5 )         /* NULL      defined    defined     */
#define MOREEQUAL  (6 )         /* NULL      defined    defined     */
#define ASSIGNMENT (7 )         /* NULL      defined    defined     */
#define PLUS       (8 )         /* NULL      defined    defined     */
#define MINUS      (9 )         /* NULL      defined    defined     */
#define MUL        (10)         /* NULL      defined    defined     */
#define DIV        (11)         /* NULL      defined    defined     */
#define AND        (12)         /* NULL      defined    defined     */
#define OR         (13)         /* NULL      defined    defined     */
#define NOT        (14)         /* NULL      NULL       defined     */
#define UNDEFINED  (99)         /* NULL      NULL       NULL        */


  int16     Parse_TExpression(
  char *pcToken,
  struct TAction *ptAction,
  struct TVariableList *ptReferenceList);

  void      WriteExpression(
  char *pcBuffer,
  struct TExpression *ptExpr,
  int16 iLevel);

  struct TType *TypeCheck(
  struct TExpression *ptExpr);

  struct TExpression *newExpression(
  void);

  struct TExpression *dupExpression(
  struct TExpression *ptOld,
  struct TVariableList *ptVarList);

  void      delExpression(
  struct TExpression **p);

  char     *EvaluateExpression(
  struct TExpression *ptExpr);

#ifdef __cplusplus
}
#endif
