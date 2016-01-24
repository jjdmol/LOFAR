#ifndef HEATHREGULATION_CONTROL_H
#define HEATHREGULATION_CONTROL_H

#include <fstream>
#include "Socket.h"
#include "SystemTime.h"
#include "DigitalInputControl.h"
#include "import/TypeNames.h"
#include "import/lofar_string.h"
#include "import/lofar_iostream.h"
#include "import/lofar_vector.h"
#include "import/LofarLogger.h"
#include "import/StringUtil.h"
#include "import/Exception.h"
#include "CalculateHeathRegulation.h"


#pragma pack(1)


		
//namespace LOFAR {

class HeathRegulationControl
{
	public:

		//
		//	struct SendToAdam	
		//
		//  Write to ADAM to make a connection with one of the ADAMS
		//
		struct SendToAdam {
			short TransactionIdentifier;			// = 2 bytes
			short ProtocolIdentifier;				// = 2 bytes
			short Length;							// = 2 bytes
			BYTE  UnitIdentifier;					// = 1 byte

		//  -modbus									= 6 bytes (telt unit identifier mee anders 5)
			BYTE Function;							//:15 = 1 bytes
			short ReferenceNumber;					// = 2 bytes
			short WordCount;						// = 2 bytes

		} ;


		//
		//	struct GetFromAdam
		// read data from the ADAMS input ports
		//	
		struct GetFromAdam {
			short TransactionIdentifier;			// = 2 bytes
			short ProtocolIdentifier;				// = 2 bytes
			short Length;							// = 2 bytes
			byte  UnitIdentifier;					// = 1 byte

			//  -modbus								= 6 bytes (telt unit identifier mee anders 5)
			byte Function;							//:15 = 1 bytes 
			byte ByteCount;							// = 1 bytes
			short T[7];								// = 7 x 2 bytes

			WORD Data[22];							// = rest data 22 bytes over

		} ;


		//
		//	struct Modbus_TCP
		//
		// Write data to the ADAM 6050 output ports
		//
		struct Modbus_TCP {							// (14 bytes) Declare Modbus_TCP struct type
					
			short TransactionIdentifier;			// = 2 bytes
			short ProtocolIdentifier;				// = 2 bytes
			short Length;							// = 2 bytes
			byte  UnitIdentifier;					// = 1 byte

		//  -modbus									= 8 bytes (telt unit identifier mee anders 7)
			byte FunctionForceMultipleCoils;		//:15 = 1 bytes 
			short ReferenceNumber;					// = 2 bytes
			short BitCount;							// = 2 bytes
			byte ByteCount;							// = 1 byte
			byte Data;								// = 1 byte (hier bepaal je wat aan en uit gaat)

		} ;


		
		HeathRegulationControl(int32 samplePerMinute, int32 numberOfDays);	
		~HeathRegulationControl();


		//
		// initialize connections
		//
		void makeConnection();

		// close connections
		//
		void closeConnection();

		//
		// initialise system for Regulation
		//
		int32 startUpMode();//int32 readConnection0);

		//
		// print sensor information on screen
		//
		void logPrint(string function);

		//
		// save information in a file
		//
		void logFile(char *problem, int16 number = -1);

		//
		// read address, ports and type out a input .txt file
		//
		int32 readInputFile();

		//
		// sensor data that is coming from the Adam 6015 and 6050
		//
		void readAdam(int16 adam, int16 connectie);

		//
		// connect to Adam 6015 for readin the ADAM out
		//
		void writeToAdam(int16 adam, int16 function);


		//
		// write en connect to Adam 6050
		//
		void writeToAdam6050();
			

		//
		// control the temperatures of the cabinets
		//
		void controlsTemperatures(); 

		//
		// Dit gedeelte werkt nogniet
		//
		string errstr() const;

			inline int32 errcode() const;
			inline int32 errnoSys() const;

			//Error codes
			typedef enum {
                HEATHREUGLATION_OK		=  0,     ///< Ok
				DOOR_OPEN				= -1,     ///< Cabinet door is open
                CONNECTION_READ			= -2,     ///< Can't create read connection
                CONNECTION_WRITE		= -3,     ///< Can't create write connection             
                READ_ERROR				= -4,     ///< Can't read from ADAM
                WRITE_ERROR				= -5,     ///< Can't write to ADAM
                TEMP_PROBLEM_BACK		= -6,     ///< Temperature are to high on the backside (fans's ??)
                TEMP_PROBLEM_FRONT		= -7,     ///< Temperature are to high on the frontside (fans's ??) 
                ADDRESS_ERROR			= -8,     ///< Wrong address from inputfile
                PORT_ERROR				= -9,     ///< Wrong port from inputfile
                TYPE_ERROR				= -10,    ///< Wrong connection type (must be TCP)
				NO_INPUT				= -11	  ///< No input file with connections values
			} ErrorCodes;

		//
		// wordt alleen voor testfase gebruikt om makelijk grafieken te makenin excel, kan er later uit
		//
		void schrijfFile();
		//

	protected:
		inline int32 setErrno(int32 ErrorNr);

	private:
		//
		// Datamembers
		//
		int32 itsSamplePerMinute;
		int32 itsNumberOfDays;
		
		CalculateHeathRegulation calculate;
		SystemTime time;
		DigitalInputControl inputadam;
		

		int32 itsReadConnection0;				//read data from ADAM 6015
		int32 itsReadConnection1;				//read data from ADAM 6015
		int32 itsWriteConnection;				//write data to ADAM 6050
		
		int32 itsRTH;							//Ventilator that regulates the outdoor convection


		string itsAddress1, itsAddress2, itsAddress3;
		int32 itsPort1, itsPort2, itsPort3;
		int32 itsProtocol1, itsProtocol2, itsProtocol3;


		bool itsConnectionFlag;

		int16 itsNumberOfLogs;

		//own error number
		int32 itsErrno;
		int32 itsSysErrno;

};

inline int32 HeathRegulationControl::errcode() const
{
	return (itsErrno);
}

inline int32 HeathRegulationControl::errnoSys() const
{
	return (itsSysErrno);
}

inline int32 HeathRegulationControl::setErrno(int32 errorNr)
{
	itsSysErrno = errno;				// save system errno
	return (itsErrno = errorNr);		// save and return given error
}

//} // namespace LOFAR


#endif