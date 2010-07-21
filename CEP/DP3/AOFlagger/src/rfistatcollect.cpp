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
#include <fstream>
#include <stdexcept>
#include <map>
#include <cmath>
#include <iomanip>

#include <AOFlagger/msio/date.h>
#include <AOFlagger/rfi/rfistatistics.h>
#include <AOFlagger/util/rng.h>
#include <sys/stat.h>

using namespace std;

void readChannels(RFIStatistics &statistics, string &filename, bool autocorrelation)
{
	ifstream f(filename.c_str());
	string headers;
	getline(f, headers);
	while(!f.eof())
	{
		RFIStatistics::ChannelInfo channel;
		f
		>> channel.frequencyHz;
		if(f.eof()) break;
		f
		>> channel.totalCount
		>> channel.totalAmplitude
		>> channel.rfiCount
		>> channel.rfiAmplitude
		>> channel.broadbandRfiCount
		>> channel.lineRfiCount
		>> channel.broadbandRfiAmplitude
		>> channel.lineRfiAmplitude;
		statistics.Add(channel, autocorrelation);
	}
}

void readTimesteps(RFIStatistics &statistics, string &filename, bool autocorrelation)
{
	ifstream f(filename.c_str());
	string headers;
	getline(f, headers);
	while(!f.eof())
	{
		RFIStatistics::TimestepInfo timestep;
		f
		>> timestep.time;
		if(f.eof()) break;
		f
		>> timestep.totalCount
		>> timestep.totalAmplitude
		>> timestep.rfiCount
		>> timestep.rfiAmplitude
		>> timestep.broadbandRfiCount
		>> timestep.lineRfiCount
		>> timestep.broadbandRfiAmplitude
		>> timestep.lineRfiAmplitude;
		statistics.Add(timestep, autocorrelation);
	}
}

void readAmplitudes(RFIStatistics &statistics, string &filename, bool autocorrelation)
{
	ifstream f(filename.c_str());
	string headers;
	getline(f, headers);
	double maxCount = 0.0;
	double maxAmp = 0.0;
	while(f.good())
	{
		double centralLogAmplitude;
		RFIStatistics::AmplitudeBin amplitude;
		f
		>> amplitude.centralAmplitude;
		if(!f.good()) break;
		f
		>> centralLogAmplitude
		>> amplitude.count
		>> amplitude.rfiCount
		>> amplitude.broadbandRfiCount
		>> amplitude.lineRfiCount
		>> amplitude.featureAvgCount
		>> amplitude.featureIntCount
		>> amplitude.featureMaxCount
		>> amplitude.xxCount
		>> amplitude.xyCount
		>> amplitude.yxCount
		>> amplitude.yyCount
		>> amplitude.xxRfiCount
		>> amplitude.xyRfiCount
		>> amplitude.yxRfiCount
		>> amplitude.yyRfiCount;
		statistics.Add(amplitude, autocorrelation);
		if(amplitude.count/amplitude.centralAmplitude > maxCount)
		{
			maxCount = amplitude.count/amplitude.centralAmplitude;
			maxAmp = amplitude.centralAmplitude;
		}
	}
	std::cout << std::setprecision(14) << " mode~=" << maxAmp << '(' << maxCount << ')' << '\n';
}

void readBaselines(RFIStatistics &statistics, string &filename)
{
	ifstream f(filename.c_str());
	string headers;
	getline(f, headers);
	while(!f.eof())
	{
		RFIStatistics::BaselineInfo baseline;
		f
		>> baseline.antenna1;
		if(f.eof()) break;
		f
		>> baseline.antenna2
		>> baseline.antenna1Name
		>> baseline.antenna2Name
		>> baseline.baselineLength
		>> baseline.baselineAngle
		>> baseline.count
		>> baseline.totalAmplitude
		>> baseline.rfiCount
		>> baseline.broadbandRfiCount
		>> baseline.lineRfiCount
		>> baseline.rfiAmplitude
		>> baseline.broadbandRfiAmplitude
		>> baseline.lineRfiAmplitude;
		statistics.Add(baseline);
	}
}

void fitGaus(RFIStatistics &statistics)
{
	const std::map<double, class RFIStatistics::AmplitudeBin> &amplitudes = statistics.GetCrossAmplitudes();
	
	std::map<double, long unsigned> distribution;
	for(std::map<double, class RFIStatistics::AmplitudeBin>::const_iterator i=amplitudes.begin();i!=amplitudes.end();++i)
	{
		distribution.insert(std::pair<double, long unsigned>(i->first, i->second.featureAvgCount));
	}
	
	// Find largest value
	long unsigned max = distribution.begin()->second;
	double ampOfMax = distribution.begin()->first;
	for(std::map<double, long unsigned>::const_iterator i=distribution.begin();i!=distribution.end();++i)
	{
		if(i->second > max) {
			max = i->second;
			ampOfMax = i->first;
		}
	}
	std::cout << "Maximum occurring amplitude=" << ampOfMax << std::endl;
	std::cout << "Count=" << max << std::endl;
	double promileArea = 0.0;
	double promileLimit = max / 1000.0;
	double promileStart = 0.0, promileEnd;
	long unsigned popSize = 0;
	for(std::map<double, long unsigned>::const_iterator i=distribution.begin();i!=distribution.end();++i)
	{
		if(i->second > promileLimit) {
			promileArea += i->second;
			promileEnd = i->first;
			if(promileStart == 0.0)
 				promileStart = i->first;
		}
		popSize += i->second;
	}
	double halfPromileArea = promileArea / 2.0;
	double mean = 0.0;
	promileArea = 0.0;
	for(std::map<double, long unsigned>::const_iterator i=distribution.begin();i!=distribution.end();++i)
	{
		if(i->second > promileLimit) {
			promileArea += i->second;
		}
		if(promileArea > halfPromileArea)
		{
			mean = i->first;
			break;
		}
	}
	std::cout << "Mean=" << mean << std::endl;
	double halfStddevArea = 0.682689492137 * halfPromileArea;
	double stddev = 0.0;
	for(std::map<double, long unsigned>::const_reverse_iterator i=distribution.rbegin();i!=distribution.rend();++i)
	{
		if(i->first <= mean) {
			halfStddevArea -= i->second;
		}
		if(halfStddevArea <= 0.0)
		{
			stddev = i->first;
			break;
		}
	}
	std::cout << "Stddev=" << stddev << std::endl;

	ofstream f("fit.txt");
	f
	<< setprecision(15)
	<< "Amplitude\tLogAmplitude\tCount\tCount\tGaussian\tGaussian\tRayleigh\tRayleigh\n";
	for(std::map<double, long unsigned>::const_iterator i=distribution.begin();i!=distribution.end();++i)
	{
		if(i != distribution.begin())
		{
			double g = RNG::EvaluateGaussian(i->first - mean, stddev)*popSize;
			double r = RNG::EvaluateRayleigh(i->first, mean)*popSize;
			double binsize = i->first / 100.0;
			g *= binsize;
			r *= binsize;
			f
			<< i->first << '\t' << log10(i->first) << '\t'
			<< i->second << '\t' << log10(i->second) << '\t'
			<< g << '\t' << log10(g) << '\t' << r << '\t' << log10(r) << '\n';
		}
	}
}

int main(int argc, char **argv)
{
	cout << 
		"RFI statistics collector\n"
		"This program will collect statistics of several rficonsole runs and\n"
		"write them in one file.\n\n"
		"Author: André Offringa (offringa@astro.rug.nl)\n"
		<< endl;

	if(argc == 1)
	{
		std::cerr << "Usage: " << argv[0] << " [files]" << std::endl;
	}
	else
	{
		ofstream amplitudeSlopeFile("amplitudeSlopes.txt");
		amplitudeSlopeFile << "0.01-0.1\t10-100\tcount\n";
		std::map<double, double> frequencyFlags;
		std::map<double, long unsigned> timeTotalCount, timeFlagsCount;
		RFIStatistics statistics;

		for(int i=1;i<argc;++i)
		{
			string filename = argv[i];
			cout << "Reading " << filename << "..." << endl;
			if(filename.find("counts-channels-auto.txt")!=string::npos)
				readChannels(statistics, filename, true);
			else if(filename.find("counts-channels-cross.txt")!=string::npos)
				readChannels(statistics, filename, false);
			else if(filename.find("counts-timesteps-auto.txt")!=string::npos)
				readTimesteps(statistics, filename, true);
			else if(filename.find("counts-timesteps-cross.txt")!=string::npos)
				readTimesteps(statistics, filename, false);
			else if(filename.find("counts-amplitudes-auto.txt")!=string::npos)
				readAmplitudes(statistics, filename, true);
			else if(filename.find("counts-amplitudes-cross.txt")!=string::npos)
			{
				readAmplitudes(statistics, filename, false);
				RFIStatistics single;
				readAmplitudes(single, filename, false);
				amplitudeSlopeFile
				<< single.AmplitudeCrossSlope(0.01, 0.1) << '\t'
				<< single.AmplitudeCrossSlope(10.0, 100.0) << '\t'
				<< single.AmplitudeCrossCount(10.0, 100.0) << '\n';
			}
			else if(filename.find("counts-baselines.txt")!=string::npos)
				readBaselines(statistics, filename);
			else
				throw runtime_error("Could not determine type of file.");
		}
		fitGaus(statistics);
		statistics.Save();
		std::cout << "Cross correlations: "
		<< (round(statistics.RFIFractionInCrossChannels()*10000)/100) << "% RFI in channels, "
		<< (round(statistics.RFIFractionInCrossTimeSteps()*10000)/100) << "% RFI in timesteps.\n"
		<< "Auto correlations: "
		<< (round(statistics.RFIFractionInAutoChannels()*10000)/100) << "% RFI in channels, "
		<< (round(statistics.RFIFractionInAutoTimeSteps()*10000)/100) << "% RFI in timesteps.\n"
		<< std::setprecision(14)
		<< "Cross correlation slope fit between 0.01 and 0.1 amplitude: "
		<< statistics.AmplitudeCrossSlope(0.01, 0.1) << "\n"
		<< "Cross correlation slope fit between 10 and 100 amplitude: "
		<< statistics.AmplitudeCrossSlope(10.0, 100.0) << "\n";
	}
}
