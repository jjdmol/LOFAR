//
//  VHECRtest.cc: framework for testing the VHECR functionality
//
//  Copyright (C) 2008
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
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <Common/lofar_fstream.h>
#include "VHECR/TBBTrigger.h"
#include "VHECR/VHECRTask.h"

//includes for the commandline options
#include <boost/program_options.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/detail/cmdline.hpp>
namespace bpo = boost::program_options;

using namespace LOFAR;
using namespace LOFAR::VHECR;

int main(int argc, char* argv[])
{
  // Get log_prop filename (do this before boost may screw up the argv)
  string logFile(argv[0]);
  logFile.append(".log_prop");
  INIT_LOGGER (logFile.c_str());

  //setup all the variables
  bool         followFile = false;
  char         triggerLine [4096];
  TBBTrigger   theTrigger;
  istream    * triggerFile = NULL;        
    
  int NoCoincidenceChannels = 48;
  float CoincidenceTime = 1e-6;
  int SamplingRate = 200;
  int DoDirectionFit = 0;
  string AntennaPositionsFile = "";
  string AntennaSelection = "LBA_OUTER";
  float MinElevation = 30.;
  float MaxFitVariance = 50.;

//######## Boost program-options section begin ############################
  bpo::options_description desc ("[VHECRTest] usage: \"VHECRTest [<options>]\"\n Note: program needs file VHECRTest.log_prop for log-system.\n Available command line options" );
  
  desc.add_options ()
    ("help,H", "Show help messages")
    //("parset",bpo::value<std::string>(), "Name of the parset (\"Observation???\") file (mandatory!)")
    ("outfile,O",bpo::value<std::string>(), "Name of the output file (default=stdout)")
    ("infile,I", bpo::value<std::string>(), "Name of the input file (default=stdin=\"-\")")
    ("keepFollwingFile,K", "Don't stop at end-of-file keep checking for additional input. (default:off)")
    ("noCoincidenceChannels,C", bpo::value<int>(), "Number of channels needed for a coincidence (default=48)")
    ("coincidenceTime,T", bpo::value<float>(), "Time window for the coincidence[sec] (default=1e-6)")
    ("samplingRate,R", bpo::value<int>(), "Sampling rate of the measurement[MHz] (default=200)")
    ("doDirectionFit,D", bpo::value<int>(), "Do a direction fit (0: none (default), 1: only AzEl, 2: AzEl and distance)")
    ("antennaPositionsFile,P", bpo::value<std::string>(), "Path to the file with the antenna positions, mandatory for D>0")
    ("antennaSelection,S", bpo::value<std::string>(), "Name of the antenna selection (default=\"LBA_OUTER\")")
    ("minElevation,E", bpo::value<float>(), "Minimum elevation for a good pulse[deg] (default=30),NIY")
    ("maxBadnessOfFit,B", bpo::value<float>(), "Maximum \"badness of fit\" [values 0.-100.] (default=50),NIY")
     ;
  
  
  //bpo::positional_options_description p;
  //p.add("parset", 1);
  bpo::variables_map vm;
  bpo::store(bpo::command_line_parser(argc, argv).options(desc)
  //     .positional(p)
  	     .allow_unregistered().run(), vm);
  bpo::notify(vm);
  
  if (vm.count("help") || argc==1 || vm.count("help")) {
    cout << "\n" << desc << endl;
    return 0;
  }

// Commented out code to deal with a parset file
//   if (vm.count("parset") != 1) {    
//     cout << endl;
//     cerr << "[VHECRTest] Missing \"parset\" option!" << endl;
//     cerr << " usage: \"VHECRTest <parset_file> [<options>]\"" << endl;
//     cerr << "    or: \"VHECRTest -H\" for a list of options" << endl;
//     return 1;    
//   };
//   // Now we know that there is exactly one parset option!
//   cout << "[VHECRTest] opening parset file: \"" << vm["parset"].as<std::string>() << "\"..." << endl;

  VHECRTask theTask;
  //set default values
  theTask.itsOutputFilename = "-";
  theTask.itsConfigurationFile = "";
  

  if (vm.count("infile")) {
    std::ifstream *in;
    in = new std::ifstream( vm["infile"].as<std::string>().c_str() ) ;
    if ( ! *in ) {
      std::cerr << "Could not open: " << vm["infile"].as<std::string>() <<  std::endl ;
      return(1);
    } else {
      std::cout << "Reading from: \"" << vm["infile"].as<std::string>() <<  "\"" << std::endl ;
      triggerFile = in ;
    }
  } else {
    std::cout << "Reading from standard input" << std::endl ;
    triggerFile = &cin;
  };

  if (vm.count("outfile")) {
    theTask.itsOutputFilename = vm["outfile"].as<std::string>();
  };

 if (vm.count("keepFollwingFile")) {
   followFile = true;
 };

 if (vm.count("noCoincidenceChannels")) {
   NoCoincidenceChannels = vm["noCoincidenceChannels"].as<int>();
 }
 
 if (vm.count("coincidenceTime")) {
   CoincidenceTime = vm["coincidenceTime"].as<float>();
 }

 if (vm.count("samplingRate")) {
   SamplingRate = vm["samplingRate"].as<int>();
 }

 if (vm.count("doDirectionFit")) {
   DoDirectionFit = vm["doDirectionFit"].as<int>();
 }

 if (vm.count("antennaPositionsFile")) {
   AntennaPositionsFile = vm["antennaPositionsFile"].as<string>();
 }
 
 if (vm.count("antennaSelection")) {
   AntennaSelection = vm["antennaSelection"].as<string>();
 }
 
 if (vm.count("minElevation")) {
   MinElevation = vm["minElevation"].as<float>();
 }

 if (vm.count("maxBadnessOfFit")) {
   MaxFitVariance = vm["maxBadnessOfFit"].as<float>();
 }

  //######## Boost program-options section end ##############################
 
 // setup the internal structures 
 theTask.setParameters(AntennaSelection, AntennaPositionsFile, SamplingRate, 
 		       NoCoincidenceChannels, CoincidenceTime, DoDirectionFit,
 		       MinElevation, MaxFitVariance);

	theTrigger.itsMissed = 0;
	// process file
        uint32 n = 0;
        uint32 badtimes = 0;
	double ddate, lastCoinCall=0.;
        cout << "Start reading trigger file" << endl;
        while (true) {
          if (!triggerFile->getline (triggerLine, 4096)) 
          {
            if (followFile == false)
            {
              break;
            }
            usleep(1000000);
            cout << "Waiting for more input..." << endl;
          }else{
	  LOG_DEBUG_STR("input: " << triggerLine);
	  if (sscanf(triggerLine, "%d %u %u %u %u %u",
		     &theTrigger.itsRcuNr,
		     &theTrigger.itsTime,
		     &theTrigger.itsSampleNr,
		     &theTrigger.itsSum,
		     &theTrigger.itsNrSamples,
		     &theTrigger.itsPeakValue) == 7) {
	    // call the VHECR task
	    theTrigger.itsNo = n;
	    n++;
            
	    theTask.addTrigger(theTrigger);
	    // call the coincidence-check if the last call was more than 100 ms ago
            if (theTrigger.itsTime < 2.1e9)
            {
              ddate = theTrigger.itsTime + theTrigger.itsSampleNr/200.0e6;
              if ((ddate-lastCoinCall) > 0.000001) { 
                std::vector<TBBReadCmd> readCmds;
                LOG_DEBUG_STR("Calling getReadCmd() at time:" << ddate);
                theTask.getReadCmd(readCmds);
                lastCoinCall=ddate;
              };
              // cout << "Discarded trigger! " << trigger.itsTime << endl;
            } else
            {
              cout << "Bad time" << endl;
              badtimes++;
            }
	  }
	  else {	// could not read 7 argments
	    LOG_ERROR_STR("Can not interpret line: " << triggerLine);
	  }
          }
	}

//	triggerFile.close();
        cout << "Total coincidences: " << theTask.totalCoincidences << "; bad fits: " << theTask.badFits << endl;
        cout << "Single triggers: " << n << " of which bad timestamps: " << badtimes << endl;

	return (0);
}
