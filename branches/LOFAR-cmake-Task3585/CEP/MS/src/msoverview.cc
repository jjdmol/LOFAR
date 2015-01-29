//# msoverview.cc: Show an overview of an MS
//#
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

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSSummary.h>
#include <tables/Tables/TableParse.h>
#include <casa/Containers/Record.h>
#include <casa/Inputs/Input.h>
#include <iostream>
#include <sstream>

using namespace casa;
using namespace std;

int main (int argc, char* argv[])
{
  try {
    // enable input in no-prompt mode
    Input inputs(1);
    // define the input structure
    inputs.version("20110407GvD");
    inputs.create ("in", "",
		   "Name of input MeasurementSet",
		   "string");
    inputs.create ("verbose", "False",
		   "Make a verbose listing?",
		   "bool");
    // Fill the input structure from the command line.
    inputs.readArguments (argc, argv);

    // Get and check the input specification.
    String msin (inputs.getString("in"));
    if (msin.empty()) {
      throw AipsError(" an input MeasurementSet must be given");
    }
    Bool verbose = inputs.getBool("verbose");

    // Show the MS summary.
    // Note that class MSSummary uses LogIO. Use that on a stringstream.
    MeasurementSet ms(msin);
    MSSummary summ(ms);
    ostringstream ostr;
    LogSink logsink(LogMessage::NORMAL, &ostr, False);
    LogIO logio(logsink);
    summ.listTitle (logio);
    // Show if the MS is a reference or concatenation.
    Block<String> partNames = ms.getPartNames();
    if (partNames.size() == 1) {
      if (partNames[0] != ms.tableName()) {
        Table tab(partNames[0]);
        logio << LogIO::NORMAL << "           "
              << "The MS references " << ms.nrow() << " out of "
              << tab.nrow() << " rows in " << partNames[0]
              << LogIO::POST;
      } else {
        // Show if it is a raw LOFAR MS (stored with LofarStMan).
        Record dminfo = ms.dataManagerInfo();
        for (unsigned i=0; i<dminfo.nfields(); ++i) {
          Record subrec = dminfo.subRecord(i);
          if (subrec.asString("TYPE") == "LofarStMan") {
            logio << LogIO::NORMAL << "           "
                  << "This is a raw LOFAR MS (stored with LofarStMan)"
                  << LogIO::POST;
            break;
          }
        }
      }
    } else if (partNames.size() > 0) {
      logio << LogIO::NORMAL << "           "
            << "The MS is a concatenation of: " << endl;
      for (uint i=0; i<partNames.size(); ++i) {
        Table tab(partNames[i]);
        logio << "               " << partNames[i] << "  (" << tab.nrow()
              << " rows) "<< endl;
      }
      logio << LogIO::POST;
    }
    logio << LogIO::NORMAL << endl << LogIO::POST;
    summ.listWhere (logio, True);
    // If possible, show the AntennaSet.
    Table obsTab(ms.keywordSet().asTable("OBSERVATION"));
    if (obsTab.tableDesc().isColumn ("LOFAR_ANTENNA_SET")) {
      logio << LogIO::NORMAL << "Antenna-set: "
            << ROScalarColumn<String>(obsTab, "LOFAR_ANTENNA_SET")(0)
            << LogIO::POST;
    }
    logio << LogIO::NORMAL << endl << LogIO::POST;
    summ.listMain (logio, False);
    logio << LogIO::NORMAL << endl << LogIO::POST;
    summ.listField (logio, False);
    logio << LogIO::NORMAL << endl << LogIO::POST;
    summ.listSpectralAndPolInfo (logio, verbose);
    if (verbose) {
      logio << LogIO::NORMAL << endl << LogIO::POST;
      summ.listAntenna (logio, True);
    }

    // Remove the extra fields (time, severity) from the output string.
    String str(ostr.str());
    str.gsub (Regex(".*\tINFO\t[+]?\t"), "");
    cout << str;

    // Test if the MS is regular.
    if (verbose) {
      uint nrdd = ms.dataDescription().nrow();
      // An MS is regular if all times have same nr of baselines.
      uint nrtime =
        tableCommand ("select from $1 orderby unique TIME", ms).table().nrow();
      Table blTab =
        tableCommand ("select from $1 orderby unique ANTENNA1,ANTENNA2", ms).table();
      uint nrbl = blTab.nrow();
      uint nrauto =
        tableCommand ("select from $1 where ANTENNA1=ANTENNA2", blTab).table().nrow(); 
        tableCommand ("select from $1 orderby unique ANTENNA1,ANTENNA2", ms).table().nrow();
      if (nrdd > 1) {
        // Get actual nr of bands.
        nrdd = tableCommand ("select from $1 orderby unique DATA_DESC_ID", ms).table().nrow();
      }
      cout << endl;
      if (ms.nrow() == nrtime*nrbl*nrdd) {
        cout << "The MS is fully regular, thus suitable for BBS" << endl;
      } else {
        cout << "The MS is not regular, thus unsuitable for BBS" << endl;
        cout << "  use msregularize in pyrap.tables to make it regular" << endl;
      }
      cout << "   nrows=" << ms.nrow() << "   ntimes=" << nrtime
           << "   nbands=" << nrdd << "   nbaselines=" << nrbl
           << " (" << nrauto << " autocorr)" << endl;
    }
    cout << endl;

  } catch (std::exception& x) {
    cerr << "Error: " << x.what() << endl;
    return 1;
  }
  return 0;
}
