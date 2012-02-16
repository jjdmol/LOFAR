//#  StatCal.h: implementation of the StatCal class
//#
//#  Copyright (C) 2002-2004
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
//#  $Id: StatCal.cc 14664 2009-12-11 09:59:34Z donker $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/LofarConstants.h>
#include <Common/LofarLocators.h>
#include <Common/StringUtil.h>
#include <ApplCommon/StationInfo.h>
#include "StatCal.h"

#include <blitz/array.h>
#include <fcntl.h>

using namespace blitz;

namespace LOFAR {
  namespace BS {

//
// StatCal()
//
StatCal::StatCal(uint mode, uint nrRSPBoards):
    itsNantennas(nrRSPBoards*NR_ANTENNAS_PER_RSPBOARD), 
	itsNpols	(2), 
	itsNsubbands(512), 
	itsMode		(mode), 
	itsIsValid	(false)
{
	LOG_DEBUG(formatString("StatCal(mode=%d,#Ant=%d,#Pol=%d,#Sub=%d)",mode, itsNantennas, itsNpols, itsNsubbands));

    itsStaticCalibration.resize(itsNantennas, itsNpols, itsNsubbands);
    itsStaticCalibration = complex<double>(0.0,0.0);
    itsIsValid = _readData(mode);
}

//
// ~StatCal
//
StatCal::~StatCal()
{
}

//
// _readData(mode)
//
bool StatCal::_readData(uint mode)
{
	ConfigLocator	CL;
	char			baseName[256];
    snprintf(baseName, sizeof baseName, "CalTable_mode%d.dat", mode);
	itsFileName = CL.locate(baseName);

	// try to open the file
    FILE 			*file;
    if (!(file = fopen(itsFileName.c_str(), "r"))) {
		LOG_ERROR_STR("Calibrationtable " << itsFileName << " can not be opened");
		return (false);
	}

	// read and check the headerinformation
	if (!_readHeaderInfo(file) || !_checkHeaderInfo(mode)) {
		return (false);
	}

	// so far so good, read the calibrationdata
    complex<double> value;
	for (uint sb = 0; sb < itsNsubbands; sb++) {
		for (uint ant = 0; ant < itsNantennas; ant++) {
			for (uint pol = 0; pol < itsNpols; pol++) {    
				if (fread(&value, sizeof(complex<double>), 1, file) != 1) {
					LOG_ERROR_STR("Error while loading calibrationtable " << itsFileName << " at element " << 
								(sb*itsNantennas+ant)*itsNpols+pol);
					fclose(file);
					return(false);
				}
				itsStaticCalibration((int)ant, (int)pol, (int)sb) = value;
			}
		}
	}
	fclose(file);
	LOG_INFO_STR("Static CalibrationTable loaded for mode " << mode);
	return (true);
}

//
// _readHeaderInfo(file)
//
bool StatCal::_readHeaderInfo(FILE*	file)
{
	char	line[1024];
	while (fgets(line, 1024,file)) {
		if (!strcmp(line, "CalTableHeader.Observation.Station"))
			itsHI.station = line;
		else if (!strcmp(line, "CalTableHeader.Observation.Mode"))
			itsHI.mode = atoi(line);
		else if (!strcmp(line, "CalTableHeader.Observation.Source"))
			itsHI.source = line;
		else if (!strcmp(line, "CalTableHeader.Observation.Date"))
			itsHI.date = line;
		else if (!strcmp(line, "CalTableHeader.Calibration.Version"))
			itsHI.calVersion = line;
		else if (!strcmp(line, "CalTableHeader.Calibration.Name"))
			itsHI.calName = line;
		else if (!strcmp(line, "CalTableHeader.Calibration.Date"))
			itsHI.calDate = line;
		else if (!strcmp(line, "CalTableHeader.Calibration.PPSDelay"))
			itsHI.calPPSdelay = line;
		else if (!strcmp(line, "CalTableHeader.Comment")) {
			itsHI.comment = line;
			fgetc(file);	// readaway last #FF character.
//		}
//		else fi (!strcmp(line, "END OF HEADERINFO")) {
			return (true);
		}
	}
	return (false);
}

//
// _checkHEaderInfo(mode)
//
bool StatCal::_checkHeaderInfo(uint	mode) const
{
	if ((uint)itsHI.mode != mode) {
		LOG_ERROR_STR("Caltable for mode " << mode << " contains weights for mode " << itsHI.mode);
		return (false);
	}

	if (itsHI.station != PVSSDatabaseName()) {
		LOG_ERROR_STR("Caltable for mode " << mode << " is ment for station " << itsHI.station);
		return (false);
	}

	return (true);
}

//
// print(os)
//
ostream& StatCal::print(ostream& os)const
{
	os << "Filename   : " << itsFileName << endl;
	os << "Station    : " << itsHI.station << endl;
	os << "Mode       : " << itsHI.mode << endl;
	os << "Source     : " << itsHI.source << endl;
	os << "Date       : " << itsHI.date << endl;
	os << "CalVersion : " << itsHI.calVersion << endl;
	os << "Calibrator : " << itsHI.calName << endl;
	os << "CalDate    : " << itsHI.calDate << endl;
	os << "CalPPSdelay: " << itsHI.calPPSdelay << endl;
	os << "Comment    : " << itsHI.comment << endl;

	return (os);
}

//
// calFactor(rcu, subband)
//
complex<double> StatCal::calFactor(uint	rcuNr, uint subbandNr) const
{
	ASSERTSTR(rcuNr < itsNantennas * itsNpols, "rcuNr " << rcuNr << " out of range");
	ASSERTSTR(subbandNr < itsNsubbands, "subbandNr " << subbandNr << " out of range");

	return (itsStaticCalibration((int)rcuNr/2, (int)rcuNr%2, (int)subbandNr));
}
	

  } // namespace BS
} // namespace LOFAR
