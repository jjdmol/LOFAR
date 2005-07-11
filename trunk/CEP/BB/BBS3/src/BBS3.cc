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


using namespace LOFAR;
using namespace std;

// This program should be called with input file name(1) and user name(2) as
// arguments

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
	usernm = argv[3];
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
      usernm = argv[3];
    }
      
#endif

    BlackBoardDemo simulator;

    cout << "Input arguments: " << "Input file name = " << name 
	 << ", User name = " << usernm << endl;
    
    simulator.setarg (argc, argv);

    ACC::APS::ParameterSet params (name.c_str());

    int nrStrategies = params.getInt32("CTRLparams.nrStrategies");

    // Loop over all strategies
    for (int i=1; i<=nrStrategies; i++)
    {
      char nrStr[32];
      sprintf(nrStr, "%i", i);
      string name = "SC" + string(nrStr) + "params";
      // Add the dbname if not defined.
      if (! params.isDefined(string("CTRLparams.")+name+".MSDBparams.DBName")) {
	params[string("CTRLparams.")+name+".MSDBparams.DBName"] = usernm;
      }
      if (! params.isDefined("BBDBname")) {
	params["BBDBname"] = usernm;
      }
    }
    cout << params << endl;

    int nrRuns = params.getInt32("nrRuns");

    simulator.setParameters(params);
    simulator.baseDefine();
    simulator.baseRun(nrRuns);
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

