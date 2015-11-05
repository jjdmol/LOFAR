//#  VHECRTask.cc: Implementation of the VHECR algoritms
//#
//#  Copyright (C) 2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
#include <Common/LofarLocators.h>
#include <ApplCommon/LofarDirs.h>
// #include <APS/ParameterSet.h>
#include <time.h>
#include <fstream>

#include "VHECR/VHECRTask.h" // path for use in online version

#define TWOPI  (2.0 * 3.1415926536)

namespace LOFAR {
    //  using namespace ACC::APS;
    namespace VHECR {

//
// VHECRTask()
//
VHECRTask::VHECRTask() :
itsInitialized (false)
{
    LOG_INFO_STR("Starting VHECRTask()");
    // set default parameters for coincidence
    itsNrTriggers =0;
    itsForcedDeadTime = 10.0;
    totalCoincidences = 0;
    badFits = 0;

    itsParameterSet = NULL;
    itsSettings = new VHECRsettings(); // empty Settings-object

    setup();
    LOG_INFO_STR("VHECR settings" << *itsSettings);
    LOG_DEBUG ("VHECR constructed with default/dummy values");
}

VHECRTask::VHECRTask(const string& cntlrName) :
itsInitialized (false)
{
    LOG_INFO_STR("Starting VHECRTask(const string& cntlrName)");
    // First readin our observation related config file.
    LOG_DEBUG_STR("Reading parset file:" << LOFAR_SHARE_LOCATION << "/" << cntlrName);
    globalParameterSet()->adoptFile(string(LOFAR_SHARE_LOCATION)+"/"+cntlrName);
    itsSettings = new VHECRsettings(globalParameterSet());    // does all nasty conversions
    
    // set default parameters for coincidence
    itsNrTriggers = 0;
    itsForcedDeadTime = 10.0;
    totalCoincidences = 0;
    badFits = 0;

    setup();
    LOG_INFO_STR("VHECR settings: " << *itsSettings);
    LOG_DEBUG ("VHECR construction complete");
}


//
// ~VHECRTask()
//
VHECRTask::~VHECRTask()
{
    LOG_DEBUG ("VHECR destruction");
}

bool VHECRTask::setup()
{
    // Set internal values from VHECRsettings object
    itsSamplingRate = itsSettings->clockFreq * 1000000; 
    
    // Initialize the trigger messages buffer
    for (uint32 i=0; i<VHECR_TASK_BUFFER_LENGTH; i++) {
        trigBuffer[i].next = i+1;
        trigBuffer[i].prev = i-1;
        trigBuffer[i].Time = 0;
        trigBuffer[i].SampleNr = 0;
        trigBuffer[i].date = static_cast<uint64>(0);
    }
    first = 0;
    last = VHECR_TASK_BUFFER_LENGTH-1;
    trigBuffer[first].prev = VHECR_TASK_BUFFER_LENGTH; //This means "not there"

    LOG_DEBUG ("VHECR construction complete");
    return true;
}


void VHECRTask::setParameters(string AntennaSet, int Clock,
      int NoCoincChann, float CoincidenceTime, int DoDirectionFit,
      float MinElevation, float MaxFitVariance, string ParamExtension,
      float forcedDeadTime)
{
    itsSettings->noCoincChann = NoCoincChann;
    itsSettings->coincidenceTime = CoincidenceTime;
    itsSettings->doDirectionFit = DoDirectionFit;
    itsSettings->minElevation = MinElevation;
    itsSettings->maxFitVariance = MaxFitVariance;
    itsSettings->clockFreq = Clock;
    itsParamExtension = ParamExtension;
    itsSettings->antennaSet = AntennaSet;
    itsForcedDeadTime = forcedDeadTime;

    setup();
    
    LOG_INFO_STR("VHECR settings" << *itsSettings);
}


//
// readTBBdata(vector<TBBReadCmd>  cmdVector)
//
void VHECRTask::readTBBdata(std::vector<TBBReadCmd>  cmdVector)
{
    // for now we only print the info that comes in.
    std::vector<TBBReadCmd>::iterator   iter = cmdVector.begin();
    std::vector<TBBReadCmd>::iterator   end  = cmdVector.end();
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
    snprintf (timeStr, sizeof timeStr, "%02d:%02d:%02.6f", (timeRec->tm_hour)%24, timeRec->tm_min, (double) timeRec->tm_sec + secfraction);
    string outString = timeStr;
    return (outString);
}

//
// addTrigger(trigger)
//
// THIS IS WHERE THE DEVELOPMENT SHOULD TAKE PLACE.
//

void VHECRTask::printCoincidence(int coincidenceIndex)
{
    LOG_INFO_STR(" --- This coincidence: --- ");
    LOG_INFO_STR("Showing time offsets w.r.t. latest timestamp");
    int runningIndex = coincidenceIndex;
    int64 refdate = trigBuffer[runningIndex].date;
    for (int k=0; k<itsSettings->noCoincChann; k++) {
        LOG_INFO_STR("RCU " << trigBuffer[runningIndex].RcuNr << ": " << (int64)trigBuffer[runningIndex].date - refdate << endl);
        runningIndex = trigBuffer[runningIndex].next;
    }
}

// ----------------------------------------------------------------------------
// addTrigger():
//  - called whenever a trigger message arrives
//  - adds the trigger message to the buffer
//  - first check if its timestamp is valid!
void VHECRTask::addTrigger(const TBBTrigger& trigger) 
{
    //      cout << "Received trigger: " << trigger.itsRcuNr << ", " << trigger.itsTime <<endl;
    if (trigger.itsTime < 2.1e9) {
        add2buffer(trigger);
    } 
    else {
        // cout << "Discarded trigger! " << trigger.itsTime << endl;
    }
}


// ----------------------------------------------------------------------------
// getReadCmd()
//  - called at regular intervals (e.g. every 100ms)
//  - the parameter is a vector in which we can return the read-commands to dump data
//  - the return value is the number of rcus to be dumped (e.g. 0 if not dump is needed)
//  - this is where most of the "magic" happends
// ***warning:*** handles only one coincidence per call and unthinkingly dumps all 96 RCUs
int VHECRTask::getReadCmd(std::vector<TBBReadCmd> &readCmd)
{
    int noOfRCUs=0;

    int coincidenceIndex;
    uint64 timeWindow = static_cast<uint64>(itsSamplingRate * itsSettings->coincidenceTime);

    coincidenceIndex = coincidenceCheck(last, itsSettings->noCoincChann, itsSettings->coincidenceTime);
//      cout << "Done coincidence check for new index: " << newindex << ", for " << itsSettings->noCoincChann << " coincindence channels, for window = " << itsSettings->coincidenceTime << " seconds; result = " << coincidenceIndex << endl;

    static uint64 latestCoincidenceTime = 0; // we'll ensure that all coincidences we find are at least 1 time window apart.
                                // That way we won't see every coincidence (n-8) times (n = # single triggers in one pulse)
    static uint64 latestDumpCommand = 0;
    static uint32 coincidenceCount = 0;
    // ***warning:*** static vars are class vars, not instance vars! Should be done differently if more than one VHECRtask...
    if ( (coincidenceIndex >= 0) && (trigBuffer[coincidenceIndex].date >= latestCoincidenceTime + timeWindow) ) {
        coincidenceCount++;
        // get PC-time to be logged
        struct timeval tv;
        gettimeofday(&tv, NULL);
        // conversion to make it go into the readableTime function
        uint64 pcTimeInSamples = (uint64)tv.tv_sec * itsSamplingRate + (uint64)tv.tv_usec * itsSamplingRate / 1000000;
        fitResultStruct directionFitResult;
        if (coincidenceCount % 1 == 0) {
        //   cout << "Coincidence at: " << readableTime(trigBuffer[coincidenceIndex].date) << "; ";
        //   cout.flush();
        //   cout << "Detected coincidence " << coincidenceCount << ": " << trigBuffer[coincidenceIndex].no << ", " << trigBuffer[coincidenceIndex].RcuNr << ", "
        //        << readableTime(pcTimeInSamples) << ", " << readableTime(trigBuffer[coincidenceIndex].date) << endl; //"; " <<                                     trigBuffer[coincidenceIndex].SampleNr << endl;
        //   printCoincidence(coincidenceIndex);
        //   for(uint32 k=0; k<1000; k++) {
            
        if (itsSettings->doDirectionFit == 1) {
            directionFitResult = fitDirectionToCoincidence(coincidenceIndex, itsSettings->noCoincChann);
        }
        if (itsSettings->doDirectionFit >= 2) {
            fitDirectionAndDistanceToCoincidence(coincidenceIndex, itsSettings->noCoincChann);
        }
        // }
        //          cout << "Done! "<<endl;
        //          cout.flush();
        }
        // log to file
        LOG_DEBUG_STR(formatString("TimingLog: %s %d %s %d\n", 
                            readableTime(pcTimeInSamples).c_str(),
                            trigBuffer[coincidenceIndex].no,
                            readableTime(trigBuffer[coincidenceIndex].date).c_str(),
                            trigBuffer[coincidenceIndex].RcuNr));

        latestCoincidenceTime = trigBuffer[coincidenceIndex].date;
        bool dumpData = false;
        if (latestCoincidenceTime - latestDumpCommand > uint64(itsForcedDeadTime * 200.0e6)) {
            // then do the actual dump command. Ensure at least 'forcedDeadTime' seconds apart.
            if (itsSettings->doDirectionFit > 0) {
                if (directionFitResult.mse > itsSettings->maxFitVariance) {
                    // be verbose about dump decisions for now... testing.
                    LOG_DEBUG_STR(formatString("Not dumping data, variance too high: %f\n", directionFitResult.mse));
                } 
                else if ( (directionFitResult.theta* (360.0 / TWOPI)) < itsSettings->minElevation) {
                    LOG_DEBUG_STR(formatString("Not dumping data, elevation too low: %f\n", directionFitResult.theta*(360./TWOPI)));
                } 
                else {
                    LOG_INFO_STR(formatString("directionFitResult good: theta= %f mse= %f\n",
                    directionFitResult.theta* (360.0 / TWOPI), directionFitResult.mse));
                    dumpData = true;
                }
            } 
            else {
                dumpData = true;
            }
        }
        if (dumpData) {
            latestDumpCommand = latestCoincidenceTime;

            // This adds the trigger to the command queue.
            //uint32 RcuNr      = trigBuffer[coincidenceIndex].RcuNr;
            uint32 RcuNr;
            uint32 Time       = trigBuffer[coincidenceIndex].Time;
            uint32 sampleTime = trigBuffer[coincidenceIndex].SampleNr;
            uint32 prePages   = 64;
            uint32 postPages  = 64;
            //itsCommandVector.push_back(TBBReadCmd(RcuNr, Time, sampleTime, prePages, postPages));
            //Add all rcus to the command vector.
            for (RcuNr =0 ; RcuNr<96 ; RcuNr++) {
                noOfRCUs++;
                readCmd.push_back(TBBReadCmd(RcuNr, Time, sampleTime, prePages, postPages));
            }
            itsNrTriggers++;
            LOG_INFO_STR("Dump data");
        }
    } // end: if ( (coincidenceIndex >= 0) ...

    // All code for this event is [TEST] code
//       if (!itsCommandVector.empty()) {
//       readTBBdata(itsCommandVector);         // report that we want everything
//       itsCommandVector.clear();              // clear buffer
//       }
    return noOfRCUs;
}

// Check the contents of the buffer if a coincidence is found
int VHECRTask::coincidenceCheck(uint32 latestindex, uint32 nChannles, double timeWindow)
{
    uint32 i,foundRCUs[nChannles],nfound;
    uint32 startindex,runindex;

    uint64 refdate;
    uint64 timeWindow64 = static_cast<uint64>(itsSamplingRate * timeWindow);

    startindex = first;
    while ((startindex!=trigBuffer[latestindex].next) && (startindex < VHECR_TASK_BUFFER_LENGTH)) {
        runindex = trigBuffer[startindex].next;
        refdate=trigBuffer[startindex].date-timeWindow64;
        nfound=0;
        while ((runindex < VHECR_TASK_BUFFER_LENGTH) && (trigBuffer[runindex].date >= refdate)) {
            for (i=0; i<nfound; i++){
                if (foundRCUs[i] == trigBuffer[runindex].RcuNr) {
                    break; //break the for-loop;
                }
            }
            if (i == nfound) {
                if (nfound+2 >= nChannles) { 
                    return (startindex);
                }
                foundRCUs[nfound] = trigBuffer[runindex].RcuNr;
                nfound++;
            }
            runindex = trigBuffer[runindex].next;
        }
        startindex = trigBuffer[startindex].next;
    }
    return -1;
}

// Add a trigger message to the buffer.
uint32 VHECRTask::add2buffer(const TBBTrigger& trigger)
{
    uint64 date;
    uint32 newindex,runindex;

    newindex = last;
    last = trigBuffer[last].prev;
    trigBuffer[last].next = VHECR_TASK_BUFFER_LENGTH;

    trigBuffer[newindex].no        = trigger.itsNo;
    trigBuffer[newindex].RcuNr     = trigger.itsRcuNr;
	 // trigBuffer[newindex].SeqNr     = trigger.itsSeqNr; // NOT USED ANYMORE (PD)
    trigBuffer[newindex].Time      = trigger.itsTime;
    trigBuffer[newindex].SampleNr  = trigger.itsSampleNr;
    trigBuffer[newindex].Sum       = trigger.itsSum;
    trigBuffer[newindex].NrSamples = trigger.itsNrSamples;
    trigBuffer[newindex].PeakValue = trigger.itsPeakValue;
    if (itsSamplingRate == 200000000) {
        if ((trigBuffer[newindex].Time)!=1) {
            trigBuffer[newindex].SampleNr += 512;
        }
    }
    date = (uint64) trigBuffer[newindex].Time * itsSamplingRate + trigBuffer[newindex].SampleNr;
    trigBuffer[newindex].date      = date;

    runindex = first;
    while (runindex < VHECR_TASK_BUFFER_LENGTH) {
        if (trigBuffer[runindex].date <= date) {
            break;
        }
        runindex = trigBuffer[runindex].next;
    }
    trigBuffer[newindex].next = runindex;
    if (runindex == first) {
        first = newindex;
        trigBuffer[newindex].prev = VHECR_TASK_BUFFER_LENGTH;
        trigBuffer[runindex].prev = newindex;
    } 
    else {
        if (runindex >= VHECR_TASK_BUFFER_LENGTH) {
            trigBuffer[last].next = newindex;
            trigBuffer[newindex].prev = last;
            last = newindex;
        } 
        else {
            trigBuffer[(trigBuffer[runindex].prev)].next = newindex;
            trigBuffer[newindex].prev = trigBuffer[runindex].prev;
            trigBuffer[runindex].prev = newindex;
        }
    }
    return (newindex);
}

VHECRTask::fitResultStruct VHECRTask::fitDirectionToCoincidence(int coincidenceIndex, int nofChannels)
{
    double theta, phi;
    double c = 2.9979e8;

    const int thetaSteps = 30;
    const int phiSteps = 120; // move to somewhere else! Parameters...

    double timeDelays[NOFANTENNAS];

    double fitResult[thetaSteps][phiSteps];
    double minTh = 1.0e9;
    double minPh = 1.0e9;
    double minSig2 = 1.0e9;
    //double debugTimeOffsets[NOFANTENNAS], minDebugTimeOffsets[NOFANTENNAS];
    int64 refdate = trigBuffer[coincidenceIndex].date; // coincidence reference timestamp to subtract from all other timestamps.

    positionStruct a;
    for (int i=0; i<thetaSteps; i++) {
        for (int j=0; j<phiSteps; j++) {
            theta = TWOPI / 4 - TWOPI / 4 * (double)i / thetaSteps;
            phi = TWOPI * (double)j / phiSteps;

            a.x = - sin(theta) * cos(phi); // + sign when relating to a point in the sky! - sign when doing spherical vector
            a.y = - sin(theta) * sin(phi); // FIXED by removing unwanted 'fix' for 90 degree angle.
            a.z = - cos(theta);

            // do inner product with antenna pos vector
            for (int rcu=0; rcu<NOFANTENNAS; rcu++) {
                // double prod = a.x * antennaPositions[rcu].x + a.y * antennaPositions[rcu].y + a.z * antennaPositions[rcu].z;
                double prod = a.x * itsSettings->rcuPosITRF(rcu, 0) + a.y * itsSettings->rcuPosITRF(rcu, 1) + a.z * itsSettings->rcuPosITRF(rcu, 2);
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

            for (int k=0; k<nofChannels; k++) { // loop through all RCUs that are there in this coincidence
                //            if (runningIndex != outlierIndex)
                //            {
                uint32 rcu = trigBuffer[runningIndex].RcuNr;
                int64 thisRelativeTime = (int64)trigBuffer[runningIndex].date - refdate; // in samples
                
                double thisTimeOffset = (double)thisRelativeTime - timeDelays[rcu];
                //debugTimeOffsets[k] = thisTimeOffset;
                
                mu += thisTimeOffset; // we'll subtract this later as the overall offset.
                sig2 += thisTimeOffset * thisTimeOffset;
                // proceed to the next RCU
                //            }
                runningIndex = trigBuffer[runningIndex].next; // next is previous in terms of timestamp
            }
            fitResult[i][j] = (sig2 - mu*mu / (nofChannels)) / (nofChannels); // sum (x_i - mu)^2 = sum (x_i^2) - N mu^2
            if (fitResult[i][j] < minSig2) {
                minSig2 = fitResult[i][j];
                minTh = TWOPI/4 - theta;
                minPh = phi;
    //              for(uint32 k=0; k<nofChannels; k++)
    //              {
    //                minDebugTimeOffsets[k] = debugTimeOffsets[k] - mu/nofChannels;
    //              }
            }
        }
    }
    totalCoincidences++;
//      cout << "Fit result: theta = " << minTh * (360.0 / TWOPI) << "; phi = " << minPh * (360.0/TWOPI) << "; variance = " << minSig2 << endl;
    if (minSig2 < 50.0) {
        LOG_INFO_STR(formatString("FitResult: %lld %f %f %f\n", refdate, minTh * (360.0 / TWOPI), minPh * (360.0 / TWOPI), minSig2));
        // debug
        //       uint32 runningIndex = coincidenceIndex;
        //        for (uint32 k=0; k<nofChannels; k++)
        //        { // the RCUs in each channel are still the same, so can cycle through the buffer again to get them
        //          uint32 thisRCU = trigBuffer[runningIndex].RcuNr;
        //          cout << k << ": RCU = " << thisRCU << ", time offset = " << minDebugTimeOffsets[k] << endl;
        //          runningIndex = trigBuffer[runningIndex].next;
        //        } // end debug
    } 
    else {
        badFits++;
        LOG_DEBUG_STR(formatString("FitResult: %lld %f %f %f BadFit!\n", refdate, minTh * (360.0 / TWOPI), minPh * (360.0 / TWOPI), minSig2));
    
        // cout << "Bad fit!" << endl;
        // debug
    
        //        uint32 runningIndex = coincidenceIndex;
        //        for (uint32 k=0; k<nofChannels; k++)
        //        {
        //          uint32 thisRCU = trigBuffer[runningIndex].RcuNr;
        //          cout << k << ": RCU = " << thisRCU << ", time offset = " << minDebugTimeOffsets[k] << endl;
        //          runningIndex = trigBuffer[runningIndex].next;
        //        } // end debug
    }
    fitResultStruct theResult;
    theResult.theta = minTh;
    theResult.phi = minPh;
    theResult.mse = minSig2;
    return (theResult);
}

void VHECRTask::fitDirectionAndDistanceToCoincidence(int coincidenceIndex, int nofChannels)
{ // number of channels known from requirement
    //     cout << "Do smart stuff... (well, we hope)" << endl;
    double theta, phi, R;
    double c = 2.9979e8;

    const int thetaSteps = 30;
    const int phiSteps = 120; // move to somewhere else! Parameters...
    const int Rsteps = 40;

    double timeDelays[NOFANTENNAS]; // get rid of that constant

    double fitResult[thetaSteps][phiSteps];
    R = 5.0;
    double minR = 1.0e12;
    double overallMinSig2 = 1.0e12;
    for (int stepR=0; stepR < Rsteps; stepR++) {
        R *= 1.2;
    
        double minTh = 1.0e9;
        double minPh = 1.0e9;
        double minSig2 = 1.0e9;
        double debugTimeOffsets[NOFANTENNAS], minDebugTimeOffsets[NOFANTENNAS];
        int64 refdate = trigBuffer[coincidenceIndex].date; // coincidence reference timestamp to subtract from all other timestamps.
    
        positionStruct a;
        for (int i=0; i<thetaSteps; i++) {
            for (int j=0; j<phiSteps; j++) {
                theta = TWOPI / 4 - TWOPI/4 * (double)i / thetaSteps;
                phi = TWOPI * (double)j / phiSteps;
        
                a.x = R * sin(theta) * cos(phi); // + sign when relating to a point in the sky!
                a.y = R * sin(theta) * sin(phi); // minus 90 degrees to relate to antenna coord system, phi=0: east, phi=90: north...
                a.z = R * cos(theta);
        
                for (int rcu=0; rcu < NOFANTENNAS; rcu++) {
                    // double distX = a.x - antennaPositions[rcu].x;
                    double distX = a.x - itsSettings->rcuPosITRF(rcu, 0);
                    // double distY = a.y - antennaPositions[rcu].y;
                    double distY = a.y - itsSettings->rcuPosITRF(rcu, 1);
                    // double distZ = a.z - antennaPositions[rcu].z;
                    double distZ = a.z - itsSettings->rcuPosITRF(rcu, 2);
                    double dist = sqrt(distX * distX + distY * distY + distZ * distZ);
                    timeDelays[rcu] = (double)itsSamplingRate * (dist - R) / c;
                }
        
                //            // do inner product with antenna pos vector
                //            for (uint32 rcu=0; rcu<NOFANTENNAS; rcu++)
                //            {
                //              double prod = a.x * antennaPositions[rcu].x + a.y * antennaPositions[rcu].y + a.z * antennaPositions[rcu].z;
                //              timeDelays[rcu] = prod * (double)itsSamplingRate / c; // in samples
                //            }
                // calculate fit result, which is standard deviation of timing residues
                double mu=0.0;
                double sig2 = 0.0;
                uint32 runningIndex = coincidenceIndex; // trigBuffer[coincidenceIndex].next;
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
                runningIndex = coincidenceIndex;
                //runningIndex = trigBuffer[coincidenceIndex].next;
                //runningIndex = trigBuffer[runningIndex].next;
                mu = 0; //average = 0; sig2 = 0; sigma = 0;
                sig2=0;
        
                for (int k=0; k<nofChannels; k++) { // loop through all RCUs that are there in this coincidence
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
                //       if (outlierIndex > 1000)
                //          {
                //            fitResult[i][j] = (sig2 - mu*mu / nofChannels) / (nofChannels); // sum (x_i - mu)^2 = sum (x_i^2) - N mu^2
                //          } else
                //          {
                fitResult[i][j] = (sig2 - mu*mu / (nofChannels)) / (nofChannels); // sum (x_i - mu)^2 = sum (x_i^2) - N mu^2
                //       }
                if (fitResult[i][j] < minSig2) {
                    minSig2 = fitResult[i][j];
                    minTh = TWOPI/4 - theta;
                    minPh = phi;
                    if (minSig2 < overallMinSig2) {
                        overallMinSig2 = minSig2;
                        minR = R;
                    }
                    for (int k=0; k<nofChannels; k++) {
                        minDebugTimeOffsets[k] = debugTimeOffsets[k] - mu/nofChannels;
                    }
                }
                (void)minDebugTimeOffsets;
                // cout << "theta = " << 360.0/TWOPI * (TWOPI/4 - theta) << ", phi = " << (double)j / phiSteps * 360 << ": fitResult = " << fitResult[i][j] << endl;
            }
        }
        //cout << "Fit result: theta = " << minTh * (360.0 / TWOPI) << "; phi = " << minPh * (360.0/TWOPI) << "; R = " << R << "; height = " << R * sin(minTh) << "; variance = " << minSig2 << endl;
    
        //      } // for stepR
        totalCoincidences++;
        //     cout << "Best fit result: theta = " << minTh * (360.0 / TWOPI) << "; phi = " << minPh * (360.0/TWOPI) << "; R = " << minR << "; height = " << R * sin(minTh) << "; variance = " << minSig2 << endl;
        if ((minSig2 < 50.0) && (stepR == Rsteps - 1)) { // hack to ensure we only call this at the end of the loop...
            LOG_DEBUG_STR(formatString("FitResult: %lld theta=%f phi=%f variance=%f minR=%F\n", refdate, minTh * (360.0 / TWOPI), minPh * (360.0 / TWOPI), minSig2, minR));
        //       for (uint32 k=0; k<nofChannels; k++)
        //        {
        //          cout << k << ": " << minDebugTimeOffsets[k] << endl;
        //        }
        } 
        else {
            badFits++;
        //   cout << "Bad fit!" << endl;
        //        for (uint32 k=0; k<nofChannels; k++)
        //        {
        //          cout << k << ": " << minDebugTimeOffsets[k] << endl;
        //        }
        }
    } // {for stepR}
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
