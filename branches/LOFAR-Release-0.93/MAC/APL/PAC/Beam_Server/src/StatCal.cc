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
#include <ApplCommon/StationConfig.h>

#include <blitz/array.h>

#include <fcntl.h>

using namespace blitz;
using namespace LOFAR;
using namespace std;

//
// StatCal()
//
StatCal::StatCal(int mode):
    itsNantennas(48), itsNpols(2), itsNsubbands(512), , itsMode(mode)
{
    makeFileName(mode);
    
    itsStaticCalibration.resize(itsNantennas, itsNpols, itsNsubbands);
    itsStaticCalibration = complex<double>(0.0,0.0);
    readData();
}

//
// ~StatCal
//
StatCal::~StatCal()
{
}

void StatCal::makeFileName(int mode)
{
    snprintf(itsFileName, sizeof itsFileName, "CalTable_mode%d.dat", mode);
}

void StatCal::readData()
{
    complex<double> value;
    
    FILE *file;
    file = fopen(itsFileName, "r");
    
    for (int sb = 0; sb < itsNsubbands; sb++) {
        for (int ant = 0; ant < itsNantennas; ant++) {
            for (int pol = 0; pol < itsNpols; pol++) {    
                fread(&value, sizeof(complex<double>), 1, file);
                itsStaticCalibration(ant, pol, sub) = value;
            }
        }
    }
    
    fclose(file);
}

