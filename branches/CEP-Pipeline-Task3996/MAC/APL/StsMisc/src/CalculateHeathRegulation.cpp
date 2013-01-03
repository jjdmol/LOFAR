#include "CalculateHeathRegulation.h"


//namespace LOFAR 
//{

CalculateHeathRegulation::CalculateHeathRegulation()//int32 samplePerMinutes, int32 numberOfDays)
//	: samplePerMinute(samplePerMinutes), numberOfDays(numberOfDays)
{
	this->itsSetpoint = 0.0;
	this->itsUpdateValue = 0.0;
	this->itsMinTamb = 50;
	this->itsMaxTamb = -50;
	this->itsTimeDifference = 0;
	this->itsDayMomentTamb = 0.0083333;		//difference average day|nicht temperature (10 degrees) / 1200 hours
	this->itsMinAverage = 0.0;					
	this->itsMaxAverage = 0.0;

}

void CalculateHeathRegulation::setup(int32 samplePerMinutes, int32 numberOfDays)
{
	this->itsSamplePerMinute = samplePerMinutes;
	this->itsNumberOfDays = numberOfDays;
}

CalculateHeathRegulation::~CalculateHeathRegulation()
{
}


//
// update setpoint every poll, when updateValue != 0
//
void CalculateHeathRegulation::calculateSetpoint()
{
	if(itsSetpoint >= 37 && itsUpdateValue >= 0)
	{
		itsSetpoint = 37;
		itsUpdateValue = 0;
	} 
	else if(itsSetpoint <= 5 && itsUpdateValue <= 0)
	{
		itsSetpoint = 5;
		itsUpdateValue = 0;
	} 
	else 
	{
		itsSetpoint += itsUpdateValue; 
	}		
	
	itsMaxTemp = itsSetpoint + 0.025;
	itsMinTemp = itsSetpoint - 0.025;

}

//
//	This function calculate the startsetpoint, when this program make a cold start
//	 RT[6] wordt uiteindelijk RT[0]
void CalculateHeathRegulation::startSetup(int32 systemMonth, int32 systemTime)
{
	int16 tAmb = RT[0] * 100;
	//
	// average month temperatures about the last 100 years coming form KNMI
	//
	double monthTempAverage[3][12] = 
	{
	{  4.4,  5.0, 8.6, 12.2, 17.0, 19.4, 21.4, 21.9, 18.2, 13.5, 8.4, 5.5},	//average month maxium temperature
	{  2.0,  2.1, 4.9,  7.5, 11.9, 14.4, 16.5, 16.5, 13.5,  9.6, 5.5, 3.2}, //average month day/night temperature
	{ -0.8, -0.8, 1.2,  2.7,  6.5,  9.1, 11.3, 11.1,  8.8,  5.6, 2.5, 0.5}  //average month minium temperature
	};
	
	//
	// fills the day by start with the average temperatures
	//
	for (int16 l = 0; l < (itsNumberOfDays); l++) 
	{

		itsMaxPoint[l] = monthTempAverage[0][systemMonth];
		itsMinPoint[l] = monthTempAverage[2][systemMonth];	
	}
		
	//
	//When the connection is not good, don't upgrade the last 
	//value with temperatures of that moment 
	//By a 11008 value you know that there is no connection with the ADAM
	//
	if(tAmb != 11008)
	{
		//
		// On the last day we set an actual temperature	that we have measure
		//
		for (int16 g = 2; g < (itsNumberOfDays); g++) 
		{
			// Calculate the time between 7:00 and 19:00 hours
			if(systemTime > 700 && systemTime <= 1900) 		
			{				
				//
				// set itsTimeDifference, that gives an idea of the day heating 
				//
				itsTimeDifference = 1900 - 400 - systemTime;			
				if(itsTimeDifference < 200)
				{
					itsTimeDifference = 0;
				}
				
				//
				// The average difference between day and night is about 10 degree Celsius
				// With itsTimeDiffefence make we an estimate of maximum and minimum 
				// temperature of that day
				//
				itsMinPoint[g] = RT[6 /*0*/] - (10 - (itsDayMomentTamb * itsTimeDifference));	// Schat de verwachtte laagste temperatuur van die dag
				itsMaxPoint[g] = RT[6 /*0*/] + (itsDayMomentTamb * itsTimeDifference);			// Schat de verwachtte hoogste temperatuur van die dag
			} else 
			{	
				// Calculate the espected temperatures in the night
				//	gives the expected temperatures before midnight
				if(systemTime > 1900)
				{											 
					itsTimeDifference = (2400 + 700) - systemTime - 400;
				} else {
					itsTimeDifference = 700 - systemTime;
				}
				
				if(itsTimeDifference < 200)
				{
					itsTimeDifference = 0;
				}			

				itsMaxPoint[g] = RT[6 /*0*/] + (10 - (itsDayMomentTamb * itsTimeDifference));
				itsMinPoint[g] = RT[6 /*0*/] - (itsDayMomentTamb * itsTimeDifference);
			}
		} 
	}

	//
	// telt de waarden van 3 dagen bij elkaar
	//
	for (int16 h = 0; h < itsNumberOfDays; h++) {
		itsMaxAverage += itsMaxPoint[h];
		itsMinAverage += itsMinPoint[h];
	} 

	//
	// Calculate the average temperature 
	//
	itsMaxAverage = (itsMaxAverage / itsNumberOfDays);
	itsMinAverage = (itsMinAverage / itsNumberOfDays);
	itsAverage = (itsMinAverage + itsMaxAverage) / 2;


	//
	// Calculate Setpoint by the start of this program
	//
	itsSetpoint = itsAverage + 11 + 5 + 1.5;  //average + verschil MaxTamb en Gemiddelde tamb + minimale verschil + marge RT1

	//
	// Control the setpoint. If setpoint higher than 37 degree Celsius, setpoint will be set on 37
	// When the setpoint is lower than 5 degree, setpoint will be set on 5. 
	// This is for safe the hardware in the Cabinets. 
	//
	if(itsSetpoint >= 37)
	{
		itsSetpoint = 37;
	} 
	if(itsSetpoint <= 5)
	{
		itsSetpoint = 5;
	}

	//
	// Set Setpoint hysterese of this regulation
	//
	itsMaxTemp = itsSetpoint + 0.025;
	itsMinTemp = itsSetpoint - 0.025;

}


//
// calculate temperatures of the sensors
//
void CalculateHeathRegulation::calculateTemp(int16 &T, int16 selectSensor) 
{	
	RT[selectSensor] = 0.0;
	
	// rtValue must be int32 to get values higher than 50 degree Celsius 
	int32 rtValue = htons(T);
	double temp = (rtValue / 327.5) - 50;
	
	RT[selectSensor] = temp;
	
}


//
// Send temperatures back out this class
//
double CalculateHeathRegulation::getTemperatures(int16 selectSensor)
{
	return RT[selectSensor];
}

//
// The fans of each rack will be controlled 
// RTH0 are the fans of rack 0. The fans are on, when RTH0 = 1
// RTH1 are the fans of rack 1. The fans are on, when RTH1 = 2
// RTH2 are the fans of rack 2. The fans are on, when RTH2 = 4
// RTH3 are the fans of rack 3. The fans are on, when RTH2 = 8
//
int32 CalculateHeathRegulation::getStateFans(int16 selectFan)
{			
	int16 stateFan;	
	
	// RTH0 == 3 in the test fase
	if(RTH[selectFan] == 3 || RTH[selectFan] == 2 || RTH[selectFan] == 4 || RTH[selectFan] == 8)
	{
		stateFan = 1;
	} 
	else
	{
		stateFan = 0;
	}

	return stateFan;
}

//
//Get the setpoint of this moment
//
double CalculateHeathRegulation::getSetpoint()
{
	return itsSetpoint;
}


//
// kan er later uit, wordt gebruikt om te testen of setpoint na 12 uur wel klopt.
//
double CalculateHeathRegulation::getAverage()
{
	double controlSetpoint;

	controlSetpoint = itsAverage + 11 + 5 + 1.5;
	
	
	return controlSetpoint;
}

//kan eruit, gebruikt voor testen
double CalculateHeathRegulation::getMaxAverage()
{
	return itsMaxAverage;
}

//kan eruit, gebruikt voor testen
double CalculateHeathRegulation::getMinAverage()
{
	return itsMinAverage;
}

//
// Regulate the temperatures of the Cabinets
//
int32 CalculateHeathRegulation::heathControl(int16 selectSensor, int16 selectFan)
{	
	if (RT[selectSensor] > itsMaxTemp)// maxTemp)			//Sensor 3, 6, 9 en 12 wordt op geregeld
	{
		//als alles goed is aangesloten. fan1 = 1, fan2 = 2, fan3 = 3 en fan4 = 4
		switch(selectFan)
		{
			case 0: RTH[0] = 3;  //must be 1 when you control all the 4 compartiments
					break;
			case 1:	RTH[1] = 2;
					break;
			case 2: RTH[2] = 4;
					break;
			case 3: RTH[3] = 8;
					break;
		}
	}
	if (RT[selectSensor] < itsMinTemp)// > RT3)
	{
		RTH[selectFan] = 0;
	}

	return RTH[selectFan]; 
}




//
// Update of the average temperatures and updateValue
//
void CalculateHeathRegulation::averageUpdate(int32 systemTime) 
{
	int32 tAmb = RT[6] * 100;			// wordt S[0]

	//
	// if there is no connection, there is no update for the maxTamb of a day and minTamb of a day 
	// else  maxTamb is searching for the maximum temperature of a day and minTamb for the minimum
	// temperature of the day
	//
	// RT[6] must be RT[0] the outdoor sensor, if every is right enclosed to every adam 
	//
	if(tAmb != 11008)  
	{
		if(RT[6] > itsMaxTamb) {				
			itsMaxTamb = RT[6];
		} else {
			itsMaxTamb = itsMaxTamb;
		}

		if(RT[6] < itsMinTamb) {
			itsMinTamb = RT[6];
		} else {
			itsMinTamb = itsMinTamb;
		}
	}

	//
	// update max values and calculate a new setpoint value
	//
	if((systemTime) >= 210000 && (systemTime ) <= 210059)
	{
		itsUpdateValue = 0;
		
		itsMaxPoint[0] = itsMaxPoint[1];
		itsMaxPoint[1] = itsMaxPoint[2];
		itsMaxPoint[2] = itsMaxTamb;
		
		// change only the temperature if there is a connection. in other way use
		// the old temperatures to get a realistic average temperature
		if(tAmb != 11009){
			itsMaxTamb = -50;
		}

		itsMaxAverage = 0;
		for(int16 j = 0; j < itsNumberOfDays; j++) {
			itsMaxAverage = itsMaxAverage + itsMaxPoint[j];
		}
		itsMaxAverage = (itsMaxAverage / itsNumberOfDays);
	
		itsOldAverage = itsAverage;
		itsAverage = 0;
		itsAverage = (itsMinAverage + itsMaxAverage) / 2;

		itsUpdateValue = itsAverage - itsOldAverage;
		itsUpdateValue = (itsUpdateValue / 12) / (60 * itsSamplePerMinute);

	}

	//
	// Update minium values and calculate a new setpoint
	//
	if((systemTime) >= 90000 && (systemTime) <= 90059)
	{
		itsUpdateValue = 0;

		itsMinPoint[0] = itsMinPoint[1];
		itsMinPoint[1] = itsMinPoint[2];
		itsMinPoint[2] = itsMinTamb;
		
		// change only the temperature if there is a connection. in other way use
		// the old temperatures to get a realistic average temperature
		if(tAmb != 11009){
			itsMinTamb = 50;			
		}
		
		itsMinAverage = 0;
		for(int16 k = 0; k < itsNumberOfDays; k++) {
			itsMinAverage = itsMinAverage + itsMinPoint[k];
		}
		itsMinAverage = (itsMinAverage / itsNumberOfDays);

		// Calculate the new average 
		itsOldAverage = itsAverage;
		itsAverage = 0;
		itsAverage = (itsMinAverage + itsMaxAverage) / 2;

		// Calculate the update value by each polling of the temperatures
		itsUpdateValue = itsAverage - itsOldAverage;
		itsUpdateValue = (itsUpdateValue / 12) / (60 * itsSamplePerMinute);  // update value setpoint every minute, but maximum 12 hours update
	
	}

}

//} //close Lofar namespace