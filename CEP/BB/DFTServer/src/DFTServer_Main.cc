//# DFTServer_Main.cc: Main loop for the DFTServer ApplicationHolder 
//#
//# Copyright (C) 2004
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

#include <DFTServer/mpidft.h>
#include <DFTServer/DH_DFTRequest.h>
#include <DFTServer/DH_DFTResult.h>
#include <DFTServer/DFTStub.h>
#include <Common/lofar_iostream.h>
#include <Common/Debug.h>
#include <Common/LofarLogger.h>
#ifdef HAVE_MPI
# include <mpi.h>
#endif


using namespace LOFAR;

#ifdef HAVE_CORBA
int atexit(void (*function)(void))
{
  return 1;
}
#endif

int main (int argc, const char** argv)
{
  // Initialize the LOFAR logger
  INIT_LOGGER("DFTServer.log_prop");

  int returnval=0;
#ifdef HAVE_MPI
  int myrank;
  MPI_Init(&argc,(char***)&argv);
  /* get this process's rank (ID) within the process group */
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  if (myrank != 0) {
    doDFTslave();
    MPI_Finalize();
    return 0;
  } 
#endif

  try {
    DH_DFTRequest req;
    DH_DFTResult res;
    DFTStub myClientStub(true);
    req.preprocess();
    res.preprocess();
    req.setID (0);
    res.setID (1);
    myClientStub.connect (req, res);

    // Read until no more requests.
    int nFreq = 2;
    int nTime = 2;
    while (nFreq > 0  &&  nTime > 0) {
      req.read();
      nFreq = req.getNFreq();
      nTime = req.getNTime();
      int nBaseline = req.getNBaseline();
      cout << "Request for nant=" << req.getNAnt()
	   << " ntime=" << nTime << " nfreq=" << nFreq
	   << " startfreq=" << req.getStartFreq()
	   << " stepfreq=" << req.getStepFreq()
	   << " L=" << req.getL()
	   << " M=" << req.getM()
	   << " N=" << req.getN()
	   << " nbasel=" << req.getNBaseline()
	   << endl;
      
      if (nFreq > 0  &&  nTime > 0) {
	res.set (nFreq, nTime, nBaseline);
      }
      doDFTmaster (req.getNAnt(), req.getAnt(), nTime, nFreq, 1, 1, 
		   req.getStartFreq(), req.getStepFreq(),
		   req.getL(), req.getM(), req.getN(),
		   req.getUVW(),
		   req.getNBaseline(), 
		   req.getAnt1(), req.getAnt2(),
		   res.accessValues());
      
      cout << "Request done for nant=" << req.getNAnt()
	   << " ntime=" << nTime << " nfreq=" << nFreq
	   << " startfreq=" << req.getStartFreq()
	   << " stepfreq=" << req.getStepFreq()
	   << " L=" << req.getL()
	   << " M=" << req.getM()
	   << " N=" << req.getN()
	   << " nbasel=" << req.getNBaseline()
	   << endl;

      if (nFreq > 0  &&  nTime > 0) {
	res.write();
      }
    }
  } catch (Exception &e) {
    cout << "Unexpected exception " << e.text() << endl;
    returnval = 1;
  }
#ifdef HAVE_MPI
  MPI_Finalize();
#endif
  return returnval;
}
