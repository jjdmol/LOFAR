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
#ifdef HAVE_MPI
#include <Transport/TH_MPI.h>
#endif
#include <Common/KeyParser.h>
#include <string>
#include <iostream>
#include <fstream>

#include <Common/BlobOBufChar.h>
#include <Common/BlobIBufChar.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/BlobIBufStream.h>
#include <Common/BlobArray.h>

#include <casa/Arrays/Matrix.h>

using namespace LOFAR;
using namespace std;

// This program should be called with input file name(1) and user name(2) as
// arguments

void readMSTimes(const string& fileName, double& startTime, double& endTime,
		 double& interval)
{
  // Get meta data from description file.
  string name(fileName+"/vis.des");
  std::ifstream istr(name.c_str());
  ASSERTSTR (istr, "File " << fileName << "/vis.des could not be opened");
  BlobIBufStream bbs(istr);
  BlobIStream bis(bbs);
  bis.getStart("ms.des");
  double ra, dec;
  bis >> ra;
  bis >> dec;
  int itsNCorr, itsNrChan;
  bis >> itsNCorr;
  bis >> itsNrChan;
  double itsStartFreq, itsEndFreq, itsStepFreq;
  bis >> itsStartFreq;
  bis >> itsEndFreq;
  bis >> itsStepFreq;
  vector<int> itsAnt1, itsAnt2;
  vector<double> itsTimes, itsIntervals;
  bis >> itsAnt1;
  bis >> itsAnt2;
  bis >> itsTimes;
  bis >> itsIntervals;
  casa::Matrix<double> itsAntPos;
  bis >> itsAntPos;
  bis.getEnd();
  istr.close();
  // Get startTime, endTime and interval size
  double sTime = itsTimes.front();
  interval = itsIntervals[0];
  double eTime = itsTimes.back();
  double eInterval = itsIntervals.back();
  startTime = sTime-(interval/2);
  endTime = eTime+(eInterval/2);
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

    // Get time parameters
    double intervalSize = params.getDouble(stratName + "timeInterval");
    double startTime = -1;
    if (params.isDefined(stratName + "startTime"))
    {
      startTime = params.getDouble(stratName + "startTime");
    }
    double endTime = -1;
    if (params.isDefined(stratName + "endTime"))
    {
      endTime = params.getDouble(stratName + "endTime");
    }

    // Read MS description file
    string msName = params.getString(stratName + "MSDBparams.MSName");   
    double msStartTime, msEndTime, msInterval;
    readMSTimes(msName, msStartTime, msEndTime, msInterval);
    // Correct start- and endtime and interval size if necessary
    if (msStartTime > startTime)
    {
      LOG_INFO_STR("Replacing start time with MS start time " << msStartTime
		   << " for strategy " << string(nrStr));
      startTime = msStartTime;
      params.replace(ACC::APS::KVpair(stratName+ "startTime", msStartTime));
    }
    if (endTime==-1 || msEndTime < endTime)
    {
      LOG_INFO_STR("Replacing end time with MS end time " << msEndTime
		   << " for strategy " << string(nrStr));
      endTime = msEndTime;
      params.replace(ACC::APS::KVpair(stratName+ "endTime", msEndTime));
    }
    if (msInterval > intervalSize)
    {
      LOG_INFO_STR("Replacing time interval size with MS interval size " << msInterval
		   << " for strategy " << string(nrStr));
      intervalSize = msInterval;
      params.replace(ACC::APS::KVpair(stratName+ "timeInterval", msInterval));
    }
    // Determine number of CEPFrame runs
    int maxIter = params.getInt32(stratName +"maxNrIterations");
    int nrIntervals = 0;
    if (intervalSize >= endTime-startTime)
    {
      nrIntervals = 1;
    }
    else
    {
      nrIntervals = (int)floor((endTime-startTime)/intervalSize)+1;
    }
    int maxRuns = maxIter*nrIntervals;
    LOG_INFO_STR("Strategy " << string(nrStr) << " max number of runs " << maxRuns);
    totalNrRuns += maxRuns;
  }

  // Set total number of CEPFrame runs
  params.replace(ACC::APS::KVpair("nrCEPFrameRuns", totalNrRuns));
  cout << params << endl;
}

int main (int argc, const char** argv)
{
  try {
    // To try out different (serial) experiments without the CEP
    // framework, use following two statements:
    INIT_LOGGER("BBS3Logger");

    // Set default values
    string name = "BBS3.inputDefault";
    const char *userName = getenv("USER");
    if (userName == 0) {
      cerr << "$USER not in environment\n";
      exit(1);
    } 
    string usernm(userName);

#ifdef HAVE_MPI
    TH_MPI::initMPI(argc, argv);
#endif

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

    BlackBoardDemo simulator;

    cout << "Input arguments: " << "Input file name = " << name 
	 << ", User name = " << usernm << endl;
    
    simulator.setarg (argc, argv);

    ACC::APS::ParameterSet params (name.c_str());

    checkParameters(params, usernm);

    int nrCEPFrameRuns = params.getInt32("nrCEPFrameRuns");

    simulator.setParameters(params);
    simulator.baseDefine();
    simulator.baseRun(nrCEPFrameRuns);
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

