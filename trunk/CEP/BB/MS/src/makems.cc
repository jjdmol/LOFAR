//#  makems.cc: Make a distributed MS
//#
//#  Copyright (C) 2005
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

#include <MS/DH_MSMake.h>
#include <MS/MSCreate.h>
#include <MS/MSDesc.h>
#include <APS/ParameterSet.h>
#include <Blob/BlobOBufStream.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobArray.h>
#include <Common/LofarLogger.h>

#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/OS/Path.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace LOFAR;
using namespace ACC;
using namespace casa;
using namespace std;


// Now write out all descriptive data.
void writeDesc (const string& fileName, bool writeDess,
		const string& msName, int nparts,
		const Array<double>& antPos,
		const Vector<String>& antNames,
		const DH_MSMake& info)
{
  MSDesc msd;
  Path mspath(msName);
  Path path = Path(mspath.absoluteName());
  msd.msPath = path.dirName();
  msd.msName = path.baseName();
  msd.npart = nparts;
  info.getPos (msd.ra, msd.dec);
  int ntime;
  info.getTime (msd.startTime, msd.endTime, ntime);
  msd.corrTypes.push_back ("XX");
  msd.corrTypes.push_back ("XY");
  msd.corrTypes.push_back ("YX");
  msd.corrTypes.push_back ("YY");
  msd.nchan.resize (1);
  msd.startFreq.resize (1);
  msd.endFreq.resize (1);
  info.getFreq (msd.startFreq[0], msd.endFreq[0], msd.nchan[0]);
  // Get nr of stations and baselines.
  uint nstat = antPos.shape()[1];
  int nbl = nstat*(nstat-1)/2;
  msd.antPos = antPos;
  ASSERT (antNames.nelements() == nstat);
  for (uint i=0; i<nstat; ++i) {
    msd.antNames.push_back (antNames(i));
  }
  for (int i=0; i<3; ++i) {
    msd.arrayPos.push_back (msd.antPos(IPosition(2,i,nstat/2)));
  }
  msd.ant1.reserve (nbl);
  msd.ant2.reserve (nbl);
  // Fill the stations of all baselines.
  for (uint i=0; i<nstat; ++i) {
    for (uint j=i+1; j<nstat; ++j) {
      msd.ant1.push_back (i);
      msd.ant2.push_back (j);
    }
  }
  ASSERT (uint(nbl) == msd.ant1.size());
  // Fill the times and intervals.
  // The times are the midpoints of the intervals.
  double timeStep = (msd.endTime-msd.startTime) / ntime;
  msd.times.reserve (ntime);
  msd.exposures.reserve (ntime);
  for (int i=0; i<ntime; ++i) {
    msd.times.push_back (msd.startTime + i*timeStep + timeStep/2);
    msd.exposures.push_back (timeStep);
  }
  {
    string fullName = fileName + ".des";
    std::ofstream ostr(fullName.c_str());
    BlobOBufStream bbs(ostr);
    BlobOStream bos(bbs);
    bos << msd;
  }
  if (writeDess) {
    string fullName = fileName + ".dess";
    std::ofstream ostr(fullName.c_str());
    ostr << msd;
  }
}

void createMS (const string& msName, const Array<double>& antPos,
	       const DH_MSMake& info)
{
  double startFreq, endFreq, startTime, endTime, ra, dec;
  int nfreq, ntime;
  info.getFreq (startFreq, endFreq, nfreq);
  info.getTime (startTime, endTime, ntime);
  info.getPos (ra, dec);
  double freqStep = (endFreq-startFreq) / nfreq;
  double freqRef = (endFreq+startFreq) / 2;
  double timeStep = (endTime-startTime) / ntime;
  MSCreate msmaker(msName, startTime, timeStep, nfreq, 4, antPos.shape()[1],
		   Matrix<double>(antPos));
  msmaker.addBand (4, nfreq, freqRef, freqStep);
  msmaker.addField (ra, dec);
  for (int i=0; i<ntime; ++i) {
    msmaker.writeTimeStep();
  }
}

void createMSSeq (const string& msName, int seqnr, const Array<double>& antPos,
		  const Vector<String>& antNames,
		  const DH_MSMake& info)
{
  // Write a part of the entire frequency range.
  ostringstream ostr;
  ostr << "_p" << seqnr;
  string msNameF = msName + ostr.str();
  createMS (msNameF, antPos, info);
  // Write the description file.
  writeDesc (msNameF + "/vis.des", false, msNameF, 0, antPos, antNames, info);
}

void doMaster (bool send)
{
  cout << "Master sends the data" << endl;
  APS::ParameterSet params ("makems.cfg");
  // Get the various parameters.
  double startFreq = params.getDouble ("StartFreq");
  double stepFreq  = params.getDouble ("StepFreq");
  string startTimeStr = params.getString ("StartTime");
  double stepTime     = params.getDouble ("StepTime");
  Quantity qn;
  ASSERT (MVTime::read (qn, startTimeStr, true));
  double startTime = qn.getValue ("s");
  string raStr  = params.getString ("RightAscension");
  string decStr = params.getString ("Declination");
  ASSERT (MVAngle::read (qn, raStr, true));
  double ra = qn.getValue ("rad");
  ASSERT (MVAngle::read (qn, decStr, true));
  double dec = qn.getValue ("rad");
  int nfreq = params.getInt32 ("NFrequencies");
  int ntime = params.getInt32 ("NTimes");
  int nnode = params.getInt32 ("NParts");
  ASSERT (nnode > 0);
  ASSERT (nfreq >= nnode);
  ASSERT (nfreq%nnode == 0);
  ASSERT (stepFreq > 0);
  ASSERT (stepTime > 0);
  int nfpn = nfreq/nnode;
  double endTime = startTime + ntime*stepTime;

  // Get the station info from the given antenna table.
  string msName = params.getString ("MSName");
  string msDesPath = params.getString ("MSDesPath");
  string tabName = params.getString ("AntennaTableName");
  Table tab(tabName, TableLock(TableLock::AutoNoReadLocking));
  ROArrayColumn<double> posCol(tab, "POSITION");
  Array<double> antPos = posCol.getColumn();
  ROScalarColumn<String> nameCol(tab, "NAME");
  Vector<String> antNames = nameCol.getColumn();

  // Setup the connections to the slaves.
  // Each slave writes a part of the entire frequency range.
  MSMakeConn conn(nnode);
  // Fill the DataHolder as much as possible.
  conn.sender.setTime (startTime, endTime, ntime);
  conn.sender.setPos (ra, dec);
  // Write the antPos and MSName in the extra blob.
  conn.sender.fillExtra (msName, antPos, antNames);
  // Write the data in the DataHolder and send it to each slave.
  for (int i=0; i<nnode; ++i) {
    conn.sender.setFreq (startFreq+i*stepFreq, startFreq+(i+1)*stepFreq, nfpn);
    if (send) {
      conn.conns[i]->write();
    } else {
      createMSSeq (msName, i+1, antPos, antNames, conn.sender);
    }
  }
  // Write the overall description files.
  // Do this on the local node.
  Path mspath(msName);
  Path path = Path(mspath.absoluteName());
  string msDesName = msDesPath + "/" + string(path.baseName());
  writeDesc (msDesName, true, path.originalName(), nnode,
	     antPos, antNames, conn.sender);
}

void doSlave (int rank)
{
  cout << "slave rank = " << rank << endl;
  // Setup the connections to the master.
  // Note that fake connections for the slave < rank have to be made to get
  // the correct tag in the Connection object.
  MSMakeConn conn(rank);
  conn.conns[rank-1]->read();
  cout << "Rank " << rank << " received " << conn.receiver.getDataSize()
       << " bytes" << endl;
  Array<double> antPos;
  Array<String> antNamesArr;
  string msName;
  conn.receiver.getExtra (msName, antPos, antNamesArr);
  Vector<String> antNames(antNamesArr);
  createMSSeq (msName, rank, antPos, antNames, conn.receiver);
}


int main (int argc, const char** argv)
{
  INIT_LOGGER("makems");
  int status = 0;
  try {
#ifdef HAVE_MPI
    TH_MPI::initMPI (argc, argv);
    // If we are the master, send the data.
    // Otherwise receive the data and generate the MS.
    int rank = TH_MPI::getCurrentRank();
    if (rank == 0) {
      doMaster (true);
    } else {
      doSlave (rank);
    }
    TH_MPI::finalize();
#else
    // No MPI, so handle everything in one process.
    doMaster (false);
#endif
  } catch (std::exception& x) {
    cout << "Unexpected exception in " << argv[0] << ": " << x.what() << endl;
    status = 1;
  }
  exit(status);
}
