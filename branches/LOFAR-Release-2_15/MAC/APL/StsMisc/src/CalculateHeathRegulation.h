#ifndef CALCULATEHEATHREGULATION_H
#define CALCULATEHEATHREGULATION_H

#include "import/LofarTypes.h"
#include "import/lofar_string.h"
#include "socket.h"



//namespace LOFAR 
//{
class CalculateHeathRegulation 
{
	
	public:

		//
		// constructor
		//
		 CalculateHeathRegulation();
		 
		 //
		 //Set values get from HeathRegulationControl
		 //
		 void setup(int32 samplePerMinutes, int32 numberOfDays);


		//
		// destructor
		//
		~CalculateHeathRegulation();

		//
		// calculate the setpoint for the regulation
		//
		void calculateSetpoint();

		//
		//  start setpoint when the program start.
		//
		void startSetup(int32 systemMonth, int32 systemTime);
		
		//
		//	get setpoint in other class
		//
		double getSetpoint();

		//
		// Shows which fans are on or off
		//
		int32 getStateFans(int16 selectFan);

		//
		//	calculate the temperatures from the sensors
		//
		void calculateTemp(int16 &T, int16 selectSensor);

		//
		// Controls the fans of the air to air exchanger
		//
		int32 heathControl(int16 selectSensor, int16 selectFan);
		
		//
		// update the setpoint slowy in 12 hours, every sample
		//
		void averageUpdate(int32 systemTime);

		//
		// get temperatures in other classes
		//
		double getTemperatures(int16 selectSensor);

		//
		// methodes die na het testen eruit kunnen
		//
		double getAverage();
		double getMinAverage();
		double getMaxAverage();

	

	private:
		//
		// the datamembers
		//
		int32 itsSamplePerMinute;				// number of sample in a minute
		int32 itsNumberOfDays;					// This value set the number of day's for the average 

		double itsMaxPoint[3];					// the highest temperature of a number of each day's
		double itsMinPoint[3];					// the lowest temperature of a number of each day's

		int32 RTH[4];

		double itsTimeDifference;
		double itsDayMomentTamb;				// 12/1200   (dag|nacht verschil / een periode van 12 uur) 

		double itsSetpoint;						// setpoint where the regulation controls the temperature in the Cabinets 
		double itsMaxTemp;						// Maxium value of hysterese 
		double itsMinTemp;						// Minium value of hysterese

		double itsMinAverage;					// Average of the lowest temperature from each day of the last 3 days.
		double itsMaxAverage;					// Average of the highest temperature from each day of the last 3 days.
		double itsAverage;						// Average of minAverage en maxAverage
		double itsOldAverage;					// The old value of average before average get his new value
		
		double itsSetPointStering;				// Value that change the setpoint slow to his new value in 12 hours

		double itsMaxTamb;						// maxium Temperature that is measurement with outside temperature sensor (start with temperature that is impossible in the Netherlands)
		double itsMinTamb;						// minium Temperature that is measurement with outside temperature sensor (start with temperature that is impossible as the lowest temperature in the Netherlands)

		double itsUpdateValue;					// Value that update the setpoint every sample

		


		// temperature sensors
		double RT[14];

};
//} close namespace LOFAR

#endif