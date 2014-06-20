//# fillRootImageGroup.cc: Program to fill the root group in an HDF5 image
//# Copyright (C) 2012
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <lofar_config.h>

#include <casa/Containers/Record.h>
#include <casa/HDF5/HDF5File.h>
#include <casa/HDF5/HDF5Group.h>
#include <casa/HDF5/HDF5Record.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/Quantum.h>

#include <Common/SystemUtil.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>

#include <ostream>
#include <sstream>

using namespace casa;
using namespace LOFAR;
using namespace std;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

// Convert time to FITS format string.
String time2String (const Quantity& time, int ndecimals)
{
  MVTime mvt(time);
  String tstr (mvt.string (MVTime::FITS, 6+ndecimals));
  return tstr;
}

// Combine the vector of strings to a single string.
String vector2String (const Vector<String>& vec, const String& sep)
{
  String str;
  for (uInt i=0; i<vec.size(); ++i) {
    if (i>0) str += sep;
    str += vec[i];
  }
  return str;
}

// Fill the root group.
void fill (const String& imageName)
{
  HDF5File hfile(imageName, ByteIO::Update);
  // Check if the LOFAR_OBSERVATION and LOFAR_ANTENNA group exists.
  ASSERTSTR (HDF5Group::exists (hfile, "ATTRGROUPS"),
             "The ATTRGROUPS group does no exist in image "
             << imageName);
  HDF5Group attr(hfile, "ATTRGROUPS");
  ASSERTSTR (HDF5Group::exists (attr, "LOFAR_OBSERVATION"),
             "The LOFAR_OBSERVATION group does no exist in image "
             << imageName);
  ASSERTSTR (HDF5Group::exists (attr, "LOFAR_ANTENNA"),
             "The LOFAR_ANTENNA group does no exist in image "
             << imageName);
  Record mainObsRec (HDF5Record::readRecord (attr, "LOFAR_OBSERVATION"));
  const Record& obsRec = mainObsRec.subRecord(0);
  Record antRec (HDF5Record::readRecord (attr, "LOFAR_ANTENNA"));
  Vector<String> stationNames(antRec.nfields());
  for (uInt i=0; i<antRec.nfields(); ++i) {
    stationNames[i] = antRec.subRecord(i).asString("NAME");
  }
  // Fill the root record.
  Record rootRec;
  rootRec.define ("GROUPTYPE",
                  "Root");
  rootRec.define ("FILENAME",
                  LOFAR::basename(imageName));
  Quantity fileDate(obsRec.asDouble("FILEDATE"),
                    Vector<String>(obsRec.asArrayString("FILEDATE_UNIT"))[0]);
  rootRec.define ("FILEDATE",
                  time2String (fileDate, 1));
  rootRec.define ("FILETYPE",
                  obsRec.asString("FILETYPE"));
  rootRec.define ("TELESCOPE",
                  obsRec.asString("TELESCOPE_NAME"));
  rootRec.define ("OBSERVER",
                  obsRec.asString("OBSERVER"));
  rootRec.define ("PROJECT_ID",
                  obsRec.asString("PROJECT"));
  rootRec.define ("PROJECT_TITLE",
                  obsRec.asString("PROJECT_TITLE"));
  rootRec.define ("PROJECT_PI",
                  obsRec.asString("PROJECT_PI"));
  rootRec.define ("PROJECT_CO_I",
                  vector2String (obsRec.asArrayString("PROJECT_CO_I"), "; "));
  rootRec.define ("PROJECT_CONTACT",
                  obsRec.asString("PROJECT_CONTACT"));
  rootRec.define ("OBSERVATION_ID",
                  obsRec.asString("OBSERVATION_ID"));
  Quantity startTime(obsRec.asDouble("OBSERVATION_START"),
                     *obsRec.asArrayString("OBSERVATION_START_UNIT").data());
  // MJDs have to be stored in days.
  rootRec.define ("OBSERVATION_START_MJD",
                  startTime.getValue("d"));
  rootRec.define ("OBSERVATION_START_UTC",
                  time2String(startTime, 9) + 'Z');     // Z means UTC
  Quantity endTime(obsRec.asDouble("OBSERVATION_END"),
                   *obsRec.asArrayString("OBSERVATION_END_UNIT").data());
  rootRec.define ("OBSERVATION_END_MJD",
                  endTime.getValue("d"));
  rootRec.define ("OBSERVATION_END_UTC",
                  time2String(endTime, 9) + 'Z');       // Z means UTC
  rootRec.define ("OBSERVATION_NOF_STATIONS",
                  Int(stationNames.size()));
  rootRec.define ("OBSERVATION_STATIONS_LIST",
                  stationNames);
  // Frequencies have to be in MHz.
  Quantity maxFreq(obsRec.asDouble("OBSERVATION_FREQUENCY_MAX"),
                   *obsRec.asArrayString("OBSERVATION_FREQUENCY_MAX_UNIT").data());
  rootRec.define ("OBSERVATION_FREQUENCY_MAX",
                  maxFreq.getValue("MHz"));
  Quantity minFreq(obsRec.asDouble("OBSERVATION_FREQUENCY_MIN"),
                   *obsRec.asArrayString("OBSERVATION_FREQUENCY_MIN_UNIT").data());
  rootRec.define ("OBSERVATION_FREQUENCY_MIN",
                  minFreq.getValue("MHz"));
  Quantity cenFreq(obsRec.asDouble("OBSERVATION_FREQUENCY_CENTER"),
                   *obsRec.asArrayString("OBSERVATION_FREQUENCY_CENTER_UNIT").data());
  rootRec.define ("OBSERVATION_FREQUENCY_CENTER",
                  cenFreq.getValue("MHz"));
  rootRec.define ("OBSERVATION_FREQUENCY_UNIT",
                  "MHz");
  rootRec.define ("OBSERVATION_NOF_BITS_PER_SAMPLE",
                  obsRec.asInt("NOF_BITS_PER_SAMPLE"));
  Quantity clockFreq(obsRec.asDouble("CLOCK_FREQUENCY"),
                     *obsRec.asArrayString("CLOCK_FREQUENCY_UNIT").data());
  rootRec.define ("CLOCK_FREQUENCY",
                  clockFreq.getValue("MHz"));
  rootRec.define ("CLOCK_FREQUENCY_UNIT",
                  "MHz");
  rootRec.define ("ANTENNA_SET",
                  obsRec.asString("ANTENNA_SET"));
  rootRec.define ("FILTER_SELECTION",
                  obsRec.asString("FILTER_SELECTION"));
  rootRec.define ("TARGET",
                  vector2String (obsRec.asArrayString("TARGET"), ", "));
  rootRec.define ("SYSTEM_VERSION",
                  obsRec.asString("SYSTEM_VERSION"));
  rootRec.define ("PIPELINE_NAME",
                  obsRec.asString("PIPELINE_NAME"));
  rootRec.define ("PIPELINE_VERSION",
                  obsRec.asString("PIPELINE_VERSION"));
  rootRec.define ("ICD_NUMBER",
                  "7");
  rootRec.define ("ICD_VERSION",
                  "0.03.00");
  rootRec.define ("NOTES",
                  String());
  HDF5Record::doWriteRecord (hfile, rootRec);
}

int main (int argc, char* argv[])
{
  try {
    if (argc < 2) {
      cerr << "Run as:   fillRootImageGroup <imageName>" << endl;
      cerr << endl;
      return 1;
    }
    fill (argv[1]);
    cout << "Filled Root group in HDF5 image " << argv[1] << endl;
  } catch (Exception& ex) {
    cerr << ex << endl;
    return 1;
  }
  return 0;
}
