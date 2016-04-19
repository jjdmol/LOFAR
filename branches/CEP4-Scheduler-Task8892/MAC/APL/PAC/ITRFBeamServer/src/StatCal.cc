//#  StatCal.h: implementation of the StatCal class
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
StatCal::StatCal(const string& antennaSet, const string& band, uint nrRSPBoards):
    itsNantennas(nrRSPBoards*NR_ANTENNAS_PER_RSPBOARD),
	itsNpols	(2),
	itsNsubbands(512),
	itsAntennaSet(antennaSet),
	itsBand(band),
	itsIsValid	(false)
{
	LOG_DEBUG(formatString("StatCal(antset=%s,band=%s,#Ant=%d,#Pol=%d,#Sub=%d)",
                            antennaSet.c_str(), band.c_str(), itsNantennas, itsNpols, itsNsubbands));

    itsStaticCalibration.resize(itsNantennas, itsNpols, itsNsubbands);
    itsStaticCalibration = complex<double>(0.0,0.0);
    itsIsValid = _readData(antennaSet, band);
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
bool StatCal::_readData(const string& antennaSet, const string& band)
{
	ConfigLocator	CL;
	char			baseName[256];
    snprintf(baseName, sizeof baseName, "CalTable-%s-%s.dat", antennaSet.c_str(), band.c_str());
	itsFileName = CL.locate(baseName);

    if (itsFileName.empty()) {
        LOG_INFO_STR("Static CalibrationTable '" << baseName << "' NOT available, use default.");
        return (false);
    }

	// try to open the file
    FILE 			*file;
    if (!(file = fopen(itsFileName.c_str(), "r"))) {
		LOG_ERROR_STR("Calibrationtable " << itsFileName << " can not be opened");
		return (false);
	}

	// read and check the headerinformation
	if (!_readHeaderInfo(file) || !_checkHeaderInfo(antennaSet, band)) {
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
	LOG_INFO_STR("Static CalibrationTable loaded for antennaSet " << antennaSet << ", band " << band << ", first value = " << itsStaticCalibration(0,0,0));
	return (true);
}

//
// _readHeaderInfo(file)
//
bool StatCal::_readHeaderInfo(FILE*	file)
{
	char		line[1024];
	if (!fgets(line,30,file) || strncmp(line, "HeaderStart", 11)) {
		fseek(file, 0, SEEK_SET);
		LOG_INFO("Caltable does not start with 'HeaderStart', no headerinfo found");
		return (true);
	}

	while (fgets(line, 1024, file)) {
		if (line[strlen(line)-1] == '\n') {	// cut trailing newline character if any
			line[strlen(line)-1] = '\0';
		}
		char*	value(strchr(line,'='));	// position value just after '=' character
		if (value && *(value+1) != '\0') {
			value++;
		}
		if (strstr(line, "CalTableHeader.Observation.Station"))
			itsHI.station = ltrim(value);
		else if (strstr(line, "CalTableHeader.Observation.AntennaSet"))
			itsHI.antennaSet = ltrim(value);
        else if (strstr(line, "CalTableHeader.Observation.Band"))
			itsHI.band = ltrim(value);
		else if (strstr(line, "CalTableHeader.Observation.Source"))
			itsHI.source = ltrim(value);
		else if (strstr(line, "CalTableHeader.Observation.Date"))
			itsHI.date = ltrim(value);
		else if (strstr(line, "CalTableHeader.Calibration.Version"))
			itsHI.calVersion = ltrim(value);
		else if (strstr(line, "CalTableHeader.Calibration.Name"))
			itsHI.calName = ltrim(value);
		else if (strstr(line, "CalTableHeader.Calibration.Date"))
			itsHI.calDate = ltrim(value);
		else if (strstr(line, "CalTableHeader.Calibration.PPSDelay"))
			itsHI.calPPSdelay = ltrim(value);
		else if (strstr(line, "CalTableHeader.Comment"))
			itsHI.comment = ltrim(value);
		else if (strstr(line, "HeaderStop")) {
			LOG_INFO("Header information read");
			return (true);
		}
	}
	return (false);
}

//
// _checkHeaderInfo(mode)
//
bool StatCal::_checkHeaderInfo(const string& antennaSet, const string& band) const
{
	if (itsHI.antennaSet == "") {	// file without header?
		return (true);
	}

	if (antennaSet.find("HBA") != string::npos) {
        if (itsHI.antennaSet.find("HBA") == string::npos) {
            LOG_ERROR_STR("CALTABLE FOR ANTENNASET " << antennaSet << " CONTAINS WEIGHTS FOR ANTENNASET " << itsHI.antennaSet);
            return (false);
        }
        if (itsHI.band != band) {
            LOG_ERROR_STR("CALTABLE FOR BAND " << band << " CONTAINS WEIGHTS FOR BAND " << itsHI.band);
            return (false);
        }
    }

    else if (antennaSet.find("LBA") != string::npos) {
        if (itsHI.antennaSet.find("LBA") == string::npos) {
            LOG_ERROR_STR("CALTABLE FOR ANTENNASET " << antennaSet << " CONTAINS WEIGHTS FOR ANTENNASET " << itsHI.antennaSet);
            return (false);
        }
    }

	if (itsHI.station != PVSSDatabaseName()) {
		LOG_ERROR_STR("CALTABLE NOT FOR THIS STATION " << itsHI.station);
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
	os << "AntennaSet : " << itsHI.antennaSet << endl;
	os << "Band       : " << itsHI.band << endl;
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
