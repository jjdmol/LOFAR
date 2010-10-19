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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>
#include <sstream>
#include <vector>

#include "msio/measurementset.h"
#include "msio/antennainfo.h"
#include "msio/timefrequencyimager.h"

#include "rfi/thresholdtools.h"

using namespace std;

void AddLengths(Mask2DCPtr mask, int *counts, int countSize);

int main(int argc, char *argv[])
{
	if(argc < 2) {
		cout << "This program will provide you general information about a measurement set,\nespecially about flagging.\nUsage:\n\t" << argv[0] << " [options] <measurement set>\n"
		     << "[options] can be:\n\t-flaglength <file> : save the length distribution in the file."
		     << endl;
		exit(-1);
	}

	bool saveFlagLength = false;
	string flagLengthFile;
	int pindex = 1;
	while(pindex < argc && argv[pindex][0] == '-') {
		string parameter = argv[pindex]+1;
		if(parameter == "flaglength") {
			saveFlagLength = true;
			flagLengthFile = argv[pindex+1];
			++pindex;
		} else {
			cerr << "Bad parameter: -" << parameter << endl;
			exit(-1);
		}
		++pindex;
	}
	
	string measurementFile = argv[pindex];
	cout << "Opening measurementset " << measurementFile << endl; 
	MeasurementSet set(measurementFile);
	size_t antennaCount = set.AntennaCount();
	cout << "Number of antennea: " << antennaCount << endl;
	cout << "Number of time scans: " << (set.MinScanIndex() - set.MaxScanIndex()) << endl;
	cout << "Number of frequencies: " << set.FrequencyCount() << endl;
	cout << "Number of fields: " << set.FieldCount() << endl;
	cout << "Last index of spectral band: " << set.MaxSpectralBandIndex() << endl;
	std::vector<long double> baselines;
	for(size_t i=0;i<antennaCount;++i)
	{
		cout << "==Antenna " << i << "==\n";
		AntennaInfo info = set.GetAntennaInfo(i);
		cout <<
			"Diameter: " << info.diameter << "\n"
			"Name: " << info.name << "\n"
			"Position: " << info.position.ToString() << "\n"
			"Mount: " << info.mount << "\n"
			"Station: " << info.station << "\n"
			"Providing baselines: ";
		for(size_t j=0;j<antennaCount;++j) {
			AntennaInfo info2 = set.GetAntennaInfo(j);
			Baseline b(info.position, info2.position);
			long double dist = b.Distance();
			long double angle = b.Angle() * 180.0 / M_PI;
			cout << dist << "m/" << angle << "` ";
			baselines.push_back(dist);
		}
		cout << "\n" << endl;
	}
	sort(baselines.begin(), baselines.end());
	cout << "All provided baselines: ";
	unsigned i=0;
	while(i<baselines.size()-1) {
		if(baselines[i+1]-baselines[i] < 1.0)
			baselines.erase(baselines.begin() + i);
		else
			++i;
	}
	for(vector<long double>::const_iterator i=baselines.begin();i!=baselines.end();++i)
		cout << (*i) << " ";
	cout << endl;

	for(unsigned i=0;i<=set.MaxSpectralBandIndex();++i) {
		cout << "== Spectral band index " << i << " ==" << endl;
		BandInfo bandInfo = set.GetBandInfo(i);
		cout << "Channel count: " << bandInfo.channelCount << std::endl;
		cout << "Channels: ";
		for(unsigned j=0;j<bandInfo.channelCount;++j) {
			if(j > 0) cout << ", ";
			cout << round(bandInfo.channels[j].frequencyHz/1000000) << "MHz";
		}
		cout << endl;
	}
	
	for(unsigned i=0;i<set.FieldCount();++i) {
		FieldInfo fieldInfo = set.GetFieldInfo(i);
		cout << "Field " << i << ":\n\tdelay direction=" << fieldInfo.delayDirectionDec << " dec, " << fieldInfo.delayDirectionRA << "ra.\n\tdelay direction (in degrees)=" << (fieldInfo.delayDirectionDec/M_PI*180.0L) << " dec," << (fieldInfo.delayDirectionRA/M_PI*180.0L) << " ra." << endl;
	}

	long unsigned flaggedCount = 0;
	long unsigned sampleCount = 0;
	for(unsigned b=0;b<=set.MaxSpectralBandIndex();++b) {
		cout << "Processing band " << b << "..." << endl;
		int lengthsSize = set.MaxScanIndex()+1;
		int *lengthCounts = new int[lengthsSize];
		for(int i=0;i<lengthsSize;++i)
			lengthCounts[i] = 0;
		for(unsigned a2=0;a2<set.AntennaCount();++a2) {
			for(unsigned a1=0;a1<set.AntennaCount();++a1) {
				//std::cout << "A" << a1 << " vs A" << a2 << endl;
				MeasurementSet set(measurementFile);
				TimeFrequencyImager imager(set);
				imager.SetReadData(false);
				imager.SetReadFlags(true);
				imager.Image(a1, a2, b);
				if(imager.HasFlags()) {
					long unsigned thisCount;
					long unsigned thisSamples;

					thisCount = imager.FlagXX()->GetCount<true>();
					thisSamples = imager.FlagXX()->Width()*imager.FlagXX()->Height();

					thisCount += imager.FlagXY()->GetCount<true>();
					thisSamples += imager.FlagXY()->Width()*imager.FlagXY()->Height();

					thisCount += imager.FlagYX()->GetCount<true>();
					thisSamples += imager.FlagYX()->Width()*imager.FlagYX()->Height();

					thisCount += imager.FlagYY()->GetCount<true>();
					thisSamples += imager.FlagYY()->Width()*imager.FlagYY()->Height();

				if(thisCount == 0 || round(1000.0L * (long double) thisCount / thisSamples)*0.1L == 100.0L) {
						//cout << " (ignored)";
					} else {
						cout << "Calculated flagging for antenna " << a1 << " vs " << a2 << ": ";
						cout << 0.1L * round(1000.0L * (long double) thisCount / thisSamples) << "%" << endl;
						flaggedCount += thisCount;
						sampleCount += thisSamples;
						AddLengths(imager.FlagXX(), lengthCounts, lengthsSize);
						AddLengths(imager.FlagXX(), lengthCounts, lengthsSize);
						AddLengths(imager.FlagXX(), lengthCounts, lengthsSize);
						AddLengths(imager.FlagXX(), lengthCounts, lengthsSize);
					}
				}
			}
		}
		cout << "Total percentage RFI in this band: " << round(flaggedCount*1000.0/sampleCount)/10.0 << "% "
		     << "(" << flaggedCount << " / " << sampleCount << ")" << endl;
		
		if(saveFlagLength) {
			long double *data = new long double[lengthsSize];
			for(int i=0;i<lengthsSize;++i)
				data[i] = lengthCounts[i];
			if(set.FrequencyCount() < (set.MaxScanIndex()+1) && set.FrequencyCount()>2 && data[set.FrequencyCount()-1] > data[set.FrequencyCount()-2])
				data[set.FrequencyCount()-1] = data[set.FrequencyCount()-2];
			ThresholdTools::OneDimensionalGausConvolution(data, lengthsSize, 200.0);
			ofstream f(flagLengthFile.c_str());
			for(int i = 0; i<lengthsSize;++i) {
				f << (i+1) << "\t" << lengthCounts[i] << "\t" << data[i] << endl;
			}
			f.close();
		}
		delete[] lengthCounts;
	}
  return EXIT_SUCCESS;
}

void AddLengths(Mask2DCPtr mask, int *counts, int countSize)
{
	int *countTmp = new int[countSize];
	ThresholdTools::CountMaskLengths(mask, countTmp, countSize);
	for(int i=0;i<countSize;++i)
		counts[i] += countTmp[i];
	delete[] countTmp; 
}
