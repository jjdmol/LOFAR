#include <string>
#include <iostream>
#include "import/LofarTypes.h"
#include "import/lofar_string.h"
#include "HeathRegulationControl.h"


// { //namespace LOFAR


int32 main()
{
	const int32 sampleMinute = 60000 / 10000;
	const int32 numberOfDays = 3;


	// Initialize class HeathRegulationControl 
	HeathRegulationControl control(sampleMinute, numberOfDays);				

	// read adress and port for ADAMS
	control.readInputFile();
	
	// make connection to the ADAMS
	control.makeConnection();
	
	// Initialize start values
	control.startUpMode();										
	
	control.closeConnection();
	//  wait 30 seconds
	Sleep(30000);
	


	// loop for regulation of the temperatures in the cabinets
	while(1) 
	{

		// control the temperatures of the Cabinets
		control.controlsTemperatures();
		
		//  wait 10 seconds (an minute  is = 60000)
		Sleep (10000);
	}

		
	control.~HeathRegulationControl();

	return 0;
}


//} // Namespace Lofar

