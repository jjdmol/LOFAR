//# VdsMaker.cc: Class to create the description of an MS
//#
//# Copyright (C) 2005
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>
#include <MS/VdsMaker.h>
#include <MWCommon/VdsPartDesc.h>
#include <MWCommon/ClusterDesc.h>
#include <Common/StreamUtil.h>
#include <Common/LofarLogger.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSMainColumns.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSPolarization.h>
#include <ms/MeasurementSets/MSPolColumns.h>
#include <ms/MeasurementSets/MSDataDescription.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MCDirection.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Containers/Record.h>
#include <casa/Utilities/LinearSearch.h>
#include <casa/OS/Path.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/HostInfo.h>
#include <casa/Exceptions/Error.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace LOFAR;
using namespace LOFAR::CEP;
using namespace casa;
using namespace std;

void VdsMaker::getFreqInfo (MS& ms, vector<int>& nrchan,
			    vector<Vector<double> >& startFreq,
			    vector<Vector<double> >& endFreq)
{
  MSDataDescription mssub1(ms.dataDescription());
  ROMSDataDescColumns mssub1c(mssub1);
  MSSpectralWindow mssub(ms.spectralWindow());
  ROMSSpWindowColumns mssubc(mssub);
  int nrspw = mssub1.nrow();
  nrchan.reserve (nrspw);
  startFreq.reserve (nrspw);
  endFreq.reserve (nrspw);
  for (int spw=0; spw<nrspw; ++spw) {
    Vector<double> chanFreq  = mssubc.chanFreq()(spw);
    Vector<double> chanWidth = mssubc.chanWidth()(spw);
    nrchan.push_back    (chanFreq.size());
    startFreq.push_back (chanFreq - chanWidth/2.);
    endFreq.push_back   (chanFreq + chanWidth/2.);
  }
}

void VdsMaker::getFields (MS& ms, vector<double>& ra, vector<double>& dec)
{
  MSField mssub(ms.field());
  ROMSFieldColumns mssubc(mssub);
  int nrf = mssub.nrow();
  ra.resize  (nrf);
  dec.resize (nrf);
  for (int i=0; i<nrf; ++i) {
    Array<MDirection> mds = mssubc.delayDirMeasCol().convert
      (i, MDirection::J2000);
    ra[i]  = mds.data()->getValue().get()[0];
    dec[i] = mds.data()->getValue().get()[1];
  }
}

void VdsMaker::getAntNames (MS& ms, vector<string>& antNames)
{
  MSAntenna mssub(ms.antenna());
  ROMSAntennaColumns mssubc(mssub);
  Vector<String> names = mssubc.name().getColumn();
  antNames.resize (names.size());
  for (uint i=0; i<names.size(); ++i) {
    antNames[i] = names[i];
  }
}

void VdsMaker::getCorrInfo (MS& ms, vector<string>& corrTypes)
{
  MSPolarization mssub(ms.polarization());
  ASSERT (mssub.nrow() > 0);
  ROMSPolarizationColumns mssubc(mssub);
  Vector<Int> ctvec = mssubc.corrType()(0);
  int nrp = ctvec.nelements();
  corrTypes.resize (nrp);
  for (int i=0; i<nrp; ++i) {
    corrTypes[i] = Stokes::name (Stokes::type(ctvec(i)));
  }
}

void VdsMaker::getDataFileInfo (MS& ms, string& name, bool& regular,
				vector<int>& tileShape, vector<int>& cubeShape)
{
  regular = false;
  Record rec = ms.dataManagerInfo();
  // Find the subrecord containing the DATA column.
  for (uint i=0; i<rec.nfields(); ++i) {
    const Record& subrec = rec.subRecord (i);
    Vector<String> colNames (subrec.asArrayString ("COLUMNS"));
    int inx = linearSearch1 (colNames, String("DATA"));
    if (inx >= 0) {
      const Record& specrec = subrec.subRecord ("SPEC");
      // Stored as a hypercube?
      if (specrec.isDefined ("HYPERCUBES")) {
	// Non-regular if multiple columns in storage manager.
	regular = (colNames.size() == 1);
	const Record& hcrec = specrec.subRecord ("HYPERCUBES");
	if (hcrec.nfields() != 1) {
	  regular = false;
	} else {
	  const Record& tsmrec = hcrec.subRecord (0);
	  tsmrec.asArrayInt ("TileShape").tovector (tileShape);
	  tsmrec.asArrayInt ("CubeShape").tovector (cubeShape);
	  // Find the name of the file containing the data.
	  // It can be TSM1 or TSM0.
	  ostringstream str;
	  str << specrec.asInt ("SEQNR");
	  name = string(ms.tableName()) + "/table.f" + str.str() + "_TSM1";
	  if (! RegularFile(name).exists()) {
	    name = string(ms.tableName()) + "/table.f" + str.str() + "_TSM0";
	    if (! RegularFile(name).exists()) {
	      regular = false;
	    }
	  }
	}
      }
      break;
    }
  }
}

string VdsMaker::findFileSys (const string& fileName,
			      const ClusterDesc& cdesc,
			      const string& hostName)
{
  // Find the file system by looking for a matching mountpoint.
  const vector<NodeDesc>& nodes = cdesc.getNodes();
  // First find the NodeDesc for this node.
  string nodeName(hostName);
  if (nodeName.empty()) {
    nodeName = HostInfo::hostName();
  }
  uint i=0;
  for (; i<nodes.size(); ++i) {
    if (nodes[i].getName() == nodeName) {
      break;
    }
  }
  ASSERTSTR (i < nodes.size(), "Hostname '" << nodeName << "' not found in "
	     "ClusterDesc file ");
  return nodes[i].findFileSys (fileName);
}


void VdsMaker::create (const string& msName, const string& outName,
		       const string& clusterDescName, const string& hostName)
{
  // Open the table.
  MS ms(msName);
  // Create and fill the Vds object.
  VdsPartDesc msd;
  ostringstream oss;
  // Fill in MS path and name.
  Path mspr(msName);
  string absName = mspr.absoluteName();
  // If the ClusterDesc file is given, try to find filesys.
  // Otherwise it is unknown.
  if (clusterDescName.empty()) {
    msd.setName (absName, "unknown");
  } else {
    ClusterDesc cdesc((ParameterSet(clusterDescName)));
    msd.setName (absName, findFileSys(absName, cdesc, hostName));
  }
  // Get freq info.
  // Fill in correlation info.
  vector<string> corrNames;
  getCorrInfo (ms, corrNames);
  ostringstream oss1;
  oss1 << corrNames;
  msd.addParm ("CorrNames", oss1.str());
  // Fill in freq info.
  vector<int> nchan;
  vector<Vector<double> > startFreq, endFreq;
  getFreqInfo (ms, nchan, startFreq, endFreq);
  for (uint i=0; i<nchan.size(); ++i) {
    vector<double> sfreq, efreq;
    startFreq[i].tovector (sfreq);
    endFreq[i].tovector   (efreq);
    msd.addBand (nchan[i], sfreq, efreq);
  }
  // Write the field directions (in J2000).
  vector<double> ra, dec;
  getFields (ms, ra, dec);
  int nrfield = ra.size();
  ostringstream oss2a, oss2b;
  oss2a << '[';
  oss2b << '[';
  for (int i=0; i<nrfield; ++i) {
    if (i > 0) {
      oss2a << ',';
      oss2b << ',';
    }
    oss2a << MVAngle::Format(MVAngle::TIME, 9)
	  << MVAngle(Quantity(ra[i], "rad"));
    oss2b << MVAngle::Format(MVAngle::ANGLE, 9)
	  << MVAngle(Quantity(dec[i], "rad"));
  }
  oss2a << ']';
  oss2b << ']';
  msd.addParm ("FieldDirectionRa",  oss2a.str());
  msd.addParm ("FieldDirectionDec", oss2b.str());
  msd.addParm ("FieldDirectionType", "J2000");
  // Fill in station names.
  vector<string> antNames;
  getAntNames (ms, antNames);
  ostringstream oss2;
  oss2 << antNames;
  msd.addParm ("StationNames", oss2.str());
  // Get the data file name.
  string dfName;
  bool dfRegular;
  vector<int> tileShape, cubeShape;
  getDataFileInfo (ms, dfName, dfRegular, tileShape, cubeShape);
  msd.addParm ("DataFileName", dfName);
  ostringstream oss3;
  oss3 << dfRegular;
  msd.addParm ("DataFileIsRegular", oss3.str());
  ostringstream oss4;
  oss4 << tileShape;
  msd.addParm ("DataTileShape", oss4.str());
  ostringstream oss5;
  oss5 << cubeShape;
  msd.addParm ("DataCubeShape", oss5.str());
  
  // Fill in times.
  ROMSMainColumns mscol(ms);
  uInt nrow = ms.nrow();
  // Get start and end time. Get the step time from the middle one.
  double stepTime = mscol.exposure()(nrow/2);
  double startTime = mscol.time()(0) - mscol.exposure()(0)/2;
  double endTime = mscol.time()(nrow-1) + mscol.exposure()(nrow-1)/2;
  msd.setTimes (startTime, endTime, stepTime);
  // Write into the vds file.
  ofstream ostr(outName.c_str());
  ASSERTSTR (ostr, "File " + outName + " could not be created");
  msd.write (ostr, "");
}
