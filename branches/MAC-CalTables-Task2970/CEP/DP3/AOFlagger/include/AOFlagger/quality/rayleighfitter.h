#ifndef RAYLEIGHFITTER_H
#define RAYLEIGHFITTER_H

#include "loghistogram.h"

class RayleighFitter
{
 public:
	void Fit(double minVal, double maxVal, LogHistogram &hist, double &sigma, double &n);
	static double SigmaEstimate(LogHistogram &hist);
	static double NEstimate(LogHistogram &hist, double rangeStart, double rangeEnd);
	static void FindFitRangeUnderRFIContamination(double minPositiveAmplitude, double sigmaEstimate, double &minValue, double &maxValue);
	static double RayleighValue(double sigma, double n, double x)
	{
		double sigmaP2 = sigma*sigma;
		return n * x / (sigmaP2) * exp(-x*x/(2*sigmaP2));
	}
	
	LogHistogram *_hist;
	double _minVal, _maxVal;
};

#endif
