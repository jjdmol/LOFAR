//  Simulate.cc:
//
//  Copyright (C) 2000-2002
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
//
/////////////////////////////////////////////////////////////////

#include <lofar_config.h>

#include <tinyCEP/SimulatorParseClass.h>
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>
#include <BBS3/BlackBoardDemo.h>

#ifdef HAVE_MPI
#include <mpi.h>
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
#ifdef HAVE_MPI
  MPI_Init(&argc, (char ***)&argv);
#endif
  try {
    // To try out different (serial) experiments without the CEP
    // framework, use following two statements:
    INIT_LOGGER("BlackboardDemo.log_prop");

    BlackBoardDemo simulator;

    simulator.setarg (argc, argv);

    // **** Set Knowledge Source parameters ****
    KeyValueMap KSparams;
    KSparams["MSName"] = "/data/meijeren/10Sources/demo10-"; // Name of the Measurement Set
                                                   // Currently each KS takes its own MS: KS1 takes <MSName>1
                                                   // (here: demo10-1,) KS3 takes <MSName>3 (demo10-3) etc.
    KSparams["DBHost"] = string("dop50");          // Parameter database host name
    KSparams["DBType"] = string("postgres");       // Parameter database type
    KSparams["DBName"] = string("meijeren");       // Parameter database name
    KSparams["DBPwd"] = string("");                // Parameter database password
    KSparams["meqTableName"] = string("meqmodel"); // Meq model table name *       
    KSparams["skyTableName"] = string("skymodel"); // Sky model table name *
                                                   // * each KS takes its own table: KS1 takes <meqTableName>1
                                                   // (meqmodel1) and KS4 takes <skyTableName>3 (skymodel3)

    // **** Set Control parameters ****
    KeyValueMap CTRLparams;
    CTRLparams["strategy"] = string("Simple");    // Strategy name
    vector<string> pNames(9);
    pNames[0] = "StokesI.CP1";
    pNames[1] = "RA.CP1";
    pNames[2] = "DEC.CP1";
    pNames[3] = "StokesI.CP2";
    pNames[4] = "RA.CP2";
    pNames[5] = "DEC.CP2";
    pNames[6] = "StokesI.CP3";
    pNames[7] = "RA.CP3";
    pNames[8] = "DEC.CP3";
    CTRLparams["solvableParams"] = pNames;        // Solvable parameter names
      // Set strategy parameters
      KeyValueMap STRATparams;
      STRATparams["nrIterations"] = 3;               // Number of iterations
      STRATparams["timeInterval"] = float(10.0);     // Time interval 
      vector<int> antNumbers;
      for (int i = 0; i <=20; i++)
      {
        antNumbers.push_back(i*4);
      }
      STRATparams["antennas"] = antNumbers;          // Baselines for which to solve
      STRATparams["startChan"] = 0;                  // Start (frequency) channel
      STRATparams["endChan"] = 0;                    // End (frequency) channel
      vector<int> srcNumbers(3);
      srcNumbers[0] = 1;
      srcNumbers[1] = 2;
      srcNumbers[2] = 3;    
      STRATparams["sources"] = srcNumbers;           // Solvable source numbers 
    CTRLparams["STRATparams"] = STRATparams;   

    // Create a KeyValueMap containing all parameter KeyValueMaps
    KeyValueMap params;
    params["nrKS"] = 1;                       // Number of Knowledge Sources
    params["BBDBname"] = string("meijeren");  // BlackBoard database name (must be a postgres database on dop50)
    params["KSparams"] = KSparams;
    params["CTRLparams"] = CTRLparams;

    simulator.baseDefine(params);
    simulator.baseRun(1);
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
    cout << "Unexpected exception in Simulate" << endl;
  }
}

