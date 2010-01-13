//#  AntennaPos.cc: one_line_description
//#
//#  Copyright (C) 2009
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/LofarConstants.h>
#include <Common/lofar_fstream.h>
#include <APL/APLCommon/AntennaPos.h>

using namespace blitz;

namespace LOFAR {
  namespace APLCommon {

const int	IDX_CORE	= 0;
const int	IDX_REMOTE	= 1;
const int	IDX_EUROPE	= 2;

//
// AntennaPos(fileName)
//
// Name
// 3 [ x y z ]
// Nant x Npol x 3 [ ... ]
//
// The info is stored in itsxxxAntPos[ant,pol,xyz] and in itsxxxRCUPos[rcu,xyz] because
// some programs are antenna based while others are rcu based.
//
AntennaPos::AntennaPos(const string& filename)
{
	ConfigLocator	cl;
	string			fullFilename(cl.locate(filename));
	ifstream		inputStream;
	inputStream.open(fullFilename.c_str());

	ASSERTSTR(inputStream.good(), "File " << fullFilename << " cannot be opened succesfully.");

	string		line;
	string		AntTypeName;
	// read file and skip lines that start with '#' or are empty
	while(getline(inputStream, line)) {
		if (line.empty() || line[0] == '\0' || line[0] == ']') {
			continue;
		}	

		// expect name
		ASSERTSTR(line=="LBA" || line=="HBA" || line=="HBA0" || line=="HBA1", 
				  "Only 'LBA','HBA','HBA0' and 'HBA1' allowed for antenna types (not '" << line << "')");
		AntTypeName = line;

		// expect centre position
		blitz::Array<double,1>	centrePos;
		inputStream >> centrePos;
		ASSERTSTR(centrePos.dimensions() == 1 && centrePos.extent(firstDim) == 3, 
					"Position of the centre should be a 1 dimensional array with 3 values ,not " << centrePos);
		LOG_DEBUG_STR(AntTypeName << " centre is at " << centrePos);

		// ignore positions of the HBA subfields
		if (line != "LBA" && line != "HBA") {
			continue;
		}

		// expect positions
		blitz::Array<double,3>	antennaPos;
		inputStream >> antennaPos;
		inputStream.ignore(80, '\n');
		ASSERTSTR(antennaPos.dimensions() == 3 && 
				  antennaPos.extent(firstDim) <= MAX_ANTENNAS &&
				  antennaPos.extent(secondDim) == N_POL && 
				  antennaPos.extent(thirdDim) <= 3, 
				  "Expected an array of size NrAntennas x nrPol x (x,y,z)");
		LOG_DEBUG_STR(AntTypeName << " dimensions = " << antennaPos.shape());

		// copy the info to the right data members
		if (AntTypeName == "LBA") {
			itsLBACentre.resize(centrePos.shape());
			itsLBACentre = centrePos;
			itsLBAAntPos.resize(antennaPos.shape());
			itsLBAAntPos = antennaPos;
			// store info also in the rcu format
			itsLBARCUPos.resize(itsLBAAntPos.extent(firstDim) * N_POL, itsLBAAntPos.extent(thirdDim));
			for (int antNr = 0; antNr < itsLBAAntPos.extent(firstDim); antNr++) {
				for (int pol = 0; pol < N_POL; pol++) {
					itsLBARCUPos(antNr*2+pol, Range::all()) = itsLBAAntPos(antNr, pol, Range::all());
				}
			}
			itsLBARCULengths.resize(itsLBARCUPos.extent(firstDim));
			itsLBARCULengths(Range::all()) = sqrt(itsLBARCUPos(Range::all(),0)*itsLBARCUPos(Range::all(),0) + 
												  itsLBARCUPos(Range::all(),1)*itsLBARCUPos(Range::all(),1) + 
												  itsLBARCUPos(Range::all(),2)*itsLBARCUPos(Range::all(),2));
		}
		else {	// handle the HBA positions
			itsHBACentre.resize(centrePos.shape());
			itsHBACentre = centrePos;
			itsHBAAntPos.resize(antennaPos.shape());
			itsHBAAntPos = antennaPos;
			// store info also in the rcu format
			itsHBARCUPos.resize(itsHBAAntPos.extent(firstDim) * N_POL, itsHBAAntPos.extent(thirdDim));
			for (int antNr = 0; antNr < itsHBAAntPos.extent(firstDim); antNr++) {
				for (int pol = 0; pol < N_POL; pol++) {
					itsHBARCUPos(antNr*2+pol, Range::all()) = itsHBAAntPos(antNr, pol, Range::all());
				}
			}
			itsHBARCULengths.resize(itsHBARCUPos.extent(firstDim));
			itsHBARCULengths(Range::all()) = sqrt(itsHBARCUPos(Range::all(),0)*itsHBARCUPos(Range::all(),0) + 
												  itsHBARCUPos(Range::all(),1)*itsHBARCUPos(Range::all(),1) + 
												  itsHBARCUPos(Range::all(),2)*itsHBARCUPos(Range::all(),2));
		}
	} // while not EOF

	ASSERTSTR(itsLBAAntPos.extent(firstDim) != 0 && itsHBAAntPos.extent(firstDim) != 0, 
				"File should contain definitions for both LBA and HBA antennafields");

	LOG_INFO_STR("Antenna positionfile " << fullFilename << " read in succesfully");

	inputStream.close();
}

//
// ~AntennaPos()
//
AntennaPos::~AntennaPos()
{
}



  } // namespace APLCommon
} // namespace LOFAR
