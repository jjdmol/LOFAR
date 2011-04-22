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
    itsNantennas(nrRSPBoards*NR_ANTENNAS_PER_RSPBOARD), itsNpols(2), itsNsubbands(512), itsMode(mode), itsIsValid(false)
{
	LOG_DEBUG(formatString("StatCal(mode=%d,#Ant=%d,#Pol=%d,#Sub=%d)",mode, itsNantennas, itsNpols, itsNsubbands));
    itsStaticCalibration.resize(itsNantennas, itsNpols, itsNsubbands);
    itsStaticCalibration = complex<double>(0.0,0.0);
    _readData(mode);
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
void StatCal::_readData(uint mode)
{
	ConfigLocator	CL;
	char			baseName[256];
    snprintf(baseName, sizeof baseName, "CalTable_mode%d.dat", mode);
	itsFileName = CL.locate(baseName);

    complex<double> value;
    FILE 			*file;
    if (!(file = fopen(itsFileName.c_str(), "r"))) {
		LOG_ERROR_STR("Calibrationtable " << itsFileName << " can not be opened");
	}
	else {
		for (uint sb = 0; sb < itsNsubbands; sb++) {
			for (uint ant = 0; ant < itsNantennas; ant++) {
				for (uint pol = 0; pol < itsNpols; pol++) {    
					if (fread(&value, sizeof(complex<double>), 1, file) != 1) {
						LOG_ERROR_STR("Error while loading calibrationtable " << itsFileName << " at element " << 
									(sb*itsNantennas+ant)*itsNpols+pol);
						fclose(file);
						return;
					}
					itsStaticCalibration((int)ant, (int)pol, (int)sb) = value;
				}
			}
		}
		fclose(file);
		itsIsValid = true;
		LOG_INFO_STR("Static CalibrationTable loaded for mode " << mode);
	}
    
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
