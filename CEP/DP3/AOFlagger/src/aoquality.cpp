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

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		cerr << "Syntax: " << argv[0] << " [MS]\n";
		return -1;
	} else {
		const std::string filename = argv[1];
		
		MeasurementSet *ms = new MeasurementSet(filename);
		const unsigned polarizationCount = ms->GetPolarizationCount();
		const unsigned bandCount = ms->BandCount();
		BandInfo *bands = new BandInfo[bandCount];
		double **frequencies = new double*[bandCount];
		for(unsigned b=0;b<bandCount;++b)
		{
			bands[b] = ms->GetBandInfo(b);
			frequencies[b] = new double[bands[b].channelCount];
			for(unsigned c=0;c<bands[b].channelCount;++c)
			{
				frequencies[b][c] = bands[b].channels[c].frequencyHz;
			}
		}
		delete ms;
		
		casa::Table table(filename, casa::Table::Update);
		StatisticsCollector collector(polarizationCount);

		casa::ROArrayColumn<casa::Complex> dataColumn(table, "DATA");
		casa::ROArrayColumn<bool> flagColumn(table, "FLAG");
		casa::ROScalarColumn<double> timeColumn(table, "TIME");
		casa::ROScalarColumn<int> antenna1Column(table, "ANTENNA1"); 
		casa::ROScalarColumn<int> antenna2Column(table, "ANTENNA2");
		casa::ROScalarColumn<int> windowColumn(table, "DATA_DESC_ID");
		
		const unsigned nrow = table.nrow();
		for(unsigned row = 0; row!=nrow; ++row)
		{
			const double time = timeColumn(row);
			const unsigned antenna1Index = antenna1Column(row);
			const unsigned antenna2Index = antenna2Column(row);
			const unsigned bandIndex = windowColumn(row);
			
			const casa::Array<casa::Complex> dataArray = dataColumn(row);
			const casa::Array<bool> flagArray = flagColumn(row);
			
			//collector.Add(antenna1Index, antenna2Index, time, frequencies[bandIndex], pol, samplevector, isrfi);
		}
		
		QualityData qualityData(table);
		
		return 0;
	}
}
