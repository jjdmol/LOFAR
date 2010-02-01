//#  VHECRTask.cc: Implementation of the VHECR algoritms
//#
//#  Copyright (C) 2007
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
#include <lofar_config.h>
#include <Common/LofarLogger.h>
// #include <APS/ParameterSet.h>
#include <time.h>
#include <fstream>

#include "VHECR/VHECRTask.h" // path for use in online version

namespace LOFAR {
  //	using namespace ACC::APS;
  namespace StationCU {
    
    //
    // VHECRTask()
    //
    VHECRTask::VHECRTask() :
      itsNrTriggers (0),
      itsSamplingRate(200000000), // NB. Sampling rate 200 MHz assumed without info! Needs to be changed in the future.
    itsInitialized (false)
    {
      // set default parameters for coincidence
      itsNoCoincidenceChannels = 32;
      itsCoincidenceTime = 1.0e-6;
      totalCoincidences = 0;
      badFits = 0;
      itsConfigurationFile = "/opt/lofar/etc/VHECRtask.conf"; // /opt/lofar/etc/
      readConfigFile(itsConfigurationFile.c_str());
//      itsOutputFilename = "/opt/lofar/etc/VHECRtaskLogfile.dat";
//      itsLogfile = fopen(itsOutputFilename.c_str(), "w"); // overwrites existing file...

      //cout << "Number of required coincident channels = " << itsNoCoincidenceChannels << ", coincidence time window = " << itsCoincidenceTime << endl;
      // Initialize the trigger messages buffer
      for (uint32 i=0; i<VHECR_TASK_BUFFER_LENGTH; i++){
	trigBuffer[i].next = i+1;
	trigBuffer[i].prev = i-1;
	trigBuffer[i].Time = 0;
	trigBuffer[i].SampleNr = 0;
	trigBuffer[i].date = 0.;
      };
      first = 0;
      last = VHECR_TASK_BUFFER_LENGTH-1;
      trigBuffer[first].prev = VHECR_TASK_BUFFER_LENGTH; //This means "not there"

    //  string infile = "/Users/acorstanje/usg/data/calibration/AntennaPos/CS021-AntennaArrays.conf";
    //  string antennaSelection = "LBA_INNER";
    //  readAntennaPositions(infile, antennaSelection);
      LOG_DEBUG ("VHECR construction complete");
    }
    
    
    //
    // ~VHECRTask()
    //
    VHECRTask::~VHECRTask()
    {
      LOG_DEBUG ("VHECR destruction");
    }
    
    void VHECRTask::readConfigFile(string fileName)
    {
//      cout << "Reading in config file..." << endl;
      
      std::ifstream configFile(fileName.c_str()); 
      string temp;
      
      while(configFile.eof() != true)
      {
        configFile >> temp;
        // switch / case won't work unfortunately...
        if (temp == "outputFilename:")
        {
          configFile >> itsOutputFilename;
          itsLogfile = fopen(itsOutputFilename.c_str(), "w"); // overwrites existing file...
//          cout << "Filename set to: " << itsOutputFilename << endl;
        } else if (temp == "coincidenceChannels:")
        {          
          configFile >> itsNoCoincidenceChannels;
//          cout << "No channels set to: " << itsNoCoincidenceChannels << endl;
        } else if (temp == "antennaPositionsFile:")
        {        
          configFile >> antennaPositionsFile;
          if (antennaSelection != "")
          {
//            cout << antennaSelection << " reading in positions." << endl;
            readAntennaPositions(antennaPositionsFile, antennaSelection);
          }
        } else if (temp == "antennaSelection:")
        {
          configFile >> antennaSelection;
        } else if (temp == "coincidenceTime:")
        {
          configFile >> itsCoincidenceTime;
        } else if (temp == "doDirectionFit:")
        {
          configFile >> doDirectionFit;
        } else
        {
          LOG_DEBUG("Error reading config file!");
        }
      }
      fprintf(itsLogfile, "Output file: %s\nCoincidence channels required: %d\nAntenna positions file: %s\nAntenna selection: %s\nCoincidence time window: %3.6f\ndo Direction fit: %d\n", 
              itsOutputFilename.c_str(), itsNoCoincidenceChannels, antennaPositionsFile.c_str(), antennaSelection.c_str(), itsCoincidenceTime, doDirectionFit);
    }
            
      
    //
    // readTBBdata(vector<TBBReadCmd>	cmdVector)
    //
    void VHECRTask::readTBBdata(std::vector<TBBReadCmd>	cmdVector)
    {
      // for now we only print the info that comes in.
      std::vector<TBBReadCmd>::iterator	iter = cmdVector.begin();
      std::vector<TBBReadCmd>::iterator	end  = cmdVector.end();
      while (iter != end) {
	LOG_INFO_STR(*iter);
	iter++;
      }
    }
    
    string VHECRTask::readableTime(const uint64 date)
    {
      time_t unixtime = date / itsSamplingRate;
      uint32 sample = date % itsSamplingRate;
      double secfraction = sample / (double) itsSamplingRate;
      
      tm * timeRec;
      timeRec = gmtime ( &unixtime );
      
      char timeStr[50];
      sprintf (timeStr, "%02d:%02d:%02.6f", (timeRec->tm_hour)%24, timeRec->tm_min, (double) timeRec->tm_sec + secfraction);
      string outString = timeStr;      
      return outString;
    }
    
    //
    // addTrigger(trigger)
    //
    // THIS IS WHERE THE DEVELOPMENT SHOULD TAKE PLACE.
    //

    void VHECRTask::printCoincidence(int coincidenceIndex)
    {
      cout << " --- This coincidence: --- " << endl;
      cout << "Showing time offsets w.r.t. latest timestamp" << endl;
      int runningIndex = coincidenceIndex;
      int64 refdate = trigBuffer[runningIndex].date; 
      for (int k=0; k<itsNoCoincidenceChannels; k++)
      {
        cout << "RCU " << trigBuffer[runningIndex].RcuNr << ": " << (int64)trigBuffer[runningIndex].date - refdate << endl;
        runningIndex = trigBuffer[runningIndex].next;
      }
    }
    
    void VHECRTask::addTrigger(const TBBTrigger& trigger) {
      int newindex, coincidenceIndex;
      //      cout << "Received trigger: " << trigger.itsRcuNr << ", " << trigger.itsTime <<endl;
      
      newindex = add2buffer(trigger);
      uint64 timeWindow = itsSamplingRate * itsCoincidenceTime;
      
      coincidenceIndex = coincidenceCheck(newindex, itsNoCoincidenceChannels, itsCoincidenceTime);
//      cout << "Done coincidence check for new index: " << newindex << ", for " << itsNoCoincidenceChannels << " coincindence channels, for window = " << itsCoincidenceTime << " seconds; result = " << coincidenceIndex << endl;

      static uint64 latestCoincidenceTime = 0; // we'll ensure that all coincidences we find are at least 1 time window apart.
                                               // That way we won't see every coincidence (n-8) times (n = # single triggers in one pulse)
      static uint32 coincidenceCount = 0;
      // warning: static vars are class vars, not instance vars! Should be done differently if more than one VHECRtask...
      if ( (coincidenceIndex >= 0) && (trigBuffer[coincidenceIndex].date >= latestCoincidenceTime + timeWindow) )
      {
	coincidenceCount++;
        // get PC-time to be logged
        struct timeval tv;
        gettimeofday(&tv, NULL); 
        // conversion to make it go into the readableTime function
        uint64 pcTimeInSamples = (uint64)tv.tv_sec * itsSamplingRate + (uint64)tv.tv_usec * itsSamplingRate / 1000000;        
        if (coincidenceCount % 1 == 0)
        {
//          cout << "Coincidence at: " << readableTime(trigBuffer[coincidenceIndex].date) << "; ";
//          cout.flush();
     //     cout << "Detected coincidence " << coincidenceCount << ": " << trigBuffer[coincidenceIndex].no << ", " << trigBuffer[coincidenceIndex].RcuNr << ", " 
     //     << readableTime(pcTimeInSamples) << ", " << readableTime(trigBuffer[coincidenceIndex].date) << endl; //"; " << trigBuffer[coincidenceIndex].SampleNr << endl;
        //  printCoincidence(coincidenceIndex);
//         for(uint32 k=0; k<1000; k++)
//          {
          if (doDirectionFit)
          {
            fitDirectionToCoincidence(coincidenceIndex, itsNoCoincidenceChannels);
          }
         // }
//          cout << "Done! "<<endl;
//          cout.flush();
        }
        // log to file
        fprintf(itsLogfile, "%s %d %s %d\n", readableTime(pcTimeInSamples).c_str(), 
                                             trigBuffer[coincidenceIndex].no, 
                                             readableTime(trigBuffer[coincidenceIndex].date).c_str(), 
                                             trigBuffer[coincidenceIndex].RcuNr);
        
        latestCoincidenceTime = trigBuffer[coincidenceIndex].date;
	// This adds the trigger to the command queue.
	uint32 RcuNr      = trigBuffer[coincidenceIndex].RcuNr;
	uint32 Time       = trigBuffer[coincidenceIndex].Time;
	uint32 sampleTime = trigBuffer[coincidenceIndex].SampleNr;
	uint32 prePages   = 1;
	uint32 postPages  = 2;
	itsCommandVector.push_back(TBBReadCmd(RcuNr, Time, sampleTime, prePages, postPages));	
	itsNrTriggers++;
      };
      
      // All code for this event is [TEST] code
      if (!itsCommandVector.empty()) {
	readTBBdata(itsCommandVector);			// report that we want everything
	itsCommandVector.clear();					// clear buffer
      }
    }
    
    // Check the contents of the buffer if a coincidence is found
    int VHECRTask::coincidenceCheck(uint32 latestindex, uint32 nChannles, double timeWindow){
      uint32 i,foundRCUs[nChannles],nfound;
      uint32 startindex,runindex;
      
      uint64 refdate;
      uint64 timeWindow64 = itsSamplingRate * timeWindow;
      
      startindex = first;
      while ((startindex!=trigBuffer[latestindex].next) && (startindex < VHECR_TASK_BUFFER_LENGTH)) {
	runindex = trigBuffer[startindex].next;
	refdate=trigBuffer[startindex].date-timeWindow64;
	nfound=0;
	while ((runindex < VHECR_TASK_BUFFER_LENGTH) && (trigBuffer[runindex].date >= refdate)){
	  for (i=0; i<nfound; i++){
	    if (foundRCUs[i] == trigBuffer[runindex].RcuNr) { 
	      break; //break the for-loop;
	    };
	  };
	  if (i == nfound) { 
	    if (nfound+2 >= nChannles) { return startindex; };
	    foundRCUs[nfound] = trigBuffer[runindex].RcuNr;
	    nfound++;
	  };
	  runindex = trigBuffer[runindex].next;
	};
	startindex = trigBuffer[startindex].next;
      };
      return -1;
    };

    // Add a trigger message to the buffer. 
    uint32 VHECRTask::add2buffer(const TBBTrigger& trigger){
      uint64 date;
      uint32 newindex,runindex;

      date = (uint64) trigger.itsTime * itsSamplingRate + trigger.itsSampleNr;
      
      newindex = last;
      last = trigBuffer[last].prev;
      trigBuffer[last].next = VHECR_TASK_BUFFER_LENGTH;
      
      trigBuffer[newindex].no        = trigger.itsNo;
      trigBuffer[newindex].RcuNr     = trigger.itsRcuNr;
      trigBuffer[newindex].SeqNr     = trigger.itsSeqNr;
      trigBuffer[newindex].Time      = trigger.itsTime;
      trigBuffer[newindex].SampleNr  = trigger.itsSampleNr;
      trigBuffer[newindex].Sum       = trigger.itsSum;
      trigBuffer[newindex].NrSamples = trigger.itsNrSamples;
      trigBuffer[newindex].PeakValue = trigger.itsPeakValue;
      trigBuffer[newindex].date      = date;
      
      runindex = first;
      while (runindex < VHECR_TASK_BUFFER_LENGTH){
	if (trigBuffer[runindex].date <= date) { 
	  break; 
	};
	runindex = trigBuffer[runindex].next;
      };
      trigBuffer[newindex].next = runindex;
      if (runindex == first){
	first = newindex;
	trigBuffer[newindex].prev = VHECR_TASK_BUFFER_LENGTH;
	trigBuffer[runindex].prev = newindex;
      } else {
	if (runindex >= VHECR_TASK_BUFFER_LENGTH){
	  trigBuffer[last].next = newindex;
	  trigBuffer[newindex].prev = last;
	  last = newindex;
	} else {
	  trigBuffer[(trigBuffer[runindex].prev)].next = newindex;
	  trigBuffer[newindex].prev = trigBuffer[runindex].prev;
	  trigBuffer[runindex].prev = newindex;
	};
      };
      return newindex;
    };
   
    void VHECRTask::fitDirectionToCoincidence(int coincidenceIndex, int nofChannels)
    { // number of channels known from requirement
      double theta, phi;
      double twopi = 2.0 * 3.1415926536;
      double c = 2.9979e8;
      
      const uint32 thetaSteps = 30;
      const uint32 phiSteps = 120; // move to somewhere else! Parameters...
      
      double timeDelays[96]; // get rid of that constant
      
      double fitResult[thetaSteps][phiSteps];

      double minTh = 1.0e9;
      double minPh = 1.0e9;
      double minSig2 = 1.0e9;
      double debugTimeOffsets[96], minDebugTimeOffsets[96];
      int64 refdate = trigBuffer[coincidenceIndex].date; // coincidence reference timestamp to subtract from all other timestamps.

      position a;
      for (uint32 i=0; i<thetaSteps; i++)
      {
        for (uint32 j=0; j<phiSteps; j++)
        {
          theta = twopi / 4 - twopi/4 * (double)i / thetaSteps;
          phi = twopi * (double)j / phiSteps;
          
         
          a.x = sin(theta) * cos(phi);
          a.y = sin(theta) * sin(phi);
          a.z = cos(theta);
          // do inner product with antenna pos vector
          for (uint32 rcu=0; rcu<96; rcu++)
          {
            double prod = a.x * antennaPositions[rcu].x + a.y * antennaPositions[rcu].y + a.z * antennaPositions[rcu].z;
            timeDelays[rcu] = prod * (double)itsSamplingRate / c; // in samples
          }
          // calculate fit result, which is standard deviation of timing residues
          double mu=0.0;
          double sig2 = 0.0;
          uint32 runningIndex = coincidenceIndex; // trigBuffer[coincidenceIndex].next;
            runningIndex = coincidenceIndex;
          //runningIndex = trigBuffer[coincidenceIndex].next;
          //runningIndex = trigBuffer[runningIndex].next;
          mu = 0; //average = 0; sig2 = 0; sigma = 0;
          sig2=0;
          
          for (uint32 k=0; k<nofChannels; k++)
          { // loop through all RCUs that are there in this coincidence
//            if (runningIndex != outlierIndex)
//            {
              uint32 rcu = trigBuffer[runningIndex].RcuNr;
              int64 thisRelativeTime = (int64)trigBuffer[runningIndex].date - refdate; // in samples
              
              double thisTimeOffset = (double)thisRelativeTime - timeDelays[rcu];
              debugTimeOffsets[k] = thisTimeOffset;

              mu += thisTimeOffset; // we'll subtract this later as the overall offset.
              sig2 += thisTimeOffset * thisTimeOffset;
              // proceed to the next RCU
//            }
            runningIndex = trigBuffer[runningIndex].next; // next is previous in terms of timestamp           
          }
//          {
            fitResult[i][j] = (sig2 - mu*mu / (nofChannels)) / (nofChannels); // sum (x_i - mu)^2 = sum (x_i^2) - N mu^2
   //       }
          if (fitResult[i][j] < minSig2)
          {
            minSig2 = fitResult[i][j];
            minTh = twopi/4 - theta;
            minPh = phi;
            for(uint32 k=0; k<nofChannels; k++)
            {
              minDebugTimeOffsets[k] = debugTimeOffsets[k] - mu/nofChannels;
            }
          }
          
         // cout << "theta = " << 360.0/twopi * (twopi/4 - theta) << ", phi = " << (double)j / phiSteps * 360 << ": fitResult = " << fitResult[i][j] << endl;
          
        }
      }
      totalCoincidences++;
//     cout << "Fit result: theta = " << minTh * (360.0 / twopi) << "; phi = " << minPh * (360.0/twopi) << "; variance = " << minSig2 << endl;
      if (minSig2 < 50.0) 
      {
//        fprintf(itsLogfile, "%lld %f %f %f\n", refdate, minTh * (360.0 / twopi), minPh * (360.0 / twopi), minSig2);
 //       for (uint32 k=0; k<nofChannels; k++)
//        {
//          cout << k << ": " << minDebugTimeOffsets[k] << endl;
//        }
      } else
      {
        badFits++;
//        cout << "Bad fit!" << endl;
//        for (uint32 k=0; k<nofChannels; k++)
//        {
//          cout << k << ": " << minDebugTimeOffsets[k] << endl;
//        }
      }
    }
    
    void VHECRTask::readAntennaPositions(string fileName, string antennaSelection)
    {
//      cout << "Reading in antenna positions..." << endl;
          
      std::ifstream antennaFile(fileName.c_str()); 
      string temp;
      int nrAntennas, nrPolarizations, nrDirections;
      //casa::Vector<MVPosition> all_positions;
      //casa::Vector<MVPosition> selected_positions;
      do{antennaFile >> temp;} while(temp != antennaSelection);
      antennaFile >> temp; antennaFile >> temp; antennaFile >> temp; antennaFile >> temp; antennaFile >> temp; 
      antennaFile >> temp; antennaFile >> nrAntennas; cout << " nr. antennas: " << nrAntennas << endl;
      antennaFile >> temp; antennaFile >> nrPolarizations; cout << " nr. polarizations: " << nrPolarizations << endl;
      antennaFile >> temp; antennaFile >> nrDirections; cout << " nr. directions: " << nrDirections << endl;
      antennaFile >> temp;

//      all_positions.resize(nrantennas);
      int nrRCUs = nrAntennas * nrPolarizations;
//      cout << "nr. RCUs: " << nrRCUs << endl;
      
     // position * antennaPositions = new position[96];
      
      double posx;
      double posy;
      double posz;
      for(int rcu=0; rcu < nrRCUs; rcu++)
      {
        antennaFile >> posx; antennaFile >> posy; antennaFile >> posz;
       // antennaFile >> temp; antennaFile >> temp; antennaFile >> temp;
        antennaPositions[rcu].x = posx; antennaPositions[rcu].y = posy; antennaPositions[rcu].z = posz;
        //        all_positions(ant)=MVPosition(posx,posy,posz);
      }
      
      for (int rcu=0; rcu < nrRCUs; rcu++)
      {
        cout << "RCU " << rcu << ": " << antennaPositions[rcu].x << "  " << antennaPositions[rcu].y << "  " << antennaPositions[rcu].z << endl;
      }      
//      return antennaPositions;
    }
  }; // StationCU
}; // LOFAR

//       runningIndex = trigBuffer[runningIndex].next;

//          uint32 outlierIndex = 1e9;
//          // find a possible outlier in the measured arrival timestamps
//          double average = 0;
//          double sigma = 0;
//          for(uint32 k=0; k<nofChannels; k++)
//          {
//            int64 thisRelativeTime = (int64)trigBuffer[runningIndex].date - refdate; // in samples
//            average += thisRelativeTime;
//            sig2 += thisRelativeTime * thisRelativeTime;
//            runningIndex = trigBuffer[runningIndex].next;
//          }
//          sig2 -= average*average / nofChannels;
//          sigma = sqrt(sig2 / nofChannels);
//          average /= nofChannels;
//          if ((i==0) && (j==0)) {
//            cout << "Average = " << average << "; sigma = " << sigma << endl;
//          }
//          runningIndex = coincidenceIndex;
//          for(uint32 k=0; k < nofChannels; k++)
//          {
//            int64 thisRelativeTime = (int64)trigBuffer[runningIndex].date - refdate; // in samples
//            double deviation = thisRelativeTime - average;
//            if (deviation > 3.0 * sigma)
//            {
//              if ((i ==0) && (j ==0)) 
//              {
//                cout << "Outlier found: deviation = " << deviation << " while sigma = " << sigma << endl;
//              }
//              outlierIndex = runningIndex;
//              break;
//            }
//            runningIndex = trigBuffer[runningIndex].next;
//          }

//       if (outlierIndex > 1000)
//          {
//            fitResult[i][j] = (sig2 - mu*mu / nofChannels) / (nofChannels); // sum (x_i - mu)^2 = sum (x_i^2) - N mu^2
//          } else 
