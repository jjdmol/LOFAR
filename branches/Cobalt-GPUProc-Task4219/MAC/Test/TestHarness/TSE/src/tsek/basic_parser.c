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

typedef int boolean;

int       iLastParsedTokenType;

int ParseToken(
  char *pcToken,
  int iTokenType)
/****************************************************************************/
/* input  : iTokenType : the expected token type                            */
/* output : pcToken    : the parsed token                                   */
/* globals: iLastParsedTokenType : the parsed token type.                   */
/* result : true : The expected token type is scanned.                      */
/*          false: Another token type is scanned.                           */
/****************************************************************************/
{
  iLastParsedTokenType = Parse(pcToken);

  if (iLastParsedTokenType == iTokenType)
  {
    return TRUE;
  }
  else
  {
    AddError4("%s is an %s. %s expected",
              pcToken, NameOf(iLastParsedTokenType), NameOf(iTokenType));
    return FALSE;
  }
}


int Parse(
  char *pcToken)
/****************************************************************************/
/* input  : --                                                              */
/* output : pcToken : the parsed token.                                     */
/* globals: iLastParsedTokenType : the parsed token type.                   */
/****************************************************************************/
{
  iLastParsedTokenType = GetToken(pcToken);
  while (iLastParsedTokenType == COMMENT_TOKEN)
  {
    iLastParsedTokenType = GetToken(pcToken);
  }
  switch (iLastParsedTokenType)
  {
    case UNEXPECTED_CHAR_READ:
      AddError2("Unexpected char read %s", pcToken);
      break;
    case UNTERMINATED_STRING_READ:
      AddError2("Unterminated string read %s", pcToken);
      break;
    case BAD_MACRO_DEFINITION:
      AddError2("Bad Macro Definition %s", pcToken);
      break;
    case INCLUDE_FILE_PROBLEM:
      AddError2("Include file problem %s", pcToken);
      break;
    case UNKNOWN_PREPROCESSOR:
      AddError2("Unknown Preprocessor directive %s", pcToken);
      break;
    case OUT_OF_MEMORY:
      AddError1("Out of memory");;
      break;
    case CANNOT_OPEN_FILE:
      AddError2("Cannot open file %s", pcToken);
      break;
    default:
      break;
  }
  return iLastParsedTokenType;
}


int ParseNumber(
  char *pcToken)
/****************************************************************************/
/* input  : --                                                              */
/* output : pcToken : the parsed token.                                     */
/* globals: iLastParsedTokenType : the parsed token type.                   */
/* return : TRUE if a NUMBER token was scanned.                             */
/****************************************************************************/
{
  iLastParsedTokenType = Parse(pcToken);

  if ((iLastParsedTokenType == HEX_NUMBER_TOKEN)
      || (iLastParsedTokenType == BIN_NUMBER_TOKEN)
      || (iLastParsedTokenType == DEC_NUMBER_TOKEN))
  {
    return TRUE;
  }
  else
  {
    AddError3("number expected, %s ( a %s) found",
              pcToken, NameOf(iLastParsedTokenType));
    return FALSE;
  }
}




int ParseFloat(
  char *pcToken)
/****************************************************************************/
/* input  : --                                                              */
/* output : pcToken : the parsed token.                                     */
/* globals: iLastParsedTokenType : the parsed token type.                   */
/* return : TRUE if a FLOAT token was scanned.                              */
/****************************************************************************/
{
  iLastParsedTokenType = Parse(pcToken);

  if (iLastParsedTokenType == FLOAT_TOKEN)
  {
    return TRUE;
  }
  else
  {
    AddError3("float expected, %s ( a %s) found",
              pcToken, NameOf(iLastParsedTokenType));
    return FALSE;
  }
}



int ParseHexNumber(
  char *pcToken)
/****************************************************************************/
/* input  : --                                                              */
/* output : pcToken : the parsed token.                                     */
/* globals: iLastParsedTokenType : the parsed token type.                   */
/* return : TRUE if a HEX value is returned.                                */
/****************************************************************************/
{
  Parse(pcToken);
  if (iLastParsedTokenType == HEX_NUMBER_TOKEN)
  {
    return TRUE;
  }
  else
  {
    AddError2("Hexadeximal (0x...) expected, %s read", pcToken);
    return FALSE;
  }
}




int ParseChar(
  char cCharacter)
/****************************************************************************/
/* input  : --                                                              */
/* output : pcToken : the parsed token.                                     */
/* globals: iLastParsedTokenType : the parsed token type.                   */
/* return : TRUE if the indicated character is parsed.                      */
/****************************************************************************/
{
  char      pcToken[TOKENSIZE];

  Parse(pcToken);

  if ((*pcToken) == cCharacter)
  {
    return TRUE;
  }
  else
  {
    AddError3("%c expected, %s read", cCharacter, pcToken);
    return FALSE;
  }
}
