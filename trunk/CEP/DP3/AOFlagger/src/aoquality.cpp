/***************************************************************************
 *   Copyright (C) 2011 by A.R. Offringa                                   *
 *   offringa@astro.rug.nl                                                 *
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

#include <AOFlagger/strategy/algorithms/statisticscollector.h>
#include <AOFlagger/msio/measurementset.h>

using namespace std;

void reportProgress(unsigned step, unsigned totalSteps)
{
	const unsigned twoPercent = (totalSteps+49)/50;
	if((step%twoPercent)==0)
	{
		if(((step/twoPercent)%5)==0)
			std::cout << (100*step/totalSteps) << std::flush;
		else
			std::cout << '.' << std::flush;
	}
}

void actionCollect(const std::string filename)
{
	MeasurementSet *ms = new MeasurementSet(filename);
	const unsigned polarizationCount = ms->GetPolarizationCount();
	const unsigned bandCount = ms->BandCount();
	BandInfo *bands = new BandInfo[bandCount];
	double **frequencies = new double*[bandCount];
	unsigned totalChannels = 0;
	for(unsigned b=0;b<bandCount;++b)
	{
		bands[b] = ms->GetBandInfo(b);
		frequencies[b] = new double[bands[b].channelCount];
		totalChannels += bands[b].channelCount;
		for(unsigned c=0;c<bands[b].channelCount;++c)
		{
			frequencies[b][c] = bands[b].channels[c].frequencyHz;
		}
	}
	delete ms;
	
	std::cout
		<< "Polarizations: " << polarizationCount << '\n'
		<< "Bands: " << bandCount << '\n'
		<< "Channels/band: " << (totalChannels / bandCount) << '\n';
	
	casa::Table table(filename, casa::Table::Update);
	StatisticsCollector collector(polarizationCount);

	casa::ROArrayColumn<casa::Complex> dataColumn(table, "DATA");
	casa::ROArrayColumn<bool> flagColumn(table, "FLAG");
	casa::ROScalarColumn<double> timeColumn(table, "TIME");
	casa::ROScalarColumn<int> antenna1Column(table, "ANTENNA1"); 
	casa::ROScalarColumn<int> antenna2Column(table, "ANTENNA2");
	casa::ROScalarColumn<int> windowColumn(table, "DATA_DESC_ID");
	
	std::cout << "Collecting statistics..." << std::endl;
	
	const unsigned nrow = table.nrow();
	for(unsigned row = 0; row!=nrow; ++row)
	{
		const double time = timeColumn(row);
		const unsigned antenna1Index = antenna1Column(row);
		const unsigned antenna2Index = antenna2Column(row);
		const unsigned bandIndex = windowColumn(row);
		
		const BandInfo &band = bands[bandIndex];
		
		const casa::Array<casa::Complex> dataArray = dataColumn(row);
		const casa::Array<bool> flagArray = flagColumn(row);
		
		std::vector<std::complex<float> > samples[polarizationCount];
		bool *isRFI[polarizationCount];
		for(unsigned p = 0; p < polarizationCount; ++p)
			isRFI[p] = new bool[band.channelCount];
		
		casa::Array<casa::Complex>::const_iterator dataIter = dataArray.begin();
		casa::Array<bool>::const_iterator flagIter = flagArray.begin();
		for(unsigned channel = 0 ; channel<band.channelCount; ++channel)
		{
			for(unsigned p = 0; p < polarizationCount; ++p)
			{
				samples[p].push_back(*dataIter);
				isRFI[p][channel] = *flagIter;
				
				++dataIter;
				++flagIter;
			}
		}
		
		for(unsigned p = 0; p < polarizationCount; ++p)
		{
			collector.Add(antenna1Index, antenna2Index, time, frequencies[bandIndex], p, samples[p], isRFI[p]);
		}

		for(unsigned p = 0; p < polarizationCount; ++p)
			delete[] isRFI[p];
		
		reportProgress(row, nrow);
	}
			
	std::cout << "100\nWriting quality tables..." << std::endl;
	
	QualityData qualityData(table);
	collector.Save(qualityData);
	
	std::cout << "Done.\n";
}

void printStatistics(std::complex<float> *complexStat, unsigned count)
{
	if(count != 1)
		std::cout << '[';
	if(count > 0)
		std::cout << complexStat[0].real() << " + " << complexStat[0].imag() << 'i';
	for(unsigned p=1;p<count;++p)
	{
		std::cout << complexStat[p].real() << " + " << complexStat[p].imag() << 'i';
	}
	if(count != 1)
		std::cout << ']';
}

void printStatistics(unsigned long *stat, unsigned count)
{
	if(count != 1)
		std::cout << '[';
	if(count > 0)
		std::cout << stat[0];
	for(unsigned p=1;p<count;++p)
	{
		std::cout << ", " << stat[p];
	}
	if(count != 1)
		std::cout << ']';
}

void printStatistics(const StatisticsCollector::DefaultStatistics &statistics)
{
}

void actionSummarize(const std::string &filename)
{
	MeasurementSet *ms = new MeasurementSet(filename);
	const unsigned polarizationCount = ms->GetPolarizationCount();
	delete ms;
	
	const QualityData qualityData(filename);
	StatisticsCollector collector(polarizationCount);
	collector.Load(qualityData);
	
	StatisticsCollector::DefaultStatistics statistics(polarizationCount);
	collector.GetGlobalTimeStatistics(statistics);
	
	std::cout << "Time statistics: \n";
	printStatistics(statistics);
}

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		cerr << "Syntax: " << argv[0] << " <action> [options]\n";
		return -1;
	} else {
		const std::string action = argv[1];
		if(action == "collect")
		{
			if(argc != 3)
			{
				cerr << "collect actions needs one parameter (the measurement set)\n";
				return -1;
			}
			else {
				actionCollect(argv[2]);
				return 0;
			}
		}
		else if(action == "summarize")
		{
			if(argc != 3)
			{
				cerr << "summarize actions needs one parameter (the measurement set)\n";
				return -1;
			}
			else {
				actionSummarize(argv[2]);
				return 0;
			}
		}
		cerr << "Unknown action '" << action << "'.\n";
		return -1;
	}
}
