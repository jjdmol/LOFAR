#ifndef FOURPRODUCTCORRELATORTESTER_H
#define FOURPRODUCTCORRELATORTESTER_H

#include <AOFlagger/msio/types.h>

class FourProductCorrelatorTester
{
	public:
		FourProductCorrelatorTester(class Model &model, class UVImager &imager, class Observatorium &observatorium);
	
		void SimulateObservation(num_t delayDirectionDEC, num_t delayDirectionRA, num_t frequency);
	
		void SimulateCorrelation(num_t delayDirectionDEC, num_t delayDirectionRA, const class AntennaInfo &a1, const class AntennaInfo &a2, const class AntennaInfo &a3, const class AntennaInfo &a4, num_t frequency, double totalTime, double integrationTime);
	private:
		class Model &_model;
		class UVImager &_imager;
		class Observatorium &_observatorium;
};

#endif
