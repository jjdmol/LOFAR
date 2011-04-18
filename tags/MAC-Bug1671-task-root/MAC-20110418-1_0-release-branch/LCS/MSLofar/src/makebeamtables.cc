//# makebeamtables.cc: Program 
//# Copyright (C) 2011
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$
//#
//# @author Ger van Diepen

#include <lofar_config.h>

#include <MSLofar/BeamTables.h>
#include <casa/Inputs.h>
#include <ms/MeasurementSets/MeasurementSet.h>

using namespace LOFAR;
using namespace casa;

int main (int argc, char* argv[])
{
  try {
    Input inputs(1);
    // define the input structure
    inputs.version("2011Mar31-GvD");
    inputs.create ("ms", "",
		   "Name of MeasurementSet",
		   "string");
    inputs.create ("antennaset", "",
		   "Antenna set used (e.g. LBA_INNER)",
		   "string");
    inputs.create ("antennasetfile", "/opt/cep/lofar/share/AntennaSets.conf",
		   "Name of the AntennaSet file",
		   "string");
    inputs.create ("antennafielddir", "/opt/cep/lofar/share/AntennaFields",
		   "Directory where the AntennaField.conf files reside",
		   "string");
    inputs.create ("ihbadeltadir", "/opt/cep/lofar/share/iHBADeltas",
		   "Directory where the iHBADelta.conf files reside",
		   "string");
    inputs.create ("overwrite", "false",
                   "Overwriting existing beam subtables?"
                   "bool");
    inputs.readArguments (argc, argv);
    String msName      = inputs.getString("ms");
    String antSet      = inputs.getString("antennaset");
    String antSetFile  = inputs.getString("antennasetfile");
    String antFieldDir = inputs.getString("antennafielddir");
    String hbaDeltaDir = inputs.getString("ihbadeltadir");
    Bool   overwrite   = inputs.getBool  ("overwrite");
    MeasurementSet ms(msName, Table::Update);
    BeamTables::create (ms, overwrite);
    BeamTables::fill   (ms, antSet, antSetFile, antFieldDir, hbaDeltaDir);
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
