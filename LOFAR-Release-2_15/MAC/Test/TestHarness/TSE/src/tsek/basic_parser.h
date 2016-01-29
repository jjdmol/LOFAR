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


#define ParseString(a) ParseToken((a),STRING_TOKEN)
#define ParseIdentifier(a) ParseToken((a),IDENTIFIER_TOKEN)

extern int iLastParsedTokenType;

int       Parse(
  char *pcToken);
int       ParseToken(
  char *pcToken,
  int iTokenType);
int       ParseNumber(
  char *pcToken);
int       ParseHexNumber(
  char *pcToken);
int       ParseChar(
  char cCharacter);
int       ParseFloat(
  char *pcToken);
