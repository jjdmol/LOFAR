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
// Description   : This module is the implementation file for the Protocol List
//
// Revisions:
//
// Date       Author                  Changes
// 18/10/2000 Dennis Olausson         Initial release
//
//=========================================================================
*/


#ifdef __cplusplus
extern    "C"
{
#endif

#ifdef WIN32
#include "windows.h"            /* For windows defines*/
#endif
#include <stdio.h>
#include <stdlib.h>
#include "codingstandard.h"     /* For common project defines */
#include "protocollist.h"


/************************************************************
 * Local Variables                                          */

  static struct TProtocolList *ptListHead = NULL;

/************************************************************
 * Local Function Declaration                               */
  void      LOC_DeleteEntry(
  struct TProtocolList *ptProtocolList);

/************************************************************
 * Function: IO_ProtocolListPurge                           *
 * Description:                                             *
 * Deletes and clean up the list                            *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-10-19    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  void      IO_ProtocolListPurge(
  void)
  {
    LOC_DeleteEntry(ptListHead);
    ptListHead = NULL;
  }



/************************************************************
 * Function: IO_AddToProtocolList                           *
 * Description:                                             *
 * -Adds an entry to the ProtocolList.                      *
 * -Further AddOns appends to the end of the list.          *
 * -If the same protocol number is added the information    *
 *  is overwritten (uppdated)                               *
 *                                                          *
 * Return value:                                            *
 * The funtion returns an integer whera a 0 indicates false *
 * and a 1 indicates true.                                  *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-10-18    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  int16     IO_AddToProtocolList(
  int16 iProtocolNumber,
  int16 iProtocol,
  int16 iTokenLength,
  char *pTerminateToken,
  int16 iMaxLength)
  {
    BOOL      bExcist = FALSE;
    struct TProtocolList *ptPresent;
    struct TProtocolList tNewEntry;
    struct TProtocolList *ptAppender;

    /*      Fill a new Entry structure */
    tNewEntry.tProtocol.iProtocolNumber = iProtocolNumber;
    tNewEntry.tProtocol.iProtocol = iProtocol;
    tNewEntry.tProtocol.iTokenLength = iTokenLength;
    tNewEntry.tProtocol.pTerminateToken = pTerminateToken;
    tNewEntry.tProtocol.iMaxLength = iMaxLength;
    tNewEntry.pNext = NULL;

/*        Get hold of the begining of the list entry        */
    if (ptListHead != NULL)
    {
      ptPresent = ptListHead;

      /* If the protocol is already defined, overwrite! (uppdate)     */
      if (ptPresent->tProtocol.iProtocolNumber ==
          tNewEntry.tProtocol.iProtocolNumber)
      {
        ptPresent->tProtocol = tNewEntry.tProtocol;
        return TRUE;
      }
      else
      {                         /* Find the last entry */
        while (ptPresent->pNext != NULL)
        {
          ptPresent = ptPresent->pNext;
          /* If the protocol is already defined, overwrite! */
          if (ptPresent->tProtocol.iProtocolNumber ==
              tNewEntry.tProtocol.iProtocolNumber)
          {
            ptPresent->tProtocol = tNewEntry.tProtocol;
            bExcist = TRUE; /* Remember that we have already found the entry */
          }
        }
        /* The end is found, but have we already updated an excisting entry?    */
        if (!bExcist)
        {                       /* New entry, append it to the end of the list */
          ptAppender =
            (struct TProtocolList *) malloc(sizeof(struct TProtocolList));
          if (ptAppender != NULL)
          {
            ptAppender->tProtocol = tNewEntry.tProtocol;
            ptAppender->pNext = tNewEntry.pNext;
            ptPresent->pNext = ptAppender;
            return TRUE;        /*Protocol Successfully added. */
          }
          else
            return FALSE;       /* malloc failed */
        }
        else
          return TRUE;          /* Protocol successfully updated */
      }
    }
    /* The list is empty append the new entry to the head */
    else
    {
      ptAppender =
        (struct TProtocolList *) malloc(sizeof(struct TProtocolList));
      if (ptAppender != NULL)
      {
        ptAppender->tProtocol = tNewEntry.tProtocol;
        ptAppender->pNext = tNewEntry.pNext;
        ptListHead = ptAppender;
        return TRUE;            /* Protocol Successfully added. */
      }
      else
        return FALSE;           /* malloc failed. */
    }
  }


/************************************************************
 * Function: IO_ProtocolListFind                            *
 * Arguments: uint16 iProtocolNumber                        *
 * Description:                                             *
 * Search up the desired Protocol Number and returns the    *
 * adress to the Protocol Structure. If there was no match  *
 * NULL is returned                                         *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-10-19    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  struct TProtocol *IO_ProtocolListFind(
  int16 iProtocolNumber)
  {
    struct TProtocolList *ptPresent;

    if (ptListHead != NULL)     /* Empty list? */
    {
      ptPresent = ptListHead;
      /* Search for the corresponding protocol number */
      while ((iProtocolNumber != ptPresent->tProtocol.iProtocolNumber) &&
             (ptPresent->pNext != NULL))
        ptPresent = ptPresent->pNext;
      if (iProtocolNumber != ptPresent->tProtocol.iProtocolNumber)
        return NULL;    /* Didn't find the protocol number return NULL */
      else
        return &ptPresent->tProtocol; /* Found the protocolnumber, return the adress  */
    }                           /* of the protocol structure */
    else
      return NULL;
  }


/************************************************************
 * Local Functions                                          *
 ************************************************************/

/************************************************************
 * Local Function: LOC_DeleteEntry                          *
 * Description:                                             *
 * Recursive Function that deletes and clean up an entry    *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-10-30    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  void      LOC_DeleteEntry(
  struct TProtocolList *ptProtocolList)
  {
    if (ptProtocolList != NULL)
    {
      LOC_DeleteEntry(ptProtocolList->pNext);
      free(ptProtocolList->tProtocol.pTerminateToken);
      free(ptProtocolList);
      ptProtocolList = NULL;
    }
  }

#ifdef __cplusplus
}
#endif
