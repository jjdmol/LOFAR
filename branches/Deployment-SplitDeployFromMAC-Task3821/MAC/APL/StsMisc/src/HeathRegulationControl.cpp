#include "HeathRegulationControl.h"


//namespace LOFAR 
//{

//
// HeathRegulation()
//
HeathRegulationControl::HeathRegulationControl(int32 samplePerMinute, int32 numberOfDays)
	:itsSamplePerMinute(samplePerMinute), itsNumberOfDays(numberOfDays)
{
	calculate.setup(samplePerMinute, numberOfDays);
	itsConnectionFlag = false;
	itsNumberOfLogs = 0;

}


//
// ~HeathRegulation()
//
HeathRegulationControl::~HeathRegulationControl() 
{
	calculate.~CalculateHeathRegulation();
	time.~SystemTime();
	inputadam.~DigitalInputControl();
	
	// close Sockets 
	closeConnection();
}

//
//	Get the connection to the Adams
//
void HeathRegulationControl::makeConnection()
{	
	Socket readConnect0("readContact 0", itsAddress1, itsPort1, itsProtocol1); //"10.151.218.1", 502, 1);
//		Socket readConnection1("readContact 1", itsAddress2, itsPort2, itsProtocol2);
	Socket writeConnect("writeContact ", itsAddress3, itsPort3, itsProtocol3); //"10.151.218.2", 502, 1); //

	itsReadConnection0 = readConnect0.connection();
	itsReadConnection1 = readConnect0.connection();  //	= readConnection1.connection();
	itsWriteConnection = writeConnect.connection();;	

	itsConnectionFlag = true;
}


//
// close connections with the adams
//
void HeathRegulationControl::closeConnection()
{	
	closesocket(itsReadConnection0);
	closesocket(itsReadConnection1);
	closesocket(itsWriteConnection);
	itsReadConnection0.~Socket();
	itsReadConnection1.~Socket();
	itsWriteConnection.~Socket();

	itsConnectionFlag = false;
}


//
// Write to the Adan for reading data from the Adam 6015 and 6050
//
void HeathRegulationControl::writeToAdam(int16 adam, int16 function) 
{
	SendToAdam demand;

	demand.TransactionIdentifier = htons(0);
	demand.ProtocolIdentifier = htons(0);
	demand.Length = htons(8);					//moeten 4 bits zijn
	demand.UnitIdentifier = 01;
	
	//
	//for reading the 6015 itsFunction must be 3 and  
	//for reading the 6050 itsFunction must be 1
	//
	demand.Function = function;				
	demand.ReferenceNumber = 0;
	demand.WordCount = htons(18);

	int32 result = send(adam, (char*)&demand, sizeof(demand), 0);
}


//
// Read input data from the Adam 6015 and 6050
// use the GetFromAdan struct to get the right bits for the information that you want to receiv
//
void HeathRegulationControl::readAdam(int16 adam, int16 connectie) 
{
	GetFromAdam receiv;
	
	int16 selectAdam = 0;
		
	receiv.TransactionIdentifier;	
	receiv.ProtocolIdentifier;
	receiv.Length;
	receiv.UnitIdentifier;
	receiv.Function;
	receiv.ByteCount;
	receiv.T[0];				// get the information from the sensor on port 0
	receiv.T[1];				// get the information from the sensor on port 1
	receiv.T[2];				// get the information from the sensor on port 2
	receiv.T[3];				// get the information from the sensor on port 3
	receiv.T[4];				// get the information from the sensor on port 4
	receiv.T[5];				// get the information from the sensor on port 5
	receiv.T[6];				// get the information from the sensor on port 6
	receiv.Data;

	int32 itsUitpt = recv(adam, (char*)&receiv, sizeof(receiv), 0);

	// choose the sensors by the right connected Adam
	// 0 = Adam0 of type 6015; 1 = Adam1 of type 6015; 3 = Adam type 6050
	//
	switch(connectie)
	{
		case 0: selectAdam = 0;		// Connection0 is Adam 0 sensors 0 t/m 6
			break;		
		case 1: selectAdam = 7;		// Connection1 is Adam 1 sensors 7 t/m 13
			break;
		case 2:	// check for connection problem with the ADAM 6050. By -13108 there is no connection
				// and create a logFile
				//
				if(receiv.T[0] == -13108)
				{
					logFile("No connection with ADAM 6050");
				} else
				{
					inputadam.setInput(receiv.T[0]);  //set the right bits true 
				}
			break;
	}

	// Data of the sensors Calculate to values that you can easy read and understand in CalculateHeathRegulation class
	if(connectie == 0 || connectie == 1)
	{		
		// check first of the connection is good
		// when there are problems, make a logFile
		//
		if(receiv.T[0] == -13108)
		{
			logFile("No connection with ADAM 6015 number ", connectie);
		}

		for(int16 sensor=0; sensor<7; sensor++)
		{
			// set values of the sensors 
			calculate.calculateTemp(receiv.T[sensor], (sensor + selectAdam));
		}
	}
}


//
// write to Adam 6050, for setting the fans "on" or "off"
//
void HeathRegulationControl::writeToAdam6050()	// (14 bytes) Declare Modbus_TCP struct type
{	
	Modbus_TCP command;

	command.TransactionIdentifier = htons(0);
	command.ProtocolIdentifier = htons(0);
	command.Length = htons(8);							//Must be 4 bits
	command.UnitIdentifier = 01;
	command.FunctionForceMultipleCoils = 15;			//set the Adam in writing mode
	command.ReferenceNumber = htons(16);				//Set this value to write to the right bit
	command.BitCount = htons(6);
	command.ByteCount = 01;

	//write to the Adam 6050 which fan you set on and which one is out
	command.Data = itsRTH; //	//controls the fans (hexadecimal value)

	//
	// Send you value to the ADAM 6050 with send and recv to make the ADAM 6050
	// ready for reading input signals. If you don't read after you send,
	// the input signal send a wrong value from the digital inputs
	//
	int32 writeResult = send(itsWriteConnection, (char*)&command, sizeof(command), 0);
	writeResult = recv(itsWriteConnection, (char*)&command, sizeof(command), 0);
}  


//
//	startup info for the system
//  with this fucntion you set recommend values to start the heathregulation
//
int32 HeathRegulationControl::startUpMode()
{
	time.updateDateTime();
	// AL the 4 fans are on when you send 15 to ADAM 6050
	itsRTH = 15;

	// set the fans on by starting the system
	writeToAdam6050();

	//
	//write to Adam 6050 for control the doors
	//the second parameter must be 1 for reading the ADAM 6050
	//
	writeToAdam(itsWriteConnection, 1);	
	readAdam(itsWriteConnection, 2);
	
	//
	//If the door is open, set this in the error logfile
	//
	if(inputadam.getInput(0) == 1)
	{
		logFile("Cabinet door is open");
	}

	//
	//write to Adam1 of 6015 for control the outdoor temperature
	//the second parameter mus be 3 for reading the ADAM 6015
	//
	writeToAdam(itsReadConnection0, 3);
	readAdam(itsReadConnection0, 0);
	writeToAdam(itsReadConnection1, 3);
	readAdam(itsReadConnection1, 1);
	
	//
	//With startSetup you set the start temperature values of the system
	//
	//parameter time.getMonth() you get this month
	//
	//with time.getTime(1) get you hours and minutes as one value, by example 2359. 
	//this stands for 23 hours and 59 seconds
	//
	calculate.startSetup(time.getMonth(), time.getTime(1));
	
	//
	//Control the setpoint of this moment and is to high of low 
	//
	if(calculate.getSetpoint() == 37)
	{
		logFile("Maximum setpoint is set, keep an eye on the temperatures");
	}
	if(calculate.getSetpoint() == 5)
	{
		logFile("Minimum setpoint is set, keep an eye on the themperatures");
	}
	
	//
	//Get time for screen values and the logFile of the temperatures
	//
	time.getTimeDate();
	
	//
	//print the information on screen with screen and make a file with LogTemperatures
	//
	logPrint("screen");
	logPrint("LogTemperatures");

	return (HEATHREUGLATION_OK);
}

//
// This function you use to control the temperatures, set the fan's on or off,
// control the door and write logFiles of temperatures and when the are ploblems 
//
void HeathRegulationControl::controlsTemperatures()
{
	time.updateDateTime();
	itsNumberOfLogs = 0;
	itsRTH = 0;
	
	//
	//If the connectionFlag is not set, make a new Connection
	//
	if(itsConnectionFlag == false)
	{
		makeConnection();
		logFile("Made a new connection to the ADAMS");
	}

	//
	// read the temperature sensors of the 2 ADAMS 6015
	// ON THIS MOMENT THERE IS ONE ADAM 6015 INSTALLED
	//
	writeToAdam(itsReadConnection0, 3);
	writeToAdam(itsReadConnection1, 3);
	readAdam(itsReadConnection0, 0);
	readAdam(itsReadConnection1, 1);

	//
	//Control the temperatures and set the fans on or of
	//
	for(int subrack = 1; subrack < 5; subrack++)
	{
		itsRTH += calculate.heathControl((subrack*3), (subrack-1));
	}
	writeToAdam6050();

	//
	// write the new setpoint, if updateValue != 0
	//
	calculate.calculateSetpoint();
	if(calculate.getSetpoint() == 37)
	{
		logFile("Maximum setpoint is set, keep an eye on the temperatures");
	}
	if(calculate.getSetpoint() == 5)
	{
		logFile("Minimum setpoint is set, keep an eye on the temperatures");
	}

	logPrint("screen");

	//
	// each minute update values and control system
	//
	if(time.getTime(5) > 49 && time.getTime(5) <= 59)		
	{
		logPrint("LogTemperatures");
		
		schrijfFile(); // moet er nog uit

		//
		//write to Adam 6050 for control the doors
		//the second parameter must be 1 for reading the ADAM 6050
		//
		writeToAdam(itsWriteConnection, 1);	
		readAdam(itsWriteConnection, 2);
		
		calculate.averageUpdate(time.getTime(0));

		// if door is open, write to logfile
		if(inputadam.getInput(0) == 1)
		{
			logFile("Cabinet door is open");
		} 

		closeConnection();
		logFile("Close the connection to the ADAMS");
	} 
}


//
//print() information on screen or in a logfile
//
void HeathRegulationControl::logPrint(string function) {		
	// LET OP: Nummering moet van 0..6 lopen niet van 1..7
	
	double tempory = 0.0;
	int16 number = -1;
	FILE *fp;
	
	if(function == "LogTemperatures")
	{
		fp = fopen("temperaturen.txt", "a");
	} else {
		fp = stderr;
	}
	
		
	for(int16 j=0; j<14; j++)
	{
		tempory = calculate.getTemperatures(j);				// get the values of the sensors out program
		int16 tAmb = tempory * 100;
		switch(j)
		{
			case 0: fprintf(fp, "%s", time.getTimeDate());
					fprintf(fp, "Outdoor Temperature=%5.2f \n", calculate.getTemperatures(0));//itsTempory);
					fprintf(fp, "Setpoint=%5.2f \n", calculate.getSetpoint());
				break;
			case 1: case 4: case 7: case 10:
					fprintf(fp, "Subrack %i: Front:RT%2i=%5.2f  ", number+=1, j, tempory);
					// By temperatures higher than 60 degree, there is a problem
					// By tAmb with a value of 11008 there is no connection
					if(tempory >= 45 && tAmb != 11008 && function == "LogTemperatures")
					{
						logFile( "High temperature, control the fan of the FRONT DOOR of Rack ", number);
					}
				break;
			case 2:	case 5: case 8: case 11:
					fprintf(fp, " Rear:RT%2i=%5.2f  ", j, tempory);
					// By temperatures higher than 60 degree, there is a problem
					// By tAmb with a value of 11008 there is no connection
					if(tempory >= 60 && tAmb != 11008 && function == "LogTemperatures")
					{
						logFile( "High temperature, control the fan of the BACK DOOR of Rack ", number);
					}
				break;
			case 3: case 6: case 9: case 12:
					fprintf(fp, " Control:RT%2i=%5.2f  Fans=%s \n", j, tempory, (calculate.getStateFans((j/3)-1)==1) ? "on" : "off");
				break;
		}
	}
	fprintf(fp, "\n" );

	if(function == "LogTemperatures")
	{
		fclose(fp);
	}
}



void HeathRegulationControl::logFile(char *problem, int16 number) 
{
	//wegschrijven naar file//

	FILE	*lf;

	lf = fopen("logfile.txt", "a");

	if (itsNumberOfLogs == 0)
	{
		fprintf(lf, "\n %s", time.getTimeDate());
	}
	
	
	if (number >= 0)
	{
		fprintf(lf, "	%s%i \n",		problem, number);
		itsNumberOfLogs = itsNumberOfLogs + 1;
	} 
	else
	{
		fprintf(lf, "	%s \n", problem);
		itsNumberOfLogs = itsNumberOfLogs + 1;
	}

	fclose(lf);
}


//
// Get ports and addresses from a file
//
int32 HeathRegulationControl::readInputFile()
{
	int counter = 0;

	std::vector<string> v;
	
	string filenaam = "inputfile\\startValues.txt" ;
	std::ifstream invoer(filenaam.c_str() );
	string itsX;
	
	if(invoer.fail())
	{
		logFile("Problems with reading from the input file");
		return (NO_INPUT);
	}

	while(invoer.good())
	{
		
		invoer >> itsX;
		v.push_back(itsX);
			
		counter++;
	}

	// controleren van adressen benamingen

	for(int32 pos = 0; pos < counter; ++pos)
	{
		if(v[pos] == "addressAdam1:") 
		{
			itsAddress1 = v[pos+1];
			itsPort1 = atoi(v[pos+3].c_str());
			itsProtocol1 = atoi(v[pos+5].c_str());
		} 
		if(v[pos] == "addressAdam2:")
		{
			itsAddress2 = v[pos+1];
			itsPort2 = atoi(v[pos+3].c_str());
			itsProtocol2 = atoi(v[pos+5].c_str());
		}
		if(v[pos] == "addressAdam3:")
		{
			itsAddress3 = v[pos+1];
			itsPort3 = atoi(v[pos+3].c_str());
			itsProtocol3 = atoi(v[pos+5].c_str());
		}
	}

	invoer.clear();
	invoer.close();
	
	return (HEATHREUGLATION_OK);
}

string HeathRegulationControl::errstr () const
{
	static char const *socketErrStr[] = {
		"OK",
		"Cabinet door is open",
		"Can't create read connection (%d: %s)",
		"Can't create write connection",
		"Can't read to ADAM (%d: %s)",
		"Can't write to ADAM",
		"Temperature are to high on the backside (%d: %s)",
		"Temperature are to high on the frontside (%d: %s)",
		"Wrong address from inputfile (%d: %s)",
		"No more clients (%d: %s)",
		"Wrong port from inputfile (%d: %s)",
		"Wrong connection type (%d: %s)",
		"No input file with connections values"
	};  

	if (itsErrno > 0) {
    	return ("");
	}

 	return 0;  //sprintf("%d %d %s ", err, errInt, errChar);
		
		//formatString(socketErrStr[-itsErrno], itsSysErrno, strerror(itsSysErrno)));
}


//
//schrijfFile() for test fase. This file can you use in excel to make a grafic ot the data
//
void HeathRegulationControl::schrijfFile() {
	//wegschrijven naar file//

	FILE	*hp;

	hp = fopen("temperaturener.txt", "a");

	char dateStr [9];
	char timeStr [9];
	_strdate( dateStr);
	_strtime( timeStr);
	fprintf(hp, "%s - ", dateStr);
	fprintf(hp, "%s ", timeStr);
	fprintf(hp, "%3.2f ", calculate.getTemperatures(0));
	fprintf(hp, "%3.2f ", calculate.getTemperatures(1));
	fprintf(hp, "%3.2f ", calculate.getTemperatures(2));
	fprintf(hp, "%3.2f ", calculate.getTemperatures(3));
	fprintf(hp, "%3.2f ", calculate.getTemperatures(4));
	fprintf(hp, "%3.2f ", calculate.getTemperatures(5));
	fprintf(hp, "%3.2f ", calculate.getTemperatures(6));
	fprintf(hp, "%3.2f ", calculate.getSetpoint());
	fprintf(hp, "%3.2f ", calculate.getAverage());
	fprintf(hp, "Outdoor Temperature=%5.2f ", calculate.getTemperatures(6));
	fprintf(hp, "(Buiten_regeling_%s)", (calculate.getStateFans((4/3)-1)==1) ? "aan" : "uit");
	fprintf(hp, "%3.2f ", calculate.getMaxAverage());
	fprintf(hp, "%3.2f \n", calculate.getMinAverage());

	fclose(hp);
}




//} //close Lofar namespace


