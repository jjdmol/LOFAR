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

#include <tables/Tables/Table.h>

#include <measures/Measures/UVWMachine.h>
#include <measures/Measures/MEpoch.h>

#include <ms/MeasurementSets/MeasurementSet.h>

#include <AOFlagger/msio/antennainfo.h>
#include <AOFlagger/msio/measurementset.h>

#include <AOFlagger/imaging/zenithimager.h>
#include <AOFlagger/msio/pngfile.h>

const casa::Unit radUnit("rad");
const casa::Unit dayUnit("d");

void repoint(casa::MDirection &phaseDirection, casa::MPosition &position, const casa::MEpoch &obstime, double &u, double &v, double &w, double &phaseRotation)
{
	//MPosition location(MVPosition(Quantity(1, "km"), Quantity(150, "deg"), Quantity(20, "deg")), MPosition::WGS84);
	
	//casa::MEpoch obstime(casa::Quantity(t, dayUnit), casa::MEpoch::UTC);
	//std::cout << "Time=" << obstime.getValue() << '\n';
	
	casa::MeasFrame timeAndLocation(obstime, position);
	
	casa::MDirection::Ref refApparent(casa::MDirection::APP, timeAndLocation);
	
	// Calculate zenith
	casa::MDirection outDirection(casa::Quantity(M_PI, radUnit), casa::Quantity(0.0, radUnit), refApparent);
	//std::cout << "Out=" << outDirection.getValue() << '\n';
	
	// Construct a CASA UVW converter
	casa::UVWMachine uvwConverter(outDirection, phaseDirection);
	casa::Vector<double> uvwVector(3);
	uvwVector[0] = u;
	uvwVector[1] = v;
	uvwVector[2] = w;
	//std::cout << "In: " << u << ',' << v << ',' << w << '\n';
	phaseRotation = uvwConverter.getPhase(uvwVector);
	//std::cout << "Phase shift: " << phaseRotation << '\n';
	//uvwConverter.convertUVW(uvwVector); // getPhase already does that!
	u = uvwVector[0];
	v = uvwVector[1];
	w = uvwVector[2];
	//std::cout << "Out: " << u << ',' << v << ',' << w << '\n';
	//std::cout << "Phase centre: " << uvwConverter.phaseCenter().getValue() << '\n';
}

int main(int argc, char *argv[])
{
	std::string filename(argv[1]);
	size_t integrationSteps;
	if(argc >= 3)
		integrationSteps = atoi(argv[2]);
	else
		integrationSteps = 60;
	
	std::cout << "Opening " << filename << "...\n";
	
	const unsigned polarizationCount = MeasurementSet::GetPolarizationCount(filename);
	const BandInfo band = MeasurementSet::GetBandInfo(filename, 0);
	
	const unsigned antennaIndex = 0;
	
	casa::MeasurementSet table(argv[1]);
	casa::MEpoch::ROScalarColumn timeColumn(table, "TIME");
	casa::ROArrayColumn<double> uvwColumn(table, "UVW");
	casa::ROScalarColumn<int> ant1Column(table, "ANTENNA1");
	casa::ROScalarColumn<int> ant2Column(table, "ANTENNA2");
	casa::ROArrayColumn<casa::Complex> dataColumn(table, "DATA");
	casa::ROArrayColumn<bool> flagColumn(table, "FLAG");
	
	casa::Table antennaTable(table.antenna());
	casa::MPosition::ROScalarColumn antPositionColumn(antennaTable, "POSITION");
	casa::ROScalarColumn<casa::String> antNameColumn(antennaTable, "NAME");
	casa::MPosition position  = antPositionColumn(antennaIndex);
	std::cout << "Imaging zenith of antenna " << antNameColumn(antennaIndex)
		<< ", pos=" << position.getValue() << '\n';
		
	casa::Table fieldTable(table.field());
	casa::MDirection::ROArrayColumn phaseDirColumn(fieldTable, "PHASE_DIR");
	casa::MDirection phaseDirection = *phaseDirColumn(0).begin();
	std::cout << "Phase direction: " << phaseDirection.getValue() << '\n';

	std::complex<float> *samples[polarizationCount];
	bool *isRFI[polarizationCount];
	for(unsigned p = 0; p < polarizationCount; ++p)
	{
		isRFI[p] = new bool[band.channelCount];
		samples[p] = new std::complex<float>[band.channelCount];
	}

	unsigned row = 0;
	ZenithImager imager;
	imager.Initialize(2048);
	size_t timeStep = 0;
	while(row<table.nrow())
	{
		const casa::MEpoch t = timeColumn(row);
		do
		{
			if(ant1Column(row) != ant2Column(row))
			{
				casa::Array<double> uvw = uvwColumn(row);
				casa::Array<double>::const_iterator uvw_i = uvw.begin();
				double u = *uvw_i; ++uvw_i;
				double v = *uvw_i; ++uvw_i;
				double w = *uvw_i;
				double phaseRotation;
				repoint(phaseDirection, position, t, u, v, w, phaseRotation);
				
				const casa::Array<casa::Complex> dataArray = dataColumn(row);
				const casa::Array<bool> flagArray = flagColumn(row);
				
				casa::Array<casa::Complex>::const_iterator dataIter = dataArray.begin();
				casa::Array<bool>::const_iterator flagIter = flagArray.begin();
				
				for(unsigned channel = 0; channel<band.channelCount; ++channel)
				{
					for(unsigned p = 0; p < polarizationCount; ++p)
					{
						samples[p][channel] = *dataIter;
						isRFI[p][channel] = *flagIter;
						
						++dataIter;
						++flagIter;
					}
				}
				
				imager.Add(band, samples[0], isRFI[0], u, v, w, phaseRotation);
			}
			++row;
		} while(row<table.nrow() && timeColumn(row).getValue() == t.getValue());
		
		timeStep++;
		if(timeStep % integrationSteps == 0)
		{
			Image2DPtr real, imaginary;
			imager.FourierTransform(real, imaginary);
			
			std::stringstream s;
			if(timeStep < 10000) s << '0';
			if(timeStep < 1000) s << '0';
			if(timeStep < 100) s << '0';
			if(timeStep < 10) s << '0';
			s << timeStep << ".png";
			std::cout << "Saving " << s.str() << "... " << std::flush;
			PngFile::Save(*real, std::string("zen/") + s.str());
			PngFile::Save(*imager.UVReal(), std::string("zen-uv/") + s.str());
			imager.Clear();
			std::cout << "Done.\n";
		}
	}
}
