/*
//this class can you use for controle input from the ADAM 6050
//on this moment this class control only the doorClosers. 
//the value that you can get from ADAM 6050 is between 0 and 4096
//if you use all the 12 Digital inputs
*/



#ifndef DIGITALINPUTCONTROL_H
#define DIGITALINPUTCONTROL_H

#include "import/LofarTypes.h"
#include "import/lofar_string.h"



//namespace LOFAR 
//{
class DigitalInputControl
{
	
	public:
		DigitalInputControl();
		~DigitalInputControl();
		
		//
		// This function split the value coming from the Adam 6050 up in
		// a bit patron of 12 bits. The bits returns 1 or 0.
		//		
		int32 setInput(int32 digitalInput, int32 maximumValue = 2048);

		//
		//read the value of the selected bit
		//
		int16 getInput(int16 selectBit);



	private:

		int16 inputBit[12];
		int16 setBit;

};
//} close namespace lofar

#endif