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

#include <AOFlagger/rfi/strategy/strategy.h>
#include <AOFlagger/rfi/strategy/xmlwriter.h>

template<typename T>
class Parameter {
	public:
		Parameter() : _isSet(false) { }
		Parameter(const T &val) : _isSet(true), _value(val) { }
		Parameter(const Parameter<T> &source)
			: _isSet(source._isSet), _value(source._value) { }
		Parameter &operator=(const Parameter<T> &source)
		{
			_isSet = source._isSet;
			_value = source._value;
		}
		Parameter &operator=(const T &val)
		{
			_isSet = true;
			_value = val;
			return *this;
		}
		bool IsSet() const { return _isSet; }
		operator T() const { return _value; }
	private:
		bool _isSet;
		T _value;
};

int main(int argc, char *argv[])
{
	std::cout << 
			"RFI strategy file writer\n"
			"This program will write an RFI strategy to a file, to run it with the\n"
			"rficonsole or the rfigui.\n\n"
			"Author: André Offringa (offringa@astro.rug.nl)\n"
			<< std::endl;

	bool threadCountSet = false;
	size_t threadCount = 3;

	size_t parameterIndex = 1;
	while(parameterIndex < (size_t) argc && argv[parameterIndex][0]=='-')
	{
		std::string flag(argv[parameterIndex]+1);

		if(flag == "b")
		{
		}
		else if(flag == "j") { ++parameterIndex; threadCountSet = true; threadCount = atoi(argv[parameterIndex]); }
		else
		{
			std::cerr << "Incorrect usage; parameter \"" << argv[parameterIndex] << "\" not understood.\nType rfistrategy without parameters for a list of commands." << std::endl;
			return 1;
		}
	}
	if((int) parameterIndex > argc-2)
	{
		std::cerr << "Usage: " << argv[0] << " [options] <profile> <filename>\n\n"
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
			"-j or -threads <threadcount>\n"
			"  Set number of threads to use. Each thread will independently process\n"
			"  whole baselines, thus this has implications on both memory usage and\n"
			"  CPU usage. Defaults to 3, also overridable in rficonsole.\n"
			"-p or -pol <all/auto/stokesi>\n"
			"  Specify how to process the polarizations. Independent of this setting,\n"
			"  the flags of all polarizations will be or-ed together and all polarizations\n"
			"  will be set to that value.\n"
			"-s or -sensitivity <threshold factor>\n"
			"  Set a factor that is applied to each (sum)threshold operation. Higher\n"
			"  values mean higher thresholds, thus less flagged samples. Default: 1.\n"
			"-ws or -window-size <width> <height>\n"
			"  Window size used in smoothing. Integers. \n"
			"  Default: 40 channels x 30 time steps (pulsar strategy: 40 x 1)\n"
			"-ks or -kernel-size <width> <height>\n"
			"  Gaussian kernel size used for smoothing. Floats. \n"
			"  Default: 15.0 channels x 7.5 time steps.\n"
			"\nScripts are recommended to use the long option names.\n";
//			"-t  or -time <time start index>-<time end index>\n"
		return 1;
	}

	std::string profile(argv[parameterIndex]), filename(argv[parameterIndex+1]);

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
		std::cerr << "Unknown profile: " << profile << std::endl;
		return 1;
	}

	if(threadCountSet)
		rfiStrategy::Strategy::SetThreadCount(*strategy, threadCount);

	rfiStrategy::XmlWriter writer;
	std::cout << "Writing strategy..." << std::endl;
	writer.WriteStrategy(*strategy, filename);
	delete strategy;
}
