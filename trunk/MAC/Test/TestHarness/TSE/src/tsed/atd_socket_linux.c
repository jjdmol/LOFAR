/*****************************************************************************/
/*                                                                           */
/* File       : atd_socket.c                                                 */
/*                                                                           */
/* Start date : 2005-03-15                                                   */
/*                                                                           */
/* Author     : Ordina TA, Richard van de Bunte				                       */
/*                                                                           */
/* Version    : 0.0                                                          */
/*                                                                           */
/* Description: Multithreaded named socket server for AutoTest DRIVER.       */
/*              Muliple clients and servers are allowed to or can wait for to*/
/*              connect.                                                     */
/*              The settings for a connection is provided by the parser.     */
/*              The settings are administrated using the SocketData          */
/*              structure. When duplicate settings, like two servers on the  */
/*              same IP address and port number, are found then there will   */
/*              be no new SocketData structure created but a new the         */
/*              ConnectData will be created. Thus there will be no duplicate */
/*              settings in the administration.                              */  
/*              Creates a thread for receiving data on a socket.             */
/*              The data claimed during setup is released at SOCKET_CLOSE.   */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "codingstandard.h"
#include "atdriver.h"
#include "atd_socket.h"
#include "dbgtrace.h"

/*****************************************************************************/
/*                                                                           */
/* Type definitions                                                          */
/*                                                                           */
/*****************************************************************************/
#define NO_FLAGS	                ((int) 0)
#define PENDING_MAX               ((int) 1)
#define MAX_NR_CONNECTIONS        ( 20 )
#define MAX_NR_SETTINGS           ( 10 )
#define MAX_LENGTH_SETTINGS       ( 80 )
#define ATD_SOCKET_MUTEX          ( (char*) "ATD_SOCKET Mutex")
#define INVALID_SOCKET            (  -1)
#define NO_ERROR                  (   0)
#define ERROR                     (!NO_ERROR)
#define SOCKET_CONNECTED          ((int16) 1)
#define SOCKET_DISCONNECTED       ((int16) 0)
#define SOCKET_ERROR              (       -1)
/* The data needed for communicating via a socket. */
typedef struct _ConnectionData
{
  int16     iDeviceNumber; /* Number of the connection used by the Kernel */            
	int       tSocket;       /* The socket data */
	pthread_t tRxThread;     /* The receive thread ID */
  BOOL      bThreadRunning;
  int16     iConnected;    /* Status of the connection */
}ConnectionData;

/* The data stored for a  */
typedef struct _SocketData 
{
  uint16              uiNoConnections;                      /* Number of connections */
  ConnectionData      atConnectionData[MAX_NR_CONNECTIONS];
	uint16				      uiNoSettings;
	char				        pcSettings[MAX_LENGTH_SETTINGS];
	char				       *ppcSettings[MAX_NR_SETTINGS];
	pthread_t           tConnectThread;
  BOOL                bConnectThreadRunning;
	struct _SocketData *ptNext;
	struct _SocketData *ptPrevious;
} SocketData;

typedef SocketData *pSocketData;


/*****************************************************************************/
/*                                                                           */
/* Constants                                                                 */
/*                                                                           */
/*****************************************************************************/
#define MAX_SOCKET_NAME_SIZE          (     25)
#define MAX_SOCKET_INSTANCES          (      1)
#define TXD_BUF_SIZE                  ( 204800)
#define RXD_BUF_SIZE                  ( 204800)
#define SOCKET_TIMEOUT                (      0)
#define LISTEN_SLEEP_TIME             (    100)
#define CONNECT_SLEEP_TIME            (   1000)

/*****************************************************************************/
/*                                                                           */
/* External references                                                       */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/* Variables                                                                 */
/*                                                                           */
/*****************************************************************************/

static pSocketData	    ptAdminSocketData = NULL;
static pthread_mutex_t  tSocketDataMutex  = PTHREAD_MUTEX_INITIALIZER;

/*****************************************************************************/
/*                                                                           */
/* Functions                                                                 */
/*                                                                           */
/*****************************************************************************/
static void			   *SocketRxThread				      ( void * );
static void			   *ServerWaitForConnectThread	( void * ); 
static void			   *ClientWaitForConnectThread	( void * ); 
static int16		    SocketServerWaitForConnect	( pSocketData );
static int16		    SocketClientTryToConnect	  ( pSocketData );
static void         GetSocketData               ( int16, 
                                                  pSocketData *pptSocketData, 
                                                  uint16 *puiConnectionId);
static void         CreateSocketData            ( pSocketData *pptSocketData, 
                                                  char        *pcSettings);
static int			    RemoveSocketData			      ( pSocketData );
static int16        CloseConnection             ( int16 iDeviceNumber );
static int16			  ParseSocketSettings			    ( char    *pcSettings, 
												                          char    *ppcReturnSettings[],
												                          uint16  *uiNoSettings);
static void			    FreeSocketSettings			    ( char    *ppcReturnSettings[], 
												                          uint16   uiNoSettings );

/* SOCKET_Init, creates the Mutex needed for the administration and starts  */
/*   Windows Sockets.                                                       */
/* ------------------------------------------------------------------------ */
/* Input       :                                                            */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value:                                                            */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
void SOCKET_Init( void )
{

}

/* SOCKET_Open Updates the administration and when the socket is a client   */
/*   it will try to connect.                                                */
/* ------------------------------------------------------------------------ */
/* Input       :                                                            */
/*   DeviceNumber - The number of the device provided by the                */
/*               Kernel.                                                    */
/*   Settings - Settings of the socket format:                              */
/*              <client/server> <ip address/hostname> <port number>         */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value:                                                            */
/*   int16 - ATD_SOCKET_OK, when successfull, ATD_SOCKET_ERROR otherwise    */
/* --                                                                       */
/* Remarks:                                                                 */
/*   Server sockets are administrated but not started. The server is        */ 
/*   started after ConfigurationDone is invoked, because the server does not*/
/*   know how many connections is must accept at this point.                */
/* ------------------------------------------------------------------------ */
int16 SOCKET_Open( int16 iDeviceNumber, char *pcSettings )
{
  int   				  tSocket;
  int16           iResult;
	int					    iOperationResult;
	struct hostent *ptServerInfo;
	pSocketData			ptSocketData;	
  char           *pcErrorString;

	tSocket			 = INVALID_SOCKET;
	ptServerInfo = NULL;
  iResult      = ATD_SOCKET_ERROR;

  /* Creates a new socket data when needed, otherwise the administration is updated */
	CreateSocketData( &ptSocketData, pcSettings );
	if( NULL != ptSocketData )
	{					
    ptSocketData->atConnectionData[(ptSocketData->uiNoConnections-1)].iDeviceNumber =
      iDeviceNumber;
		iOperationResult = strcmp(ptSocketData->ppcSettings[0], "server" );
    /* When not a server, check if it is a client */
		if( 0 != iOperationResult )
		{
			iOperationResult = strcmp(ptSocketData->ppcSettings[0], "client" );
      /* If not a client report error */
			if( 0 != iOperationResult )
			{
	      DbgTrace2( "SOCKET_Open() invalid setting %s must be \"client\"/\"server\"\n", 
                    ptSocketData->ppcSettings[0]);
				RemoveSocketData(ptSocketData);
        /* Notify the test script of a failure */
        pcErrorString = malloc( (strlen("SOCKET_UNKNOWN_ERROR")+1) * sizeof(char));
        strcpy( pcErrorString,"SOCKET_UNKNOWN_ERROR"); 
        BSED_InternalEvent(iDeviceNumber,pcErrorString);
      }
      else
      {
        iResult = ATD_SOCKET_OK;
      }
		}
    else
    {
      iResult = ATD_SOCKET_OK;
    }
	}
	return(iResult);
}

/* SOCKET_Close closes the socket corresponding to the DeviceNumber         */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*   DeviceNumber - the device to which the data is sent.                   */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value:                                                            */
/*   int16 - ATD_SOCKET_OK, when successfull, ATD_SOCKET_ERROR otherwise    */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
int16 SOCKET_Close( int16 iDeviceNumber )
{
  BOOL		     bSuccess;
  int16		     iResult;
  int32        iOperationResult;
  uint16       uiConnectionId;
	pSocketData	 ptSocketData;
  char        *pcErrorString;

  iResult		   = ATD_SOCKET_ERROR;
	GetSocketData( iDeviceNumber, &ptSocketData, &uiConnectionId );

  if (NULL != ptSocketData)
	{
		DbgTrace1( "SOCKET_Close() called...\n" );

		if (TRUE == 
      ptSocketData->atConnectionData[uiConnectionId].bThreadRunning)
		{
			/* Kill thread that handles pipe client */
			if( pthread_cancel( 
             ptSocketData->atConnectionData[uiConnectionId].tRxThread) == 0)
      {
        bSuccess = TRUE;
      }
      else
      {
        bSuccess = FALSE;
      }
		}
		if (FALSE == bSuccess)
		{
			DbgTrace2( "SOCKET_Close():TerminateReceiveThread():ERROR:%s\n", 
                 strerror(errno) );
		}
    if (INVALID_SOCKET != 
      ptSocketData->atConnectionData[uiConnectionId].tSocket)
		{
 			/* Close connection socket returned after connect or accept */
			iOperationResult = close( 
        ptSocketData->atConnectionData[uiConnectionId].tSocket );

			if (0 != iOperationResult)
			{
				DbgTrace2( "SOCKET_Close():DisconnectNamedSocket():ERROR:%s\n", 
                   strerror(errno) );
			}
      else
      {
       pcErrorString = malloc( (strlen("CONNECTION_LOST")+1) * sizeof(char));
       strcpy( pcErrorString,"CONNECTION_LOST"); 
       BSED_InternalEvent(iDeviceNumber,pcErrorString);
      }
		}
    ptSocketData->atConnectionData[uiConnectionId].iConnected = SOCKET_DISCONNECTED;
    ptSocketData->uiNoConnections--;
    if (0 == ptSocketData->uiNoConnections)
    {
      if (TRUE == ptSocketData->bConnectThreadRunning)
		  {
			  /* Kill thread that handles connection of pipe client */
			  pthread_cancel( ptSocketData->tConnectThread );
		  }
      else
      {
			  DbgTrace2( "SOCKET_Close():TerminateConnectThread():ERROR:%s\n", 
                  strerror(errno) );
		  }
		  FreeSocketSettings(	ptSocketData->ppcSettings,
										      ptSocketData->uiNoSettings);
		  if (ATD_SOCKET_OK == RemoveSocketData(ptSocketData))
      {
        /* Clean up after the last administration removed */
        pthread_mutex_destroy( &tSocketDataMutex );
      }
    }
	}
  DbgTrace2( "SOCKET_Close() returns %d\n", iResult );

  return( iResult );
}

/* SOCKET_SendData sends data to the specified DeviceNumber.                */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*   DeviceNumber - the device to which the data is sent.                   */
/*   ByteCount    - Number of bytes to send                                 */
/*   Data         - Data to transmit                                        */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value:                                                            */
/*   int16 - ATD_SOCKET_OK, when successfull, ATD_SOCKET_ERROR otherwise    */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
int16 SOCKET_SendData(int16   iDeviceNumber,
						          int16	  iByteCount,
						          uint8	 *pData )
{
  uint8	  	   chWriteBuf[TXD_BUF_SIZE]; 
  int			     iWriteBytes;
  int 		     iWritten;
  int16		     iReturn;
  int16		     i;
  uint16       uiConnectionId;
	pSocketData	 ptSocketData;
  char        *pcErrorString;
		
	iReturn = ATD_SOCKET_ERROR;

  /* Retreive the socket data */
	GetSocketData( iDeviceNumber, &ptSocketData, &uiConnectionId);

  /* When data found */
	if( NULL != ptSocketData )
	{
    /* and there is still a connection */
		if( SOCKET_CONNECTED == ptSocketData->atConnectionData[uiConnectionId].iConnected)
		{
			DbgTrace3( "SOCKET_SendData() sending %d bytes on handle %d...\n", 
                  iByteCount, 
                  ptSocketData->atConnectionData[uiConnectionId].tSocket );
  
			if ( iByteCount > TXD_BUF_SIZE )
			{
				DbgTrace1( "SOCKET_SendData():Buffer overflow:%d\n" );
			}
			else
			{
				iWriteBytes = iByteCount;
				memcpy( chWriteBuf, pData, iWriteBytes );

        if (iByteCount < 200)
        {
  				for ( i=0; i<iByteCount; i++ )
          {
  					DbgTrace3( "%c 0x%02x\n", pData[i], pData[i] );
          }
        }
        else
        {
          for ( i=(iByteCount-30); i<iByteCount; i++ )
          {
           DbgTrace3( "%c 0x%02x\n", pData[i], pData[i] );
          }
        }
				iWritten = send( ptSocketData->atConnectionData[uiConnectionId].tSocket, /* handle to pipe */
									       chWriteBuf,                       /* buffer to write from */
									       iWriteBytes,                      /* number of bytes to write */
									       NO_FLAGS );                       /* overlapped I/O */

				if (SOCKET_ERROR != iWritten)
        {
 					DbgTrace3( "SOCKET_SendData():WriteFile():%d(%d) bytes\n", 
                    iWritten, iWriteBytes );
					if ( iWritten == iWriteBytes )
					{
						iReturn = ATD_SOCKET_OK;
					}
				}
				else
				{
					switch( errno )
					{
						case EBADF :	
              CloseConnection(ptSocketData->atConnectionData[uiConnectionId].iDeviceNumber);
              DbgTrace1( "SOCKET_SendData() using bad file descriptor\n");
		          if (TRUE == 
                ptSocketData->atConnectionData[uiConnectionId].bThreadRunning)
		          {
			          /* Kill thread that handles pipe client */
			          pthread_cancel( 
                  ptSocketData->atConnectionData[uiConnectionId].tRxThread);
		          }
              pcErrorString = malloc( (strlen("SOCKET_SEND_ERROR")+1) * sizeof(char));
              strcpy( pcErrorString,"SOCKET_SEND_ERROR"); 
              BSED_InternalEvent(iDeviceNumber,pcErrorString);
							break;
						case ENOTSOCK:
              CloseConnection(ptSocketData->atConnectionData[uiConnectionId].iDeviceNumber);
              DbgTrace1( "SOCKET_SendData() not using a socket\n");
		          if (FALSE == 
                ptSocketData->atConnectionData[uiConnectionId].bThreadRunning)
		          {
			          /* Kill thread that handles pipe client */
			          pthread_cancel( 
                  ptSocketData->atConnectionData[uiConnectionId].tRxThread);
		          }
              pcErrorString = malloc( (strlen("SOCKET_SEND_ERROR")+1) * sizeof(char));
              strcpy( pcErrorString,"SOCKET_SEND_ERROR"); 
              BSED_InternalEvent(iDeviceNumber,pcErrorString);
							break;
						case EFAULT:
              CloseConnection(ptSocketData->atConnectionData[uiConnectionId].iDeviceNumber);
              DbgTrace1( "SOCKET_SendData() no user space data is sent\n");
		          if (TRUE == 
                ptSocketData->atConnectionData[uiConnectionId].bThreadRunning)
		          {
			          /* Kill thread that handles pipe client */
			          pthread_cancel( 
                  ptSocketData->atConnectionData[uiConnectionId].tRxThread);
		          }
              pcErrorString = malloc( (strlen("SOCKET_SEND_ERROR")+1) * sizeof(char));
              strcpy( pcErrorString,"SOCKET_SEND_ERROR"); 
              BSED_InternalEvent(iDeviceNumber,pcErrorString);
							break;

						case ENOMEM:
              CloseConnection(ptSocketData->atConnectionData[uiConnectionId].iDeviceNumber);
              DbgTrace1( "SOCKET_SendData() invalid parameter provided\n");
		          if (TRUE == 
                ptSocketData->atConnectionData[uiConnectionId].bThreadRunning)
		          {
			          /* Kill thread that handles pipe client */
			          pthread_cancel( 
                  ptSocketData->atConnectionData[uiConnectionId].tRxThread );
		          }
              pcErrorString = malloc( (strlen("SOCKET_SEND_ERROR")+1) * sizeof(char));
              strcpy( pcErrorString,"SOCKET_SEND_ERROR"); 
              BSED_InternalEvent(iDeviceNumber,pcErrorString);
							break;

						default:
							DbgTrace2( "SOCKET_SendData():WriteFile():ERROR:%s\n", 
                         strerror(errno) );
              pcErrorString = malloc( (strlen("SOCKET_SEND_ERROR")+1) * 
                                      sizeof(char));
              strcpy( pcErrorString,"SOCKET_SEND_ERROR"); 
              BSED_InternalEvent(iDeviceNumber,pcErrorString);
							break;
					} /* end switch( dwError ) */
				} /* end else !fSuccess */

			} /* end else */
		}
	}

  DbgTrace2( "SOCKET_SendData() returns %d\n", iReturn );

  return( iReturn );
}

/* SOCKET_StartConfiguration starts all of the TCP/IP servers sepcified in  */
/* the IO file	                                                            */
/* ------------------------------------------------------------------------ */
/* Input parameters : None                                                  */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value:                                                            */
/* None                                                                     */
/* --                                                                       */
/* Remarks:                                                                 */
/* This function is intended to be called from the kernel, to start the     */
/* TCP/IP servers specified in the IO file                                  */
/* ------------------------------------------------------------------------ */
int16 SOCKET_StartConfiguration( void )
{
  pSocketData   ptSocketData;
  char         *pcServerString;
  char         *pcTempPointer;
  char         *pcErrorString;
  int16         iError;
  int16         iResult;
  uint16        uiCounter;

	ptSocketData = ptAdminSocketData;
  iError       = ATD_SOCKET_OK;
  
	while ((ptSocketData != NULL) && (iError == ATD_SOCKET_OK))
	{
    /* lower all characters better for comparison */
		pcTempPointer = ptSocketData->ppcSettings[0];
		while( 0 != *pcTempPointer )
		{
			*pcTempPointer = (char) tolower( (int) *pcTempPointer );
			*pcTempPointer++;
		}
    pcServerString = strstr( ptSocketData->ppcSettings[0],
                             "server");
    if (NULL != pcServerString)
		{
			iError = SocketServerWaitForConnect(ptSocketData);
			if( ATD_SOCKET_ERROR == iError)
			{
        DbgTrace2( "Socket_Open() Could not open socket of %s", 
                    ptSocketData->pcSettings);
        FreeSocketSettings(	ptSocketData->ppcSettings,
										        ptSocketData->uiNoSettings);
				RemoveSocketData(ptSocketData);
        /* Update all of the devices listed in the connection */
        for (uiCounter = 0; uiCounter < ptSocketData->uiNoConnections; uiCounter++)
        {
          pcErrorString = malloc( (strlen("SOCKET_ACCEPT_ERROR")+1) * sizeof(char));
          strcpy( pcErrorString,"SOCKET_ACCEPT_ERROR"); 
          BSED_InternalEvent( ptSocketData->atConnectionData[uiCounter].iDeviceNumber, 
                              pcErrorString);
        }
      }
		}
    else
    {
			iError = SocketClientTryToConnect(ptSocketData);
			if( ATD_SOCKET_ERROR == iError)
      {
        /* Update all of the devices listed in the connection */
        for (uiCounter = 0; uiCounter < ptSocketData->uiNoConnections; uiCounter++)
        {
          pcErrorString = malloc( (strlen("SOCKET_ACCEPT_ERROR")+1) * sizeof(char));
          strcpy( pcErrorString,"SOCKET_ACCEPT_ERROR"); 
          BSED_InternalEvent( ptSocketData->atConnectionData[uiCounter].iDeviceNumber, 
                              pcErrorString);
        }
        DbgTrace2( "Socket_Open() Could not open socket of %s", 
                    ptSocketData->pcSettings);
        FreeSocketSettings(	ptSocketData->ppcSettings,
										        ptSocketData->uiNoSettings);
				RemoveSocketData(ptSocketData);
      }
    }
		ptSocketData = (pSocketData) ptSocketData->ptNext;
	}
  /* Setting the error condition */
  iResult = iError;

  return(iResult);
}

/* ServerWaitForConnectThread, is the thread for accepting clients on a     */
/*   server socket. After a client has connected a thread is started to     */
/*   receive data.                                                          */
/* the IO file	                                                            */
/* ------------------------------------------------------------------------ */
/* Input parameters : ptSocketData- Data of the server socket.              */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value:                                                            */
/* None                                                                     */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
void *ServerWaitForConnectThread( void * lpvParam ) 
{
  int                 tSocket;
  pSocketData			    ptSocketData;
  SocketData          tSocketData;
  int                 iConnectionSocket;
  int 				        iErrorLoop;
	uint16				      uiPortNumber;
	struct sockaddr_in	tSocketAddress;
	int					        iResult;
	unsigned long		    ulIpAddress;
	struct hostent	   *ptHostEnt;
  uint16              uiConnectionIndex;
	BOOL				        bHostAddressValid;
  char               *pcErrorString;
	
	ptSocketData = (pSocketData)lpvParam; 
  memcpy( (void*)&tSocketData, (void*)ptSocketData, sizeof(SocketData));
 	DbgTrace2( "ServerWaitForConnectThread() Waiting for Client(s) on port %s\n",
              tSocketData.ppcSettings[2]);
	/* Socket setup */
	/* when first charachter is an alpha character then a hostname was provided */
	if( isalpha(tSocketData.ppcSettings[1][0]) )
	{
		ptHostEnt = gethostbyname( tSocketData.ppcSettings[1] );
		if( NULL != ptHostEnt)
		{
			memcpy(&tSocketAddress.sin_addr,ptHostEnt->h_addr,ptHostEnt->h_length);
			bHostAddressValid = TRUE;
		}
		else
		{
			bHostAddressValid = FALSE;
		}
	}
	/* Otherwise an IP address was provided */
	else
	{
		ulIpAddress = htonl(inet_addr(tSocketData.ppcSettings[1]));
		if (INADDR_NONE != ulIpAddress)
		{
			tSocketAddress.sin_addr.s_addr	= htonl(ulIpAddress);
			bHostAddressValid = TRUE;
		}
		else
		{
			bHostAddressValid = FALSE;
		}
	}

	if ( TRUE == bHostAddressValid )
	{
		/* Socket creation */
		tSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET != tSocket ) 
    {
		  sscanf(tSocketData.ppcSettings[2], "%u", &uiPortNumber);		
		  tSocketAddress.sin_family		= AF_INET;
		  tSocketAddress.sin_port			= htons((short)uiPortNumber);

		  /* binding the Socket to network connection */
		  iResult = bind( tSocket, 
			               (struct sockaddr *) &tSocketAddress, 
			               sizeof(struct sockaddr));
		  if ( SOCKET_ERROR != iResult ) 
		  {
			  /* Listen for client connection on the socket */
			  iResult = listen( tSocket, PENDING_MAX);
			  if ( SOCKET_ERROR != iResult ) 
			  {
          /* The first devicenumber registered will get the first connection */
          uiConnectionIndex = 0;
          iErrorLoop = NO_ERROR;
          while ( (uiConnectionIndex < tSocketData.uiNoConnections) &&
                  (NO_ERROR == iErrorLoop) )
          {
				    /* Accept some communication on the initial socket not interested in the address */
            /* providing NULL                                                                */
				    iConnectionSocket = accept( tSocket, NULL, NULL);

				    if (SOCKET_ERROR != iConnectionSocket) 
            {
              /* Enter critical section */
              pthread_mutex_lock( &tSocketDataMutex );

              ptSocketData->atConnectionData[uiConnectionIndex].tSocket = 
                iConnectionSocket;
              ptSocketData->atConnectionData[uiConnectionIndex].iConnected = 
                SOCKET_CONNECTED;
  		       /* Create a thread for this client */
              iResult = pthread_create( 
                  &ptSocketData->atConnectionData[uiConnectionIndex].tRxThread,
                  NULL,
                  SocketRxThread,
              		(void *)&ptSocketData->atConnectionData[uiConnectionIndex]);         /* thread parameter */

              if (NO_ERROR == iResult)
					    {
                ptSocketData->atConnectionData[uiConnectionIndex].bThreadRunning = TRUE;
                pcErrorString = malloc( (strlen("CONNECTION_ESTABLISHED")+1) * sizeof(char));
                strcpy( pcErrorString,"CONNECTION_ESTABLISHED"); 
                BSED_InternalEvent(ptSocketData->atConnectionData[uiConnectionIndex].iDeviceNumber,
                  pcErrorString);

              }
              else
              {
                iErrorLoop = ERROR;
                ptSocketData->atConnectionData[uiConnectionIndex].bThreadRunning = FALSE;
                pcErrorString = malloc( (strlen("SOCKET_ACCEPT_ERROR")+1) * sizeof(char));
                strcpy( pcErrorString,"SOCKET_ACCEPT_ERROR"); 
                BSED_InternalEvent(ptSocketData->atConnectionData[uiConnectionIndex].iDeviceNumber,
                                   pcErrorString);
						    /* Not possible to create a thread for this client, close the socket */
						    DbgTrace2( "ServerWaitForConnectThread():CreateThread():ERROR %s\n", 
                            strerror(errno) );
                CloseConnection(ptSocketData->atConnectionData[uiConnectionIndex].iDeviceNumber);
					    }
              /* Leave critical section */
              pthread_mutex_unlock( &tSocketDataMutex );

				    }
            else
            {
              iErrorLoop = ERROR;
              pcErrorString = malloc( (strlen("SOCKET_ACCEPT_ERROR")+1) * sizeof(char));
              strcpy( pcErrorString,"SOCKET_ACCEPT_ERROR"); 
              BSED_InternalEvent(ptSocketData->atConnectionData[uiConnectionIndex].iDeviceNumber,
                   pcErrorString);
					    DbgTrace2( "ServerWaitForConnectThread():accept():ERROR %s\n", 
                          strerror(errno) );
				    } 
            uiConnectionIndex++;
          } /* while */
			  }
			  else /* if listen failure */
			  {
          for ( uiConnectionIndex = 0;
                uiConnectionIndex < ptSocketData->uiNoConnections;
                uiConnectionIndex++  )
          {
            pcErrorString = malloc( (strlen("SOCKET_ACCEPT_ERROR")+1) * sizeof(char));
            strcpy( pcErrorString,"SOCKET_ACCEPT_ERROR"); 
            BSED_InternalEvent(ptSocketData->atConnectionData[uiConnectionIndex].iDeviceNumber,
              pcErrorString);
          }            
				  DbgTrace2( "ServerWaitForConnectThread():listen():ERROR %s\n", 
                    strerror(errno) );
			  }
      }
		  else /* if bind failed */
		  {
        for ( uiConnectionIndex = 0;
              uiConnectionIndex < ptSocketData->uiNoConnections;
              uiConnectionIndex++  )
        {
          pcErrorString = malloc( (strlen("SOCKET_ACCEPT_ERROR")+1) * sizeof(char));
          strcpy( pcErrorString,"SOCKET_ACCEPT_ERROR"); 
          BSED_InternalEvent(ptSocketData->atConnectionData[uiConnectionIndex].iDeviceNumber,
            pcErrorString);
        }
 			  DbgTrace2( "SocketConnectThread():bind():ERROR %s\n", strerror(errno) );
		  }
		}
		else /* Socket creation failed */
		{
      for ( uiConnectionIndex = 0;
            uiConnectionIndex < ptSocketData->uiNoConnections;
            uiConnectionIndex++  )
      {
        pcErrorString = malloc( (strlen("SOCKET_ACCEPT_ERROR")+1) * sizeof(char));
        strcpy( pcErrorString,"SOCKET_ACCEPT_ERROR"); 
        BSED_InternalEvent(ptSocketData->atConnectionData[uiConnectionIndex].iDeviceNumber,
          pcErrorString);
      }
 			DbgTrace2( "ServerWaitForConnectThread():socket():ERROR %s\n", 
                strerror(errno) );
		}
	}
	else
	{
    for ( uiConnectionIndex = 0;
          uiConnectionIndex < ptSocketData->uiNoConnections;
          uiConnectionIndex++  )
    {
      pcErrorString = malloc( (strlen("SOCKET_ACCEPT_ERROR")+1) * sizeof(char));
      strcpy( pcErrorString,"SOCKET_ACCEPT_ERROR"); 
      BSED_InternalEvent(ptSocketData->atConnectionData[uiConnectionIndex].iDeviceNumber,
        pcErrorString);
    }
 		DbgTrace2( "SocketConnectThread():CreateThread():ERROR %s\n", strerror(errno) );
	}
  pthread_mutex_lock( &tSocketDataMutex );
  ptSocketData->bConnectThreadRunning = FALSE;
  pthread_mutex_unlock( &tSocketDataMutex );
  return( NULL );
}

/* SocketServerWaitForConnect, starts the thread that handles client        */
/*  connections.                                                            */
/* ------------------------------------------------------------------------ */
/* Input parameters : ptSocketData- Data of the server socket.              */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value:                                                            */
/* None                                                                     */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
int16 SocketServerWaitForConnect( pSocketData ptSocketData )
{
    int   iResult;
    int16 iError;

    iResult = pthread_create( &ptSocketData->tConnectThread,
                              NULL,
                              ServerWaitForConnectThread,
                              (void *)ptSocketData);

    if ( iResult != NO_ERROR )
    {
      iError = ATD_SOCKET_ERROR;
      /* Impossible to create a thread for connection of client. */
      ptSocketData->bConnectThreadRunning = FALSE;
      DbgTrace2( "SocketServerWaitForConnect():CreateThread():ERROR:%s\n", 
                (char*) strerror(errno) );
    }
    else
    {
      iError = ATD_SOCKET_OK;
      ptSocketData->bConnectThreadRunning = TRUE;
    }

    return( iError );
}

/* ClientWaitForConnectThread, starts the thread that handles client        */
/*  connections.                                                            */
/* ------------------------------------------------------------------------ */
/* Input parameters : ptSocketData- Data of the server socket.              */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value:                                                            */
/* None                                                                     */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
void *ClientWaitForConnectThread( void * lpvParam ) 
{
  int			            iResult;
  DWORD			          dwError;
	int				          iOperationResult;
	struct sockaddr_in  tSocketAddress;
  int                 iConnectionSocket;
	pSocketData		      ptSocketData;
	uint16			        uiPortNumber;
	uint16			        uiConnectionIndex;
	unsigned long		    ulIpAddress;
	struct hostent	   *ptHostEnt;
	BOOL				        bHostAddressValid;
	BOOL				        bConnected;
	BOOL				        bError;
  char               *pcErrorString;

	ptSocketData = (pSocketData) lpvParam;

  bConnected = FALSE;
	bError     = FALSE;
  dwError    = NO_ERROR;

 	DbgTrace2( "SocketConnectThread() Trying to connect on port %s\n",
              ptSocketData->ppcSettings[2]);
	/* Socket setup */
	/* when first charachter is alpha then a hostname was provided */
	if( isalpha(ptSocketData->ppcSettings[1][0]) )
	{
		ptHostEnt = gethostbyname((char*) ptSocketData->ppcSettings[1] );
		if( NULL != ptHostEnt)
		{
			memcpy(&tSocketAddress.sin_addr,ptHostEnt->h_addr,ptHostEnt->h_length);
			bHostAddressValid = TRUE;
		}
		else
		{
			bHostAddressValid = FALSE;
		}
	}
	/* Otherwise an IP address was provided */
	else
	{
		ulIpAddress = htonl(inet_addr(ptSocketData->ppcSettings[1]));
		if (INADDR_NONE != ulIpAddress)
		{
			tSocketAddress.sin_addr.s_addr	= htonl(ulIpAddress);
			bHostAddressValid = TRUE;
		}
		else
		{
			bHostAddressValid = FALSE;
		}
	}

	if ( TRUE == bHostAddressValid )
	{
    /* read port numbers as decimal */
		sscanf( ptSocketData->ppcSettings[2], "%u", &uiPortNumber);
		tSocketAddress.sin_family		= AF_INET;
		tSocketAddress.sin_port			= htons((short)uiPortNumber);

    for ( uiConnectionIndex = 0;
          uiConnectionIndex < ptSocketData->uiNoConnections;
          uiConnectionIndex++  )
    {
		  /* Socket creation */
		  iConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		  if (INVALID_SOCKET != iConnectionSocket) 
      {
        bConnected = FALSE;
        bError     = FALSE;
        while ((FALSE == bConnected) && (FALSE == bError))
        {
		      /* Connect to the server */
		      iOperationResult = connect(	iConnectionSocket,
									                    (struct sockaddr *) &tSocketAddress, 
									                    sizeof(struct sockaddr));

		      if (SOCKET_ERROR == iOperationResult) 
		      {
            if (errno == ECONNREFUSED)
            {
              usleep(100000);
            }
            else
            {
              pcErrorString = malloc( (strlen("SOCKET_CONNECT_ERROR")+1) * sizeof(char));
              strcpy( pcErrorString,"SOCKET_CONNECT_ERROR"); 
              BSED_InternalEvent( ptSocketData->atConnectionData[uiConnectionIndex].iDeviceNumber,
                                  pcErrorString);
              bError = TRUE;
			        DbgTrace3( "SocketConnectThread():connect(%s):ERROR %s\n",
                          ptSocketData->ppcSettings[2], 
                          strerror(errno) );
            }
		      } 
		      else 
		      {
            /* Enter critical section */
            pthread_mutex_lock( &tSocketDataMutex );

            ptSocketData->atConnectionData[uiConnectionIndex].iConnected = 
              SOCKET_CONNECTED;
            ptSocketData->atConnectionData[uiConnectionIndex].tSocket = 
              iConnectionSocket;
            iResult = pthread_create( 
               &ptSocketData->atConnectionData[uiConnectionIndex].tRxThread,
                NULL,
                SocketRxThread,
                (void *)&ptSocketData->atConnectionData[uiConnectionIndex]);
              
			      if ( NO_ERROR == iResult)
			      {
              pcErrorString = 
                malloc( (strlen("CONNECTION_ESTABLISHED")+1) * sizeof(char));
              strcpy( pcErrorString,"CONNECTION_ESTABLISHED"); 
              BSED_InternalEvent(ptSocketData->atConnectionData[uiConnectionIndex].iDeviceNumber,
                pcErrorString);

              dwError    = NO_ERROR; 
              bError     = FALSE;
              bConnected = TRUE;
            }
            else
            {
              pcErrorString = malloc( (strlen("SOCKET_CONNECT_ERROR")+1) * sizeof(char));
              strcpy( pcErrorString,"SOCKET_CONNECT_ERROR"); 
              BSED_InternalEvent( ptSocketData->atConnectionData[uiConnectionIndex].iDeviceNumber,
                                  pcErrorString);
				      /* Impossible to create a thread for this client, close the socket */
              bError     = TRUE;
				      DbgTrace2( "SocketConnectThread():CreateThread():ERROR %s\n", 
                          strerror(errno) );
			      }
            /* Leave critical section */
            pthread_mutex_unlock( &tSocketDataMutex );
		      }	
        }
        if (TRUE == bError)
        {
          pcErrorString = malloc( (strlen("SOCKET_CONNECT_ERROR")+1) * sizeof(char));
          strcpy( pcErrorString,"SOCKET_CONNECT_ERROR"); 
          BSED_InternalEvent( ptSocketData->atConnectionData[uiConnectionIndex].iDeviceNumber,
                              pcErrorString);
          CloseConnection(ptSocketData->atConnectionData[uiConnectionIndex].iDeviceNumber);
        }
      }
      else
      {
        pcErrorString = malloc( (strlen("SOCKET_CONNECT_ERROR")+1) * sizeof(char));
        strcpy( pcErrorString,"SOCKET_CONNECT_ERROR"); 
        BSED_InternalEvent( ptSocketData->atConnectionData[uiConnectionIndex].iDeviceNumber,
                            pcErrorString);
				/* Impossible to create a socket for this client */
				DbgTrace2( "SocketConnectThread():CreateThread():ERROR %s\n", 
                   strerror(errno) );
      }
    } /* end for loop */
	}
	else
	{
    for ( uiConnectionIndex = 0;
          uiConnectionIndex < ptSocketData->uiNoConnections;
          uiConnectionIndex++  )
    {
      pcErrorString = malloc( (strlen("SOCKET_CONNECT_ERROR")+1) * sizeof(char));
      strcpy( pcErrorString,"SOCKET_CONNECT_ERROR"); 
      BSED_InternalEvent(ptSocketData->atConnectionData[uiConnectionIndex].iDeviceNumber,
        pcErrorString);
    }
 		DbgTrace1( "SocketConnectThread():Invalid name\n");
	}
  if (FALSE == bError)
  {
	  DbgTrace2( "SocketConnectThread() Connected on port %s\n", 
                ptSocketData->ppcSettings[2]);
  }
  pthread_mutex_lock( &tSocketDataMutex );
  ptSocketData->bConnectThreadRunning = FALSE;
  pthread_mutex_unlock( &tSocketDataMutex );
  return( NULL );
}

/* SocketClientTryToConnect, starts the thread that tries to connect to the */
/* the server                                                               */
/* ------------------------------------------------------------------------ */
/* Input parameters : ptSocketData- Data of the server socket.              */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value:                                                            */
/* None                                                                     */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
int16 SocketClientTryToConnect( SocketData *ptSocketData )
{
  int16 iError;
  int16 iResult;

  /* Starting the connect thread which blocks until connection has been setup */
  iError = pthread_create(  &ptSocketData->tConnectThread,
                            NULL,
                            ClientWaitForConnectThread,
                            (void *)ptSocketData);

  if ( iError != NO_ERROR )
  {
    ptSocketData->bConnectThreadRunning = FALSE;
    iResult = ATD_SOCKET_ERROR;
    /* Impossible to create a thread */
    DbgTrace2( "SocketClientTryToConnect():CreateThread():ERROR:%s\n", 
              strerror(errno) );
  }
  else
  {
    ptSocketData->bConnectThreadRunning = TRUE;
    iResult = ATD_SOCKET_OK;
  }
  return( iResult );
}

/* SocketRxThread, this thread handles the incoming data from a socket.     */
/* ------------------------------------------------------------------------ */
/* Input parameters : ConnectionData - Data of the socket.                  */
/* --                                                                       */
/* Output parameters: None                                                  */
/* --                                                                       */
/* Return value:                                                            */
/* None                                                                     */
/* --                                                                       */
/* Remarks:                                                                 */
/*                                                                         */
/* ------------------------------------------------------------------------ */
void *SocketRxThread( void * lpvParam ) 
{ 
  uint8		         acReadBuf[RXD_BUF_SIZE]; 
  int			         iBytesRead; 
  int              i;
  BOOL		         bRxThreadRunning = TRUE;
  ConnectionData	*ptConnectionData;
  int              iOSECrashDetected;
  char            *pcErrorString;

  /* The thread's parameter is the socket */ 
  ptConnectionData = (ConnectionData *)lpvParam; 
  iOSECrashDetected = 0;
  while ((bRxThreadRunning) && 
         (SOCKET_CONNECTED == ptConnectionData->iConnected)) 
  {
		/* Read client requests from the pipe */
		iBytesRead = recv( ptConnectionData->tSocket,  /* handle to socket */
								       acReadBuf,                  /* buffer to receive data */
								       RXD_BUF_SIZE,               /* size of buffer */
								       NO_FLAGS );			           /* No overlapped I/O */

		if (SOCKET_ERROR < iBytesRead)
		{
      if (iBytesRead > 0)
      {
		    DbgTrace2( "SocketRxThread():recv():%d bytes\n", iBytesRead );
        if (iBytesRead > 10000)
        {
          for ( i=(iBytesRead-30); i<iBytesRead; i++ )
          {
           DbgTrace3( "%c 0x%02x\n", acReadBuf[i], acReadBuf[i] );
          }
        }
		    IO_ReceiveData( ptConnectionData->iDeviceNumber, 
                        iBytesRead, 
                        acReadBuf, 
                        iOSECrashDetected);
      }
      else
      {
        /* Socket has been closed gracefully */
        ptConnectionData->iConnected = SOCKET_DISCONNECTED;
        bRxThreadRunning = FALSE;
        pcErrorString = malloc( (strlen("CONNECTION_LOST")+1) * sizeof(char));
        strcpy( pcErrorString,"CONNECTION_LOST"); 
        BSED_InternalEvent(ptConnectionData->iDeviceNumber,pcErrorString);
      }
		}
		else
		{
			switch( errno )
			{
				case ECONNREFUSED :	
					bRxThreadRunning = FALSE;
          CloseConnection(ptConnectionData->iDeviceNumber);
          pcErrorString = malloc( (strlen("CONNECTION_LOST")+1) * sizeof(char));
          strcpy( pcErrorString,"CONNECTION_LOST"); 
          BSED_InternalEvent(ptConnectionData->iDeviceNumber,pcErrorString);
					break;
				case ENOTCONN:
					bRxThreadRunning = FALSE;
          CloseConnection(ptConnectionData->iDeviceNumber);
          pcErrorString = malloc( (strlen("CONNECTION_LOST")+1) * sizeof(char));
          strcpy( pcErrorString,"CONNECTION_LOST"); 
          BSED_InternalEvent(ptConnectionData->iDeviceNumber,pcErrorString);
					break;
				case ESHUTDOWN:
					bRxThreadRunning = FALSE;
          CloseConnection(ptConnectionData->iDeviceNumber);
          pcErrorString = malloc( (strlen("CONNECTION_LOST")+1) * sizeof(char));
          strcpy( pcErrorString,"CONNECTION_LOST"); 
          BSED_InternalEvent(ptConnectionData->iDeviceNumber,pcErrorString);
					break;

				case ENOTSOCK:
					bRxThreadRunning = FALSE;
          CloseConnection(ptConnectionData->iDeviceNumber);
          pcErrorString = malloc( (strlen("SOCKET_RECEIVE_ERROR")+1) * sizeof(char));
          strcpy( pcErrorString,"SOCKET_RECEIVE_ERROR"); 
          BSED_InternalEvent(ptConnectionData->iDeviceNumber,pcErrorString);
					break;
				case EOPNOTSUPP:
					bRxThreadRunning = FALSE;
          CloseConnection(ptConnectionData->iDeviceNumber);
          pcErrorString = malloc( (strlen("SOCKET_RECEIVE_ERROR")+1) * sizeof(char));
          strcpy( pcErrorString,"SOCKET_RECEIVE_ERROR"); 
          BSED_InternalEvent(ptConnectionData->iDeviceNumber,pcErrorString);
					break;
				case ECONNRESET:
					bRxThreadRunning = FALSE;
          CloseConnection(ptConnectionData->iDeviceNumber);
          pcErrorString = malloc( (strlen("SOCKET_RECEIVE_ERROR")+1) * sizeof(char));
          strcpy( pcErrorString,"SOCKET_RECEIVE_ERROR"); 
          BSED_InternalEvent(ptConnectionData->iDeviceNumber,pcErrorString);
					break;
				case ECONNABORTED:
					bRxThreadRunning = FALSE;
          CloseConnection(ptConnectionData->iDeviceNumber);
          pcErrorString = malloc( (strlen("CONNECTION_LOST")+1) * sizeof(char));
          strcpy( pcErrorString,"CONNECTION_LOST"); 
          BSED_InternalEvent(ptConnectionData->iDeviceNumber,pcErrorString);
					break;

				default:
					DbgTrace2( "SocketRxThread():Socket():ERROR:%s\n", strerror(errno) );
          usleep(500000);
					break;
			} /* end switch( dwError ) */
		}
	} /* end while( fRxThreadRunning ) */
  DbgTrace2( "SocketRxThread():EXIT device %d\n", ptConnectionData->iDeviceNumber);
  ptConnectionData->bThreadRunning = FALSE;
  return( NULL );
}

/* GetSocketData, retrieves the data from a socket indicated by DeviceNumber*/
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*   DeviceNumber - The device of which the data is required.               */
/* --                                                                       */
/* Output parameters:                                                       */
/*   SocketData   - The data of the socket correspondig the DeviceNumber.   */
/*   ConnectionId - The Id of the connection is the socket administration.  */
/* --                                                                       */
/* Return value:                                                            */
/* None                                                                     */
/* --                                                                       */
/* Remarks:                                                                 */
/*   SocketData is NULL when the DeviceNumber could not be found.           */
/* ------------------------------------------------------------------------ */
static void GetSocketData( int16        iDeviceNumber, 
                           pSocketData *pptSocketData, 
                           uint16      *puiConnectionId)
{
	BOOL		    bFound;
  uint16      uiCounter;

	bFound           = FALSE;
	*pptSocketData   = ptAdminSocketData;
  *puiConnectionId = 0;

  pthread_mutex_lock( &tSocketDataMutex );
	if (NULL != *pptSocketData)
	{
		while ((*pptSocketData != NULL) && (bFound == FALSE))
		{
      for (uiCounter = 0; 
           (uiCounter < (*pptSocketData)->uiNoConnections) && (FALSE == bFound);
           uiCounter++ )
      {
        if ((*pptSocketData)->atConnectionData[uiCounter].iDeviceNumber == iDeviceNumber)
			  {
				  bFound           = TRUE;
          *puiConnectionId = uiCounter;
			  }
      }
      if (FALSE == bFound)
      {
			  (*pptSocketData) = (pSocketData) (*pptSocketData)->ptNext;
		  }
		}
	}
  pthread_mutex_unlock( &tSocketDataMutex );
  /* Setting the error condition */
}

/* FindSettings, Searches the administration for given Settings.            */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*  Settings - Settings to be found.                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/*   SocketData   - The data of the socket correspondig the Settings.       */
/*   ConnectionId - The Id of the connection is the socket administration.  */
/* --                                                                       */
/* Return value:                                                            */
/* None                                                                     */
/* --                                                                       */
/* Remarks:                                                                 */
/*   SocketData is NULL when the Settings could not be found.               */
/* ------------------------------------------------------------------------ */
static void FindSettings( char        *pcSettings, 
                          pSocketData *pptSocketData)
{
	BOOL		    bFound;

	bFound           = FALSE;
	*pptSocketData   = ptAdminSocketData;

  pthread_mutex_lock( &tSocketDataMutex );
	if (NULL != *pptSocketData)
	{
		while ((*pptSocketData != NULL) && (bFound == FALSE))
		{
      if (strcmp((*pptSocketData)->pcSettings,pcSettings) == 0)
			{
				bFound           = TRUE;
			}
      if (FALSE == bFound)
      {
			  (*pptSocketData) = (pSocketData) (*pptSocketData)->ptNext;
		  }
		}
	}
  pthread_mutex_unlock( &tSocketDataMutex );
  /* Setting the error condition */
}


/* CreateSocketData, Creates a new socket entry in the administration. It   */
/*   first searches the administration if the settings already excists in   */
/*   when so only the number of connections is updated, otherwise an new    */
/*   entry is created.                                                      */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*  Settings - Settings to be found.                                        */
/* --                                                                       */
/* Output parameters:                                                       */
/*   SocketData - Points to the newly created socket                        */
/*               administration entry.                                      */
/*   ConnectionId - The Id of the connection is the socket administration.  */
/* --                                                                       */
/* Return value:                                                            */
/* None                                                                     */
/* --                                                                       */
/* Remarks:                                                                 */
/*   SocketData is NULL when the data could not be created.                 */
/* ------------------------------------------------------------------------ */
static void CreateSocketData( pSocketData *pptSocketData, 
                              char        *pcSettings)
{
  int16        iOperationResult;
  uint16       uiCounter;
  pSocketData  ptSocketDataWalker; 

  *pptSocketData = NULL;

  FindSettings(pcSettings, pptSocketData);
  /* If a the settings are not found then create a new administration */
  if (NULL == *pptSocketData)
  {
    *pptSocketData = (pSocketData) malloc(sizeof(SocketData));
    if (NULL != *pptSocketData)
    {
      /* Enter the critical section */
      pthread_mutex_lock( &tSocketDataMutex );
      /*Parser the new settings */
      iOperationResult = ParseSocketSettings( pcSettings, 
						                                  (*pptSocketData)->ppcSettings, 
						                                  &(*pptSocketData)->uiNoSettings );
      if (ATD_SOCKET_OK == iOperationResult)
      {
        /* add a new element */
	      if( NULL != ptAdminSocketData ) /* There are more data elements */
	      {
	        ptSocketDataWalker = ptAdminSocketData;
          /* Find the last element */
		      while( NULL != ptSocketDataWalker->ptNext )
		      {
			      ptSocketDataWalker = ptSocketDataWalker->ptNext;
		      }
          if (NULL != ptSocketDataWalker)
          {
            ptSocketDataWalker->ptNext   = (*pptSocketData);
		        (*pptSocketData)->ptNext		 = NULL;
		        (*pptSocketData)->ptPrevious = ptSocketDataWalker;
          }
	      }
	      else /* First element */
	      {
          ptAdminSocketData            = *pptSocketData;
		      (*pptSocketData)->ptNext		 = NULL;
		      (*pptSocketData)->ptPrevious = NULL;
	      }
        /* Fill the new Socket data */
        for (uiCounter = 0; uiCounter < MAX_NR_CONNECTIONS; uiCounter++)
        {
		      (*pptSocketData)->atConnectionData[uiCounter].tSocket	       = 
            INVALID_SOCKET;
		      (*pptSocketData)->atConnectionData[uiCounter].iDeviceNumber	 = 
            0;
          (*pptSocketData)->atConnectionData[uiCounter].bThreadRunning = 
            FALSE;
          (*pptSocketData)->atConnectionData[uiCounter].iConnected     = 
            SOCKET_DISCONNECTED;
       }
        (*pptSocketData)->bConnectThreadRunning = FALSE;
        (*pptSocketData)->uiNoConnections       = 1;
        strcpy((*pptSocketData)->pcSettings,pcSettings);
      }
      else
      {
        free(*pptSocketData);
        *pptSocketData = NULL;
      }
      /* Leave the critical section */
      pthread_mutex_unlock( &tSocketDataMutex );
    }
  }
  else if (NULL != *pptSocketData)
  {
    /* There is already a server with the same settings */
    /* increase the number of allowed connections       */
    (*pptSocketData)->uiNoConnections++;
  }
}

/* CloseConnection, Closes the socket corresponding to the DeviceNumber and */
/*   updates the administration.                                            */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*   DeviceNumber - DeviceNumber of the socket to be closed.                */
/* --                                                                       */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/*   ATD_SOCKET_OK    - when the last element is removed                    */
/*   ATD_SOCKET_ERROR - Otherwise                                           */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
static int16 CloseConnection( int16 iDeviceNumber )
{
  uint16      uiConnectionId;
  pSocketData ptSocketData;
  int16       iResult;

  iResult = ATD_SOCKET_ERROR;

  GetSocketData( iDeviceNumber, &ptSocketData, &uiConnectionId);
  if (NULL != ptSocketData)
  {
    if (INVALID_SOCKET != ptSocketData->atConnectionData[uiConnectionId].tSocket)
    {
      pthread_mutex_lock( &tSocketDataMutex );

      /* Close the Socket and update the administration */
			if ( !close(ptSocketData->atConnectionData[uiConnectionId].tSocket) )
			{
				DbgTrace2( "CloseConnection():close():ERROR:%s\n", strerror(errno));
			}
      ptSocketData->atConnectionData[uiConnectionId].tSocket    = INVALID_SOCKET;
      ptSocketData->atConnectionData[uiConnectionId].iConnected = SOCKET_DISCONNECTED;
      ptSocketData->uiNoConnections--;
      if (0 == ptSocketData->uiNoConnections)
      {
        iResult = ATD_SOCKET_OK;
      }
      pthread_mutex_unlock( &tSocketDataMutex );
    }
  }

  return(iResult);

}

/* RemoveSocketData, Removes socket date from the administration and release*/
/*   the data allocated                                                     */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*   SocketData - Socket data which has to be released.                     */
/* --                                                                       */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/*   ATD_SOCKET_OK    - when the last element is removed                    */
/*   ATD_SOCKET_ERROR - Otherwise                                           */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
static int16	RemoveSocketData( pSocketData ptSocketData)
{
	int16			  iResult;
	BOOL		    bFound;
	pSocketData ptCurrentSocketData;	
	pSocketData ptPrevSocketData;	

	bFound				       = FALSE;
	iResult				       = ATD_SOCKET_ERROR;
	ptCurrentSocketData	 = ptAdminSocketData;
	ptPrevSocketData	   = NULL;

	if (NULL != ptCurrentSocketData)
	{
    pthread_mutex_lock( &tSocketDataMutex );
		while ((NULL != ptCurrentSocketData) && (FALSE == bFound))
		{
			if (ptSocketData == ptCurrentSocketData)
			{
				bFound = TRUE;
			}
			else
			{
				ptPrevSocketData	  = ptCurrentSocketData; /* update previous pointer */
				ptCurrentSocketData	= (pSocketData)ptCurrentSocketData->ptNext;
			}
		}
		if (NULL == ptPrevSocketData) /* Start of the list */
		{
			if( NULL == (SocketData *)ptCurrentSocketData->ptNext )
			{
		    FreeSocketSettings(	ptCurrentSocketData->ppcSettings,
										        ptCurrentSocketData->uiNoSettings);
				free( (void*)ptCurrentSocketData);
				ptAdminSocketData = NULL;
		    iResult = ATD_SOCKET_OK;
			}
			else
			{
				ptAdminSocketData			        = (SocketData *)ptCurrentSocketData->ptNext;
				ptAdminSocketData->ptPrevious = (SocketData *)ptPrevSocketData;
		    FreeSocketSettings(	ptCurrentSocketData->ppcSettings,
										        ptCurrentSocketData->uiNoSettings);
				free( (void*)ptCurrentSocketData);
			}
		}
		else if (NULL == ptCurrentSocketData->ptNext) /* End of the list */
		{
			ptPrevSocketData->ptNext = NULL;
			
		  FreeSocketSettings(	ptCurrentSocketData->ppcSettings,
										      ptCurrentSocketData->uiNoSettings);
			free((void*) ptCurrentSocketData);
		}
		else /* Middle of the list */
		{
			ptCurrentSocketData->ptNext->ptPrevious = ptCurrentSocketData->ptPrevious;
			ptPrevSocketData->ptNext				        = ptCurrentSocketData->ptNext;
		  FreeSocketSettings(	ptCurrentSocketData->ppcSettings,
										      ptCurrentSocketData->uiNoSettings);
  		free( (void*) ptCurrentSocketData);
		}
    pthread_mutex_unlock( &tSocketDataMutex );
	}
	return( iResult );
}

/* ParseSocketSettings, Parses the socket setting string for the settings.  */
/*  The string should have the following layout:                            */
/*    <client/server> <IP address/hostname> <port number>                   */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*   Settings - The settings string specified in the IO file.               */
/* --                                                                       */
/* Output parameters:                                                       */
/*  ReturnSettings - The settings found returned in a string array.         */
/*  NoSettings     - Number of settings found.                              */
/* --                                                                       */
/* Return value:                                                            */
/*   ATD_SOCKET_OK    - Parsing successfull                                 */
/*   ATD_SOCKET_ERROR - Parsing failure                                     */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
static int16 ParseSocketSettings( char    *pcSettings, 
                                  char    *ppcReturnSettings[], 
                                  uint16  *puiNoSettings )
{
	int16	  iResult;
	char   *pcSettingString;
	char   *pcSettingsTempPtr;
	char	  pcSettingsTemp[80];
	uint16	uiSettingIndex;
	size_t  length;	
	
	uiSettingIndex = 0;
	*puiNoSettings	= 0;

	strcpy( pcSettingsTemp,pcSettings);
	pcSettingString	  = pcSettingsTemp;
	pcSettingsTempPtr = pcSettingsTemp;

	iResult = ATD_SOCKET_OK;
	while ((0 != *pcSettingsTempPtr) && (iResult == ATD_SOCKET_OK))
	{
		/* Settings are space seperated */
		if (*pcSettingsTempPtr == ' ')
		{
			*pcSettingsTempPtr = 0;
			length = strlen( pcSettingString );
			ppcReturnSettings[uiSettingIndex] = (char*) malloc(sizeof(char)*(length+1));
			if (NULL == ppcReturnSettings[uiSettingIndex])
			{
        FreeSocketSettings(ppcReturnSettings,uiSettingIndex);
				iResult = ATD_SOCKET_ERROR;
			}
			else
			{
				strcpy(ppcReturnSettings[uiSettingIndex],pcSettingString);
				uiSettingIndex++;
				pcSettingsTempPtr++;
				pcSettingString = pcSettingsTempPtr;
				(*puiNoSettings)++;
			}

		}
		pcSettingsTempPtr++;
	}
  if (ATD_SOCKET_OK == iResult)
  {
	  length = strlen( pcSettingString );
	  if (length > 0)
	  {
		  ppcReturnSettings[uiSettingIndex] = (char*) malloc(sizeof(char)*(length+1));
		  if (NULL == ppcReturnSettings[uiSettingIndex])
		  {
        FreeSocketSettings(ppcReturnSettings,uiSettingIndex);
			  iResult = ATD_SOCKET_ERROR;
		  }
		  else
		  {
			  strcpy(ppcReturnSettings[uiSettingIndex],pcSettingString);
			  (*puiNoSettings)++;
		  }
	  }
  }
	return(iResult);
}

/* FreeSocketSettings, Frees up memory allocate during the Parse action.    */
/* ------------------------------------------------------------------------ */
/* Input parameters :                                                       */
/*  ReturnSettings - Points to the Settings structure.                      */
/*	NoSettings     - The number of settings                                 */
/* --                                                                       */
/* Output parameters:                                                       */
/* --                                                                       */
/* Return value:                                                            */
/* --                                                                       */
/* Remarks:                                                                 */
/* ------------------------------------------------------------------------ */
static void FreeSocketSettings( char *ppcReturnSettings[], uint16 uiNoSettings )
{
	uint16 uiCounter;

	for (uiCounter = 0; uiCounter < uiNoSettings; uiCounter++)
	{
		if (NULL != ppcReturnSettings[uiCounter])
		{
			free(ppcReturnSettings[uiCounter]);
		}
	}
}
