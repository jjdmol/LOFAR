#ifndef RAYLEIGHFITTER_H
#define RAYLEIGHFITTER_H

#include "loghistogram.h"

class RayleighFitter
{
 public:
	void Fit(double minVal, double maxVal, LogHistogram &hist, double &sigma, double &n);
	LogHistogram *_hist;
	double _minVal, _maxVal;
};

#endif
