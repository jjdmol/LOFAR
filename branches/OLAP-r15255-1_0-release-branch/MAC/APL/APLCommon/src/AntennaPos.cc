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

const int	LBA_IDX		= 0;
const int	HBA_IDX		= 1;
const int	HBA0_IDX	= 2;
const int	HBA1_IDX	= 3;
const int	MAX_FIELDS	= 4;

//-------------------------- creation and destroy ---------------------------
static AntennaPos* globalAntennaPosInstance = 0;

AntennaPos* globalAntennaPos()
{
  if (globalAntennaPosInstance == 0) {
    globalAntennaPosInstance = new AntennaPos("AntennaPos.conf");
  }
  return (globalAntennaPosInstance);
}

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

	// reserve space for expected info.
	itsAntPos.resize(MAX_FIELDS);
	itsRCUPos.resize(MAX_FIELDS);
	itsFieldCentres.resize(MAX_FIELDS);
	itsRCULengths.resize(MAX_FIELDS);

	string		fieldName;
	int			fieldIndex;
	// read file and skip lines that start with '#' or are empty
	while(getline(inputStream, fieldName)) {
		if (fieldName.empty() || fieldName[0] == '\0' || fieldName[0] == ']' || fieldName[0] == '#') {
			continue;
		}	

		// expect name
		fieldIndex = name2Index(fieldName);
		ASSERTSTR(fieldIndex >= 0, "Only 'LBA','HBA','HBA0' and 'HBA1' allowed for antenna field (not '" << fieldName << "')");

		// expect centre position
		blitz::Array<double,1>	centrePos;
		inputStream >> centrePos;
		ASSERTSTR(centrePos.dimensions() == 1 && centrePos.extent(firstDim) == 3, 
					"Position of the centre should be a 1 dimensional array with 3 values ,not " << centrePos);
		LOG_DEBUG_STR(fieldName << " centre is at " << centrePos);
		itsFieldCentres[fieldIndex].resize(centrePos.shape());
		itsFieldCentres[fieldIndex] = centrePos;

		// TODO? : allow HBA0 and HBA1 the be defined in core stations in stead of HBA ???
		//		   the complicates reading in the file but is more intuitive for the user.
		//		   For now we require that LBA and HBA must be defined always.

		// ignore positions of the HBA subfields
		if (fieldName != "LBA" && fieldName != "HBA") {
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
		LOG_DEBUG_STR(fieldName << " dimensions = " << antennaPos.shape());

		// update our admin with this info
		itsAntPos[fieldIndex].resize(antennaPos.shape());
		itsAntPos[fieldIndex] = antennaPos;
		// store info also in the rcu format
		itsRCUPos[fieldIndex].resize(itsAntPos[fieldIndex].extent(firstDim) * N_POL, itsAntPos[fieldIndex].extent(thirdDim));
		for (int antNr = 0; antNr < itsAntPos[fieldIndex].extent(firstDim); antNr++) {
			for (int pol = 0; pol < N_POL; pol++) {
				itsRCUPos[fieldIndex](antNr*2+pol, Range::all()) = itsAntPos[fieldIndex](antNr, pol, Range::all());
			}
		}
		itsRCULengths[fieldIndex].resize(itsRCUPos[fieldIndex].extent(firstDim));
		itsRCULengths[fieldIndex](Range::all()) = 
						 sqrt(itsRCUPos[fieldIndex](Range::all(),0)*itsRCUPos[fieldIndex](Range::all(),0) + 
							  itsRCUPos[fieldIndex](Range::all(),1)*itsRCUPos[fieldIndex](Range::all(),1) + 
							  itsRCUPos[fieldIndex](Range::all(),2)*itsRCUPos[fieldIndex](Range::all(),2));
	} // while not EOF
	inputStream.close();

	ASSERTSTR(itsAntPos[LBA_IDX].extent(firstDim) != 0 && itsAntPos[HBA_IDX].extent(firstDim) != 0, 
				"File should contain definitions for both LBA and HBA antennafields");

	// Finally construct the HBA0 and HBA1 info when centre positions are given.
	if (itsFieldCentres[HBA0_IDX].extent(firstDim) && itsFieldCentres[HBA1_IDX].extent(firstDim)) {
		LOG_DEBUG("Constructing HBA0 and HBA1 from HBA information");
		int		halfNrRCUs = itsRCUPos[HBA_IDX].extent(firstDim)/2;
		itsRCUPos[HBA0_IDX].resize(halfNrRCUs, itsRCUPos[HBA_IDX].extent(secondDim));
		itsRCUPos[HBA1_IDX].resize(halfNrRCUs, itsRCUPos[HBA_IDX].extent(secondDim));
		itsRCUPos[HBA0_IDX] = itsRCUPos[HBA_IDX](Range(0, halfNrRCUs-1) , Range::all());
		itsRCUPos[HBA1_IDX] = itsRCUPos[HBA_IDX](Range(halfNrRCUs, 2*halfNrRCUs-1) , Range::all());

		int		halfNrHBAs = itsAntPos[HBA_IDX].extent(firstDim)/2;
		itsAntPos[HBA0_IDX].resize(halfNrHBAs, N_POL, itsAntPos[HBA_IDX].extent(thirdDim));
		itsAntPos[HBA1_IDX].resize(halfNrHBAs, N_POL, itsAntPos[HBA_IDX].extent(thirdDim));
		itsAntPos[HBA0_IDX] = itsAntPos[HBA_IDX](Range(0, halfNrHBAs-1) , Range::all(), Range::all());
		itsAntPos[HBA1_IDX] = itsAntPos[HBA_IDX](Range(halfNrHBAs, 2*halfNrHBAs-1) , Range::all(), Range::all());

		// make a correction for the difference of the centres
		blitz::Array<double,1>	HBA0deltas(3);
		blitz::Array<double,1>	HBA1deltas(3);
		HBA0deltas = itsFieldCentres[HBA0_IDX] - itsFieldCentres[HBA_IDX];
		HBA1deltas = itsFieldCentres[HBA1_IDX] - itsFieldCentres[HBA_IDX];

		itsRCUPos[HBA0_IDX] -= HBA0deltas(tensor::j);
		itsRCUPos[HBA1_IDX] -= HBA1deltas(tensor::j);

		itsAntPos[HBA0_IDX] -= HBA0deltas(tensor::k);
		itsAntPos[HBA1_IDX] -= HBA1deltas(tensor::k);

		// finally calculate the lengths of the sub HBA fields
		itsRCULengths[HBA0_IDX].resize(halfNrRCUs);
		itsRCULengths[HBA1_IDX].resize(halfNrRCUs);
		itsRCULengths[HBA0_IDX](Range::all()) = 
						 sqrt(itsRCUPos[HBA0_IDX](Range::all(),0)*itsRCUPos[HBA0_IDX](Range::all(),0) + 
							  itsRCUPos[HBA0_IDX](Range::all(),1)*itsRCUPos[HBA0_IDX](Range::all(),1) + 
							  itsRCUPos[HBA0_IDX](Range::all(),2)*itsRCUPos[HBA0_IDX](Range::all(),2));
		itsRCULengths[HBA1_IDX](Range::all()) = 
						 sqrt(itsRCUPos[HBA1_IDX](Range::all(),0)*itsRCUPos[HBA1_IDX](Range::all(),0) + 
							  itsRCUPos[HBA1_IDX](Range::all(),1)*itsRCUPos[HBA1_IDX](Range::all(),1) + 
							  itsRCUPos[HBA1_IDX](Range::all(),2)*itsRCUPos[HBA1_IDX](Range::all(),2));
	}

	LOG_INFO_STR("Antenna positionfile " << fullFilename << " read in succesfully");

}

//
// ~AntennaPos()
//
AntennaPos::~AntennaPos()
{
}

int AntennaPos::name2Index(const string& fieldName) const
{
	if (fieldName == "LBA")		return(LBA_IDX);
	if (fieldName == "HBA")		return(HBA_IDX);
	if (fieldName == "HBA0")	return(HBA0_IDX);
	if (fieldName == "HBA1")	return(HBA1_IDX);
	return (-1);
}

  } // namespace APLCommon
} // namespace LOFAR
