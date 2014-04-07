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
// 09/02/2001 E.A. Nijboer            
//
//=========================================================================
*/

#ifndef _PARSER_H_
#define _PARSER_H_

/* Defines              */

#define P_OK                    ((int16) (  0))
#define P_FILE_NOT_FOUND        ((int16) ( -1))
#define P_ERROR                 ((int16) (-99))

/* exported variabeles: */

extern char *cErrorLine[240];

/* exported functions */

extern int       Parse_Specification_File(
  char *pcFileName);
extern int       Parse_Script_File(
  char *pcFileName);
extern void      Add_IO(
  void);
extern int       Parse_IO_File(
  char *pcFileName);

#endif /* _PARSER_H_ */
