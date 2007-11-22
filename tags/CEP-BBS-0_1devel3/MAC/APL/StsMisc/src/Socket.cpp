// Voor deze class wordt zometeen lofar socket gebruikt.
// De aanroep van sommige onderdelen moet dan wat worden aangepast.



#include "Socket.h"



Socket::Socket(const std::string& socketname, const std::string& hostname, int service, int protocol)
 :itsSocketname(socketname),itsSocketID(-1),	itsIsServer(false),	itsIsConnected(false),	itsHost(hostname),
	itsPort(service),	itsAllowIntr(false),	itsIsInitialized(false),	itsIsBlocking(false)
{		
	
}

Socket::~Socket(){}

int Socket::connection()
{	
	WORD wVersionRequested = MAKEWORD(1,1);
	WSADATA wsaData;
	int nRet;

	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested)
	{	
		fprintf(stderr,"\n Wrong version\n");
		return(-1);
	}
	return(open_connection());

}


int Socket::open_connection() 
{
	SOCKET mySocket =  socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in  itsTCPAddr;
	memset (&itsTCPAddr, 0, sizeof(itsTCPAddr));
	itsTCPAddr.sin_family = AF_INET;

	unsigned int IPbytes;

	// try if hostname is hard ip address
	if ((IPbytes = inet_addr(itsHost.c_str())) == INADDR_NONE) {
		
		// No, try to resolve the name
		struct hostent*       hostEnt;        // server host entry
		if (!(hostEnt = gethostbyname(itsHost.c_str()))) {
                           
			return (-2);
		}

		// Check type
		if (hostEnt->h_addrtype != AF_INET) {
			return (-3);
		}
     
		memcpy (&IPbytes, hostEnt->h_addr, sizeof (IPbytes));
	}
	memcpy ((char*) &itsTCPAddr.sin_addr.s_addr, (char*) &IPbytes, sizeof(IPbytes));

	itsTCPAddr.sin_port = htons(itsPort);
	
	int result = connect(mySocket, (struct sockaddr *)&itsTCPAddr, sizeof(struct sockaddr_in));
	return (mySocket);
}

//void   Close(){
//}


