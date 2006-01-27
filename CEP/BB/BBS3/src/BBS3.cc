//  BBS3.cc:
//
//  Copyright (C) 2004
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include <lofar_config.h>

#include <BBS3/BlackBoardDemo.h>
#include <MS/MSDesc.h>
#ifdef HAVE_MPI
#include <Transport/TH_MPI.h>
#endif
#include <Blob/KeyParser.h>
#include <string>
#include <iostream>
#include <fstream>

#include <Blob/BlobOBufChar.h>
#include <Blob/BlobIBufChar.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobIBufStream.h>
#include <Blob/BlobArray.h>

#include <casa/Arrays/Matrix.h>

#include <BBS3/BBSTestLogger.h>

using namespace LOFAR;
using namespace std;

// This program should be called with input file name(1) and user name(2) as
// arguments

void readMSTimes(const string& fileName, double& startTime, double& endTime,
		 double& interval)
{
  // Get meta data from global description file.
  string name(fileName+".des");
  std::ifstream istr(name.c_str());
  ASSERTSTR (istr, "File " << fileName << ".des could not be opened");
  BlobIBufStream bbs(istr);
  BlobIStream bis(bbs);
  MSDesc msd;
  bis >> msd;
  startTime = msd.startTime;
  endTime = msd.endTime;
  interval = msd.exposures.back();
}

void checkParameters(ACC::APS::ParameterSet& params, const string& usernm)
{
  int nrStrategies = params.getInt32("CTRLparams.nrStrategies");
  
  int totalNrRuns=0;

  // Loop over all strategies
  for (int i=1; i<=nrStrategies; i++)
  {
    char nrStr[32];
    sprintf(nrStr, "%i", i);
    string stratName = "CTRLparams.SC" + string(nrStr) + "params.";
      // Add the dbname if not defined.
    if (! params.isDefined(stratName+"MSDBparams.DBName")) {
      params[stratName+"MSDBparams.DBName"] = usernm;
    }
    if (! params.isDefined("BBDBname")) {
      params["BBDBname"] = usernm;
    }

    // Read MS description file
    string msName = params.getString(stratName + "MSDBparams.generalMSPath") + params.getString(stratName + "MSDBparams.MSName");   
    double msStartTime, msEndTime, msInterval;
    readMSTimes(msName, msStartTime, msEndTime, msInterval);

    // Get time parameters
    double intervalSize = params.getDouble(stratName + "timeInterval");
    double startTime = -1;
    if (params.isDefined(stratName + "startTime"))
    {
      startTime = params.getDouble(stratName + "startTime");
    }
    if (startTime < 0)
    {
      // Correct start time
      LOG_INFO_STR("Using " << msName << " start time " << msStartTime
		   << " for strategy " << string(nrStr));
      startTime = msStartTime;
      params.replace(ACC::APS::KVpair(stratName+ "startTime", msStartTime));
    }
    
    double endTime = -1;
    if (params.isDefined(stratName + "endTime"))
    {
      endTime = params.getDouble(stratName + "endTime");
    }
    if (endTime < 0)
    {
      // Correct end time
      LOG_INFO_STR("Using " << msName << " end time " << msEndTime
		   << " for strategy " << string(nrStr));
      endTime = msEndTime;
      params.replace(ACC::APS::KVpair(stratName+ "endTime", msEndTime));
    }

    if (msInterval > intervalSize)
    {
      LOG_INFO_STR("Replacing time interval size with " << msName << " interval size " 
		   << msInterval << " for strategy " << string(nrStr));
      intervalSize = msInterval;
      params.replace(ACC::APS::KVpair(stratName+ "timeInterval", msInterval));
    }

    // Determine number of CEPFrame runs
    int maxIter = params.getInt32(stratName +"maxNrIterations");
    string stratType = params.getString(stratName + "strategy");
    if ((stratType == "CompoundIter"))
    {
      maxIter = 1;     // Iterations are done within one CEPFrame run.
    }
    int nrIntervals = 0;
    if (intervalSize >= endTime-startTime)
    {
      nrIntervals = 1;
    }
    else
    {
      nrIntervals = (int)ceil((endTime-startTime)/intervalSize);
    }
    int maxRuns = maxIter*nrIntervals;
    LOG_INFO_STR("Strategy " << string(nrStr) << " max number of runs " << maxRuns);
    totalNrRuns += maxRuns;
  }

  totalNrRuns += nrStrategies-1; // Add 1 extra CEPFrame run per strategy for possible
                                 // clean-up orders

  // Set total number of CEPFrame runs
  params.replace(ACC::APS::KVpair("nrCEPFrameRuns", totalNrRuns));
#ifdef HAVE_MPI
  if (TH_MPI::getCurrentRank() == 0) {
    cout << params << endl;
  };
#else
  cout << params << endl;
#endif
}

int main (int argc, const char** argv)
{
  BBSTest::ScopedUSRTimer totTimer("total-execute-with-MPI");
  try {
    // To try out different (serial) experiments without the CEP
    // framework, use following two statements:
    INIT_LOGGER("BBS3Logger");
    // init the BBSTestLogger
#ifdef HAVE_MPI
    TH_MPI::initMPI(argc, argv);
#endif

    //BBSTest::Logger::init();
    BlackBoardDemo simulator;
    {
      BBSTest::ScopedUSRTimer st("total-execute-without-MPI");

      // Set default values
      string name = "BBS3.inputDefault";
      const char *userName = getenv("USER");
      if (userName == 0) {
	cerr << "$USER not in environment\n";
	exit(1);
      } 
      string usernm(userName);

#ifdef HAVE_MPICH
      if (argc > 3)
	{
	  // Broadcast input arguments to all processes
	  int myRank = TH_MPI::getCurrentRank();
	  if (myRank == 0)
	    {
	      // Get input file name.
	      name = argv[1];
	      // Get user name
	      usernm = argv[2];
	      BlobOBufChar bufo;
	      // Fill the buffer
	      BlobOStream bos(bufo);
	      bos.putStart ("InputArgs", 1);
	      bos << name;
	      bos << usernm;
	      bos.putEnd();
	      uint64 bufSize = bufo.size();
	      // Broadcast buffer size
	      MPI_Bcast(&bufSize, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
	      // Broadcast buffer
	      MPI_Bcast((void*)(bufo.getBuffer()), bufSize, MPI_BYTE, 0, MPI_COMM_WORLD);
	    }
	  else
	    {
	      uint64 bufSize=0;
	      // Receive buffer size
	      MPI_Bcast(&bufSize, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
	      char buffer[bufSize];
	      MPI_Bcast(buffer, bufSize, MPI_BYTE, 0, MPI_COMM_WORLD);

	      BlobIBufChar bufi(buffer, bufSize);
	      BlobIStream bis(bufi);
	      bis.getStart ("InputArgs");
	      bis >> name;
	      bis >> usernm;
	      bis.getEnd();
	    }
	}

#else 

      // Get input file name.
      if (argc > 1) {
	name = argv[1];
      }
      // Get user name
      if (argc > 2) {
	usernm = argv[2];
      }
      
#endif

      cout << "Input arguments: " << "Input file name = " << name 
	   << ", User name = " << usernm << endl;
    
      simulator.setarg (argc, argv);

      ACC::APS::ParameterSet params (name.c_str());

      checkParameters(params, usernm);

      int nrCEPFrameRuns = params.getInt32("nrCEPFrameRuns");

      simulator.setParameters(params);
      simulator.baseDefine();
      simulator.baseRun(nrCEPFrameRuns);
    } // scopedTimerBlock
    simulator.baseQuit();

  }
  catch (LOFAR::Exception& e)
  {
    cout << "Lofar exception: " << e.what() << endl;
  }
  catch (std::exception& e)
  {
    cout << "Standard exception: " << e.what() << endl;
  }
  catch (...) {
    cout << "Unexpected exception in BBS3" << endl;
  }
}

