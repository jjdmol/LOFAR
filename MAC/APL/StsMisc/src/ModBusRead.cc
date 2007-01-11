//ModBusRead.am

#include <time.h>
#include <lofar_config.h>
#include <Common/LofarTypedefs.h>
#include <Common/Net/Socket.h>
#include <Common/hexdump.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h>


using namespace LOFAR;
using namespace std;

double setpoint = 17;

int RTH4;

int open_connection (const string& hostname, int portnr) {

	int mySocket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in  itsTCPAddr;
	memset (&itsTCPAddr, 0, sizeof(itsTCPAddr));
	itsTCPAddr.sin_family = AF_INET;

	unsigned int IPbytes;

	// try if hostname is hard ip address
	if ((IPbytes = inet_addr(hostname.c_str())) == INADDR_NONE) {
		
		// No, try to resolve the name
		struct hostent*       hostEnt;        // server host entry
		if (!(hostEnt = gethostbyname(hostname.c_str()))) {
                           
			return (-2);
		}


		// Check type
		if (hostEnt->h_addrtype != AF_INET) {
			return (-3);
		}
     
		memcpy (&IPbytes, hostEnt->h_addr, sizeof (IPbytes));
	}
	memcpy ((char*) &itsTCPAddr.sin_addr.s_addr, (char*) &IPbytes, sizeof(IPbytes));

	itsTCPAddr.sin_port = htons(portnr);
	
	int result = connect(mySocket, (struct sockaddr *)&itsTCPAddr, sizeof(struct sockaddr_in));
	return (mySocket);
	
}



struct StuurUitlezen {			// (14 int8s) Declare Modbus_TCP struct type
	short TransactionIdentifier;			// = 2 int8s

	short ProtocolIdentifier;			// = 2 int8s
	short Length;					// = 2 int8s
	int8  UnitIdentifier;				// = 1 int8

//  -modbus				= 6 int8s (telt unit identifier mee anders 5)
        int8 Function3;						//:15 = 1 int8s 
        short ReferenceNumber;					// = 2 int8s
        short WordCount;					// = 2 int8s

} StuurUitlezen_t;


///////////////////////////////////////////////////////////////////////

#pragma pack(1)
struct StuurUitpt {		// (14 int8s) Declare Modbus_TCP struct type

	short TransactionIdentifie;				// = 2 int8s
	short ProtocolIdentifie;				// = 2 int8s
	short Lengt;						// = 2 int8s
	int8  UnitIdentifie;					// = 1 int8

//  -modbus			= 6 int8s (telt unit identifier mee anders 5)
	int8 Functio;						//:15 = 1 int8s 
	int8 ByteCount;						// = 1 int8
	short T1;						// = 2 int8s
	short T2;						// = 2 int8s
	short T3;						// = 2 int8s
	short T4;						// = 2 int8s
	short T5;						// = 2 int8s
	short T6;						// = 2 int8s
	short T7;						// = 2 int8s
	char Data[22];					// = rest data 22 int8s over


} StuurUitpt_t;

/////////////////////////////////////////////////////////////////////////


int main() {

/////////////herhalen van het geheel///////////////////////////////////////
while(1) {

	int	readConnection  = open_connection("10.151.218.1", 502);
	int	writeConnection = open_connection("10.151.218.2", 502);

	struct StuurUitlezen demand;

		demand.TransactionIdentifier = htons(0);
		demand.ProtocolIdentifier = htons(0);
		demand.Length = htons(8); //moeten 4 bits zijn
		demand.UnitIdentifier = 01;
		demand.Function3 = 3;
		demand.ReferenceNumber = 0;
		demand.WordCount = htons(18);

		int result = send(readConnection, (char*)&demand, sizeof(demand), 0);


		struct StuurUitpt receiv;
		recv(readConnection, (char*)&receiv, sizeof(receiv), 0); // NB ignoring result

		hexdump ((char*)&receiv, sizeof(receiv));


		short HT1 = htons(receiv.T1);
		short HT2 = htons(receiv.T2);
		short HT3 = htons(receiv.T3);
		short HT4 = htons(receiv.T4);
		short HT5 = htons(receiv.T5);
		short HT6 = htons(receiv.T6);
		short HT7 = htons(receiv.T7);

		
		double RT1 = (HT1 / 327.5) - 50;
		double RT2 = (HT2 / 327.5) - 50;
		double RT3 = (HT3 / 327.5) - 50;
		double RT4 = (HT4 / 327.5) - 50;
		double RT5 = (HT5 / 327.5) - 50;
		double RT6 = (HT6 / 327.5) - 50;
		double RT7 = (HT7 / 327.5) - 50;

		fprintf(stderr,"RT1: %3.2f\n"  , RT1);
		fprintf(stderr,"RT2: %3.2f\n"  , RT2);
		fprintf(stderr,"RT3: %3.2f\n"  , RT3);
		fprintf(stderr,"RT4: %3.2f\n"  , RT4);
		fprintf(stderr,"RT5: %3.2f\n"  , RT5);
		fprintf(stderr,"RT6: %3.2f\n"  , RT6);
		fprintf(stderr,"RT7: %3.2f\n\n", RT7);

// naar file schrijven

FILE	*fp;

	fp = fopen("temperaturen.txt", "a");
	char dateStr [9];
	char timeStr [9];
//	_strdate( dateStr);
//	_strtime( timeStr);
	fprintf(fp, "%s - ", dateStr);
	fprintf(fp, "%s ", timeStr);
	fprintf(fp, "%3.2f ", RT1);
	fprintf(fp, "%3.2f ", RT2);
	fprintf(fp, "%3.2f ", RT3);
	fprintf(fp, "%3.2f ", RT4);
	fprintf(fp, "%3.2f ", RT5);
	fprintf(fp, "%3.2f ", RT6);
	fprintf(fp, "%3.2f\n", RT7);
	fclose(fp);


		//Temperatuur is uitgelezen. vanaf hier gaan we uitschrijven.


		double MaxTemp = setpoint + 0.3;
		double MinTemp = setpoint - 0.3;

/*
er word nog maar in 1 kabinet geregeld

		int RTH1;
		int RTH2;
		int RTH3;
		int RTH4;
		int RTH5;
		int RTH6;
		int RTH7;


		if (RT1 > MaxTemp) { RTH1 = 1; };
		if (RT2 > MaxTemp) { RTH2 = 1; };
		if (RT3 > MaxTemp) { RTH3 = 1; };
		if (RT4 > MaxTemp) { RTH4 = 1; };
		if (RT5 > MaxTemp) { RTH5 = 1; };
		if (RT6 > MaxTemp) { RTH6 = 1; };
		if (RT7 > MaxTemp) { RTH7 = 1; };

		if (RT1 < MinTemp) { RTH1 = 0; };
		if (RT2 < MinTemp) { RTH2 = 0; };
		if (RT3 < MinTemp) { RTH3 = 0; };
		if (RT4 < MinTemp) { RTH4 = 0; };
		if (RT5 < MinTemp) { RTH5 = 0; };
		if (RT6 < MinTemp) { RTH6 = 0; };
		if (RT7 < MinTemp) { RTH7 = 0; };
*/
		int RTH4;
		if (RT4 > MaxTemp) { RTH4 = 7; };
		if (RT4 < MinTemp) { RTH4 = 4; };




		struct Modbus_TCP {		// (14 int8s) Declare Modbus_TCP struct type
		
			short TransactionIdentifiers;		// = 2 int8s
			short ProtocolIdentifiers;		// = 2 int8s
			short Lengths;				// = 2 int8s
			int8  UnitIdentifiers;			// = 1 int8

		//  -modbus		= 8 int8s (telt unit identifier mee anders 7)
			int8 FunctionForceMultipleCoilss;	//:15 = 1 int8s 
			short ReferenceNumbers;			// = 2 int8s
			short BitCounts;			// = 2 int8s
			int8 ByteCounts;			// = 1 int8
			int8 Datas;				// = 1 int8

		} command;



//		Modbus_TCP command;

		command.TransactionIdentifiers = htons(0);
		command.ProtocolIdentifiers = htons(0);
		command.Lengths = htons(8); //moeten 4 bits zijn
		command.UnitIdentifiers = 01;
		command.FunctionForceMultipleCoilss = 15;
		command.ReferenceNumbers = htons(16);
		command.BitCounts = htons(6);
		command.ByteCounts = 01;
//		command.Datas = RTH1+(RTH2*2)+(RTH3*4)+(RTH4*8)+(RTH5*16)+(RTH6*32)+(RTH6*64);
		command.Datas = RTH4;
		result = send(writeConnection, (char*)&command, sizeof(command), 0);

		sleep (60); //wacht 1 minuut

	close(readConnection);
	close(writeConnection);

	} //while

	return 0;

}
