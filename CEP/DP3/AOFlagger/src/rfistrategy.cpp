/***************************************************************************
 *   Copyright (C) 2008 by A.R. Offringa   *
 *   offringa@astro.rug.nl   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <iostream>
#include <stdexcept>

#include <AOFlagger/rfi/strategy/strategy.h>
#include <AOFlagger/rfi/strategy/xmlwriter.h>

using namespace rfiStrategy;
using namespace std;

template<typename T>
class Parameter {
	public:
		Parameter() : _isSet(false), _value() { }
		Parameter(const T val) : _isSet(true), _value(val) { }
		Parameter(const Parameter<T> &source)
			: _isSet(source._isSet), _value(source._value) { }
		Parameter &operator=(const Parameter<T> &source)
		{
			_isSet = source._isSet;
			_value = source._value;
		}
		Parameter &operator=(T val)
		{
			_isSet = true;
			_value = val;
			return *this;
		}
		bool IsSet() const { return _isSet; }
		operator T() const
		{
			return Value();
		}
		T Value() const
		{
			if(_isSet)
				return _value;
			else
				throw runtime_error("Trying to access unset parameter");
		}
	private:
		bool _isSet;
		T _value;
};

int main(int argc, char *argv[])
{
	cout << 
			"RFI strategy file writer\n"
			"This program will write an RFI strategy to a file, to run it with the\n"
			"rficonsole or the rfigui.\n\n"
			"Author: André Offringa (offringa@astro.rug.nl)\n"
			<< endl;

	Parameter<enum BaselineSelection> baselineSelection;
	Parameter<enum DataKind> dataKind;
	Parameter<bool> frequencyBasedFlagging;
	Parameter<bool> flagStokes;
	Parameter<size_t> threadCount;
	Parameter<pair<double, double> > kernelSize;
	Parameter<enum PolarisationType> polarisation;
	Parameter<num_t> sensitivity;
	Parameter<pair<size_t, size_t> > windowSize;

	size_t parameterIndex = 1;
	while(parameterIndex < (size_t) argc && argv[parameterIndex][0]=='-')
	{
		string flag(argv[parameterIndex]+1);

		if(flag == "b" || flag == "baseline")
		{
			++parameterIndex;
			string baselineStr(argv[parameterIndex]); 
			if(baselineStr == "all") baselineSelection = All;
			else if(baselineStr == "auto") baselineSelection = AutoCorrelations;
			else if(baselineStr == "cross") baselineSelection = CrossCorrelations;
			else throw runtime_error("Unknown baseline selection for -b");
		}
		else if(flag == "c" || flag == "column")
		{
			++parameterIndex;
			string columnStr(argv[parameterIndex]); 
			if(columnStr == "DATA") dataKind = ObservedData;
			else if(columnStr == "CORRECTED_DATA") dataKind = CorrectedData;
			else if(columnStr == "residuals") dataKind = ResidualData;
			else throw runtime_error("Column parameter -c can only be followed by DATA, CORRECTED_DATA or residuals");
		}
		else if(flag == "ff" || flag == "freq-based-flagging")	{ frequencyBasedFlagging = true;	}
		else if(flag == "fs" || flag == "flag-stokes")	{ flagStokes = true;	}
		else if(flag == "j" || flag == "threads") { ++parameterIndex; threadCount = atoi(argv[parameterIndex]); }
		else if(flag == "ks" || flag == "kernel-size")
		{
			++parameterIndex;
			kernelSize = pair<double, double>( atof(argv[parameterIndex]), atof(argv[parameterIndex+1]));
			++parameterIndex;
		}
		else if(flag == "p" || flag == "polarizations")
		{
			++parameterIndex;
			string polStr(argv[parameterIndex]);
			if(polStr == "all") polarisation = DipolePolarisation;
			else if(polStr == "auto") polarisation = AutoDipolePolarisation;
			else if(polStr == "stokesi") polarisation = StokesIPolarisation;
			else throw runtime_error("Unknown polarisation type for -p");
		}
		else if(flag == "s" || flag == "sensitivity") { ++parameterIndex; sensitivity = atof(argv[parameterIndex]); }
		else if(flag == "ws" || flag == "window-size")
		{
			++parameterIndex;
			windowSize = pair<size_t, size_t>( atoi(argv[parameterIndex]), atoi(argv[parameterIndex+1]));
			++parameterIndex;
		}
		else
		{
			cerr << "Incorrect usage; parameter \"" << argv[parameterIndex] << "\" not understood.\nType rfistrategy without parameters for a list of commands." << endl;
			return 1;
		}
		++parameterIndex;
	}
	if((int) parameterIndex > argc-2)
	{
		cerr << "Usage: " << argv[0] << " [options] <profile> <filename>\n\n"
			"Profiles:\n"
			"  fast     Fastest strategy that provides a moderate\n"
			"           result in quality. Will flag the measurement set using\n"
			"           Stokes-I values.\n"
			"  average  Best trade-off between speed and quality. Will\n"
			" /default  flag the measurement set using XX and YY values using\n"
			"           an average sliding window size.\n"
			"  best     Highest quality detection. Will flag each\n"
			"           polarization individually.\n"
			"  pedantic Pedantic detection. Like the 'best' profile,\n"
			"           but will flag all channels completely that are still\n"
			"           deviating from others after flagging. Flags about twice\n"
			"           as much.\n"
			"  pulsar   Like the best strategy, but will not assume\n"
			"           smoothness in time; especially usefull for pulsar\n"
			"           observations.\n"
			"<filename> is the filename to which the strategy is written. This\n"
			"file should have the extension \".rfis\".\n\n"
			"All profiles implement the SumThreshold method. The details of this\n"
			"method are described in the article named \"Post-correlation radio\n"
			"frequency interference classiﬁcation methods\", submitted to MNRAS.\n"
			"\n"
			"Possible options:\n"
//			"-a -antennae"
			"-b or -baseline <all/auto/cross>\n"
			"  Specify which baselines to process (default: all)\n"
			"-c or -column <DATA/CORRECTED_DATA/residual>\n"
			"  Specify which column to use when reading the data (default: DATA)\n"
			"  (residual = MODEL_DATA - CORRECTED_DATA)\n"
//			"-cf or -clear-flags\n"
//			"-f  or -freq <channel start>-<channel end>\n"
			"-ff or -freq-based-flagging\n"
			"  Overrides default behaviour of smoothing in both time and frequency:\n"
			"  does not assume time smoothness. Useful e.g. if strong time-dependent\n"
			"  sources are expected (e.g. pulsars). Default: not enabled, except in\n"
			"  \'pulsar\' strategy.\n"
			"-fs or -flag-stokes\n"
			"  Will calculate the stokes I, Q, U and V components from the orthogonal\n"
			"  components (calculated with I=XX + YY, Q=XX - YY, U=XY + YX, V=i*XY - YX),\n"
			"  and use these values for flagging. All polarisations nead to be read for this,\n"
			"  thus this option is only useful together with '-polarizations all'.\n"
			"-j or -threads <threadcount>\n"
			"  Set number of threads to use. Each thread will independently process\n"
			"  whole baselines, thus this has implications on both memory usage and\n"
			"  CPU usage. Defaults to 3, also overridable in rficonsole.\n"
			"-ks or -kernel-size <width> <height>\n"
			"  Gaussian kernel size used for smoothing. Floats. \n"
			"  Default: 15.0 channels x 7.5 time steps.\n"
			"-p or -polarizations <all/auto/stokesi>\n"
			"  Specify what polarizations to read and process. Independent of this setting,\n"
			"  the flags of all polarizations will be or-ed together and all polarizations\n"
			"  will be set to that value.\n"
			"-s or -sensitivity <threshold factor>\n"
			"  Set a factor that is applied to each (sum)threshold operation. Higher\n"
			"  values mean higher thresholds, thus less flagged samples. Default: 1.\n"
//			"-t  or -time <time start index>-<time end index>\n"
			"-ws or -window-size <width> <height>\n"
			"  Window size used in smoothing. Integers. \n"
			"  Default: 40 channels x 30 time steps (pulsar strategy: 40 x 1)\n"
			"\nScripts are recommended to use the long option names.\n";
		return 1;
	}

	string profile(argv[parameterIndex]), filename(argv[parameterIndex+1]);

	rfiStrategy::Strategy *strategy = new rfiStrategy::Strategy();
	if(profile == "fast")
		strategy->LoadFastStrategy();
	else if(profile == "average" || profile == "default")
		strategy->LoadAverageStrategy();
	else if(profile == "best")
		strategy->LoadBestStrategy();
	else if(profile == "pedantic")
		strategy->LoadBestStrategy(true);
	else if(profile == "pulsar")
		strategy->LoadBestStrategy(true, true);
	else {
		cerr << "Unknown profile: " << profile << endl;
		return 1;
	}

	if(baselineSelection.IsSet())
		Strategy::SetBaselines(*strategy, baselineSelection);
	if(dataKind.IsSet())
		Strategy::SetDataKind(*strategy, dataKind);
	if(flagStokes.IsSet())
		Strategy::SetFlagStokes(*strategy, flagStokes.Value());
	if(frequencyBasedFlagging.IsSet() && frequencyBasedFlagging.Value())
		Strategy::SetTransientCompatibility(*strategy);
	if(threadCount.IsSet())
		Strategy::SetThreadCount(*strategy, threadCount);
	if(kernelSize.IsSet())
		Strategy::SetFittingKernelSize(*strategy, kernelSize.Value().first, kernelSize.Value().second);
	if(polarisation.IsSet())
		Strategy::SetPolarisations(*strategy, polarisation);
	if(sensitivity.IsSet())
		Strategy::SetMultiplySensitivity(*strategy, sensitivity);
	if(windowSize.IsSet())
		Strategy::SetFittingWindowSize(*strategy, windowSize.Value().first, windowSize.Value().second);

	rfiStrategy::XmlWriter writer;
	cout << "Writing strategy..." << endl;
	writer.WriteStrategy(*strategy, filename);
	delete strategy;
}
