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
#include <casa/OS/Path.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ArrayColumn.h>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace LOFAR;
using namespace ACC;
using namespace casa;
using namespace std;


// Now write out all descriptive data.
void writeDesc (const string& fileName, int nparts,
		const Array<double>& antPos,
		const DH_MSMake& info)
{
  double startFreq, endFreq, startTime, endTime, ra, dec;
  int nfreq, ntime;
  info.getFreq (startFreq, endFreq, nfreq);
  info.getTime (startTime, endTime, ntime);
  info.getPos (ra, dec);
  // Get nr of stations and baselines.
  int nstat = antPos.shape()[1];
  int nbl = nstat*(nstat-1)/2;
  vector<int> ant1, ant2;
  ant1.reserve (nbl);
  ant2.reserve (nbl);
  // Fill the stations of all baselines.
  for (int i=0; i<nstat; ++i) {
    for (int j=i+1; j<nstat; ++j) {
      ant1.push_back (i);
      ant2.push_back (j);
    }
  }
  ASSERT (uint(nbl) == ant1.size());
  // Fill the times and intervals.
  // The times are the midpoints of the intervals.
  double timeStep = (endTime-startTime) / ntime;
  vector<double> times(ntime);
  vector<double> intervals(ntime);
  for (int i=0; i<ntime; ++i) {
    times[i] = startTime + i*timeStep + timeStep/2;
    intervals[i] = timeStep;
  }
  std::ofstream ostr(fileName.c_str());
  BlobOBufStream bbs(ostr);
  BlobOStream bos(bbs);
  bos.putStart("ms.des", 1);
  bos << nparts;
  bos << ra << dec << 4 << nfreq << startFreq << endFreq
      << (endFreq-startFreq)/2;
  bos << ant1 << ant2;
  bos << times;
  bos << intervals;
  bos << antPos;
  bos.putEnd();
  cout << "freq: " << startFreq << ' ' << endFreq << ' ' << nfreq << endl;
  cout << "time: " << startTime << ' ' << endTime << ' ' << ntime << endl;
  cout << "pos: " << ra << ' ' << dec << endl;
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
		  const DH_MSMake& info)
{
  // Write a part of the entire frequency range.
  ostringstream ostr;
  ostr << "_p" << seqnr;
  string msNameF = msName + ostr.str();
  createMS (msNameF, antPos, info);
  // Write the description file.
  writeDesc (msNameF + "/vis.des", 0, antPos, info);
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
  double endFreq = startFreq + nfreq*stepFreq;
  double endTime = startTime + ntime*stepTime;

  // Get the station info from the given antenna table.
  string msName = params.getString ("MSName");
  string msDesPath = params.getString ("MSDesPath");
  string tabName = params.getString ("AntennaTableName");
  Table tab(tabName, TableLock(TableLock::AutoNoReadLocking));
  ROArrayColumn<double> posCol(tab, "POSITION");
  Array<double> antPos = posCol.getColumn();

  // Setup the connections to the slaves.
  // Each slave writes a part of the entire frequency range.
  MSMakeConn conn(nnode);
  // Fill the DataHolder as much as possible.
  conn.sender.setTime (startTime, endTime, ntime);
  conn.sender.setPos (ra, dec);
  // Write the antPos and MSName in the extra blob.
  conn.sender.fillExtra (msName, antPos);
  // Write the data in the DataHolder and send it to each slave.
  for (int i=0; i<nnode; ++i) {
    conn.sender.setFreq (startFreq+i*stepFreq, startFreq+(i+1)*stepFreq, nfpn);
    if (send) {
      conn.conns[i]->write();
    } else {
      createMSSeq (msName, i+1, antPos, conn.sender);
    }
  }
  // Write the overall description files.
  // Do this on the local node.
  Path path(msName);
  string msDesName = msDesPath + "/" + string(path.baseName());
  writeDesc (msDesName + ".des", nnode, antPos, conn.sender);
  string fileName = msDesName+".dess";
  std::ofstream ostr(fileName.c_str());
  int nstat = antPos.shape()[1];
  ostr << "npol=4  nstat=" << nstat << "  nbl=" << nstat*(nstat-1)/2 << endl;
  ostr << "freq: start=" << startFreq/1e6 << " Mhz  end=" << endFreq/1e6
       << " Mhz  step=" << (endFreq-startFreq)/nfreq/1e3
       << " Khz" << endl;
  ostr << "      nchan=" << nfreq << "  npart=" << nnode
       << " (" << nfpn << " chan per part)" << endl;
  ostr << "time: start="
       << MVTime::Format(MVTime::DMY) << MVTime(Quantity(startTime,"s"))
       << "  end="
       << MVTime::Format(MVTime::DMY) << MVTime(Quantity(endTime,"s"))
       << "  step=" << (endTime-startTime)/ntime << " sec" << endl;
  ostr << "      ntime=" << ntime << endl;
}

void doSlave (int rank)
{
  cout << "slave rank = " << rank << endl;
  // Setup the connections to the master.
  // Note that fake connections for the slave < rank have to be made to get
  // the correct tag in the Connection object.
  MSMakeConn conn(rank);
  conn.conns[rank-1]->read();
  cout << "Rank " << rank << " received " << conn.receiver.getDataSize() << " bytes" << endl;
  Array<double> antPos;
  string msName;
  conn.receiver.getExtra (msName, antPos);
  createMSSeq (msName, rank, antPos, conn.receiver);
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
