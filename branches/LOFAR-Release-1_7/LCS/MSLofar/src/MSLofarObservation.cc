//# MSLofarObservation.cc: MS OBSERVATION subtable with LOFAR extensions
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
#include <MSLofar/MSLofarObservation.h>
#include <measures/Measures/MEpoch.h>

using namespace casa;

namespace LOFAR {

  MSLofarObservation::MSLofarObservation()
  {}

  MSLofarObservation::MSLofarObservation (const String& tableName,
                                  Table::TableOption option) 
    : MSObservation (tableName, option)
  {}

  MSLofarObservation::MSLofarObservation (SetupNewTable& newTab, uInt nrrow,
                                  Bool initialize)
    : MSObservation (newTab, nrrow, initialize)
  {}

  MSLofarObservation::MSLofarObservation (const Table& table)
    : MSObservation (table)
  {}

  MSLofarObservation::MSLofarObservation (const MSLofarObservation& that)
    : MSObservation (that)
  {}

  MSLofarObservation::~MSLofarObservation()
  {}

  MSLofarObservation& MSLofarObservation::operator= (const MSLofarObservation& that)
  {
    MSObservation::operator= (that);
    return *this;
  }

  TableDesc MSLofarObservation::requiredTableDesc()
  {
    TableDesc td (MSObservation::requiredTableDesc());
    MSLofarTable::addColumn (td, "LOFAR_PROJECT_TITLE", TpString,
                             "Project description");
    MSLofarTable::addColumn (td, "LOFAR_PROJECT_PI", TpString,
                             "Principal investigator");
    MSLofarTable::addColumn (td, "LOFAR_PROJECT_CO_I", TpArrayString,
                             "Co investigators");
    MSLofarTable::addColumn (td, "LOFAR_PROJECT_CONTACT", TpString,
                             "Contact author");
    MSLofarTable::addColumn (td, "LOFAR_OBSERVATION_ID", TpString,
                             "Observation ID");
    MSLofarTable::addColumn (td, "LOFAR_OBSERVATION_START", TpDouble,
                             "Observation start",
                             "s", "EPOCH", MEpoch::UTC);
    MSLofarTable::addColumn (td, "LOFAR_OBSERVATION_END", TpDouble,
                             "Observation end",
                             "s", "EPOCH", MEpoch::UTC);
    MSLofarTable::addColumn (td, "LOFAR_OBSERVATION_FREQUENCY_MAX", TpDouble,
                             "Maximum frequency",
                             "MHz");
    MSLofarTable::addColumn (td, "LOFAR_OBSERVATION_FREQUENCY_MIN", TpDouble,
                             "Minimum frequency",
                             "MHz");
    MSLofarTable::addColumn (td, "LOFAR_OBSERVATION_FREQUENCY_CENTER", TpDouble,
                             "Center frequency",
                             "MHz");
    MSLofarTable::addColumn (td, "LOFAR_SUB_ARRAY_POINTING", TpInt,
                             "Subarray pointing id");
    MSLofarTable::addColumn (td, "LOFAR_NOF_BITS_PER_SAMPLE", TpInt,
                             "Number of bits per sample");
    MSLofarTable::addColumn (td, "LOFAR_ANTENNA_SET", TpString,
                             "SAS Antenna set name");
    MSLofarTable::addColumn (td, "LOFAR_FILTER_SELECTION", TpString,
                             "SAS Filter selection");
    MSLofarTable::addColumn (td, "LOFAR_CLOCK_FREQUENCY", TpDouble,
                             "SAS Clock setting",
                             "MHz");
    MSLofarTable::addColumn (td, "LOFAR_TARGET", TpArrayString,
                             "List of targets");
    MSLofarTable::addColumn (td, "LOFAR_SYSTEM_VERSION", TpString,
                             "Version of the system");
    MSLofarTable::addColumn (td, "LOFAR_PIPELINE_NAME", TpString,
                             "Pipeline identification");
    MSLofarTable::addColumn (td, "LOFAR_PIPELINE_VERSION", TpString,
                             "Pipeline version");
    MSLofarTable::addColumn (td, "LOFAR_FILENAME", TpString,
                             "Name of raw data set");
    MSLofarTable::addColumn (td, "LOFAR_FILETYPE", TpString,
                             "Data type (uv)");
    MSLofarTable::addColumn (td, "LOFAR_FILEDATE", TpDouble,
                             "File creation time",
                             "s", "EPOCH", MEpoch::UTC);
    return td;
  }

} //# end namespace
