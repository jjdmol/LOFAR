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
// Description   : This module is the implementation file for the Receive Buffer
//
// Revisions:
//
// Date       Author                  Changes
// 31/10/2000 Dennis Olausson         Initial release
//
//=========================================================================
*/


#ifdef __cplusplus
extern    "C"
{
#endif

#include <stdlib.h>

#include "receivebuffer.h"
#include "atdriver.h"
#include "atkernel.h"
#include "memory.h"




/************************************************************
 * Local Variables                                          */
  static struct TRBufferList *ptListHead = NULL;

  const int16 iBufferMargin = 3838;

  struct TParseResult
  {
    BOOL      bSearchResult;
    int16     iStringEnd;
  };

/************************************************************
 * Local Function Prototypes                                */

  void      LOC_DeleteBuffer(
  struct TRBufferList *tBufferList);
  struct TParseResult LOC_Compare(
  struct TRBufferList *tBuffer,
  int16 iCharIndex);

/************************************************************
 * Exported Functions                                       *
 ************************************************************/

/************************************************************
 * Function: IO_BufferFlush                                 *
 * Description:                                             *
 * Flush the buffer for Port Handle                         *
 *                                                          *
 * Return value:                                            *
 * The funtion returns one of the defined return codes      *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-12-07    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  int8      IO_BufferFlush(
  int16 iDeviceNr /*HANDLE hPort */ )
  {
    struct TRBufferList *tBuffer;
    
    tBuffer = ptListHead;

    /* Find the end or the correct buffer */
    while ((tBuffer != NULL) && (tBuffer->tBuffer.iDeviceNr != iDeviceNr))
    {
      tBuffer = tBuffer->ptNext;
    }
    if (tBuffer == NULL)
      return BUFFER_NOT_FOUND;
    if (tBuffer->tBuffer.bBusy)       /* This buffer is used for */
    {
      return BUFFER_BUSY;     /* the moment, please wait */
    }
    else                        /* We are using the buffer right now */
    {
      tBuffer->tBuffer.bBusy = TRUE;
    }
    
    tBuffer->tBuffer.iBufferIndex = 0;
    tBuffer->tBuffer.bBusy = FALSE;
    return BUFFER_OK;
  }


/************************************************************
 * Function: IO_BufferCreate                                *
 * Description:                                             *
 * Creates a buffer for Port Handle                         *
 *                                                          *
 * Return value:                                            *
 * The funtion returns one of the defined return codes      *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-10-31    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  int8      IO_BufferCreate(
  /*HANDLE hPort, */ int16 iBufferSize,
  struct TProtocol * ptProtocol,
  int16 iDeviceNr)
  {
    struct TRBufferList tNewEntry;
    struct TRBufferList *ptTempListEntry;
    struct TRBufferList *ptAddEntry;

    /* Check for illegal function call */
    if (( /*hPort */ iDeviceNr == 0 /*NULL*/) || (iBufferSize == 0) ||
        (ptProtocol == NULL))
      return BUFFER_MISSING_ARGUMENT;

    /* Fill new entry */
    /* tNewEntry.tBuffer.hPort = hPort; */
    tNewEntry.tBuffer.bBusy = FALSE;
    tNewEntry.tBuffer.iBufferIndex = 0;
    tNewEntry.tBuffer.ptProtocol = ptProtocol;
    tNewEntry.tBuffer.iDeviceNr = iDeviceNr;
    tNewEntry.tBuffer.pBuffer = (uint8 *) malloc(iBufferSize + iBufferMargin);
    tNewEntry.ptNext = NULL;

    /* Get the last entry */
    if (ptListHead != NULL)
    {
      ptTempListEntry = ptListHead;

      while (ptTempListEntry->ptNext != NULL)
      {                         /* If the port already has a buffer -> error */
        if (ptTempListEntry->tBuffer.iDeviceNr /*hPort */  ==
            tNewEntry.tBuffer.iDeviceNr /*hPort */ )
          return BUFFER_ALREADY_EXCIST;
        else
          ptTempListEntry = ptTempListEntry->ptNext;
      }
      /* If the port already has a buffer -> error */
      if (ptTempListEntry->tBuffer.iDeviceNr /*hPort */  ==
          tNewEntry.tBuffer.iDeviceNr /*hPort */ )
        return BUFFER_ALREADY_EXCIST;
      else
      {
        /* The end is found, add the new entry */
        ptAddEntry =
          (struct TRBufferList *) malloc(sizeof(struct TRBufferList));
        if (ptAddEntry == NULL)
          return BUFFER_OUT_OF_MEMORY;  /* malloc failed */
        else
        {
          ptAddEntry->tBuffer = tNewEntry.tBuffer;
          ptAddEntry->ptNext = NULL;    /* Terminating List */
          ptTempListEntry->ptNext = ptAddEntry;
          return BUFFER_OK;
        }
      }
    }
    else
    {                           /* The list is empty add the new entry */
      ptAddEntry = (struct TRBufferList *) malloc(sizeof(struct TRBufferList));
      if (ptAddEntry == NULL)
        return BUFFER_OUT_OF_MEMORY;    /* malloc failed */
      else
      {
        ptAddEntry->tBuffer = tNewEntry.tBuffer;
        ptAddEntry->ptNext = NULL;      /* Terminating List */
        ptListHead = ptAddEntry;
        return BUFFER_OK;
      }
    }
  }

/************************************************************
 * Function: IO_BufferDeleteEntry                           *
 * Description:                                             *
 * Delete an entry in the buffer list                       *
 *                                                          *
 * Return value:                                            *
 * The funtion returns one of the defined return codes      *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-10-31    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  int8      IO_BufferDeleteEntry(
  int16 iDeviceNr /*HANDLE hPort */ )
  {
    struct TRBufferList *ptTempListEntry;
    struct TRBufferList *ptPrevListEntry;

    ptTempListEntry = ptListHead;
    ptPrevListEntry = ptListHead;

    if (ptTempListEntry != NULL)        /* List is already empty */
    {
      while (ptTempListEntry != NULL)
      {
        if (ptTempListEntry->tBuffer.iDeviceNr /*HANDLE hPort */  ==
            iDeviceNr /*HANDLE hPort */ )
        {
          free(ptTempListEntry->tBuffer.pBuffer);
          if (ptTempListEntry != ptListHead)    /* Is it the first entry? */
            ptPrevListEntry->ptNext = ptTempListEntry->ptNext;
          else
            ptListHead = ptTempListEntry->ptNext;
          free(ptTempListEntry);
          return BUFFER_OK;
        }
        ptPrevListEntry = ptTempListEntry;
        ptTempListEntry = ptTempListEntry->ptNext;
      }
    }
    return BUFFER_NOT_FOUND;
  }


/************************************************************
 * Function: IO_BufferDeleteAll                             *
 * Description:                                             *
 * Deletes the entire Buffer list                           *
 *                                                          *
 * Return value:                                            *
 * None (void)                                              *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-10-31    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  void      IO_BufferDeleteAll(
  void)
  {
    LOC_DeleteBuffer(ptListHead);
    ptListHead = NULL;
  }


/************************************************************
 * Function: IO_BufferAdd                                   *
 * Description:                                             *
 * Add chars to the buffer, parse the data and triggers     *
 * an BSEK_ReceiveString if the Protocol fits.              *
 *                                                          *
 * Return value:                                            *
 * A predefined Error Message.                              *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-10-31    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  int8      IO_BufferAdd(
  int16 iDeviceNr /*HANDLE hPort */ ,
  uint8 * pAddStr,
  int16 iCharCount,
  int *OSCrashDetected)
  {

    struct TRBufferList *tBuffer;
    int16     iCharIndex;
    enum PROT_Protocol
    { PROT_Command = 0x01,
      PROT_ACL = 0x02,
      PROT_SCO = 0x03,
      PROT_Event = 0x04
    };
    uint8    *pSendString;
    int8      iResult;
    int8      bBMoved;
    struct TParseResult tParseResult;
    int16     iMessageEnd;
	  int16		 *piLengthValue;

  
    tBuffer = ptListHead;

    /* Find the end of the correct buffer */
    while ((tBuffer != NULL) &&
           (tBuffer->tBuffer.iDeviceNr /*HANDLE hPort */  !=
            iDeviceNr /*HANDLE hPort */ ))
      tBuffer = tBuffer->ptNext;
    if (tBuffer == NULL)
      return BUFFER_NOT_FOUND;
    if (tBuffer->tBuffer.bBusy) /* This buffer is used for */
      return BUFFER_BUSY;       /* the moment, please wait */
    else                        /* We are using the buffer right now */
      tBuffer->tBuffer.bBusy = TRUE;

    /* So far the Buffer is ok      */
    iResult = BUFFER_OK;

#ifdef _NO_CONTROL_PANEL_
	  /* Add the characters to the Buffer */
	  iCharIndex = tBuffer->tBuffer.iBufferIndex;

	  /* Check if the Buffer can receive the message (buffer overflow) */
	  if( (iCharIndex + iCharCount) < (tBuffer->tBuffer.ptProtocol->iMaxLength + iBufferMargin))
	  {
		  memcpy(tBuffer->tBuffer.pBuffer + iCharIndex, pAddStr, iCharCount);
		  tBuffer->tBuffer.iBufferIndex = iCharIndex + iCharCount;

		  /* Time to check for message completion */
		  switch(tBuffer->tBuffer.ptProtocol->iProtocol)	/* Parse according to the Protocol */
		  {
		  case TOKEN_TERMINATED:
			  /****************************************************
			   * Token Terminated Protocol						*
			   ****************************************************/
			  /* iCharIndex starts (tokenlength+newly added characters) from he end	*/
			  /* of the Buffer at the most, from the beginning of the Buffer.			*/
			  iCharIndex = tBuffer->tBuffer.iBufferIndex - (iCharCount + tBuffer->tBuffer.ptProtocol->iTokenLength);
			  if(iCharIndex < 0)
				  iCharIndex = 0;
			  do	/* Repeat as long as the Buffer has moved */
			  {
				  /* Set the moved flag to False	*/
				  bBMoved = FALSE;

				  /* Check if the terminating string can fit in the buffer */
				  if(tBuffer->tBuffer.iBufferIndex >= tBuffer->tBuffer.ptProtocol->iTokenLength)
				  {
					  /* Test the tokens from iCharIndex to the end of the buffer */
					  /* as long as the tokenstring fits itself					*/
					  tParseResult = LOC_Compare(tBuffer, iCharIndex);

					  /* Did we find the Token String? */
					  if(tParseResult.bSearchResult)
					  {
						  pSendString = (uint8*)malloc(tParseResult.iStringEnd);
						  if(pSendString == NULL)
							  iResult = BUFFER_OUT_OF_MEMORY;	/* Malloc failed! */
						  else
						  {
							  /* Copy the message from the buffer to the Send String */
							  memcpy(pSendString, tBuffer->tBuffer.pBuffer, tParseResult.iStringEnd);
			  
							  /* Send the message to the Kernel */
							  BSEK_ReceiveString( tBuffer->tBuffer.iDeviceNr, 
                                    tParseResult.iStringEnd, 
                                    (char *)pSendString);

							  /* Are there any remaining characters in the Buffer? */
							  /* Move them to the beginning of the buffer */
							  if(tBuffer->tBuffer.iBufferIndex > tParseResult.iStringEnd)
							  {
								  iCharIndex = 0;
								  tParseResult.iStringEnd--;	/* To get the right start index */
								  do
								  {
									  tParseResult.iStringEnd++;
									  tBuffer->tBuffer.pBuffer[iCharIndex] = tBuffer->tBuffer.pBuffer[tParseResult.iStringEnd];
									  iCharIndex++;
								  }while(tParseResult.iStringEnd+1 < tBuffer->tBuffer.iBufferIndex);
								  /* Update the Buffer pointer */
								  tBuffer->tBuffer.iBufferIndex = iCharIndex;
		  
								  /* The Buffer has moved, do recheck for terminating strings	*/
								  bBMoved = TRUE;
								  iCharIndex = 0;	/* Check from the beginning of Buffer */
		  
							  }
							  else
							  {
								  /* No remaining characters reset the buffer pointer */
								  tBuffer->tBuffer.iBufferIndex = 0;
							  }
							  iResult = BUFFER_OK;
						  }
					  }
					  else
					  {	/* Buffer Overflow?	*/
						  if(tBuffer->tBuffer.iBufferIndex >= tBuffer->tBuffer.ptProtocol->iMaxLength)
							  tBuffer->tBuffer.iBufferIndex = 0;	/* Skip the message, reset Buffer */
						  /* No token string match */
						  iResult = BUFFER_NOT_FOUND;
					  }
				  }
			  }while(bBMoved);
			  break;
		  case ONE_LENGTH_BYTE:
			  /****************************************************
			   * ONE LENGTH BYTE Protocol							*
			   ****************************************************/
			  iResult = BUFFER_OK;
			  do
			  {	/* Do this over again if contents in the Buffer was moved	*/
				  bBMoved = FALSE;

				  /* The first byte contains the message length */

				  iMessageEnd = (int16)tBuffer->tBuffer.pBuffer[0]+1;
				  

				  /* Have we reached the message limit?	*/
				  if(tBuffer->tBuffer.iBufferIndex >= iMessageEnd )
				  {
					  /* Allocate, copy to, and send the message string to the Kernel	*/
					  pSendString = (uint8*)malloc(iMessageEnd);
					  if(pSendString == NULL)
						  iResult = BUFFER_OUT_OF_MEMORY;	/* Malloc failed! */
					  else
					  {
						  memcpy(pSendString, tBuffer->tBuffer.pBuffer, iMessageEnd);
					  
						  BSEK_ReceiveString(tBuffer->tBuffer.iDeviceNr, 
                                 iMessageEnd, 
                                 (char *)pSendString);
		  
						  /* Are there any remaining characters? Move them to the beginning	*/
						  iCharIndex = 0;
						  if(tBuffer->tBuffer.iBufferIndex > iMessageEnd)
						  {
							  memcpy(tBuffer->tBuffer.pBuffer, tBuffer->tBuffer.pBuffer + iMessageEnd, tBuffer->tBuffer.iBufferIndex - iMessageEnd);
							  iCharIndex = tBuffer->tBuffer.iBufferIndex - iMessageEnd;
							  bBMoved = TRUE;
						  }
						  tBuffer->tBuffer.iBufferIndex = iCharIndex;
						  iResult = BUFFER_OK;	/* Message sent (and moved) */
					  }
				  }
			  }while(bBMoved);
			  break;

		  case TWO_LENGTH_BYTES_LO_HI:
			  /****************************************************
			   * TWO LENGTH BYTE LO HI Protocol					*
			   ****************************************************/
			  iResult = BUFFER_OK;
			  do
			  {	/* Do this over again if contents in the Buffer was moved	*/
				  bBMoved = FALSE;

				  /* The first two bytes contains the message length, little endian style */

				  iMessageEnd = (int16)(tBuffer->tBuffer.pBuffer[0] + tBuffer->tBuffer.pBuffer[1] * 256) + 2;
				  

				  /* Have we reached the message limit?	*/
				  if(tBuffer->tBuffer.iBufferIndex >= iMessageEnd )
				  {
					  /* Allocate, copy to, and send the message string to the Kernel	*/
					  pSendString = (uint8*)malloc(iMessageEnd);
					  if(pSendString == NULL)
						  iResult = BUFFER_OUT_OF_MEMORY;	/* Malloc failed! */
					  else
					  {
						  memcpy((void*)pSendString, 
                     (void*)tBuffer->tBuffer.pBuffer, 
                     iMessageEnd);
					  
						  BSEK_ReceiveString(tBuffer->tBuffer.iDeviceNr, 
                                 iMessageEnd, 
                                 (char *)pSendString);

						  /* Are there any remaining characters? Move them to the beginning	*/
						  iCharIndex = 0;
						  if(tBuffer->tBuffer.iBufferIndex > iMessageEnd)
						  {
							  memcpy(tBuffer->tBuffer.pBuffer, tBuffer->tBuffer.pBuffer + iMessageEnd, tBuffer->tBuffer.iBufferIndex - iMessageEnd);
							  iCharIndex = tBuffer->tBuffer.iBufferIndex - iMessageEnd;
							  bBMoved = TRUE;
						  }
						  tBuffer->tBuffer.iBufferIndex = iCharIndex;
						  iResult = BUFFER_OK;	/* Message sent (and moved) */
					  }
				  }
			  }while(bBMoved);
			  break;

		  case TWO_LENGTH_BYTES_HI_LO:
			  /****************************************************
			   * TWO LENGTH BYTES HI LO Protocol					*
			   ****************************************************/
			  iResult = BUFFER_OK;
			  do
			  {	/* Do this over again if contents in the Buffer was moved	*/
				  bBMoved = FALSE;

				  /* The first two bytes contains the message length, big endian style */

				  iMessageEnd = (int16)(tBuffer->tBuffer.pBuffer[0] * 256 + tBuffer->tBuffer.pBuffer[1]) + 2;
				  

				  /* Have we reached the message limit?	*/
				  if(tBuffer->tBuffer.iBufferIndex >= iMessageEnd )
				  {
					  /* Allocate, copy to, and send the message string to the Kernel	*/
					  pSendString = (uint8*)malloc(iMessageEnd);
					  if(pSendString == NULL)
						  iResult = BUFFER_OUT_OF_MEMORY;	/* Malloc failed! */
					  else
					  {
						  memcpy((void*)pSendString, 
                     (void*)tBuffer->tBuffer.pBuffer, 
                     iMessageEnd);
						  
						  BSEK_ReceiveString(tBuffer->tBuffer.iDeviceNr, 
                                 iMessageEnd, 
                                 (char *)pSendString);
		  
						  /* Are there any remaining characters? Move them to the beginning	*/
						  iCharIndex = 0;
						  if(tBuffer->tBuffer.iBufferIndex > iMessageEnd)
						  {
							  memcpy(tBuffer->tBuffer.pBuffer, tBuffer->tBuffer.pBuffer + iMessageEnd, tBuffer->tBuffer.iBufferIndex - iMessageEnd);
							  iCharIndex = tBuffer->tBuffer.iBufferIndex - iMessageEnd;
							  bBMoved = TRUE;
						  }
						  tBuffer->tBuffer.iBufferIndex = iCharIndex;
						  iResult = BUFFER_OK;	/* Message sent (and moved) */
					  }
				  }
			  }while(bBMoved);
			  break;

		  case FIXED_LENGTH:
			  /****************************************************
			   * FIXED LENGTH Protocol							*
			   ****************************************************/
			  iResult = BUFFER_OK;
			  do
			  {	/* Do this over again if contents in the Buffer was moved	*/
				  bBMoved = FALSE;

				  /* Have we reached the message limit	*/
				  if(tBuffer->tBuffer.iBufferIndex >= tBuffer->tBuffer.ptProtocol->iMaxLength)
				  {
					  /* Allocate, copy to, and send the message string to the Kernel	*/
					  pSendString = (uint8*)malloc(tBuffer->tBuffer.ptProtocol->iMaxLength);
					  if(pSendString == NULL)
						  iResult = BUFFER_OUT_OF_MEMORY;	/* Malloc failed! */
					  else
					  {
						  memcpy((void*)pSendString, 
                     (void*)tBuffer->tBuffer.pBuffer, 
                     tBuffer->tBuffer.ptProtocol->iMaxLength);
					  
						  BSEK_ReceiveString(tBuffer->tBuffer.iDeviceNr, 
                                 tBuffer->tBuffer.ptProtocol->iMaxLength, 
                                 (char *)pSendString);
		  
						  /* Are there any remaining characters? Move them to the beginning	*/
						  iCharIndex = 0;
						  if(tBuffer->tBuffer.iBufferIndex > tBuffer->tBuffer.ptProtocol->iMaxLength)
						  {
							  memcpy(tBuffer->tBuffer.pBuffer, tBuffer->tBuffer.pBuffer + tBuffer->tBuffer.ptProtocol->iMaxLength, tBuffer->tBuffer.iBufferIndex - tBuffer->tBuffer.ptProtocol->iMaxLength);
							  iCharIndex = tBuffer->tBuffer.iBufferIndex - tBuffer->tBuffer.ptProtocol->iMaxLength;
							  bBMoved = TRUE;
						  }
						  tBuffer->tBuffer.iBufferIndex = iCharIndex;
						  iResult = BUFFER_OK;	/* Message sent (and moved) */
					  }
				  }
			  }while(bBMoved);
			  break;

		  case LOFAR:
			  /****************************************************
			   * LOFAR Protocol							                      *
			   ****************************************************/
			  iResult = BUFFER_OK;
			  do
			  {	/* Do this over again if contents in the Buffer was moved	*/
				  bBMoved = FALSE;

				  /*uiMessageEnd = (int16)tBuffer->tBuffer.pBuffer[2]+6; */
          piLengthValue = ((int16*)(tBuffer->tBuffer.pBuffer+2));
				  iMessageEnd = (int16)*piLengthValue;

          /* Length does not include the header en lenght occupation */
          iMessageEnd +=6; 

				  /* Have we reached the message limit?	*/
				  if(tBuffer->tBuffer.iBufferIndex >= iMessageEnd )
				  {
					  /* Allocate, copy to, and send the message string to the Kernel	*/
					  pSendString = (uint8*)malloc(iMessageEnd);
					  if(pSendString == NULL)
						  iResult = BUFFER_OUT_OF_MEMORY;	/* Malloc failed! */
					  else
					  {
						  memcpy((void*)pSendString, 
                     (void*)tBuffer->tBuffer.pBuffer, 
                     iMessageEnd);
					  
						  BSEK_ReceiveString(tBuffer->tBuffer.iDeviceNr, 
                                 iMessageEnd, 
                                 (char *)pSendString);
		  
						  /* Are there any remaining characters? Move them to the beginning	*/
						  iCharIndex = 0;
						  if(tBuffer->tBuffer.iBufferIndex > iMessageEnd)
						  {
							  memcpy(tBuffer->tBuffer.pBuffer, tBuffer->tBuffer.pBuffer + iMessageEnd, tBuffer->tBuffer.iBufferIndex - iMessageEnd);
							  iCharIndex = tBuffer->tBuffer.iBufferIndex - iMessageEnd;
							  bBMoved = TRUE;
						  }
						  tBuffer->tBuffer.iBufferIndex = iCharIndex;
						  iResult = BUFFER_OK;	/* Message sent (and moved) */
              
					  }
				  }
			  }while(bBMoved);
			  break;

		  default:
			  iResult = BUFFER_NOT_FOUND;
			  break;
		  }
	  }
	  else
	  {
		  /* New message to big for the defined buffer */ 
      LogLine("Message received to big for buffer maybe you should update the IO file!");
		  tBuffer->tBuffer.iBufferIndex = 0; /* Ignore the new message and reset the buffer */
		  iResult = BUFFER_OUT_OF_MEMORY;
	  }
#else
    if (iCharCount > 0)
    {
      /* Add the characters to the Buffer */
      iCharIndex = tBuffer->tBuffer.iBufferIndex;

      /* Check if the Buffer can receive the message (buffer overflow) */
      if ((iCharIndex + iCharCount) <
          (tBuffer->tBuffer.ptProtocol->iMaxLength + iBufferMargin))
      {
        memcpy(tBuffer->tBuffer.pBuffer + iCharIndex, pAddStr, iCharCount);
        tBuffer->tBuffer.iBufferIndex = iCharIndex + iCharCount;
      }
      else
      {
        /* New message to big for the defined buffer */
        tBuffer->tBuffer.iBufferIndex = 0;      /* Ignore the new message and reset the buffer */
        iResult = BUFFER_OUT_OF_MEMORY;
      }
    }
    else
    {
      /* Empty message received. Empty message indicates the end of the protocol message */

      pSendString = malloc(tBuffer->tBuffer.iBufferIndex);
      memcpy(pSendString, tBuffer->tBuffer.pBuffer,
             tBuffer->tBuffer.iBufferIndex);
      BSEK_ReceiveString(tBuffer->tBuffer.iDeviceNr,
                         tBuffer->tBuffer.iBufferIndex, pSendString);
      tBuffer->tBuffer.iBufferIndex = 0;
    }
#endif /* _NO_CONTROL_PANEL_ */
    tBuffer->tBuffer.bBusy = FALSE;
    return iResult;
  }



/************************************************************
 * LOCAL FUNCTIONS                                          *
 ************************************************************/

/************************************************************
 * Function: LOC_DeleteBuffer                               *
 * Description:                                             *
 * Local recursive function that deletes the entire         *
 * Buffer list.                                             *
 *                                                          *
 * Return value:                                            *
 * None (void)                                              *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-10-31    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  void      LOC_DeleteBuffer(
  struct TRBufferList *tBufferList)
  {
    if (tBufferList != NULL)
    {
      LOC_DeleteBuffer(tBufferList->ptNext);
      free(tBufferList->tBuffer.pBuffer);
      free(tBufferList);
    }
  }


/************************************************************
 * Function: LOC_Compare                                    *
 * Description:                                             *
 * Local recursive function that compares a number of chars *
 * in the Buffer list at a given location.                  *
 *                                                          *
 * Return value:                                            *
 * True if identical                                        *
 *                                                          *
 * Revision:                                                *
 * Date        Comment                  Author              *
 * 00-11-01    Created                  AUS/Dennis Olausson *
 *                                                          *
 ************************************************************/
  struct TParseResult LOC_Compare(
  struct TRBufferList *tBuffer,
  int16 iCharPointer)
  {
    struct TParseResult tParseResult;
    int16     iSearchEnd;
    int16     iTokenPointer;


    if (tBuffer->tBuffer.iBufferIndex > tBuffer->tBuffer.ptProtocol->iMaxLength)
      iSearchEnd =
        tBuffer->tBuffer.ptProtocol->iMaxLength -
        tBuffer->tBuffer.ptProtocol->iTokenLength;
    else
      iSearchEnd =
        tBuffer->tBuffer.iBufferIndex -
        tBuffer->tBuffer.ptProtocol->iTokenLength;
    do
    { /* Check for the tokenstring at iCharPointer, if found the end of the message is saved */
      tParseResult.bSearchResult = TRUE;
      for (iTokenPointer = iCharPointer;
           iTokenPointer <
           iCharPointer + tBuffer->tBuffer.ptProtocol->iTokenLength;
           iTokenPointer++)
        tParseResult.bSearchResult = tParseResult.bSearchResult &&
          (tBuffer->tBuffer.pBuffer[iTokenPointer] ==
           tBuffer->tBuffer.ptProtocol->pTerminateToken[iTokenPointer -
                                                        iCharPointer]);
      tParseResult.iStringEnd = iTokenPointer;
      iCharPointer++;
    } while (!tParseResult.bSearchResult && iCharPointer <= iSearchEnd);

    return tParseResult;

  }


#ifdef __cplusplus
}
#endif /* __cplusplus */
