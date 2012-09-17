/*
//this class can you use for controle input from the ADAM 6050
//on this moment this class control only the doorClosers. 
//the value that you can get from ADAM 6050 is between 0 and 4096
//if you use all the 12 Digital inputs
*/

#include  "DigitalInputControl.h"


//namespace LOFAR 
//{
DigitalInputControl::DigitalInputControl()
{
	// there are 12 input ports on an ADAM 6050
	setBit = 11;
}

DigitalInputControl::~DigitalInputControl()
{
}

//
//This function set the right bits true from the input value
//MaximumValue have the default value of 2048
//
int32 DigitalInputControl::setInput(int32 digitalInput, int32 maximumValue)
{
	inputBit[setBit] = digitalInput/maximumValue;

	if(maximumValue <= 1)
	{
		setBit = 11;
		return 0;
	}	
	else if(digitalInput < maximumValue)
	{
		setBit= setBit - 1;
		return setInput(digitalInput, (maximumValue/2));
	}	
	else 
	{
		setBit= setBit - 1;
		return setInput((digitalInput%maximumValue), (maximumValue/2));
	}
}


//
//returns the value of the selected bit
//
int16 DigitalInputControl::getInput(int16 selectBit)
{
	return inputBit[selectBit];
}

//} // close namespace lofar