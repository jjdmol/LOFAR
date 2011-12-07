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


#ifndef CODING_STANDARD_H
#define CODING_STANDARD_H


#ifdef __cplusplus
extern    "C"
{
#endif

#ifndef TRUE
#define TRUE  (0==0)
#endif

#ifndef FALSE
#define FALSE (!TRUE)
#endif


#ifndef WIN32  
  typedef char          BOOL;
  typedef int           DWORD;
#endif  
  typedef long          int32;
  typedef int           int16;
  typedef char          int8;
  typedef unsigned char uint8;
  typedef unsigned int  uint16;
  typedef unsigned long uint32;

#ifdef __cplusplus
}
#endif

#endif
