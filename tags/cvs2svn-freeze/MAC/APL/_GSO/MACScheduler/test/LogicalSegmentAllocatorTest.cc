//#  LogicalSegmentAllocatorTest.cc: Main entry for the LogicalSegmentAllocator test
//#
//#  Copyright (C) 2002-2004
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
#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <Common/lofar_sstream.h>

#include <sys/time.h>
#include <APL/APLCommon/APLUtilities.h>
#include "../src/LogicalSegmentAllocator.h"

using namespace LOFAR;
using namespace LOFAR::GSO;
using namespace LOFAR::APLCommon;

string formatTime(time_t t)
{
  tm* pTm = localtime(&t);
  char timeStr[100];
  sprintf(timeStr,"%02d-%02d-%04d %02d:%02d:%02d",pTm->tm_mday,pTm->tm_mon+1,pTm->tm_year+1900,pTm->tm_hour,pTm->tm_min,pTm->tm_sec);
  return string(timeStr);
}

void printResumeSuspendVRs(
  set<string> resumeVRs,
  set<string> suspendVRs)
{
  set<string>::iterator itR;
  set<string>::iterator itS;
  stringstream logStream;
  logStream << "ResumeVRs: ";
  for(itR=resumeVRs.begin();itR!=resumeVRs.end();++itR)
  {
    logStream << itR->c_str() << ", ";
  }
  logStream << endl;
  LOG_INFO(formatString("%s",logStream.str().c_str()));
  
  logStream.str(string(""));
  logStream << "suspendVRs: ";
  for(itS=suspendVRs.begin();itS!=suspendVRs.end();++itS)
  {
    logStream << itS->c_str() << ", ";
  }
  logStream << endl;
  LOG_INFO(formatString("%s",logStream.str().c_str()));
}

int main(int /*argc*/, char** /*argv*/)
{
  int retval=0;
  
  INIT_LOGGER("./LogicalSegmentAllocatorTest.log_prop");   

  vector<string> logicalSegments1;
  logicalSegments1.push_back(string("BackBone"));
  
  vector<string> logicalSegments2;
  logicalSegments2.push_back(string("BackBone"));
  logicalSegments2.push_back(string("FieldCN_RS1"));
  
  vector<string> logicalSegments3;
  logicalSegments3.push_back(string("BackBone"));
  logicalSegments3.push_back(string("FieldCN_RS1"));
  logicalSegments3.push_back(string("FieldCN_RS2"));

  vector<string> logicalSegments4;
  logicalSegments4.push_back(string("BackBone"));
  logicalSegments4.push_back(string("FieldCN_RS1"));
  logicalSegments4.push_back(string("FieldCN_RS2"));
  logicalSegments4.push_back(string("FieldCN_RS3"));
  
  LogicalSegmentAllocator::TLogicalSegmentBandwidth lsFreeBandWidths;
  lsFreeBandWidths[string("BackBone")]    = 1024.0;
  lsFreeBandWidths[string("FieldCN_RS1")] = 1024.0;
  lsFreeBandWidths[string("FieldCN_RS2")] = 1024.0;
  lsFreeBandWidths[string("FieldCN_RS3")] = 1024.0;
  
  vector<string> vrs;
  vrs.push_back(string("VR1"));
  vrs.push_back(string("VR2"));
  vrs.push_back(string("VR3"));
  vrs.push_back(string("VR4"));
  vrs.push_back(string("VR5"));
  vrs.push_back(string("VR6"));
  vrs.push_back(string("VR7"));
  
  vector<uint16> priorities;
  priorities.push_back(100); // 0
  priorities.push_back(100); // 1
  priorities.push_back(100); // 2
  priorities.push_back(100); // 3
  priorities.push_back(100); // 4
  priorities.push_back(100); // 5
  priorities.push_back(100); // 6
  priorities.push_back(10);  // 7
  priorities.push_back(5);   // 8
 
  vector<double> requiredBandwidth;
  requiredBandwidth.push_back(400.0); // 0
  requiredBandwidth.push_back(400.0); // 1
  requiredBandwidth.push_back(400.0); // 2
  requiredBandwidth.push_back(400.0); // 3
  requiredBandwidth.push_back(200.0); // 4
  requiredBandwidth.push_back(400.0); // 5
  requiredBandwidth.push_back(400.0); // 6
  requiredBandwidth.push_back(400.0); // 7
  requiredBandwidth.push_back(400.0); // 8
  requiredBandwidth.push_back(400.0); // 9
  
  vector<time_t> startTimes;
  vector<time_t> stopTimes;
  startTimes.push_back(APLUtilities::getUTCtime() +100);
  stopTimes.push_back (APLUtilities::getUTCtime() +120);
  startTimes.push_back(APLUtilities::getUTCtime() +50);
  stopTimes.push_back (APLUtilities::getUTCtime() +70);
  startTimes.push_back(APLUtilities::getUTCtime() +120);
  stopTimes.push_back (APLUtilities::getUTCtime() +140);
  startTimes.push_back(APLUtilities::getUTCtime() +90);
  stopTimes.push_back (APLUtilities::getUTCtime() +110);
  startTimes.push_back(APLUtilities::getUTCtime() +110);
  stopTimes.push_back (APLUtilities::getUTCtime() +130);
  startTimes.push_back(APLUtilities::getUTCtime() +90);
  stopTimes.push_back (APLUtilities::getUTCtime() +130);
  startTimes.push_back(APLUtilities::getUTCtime() +105);
  stopTimes.push_back (APLUtilities::getUTCtime() +115);
  
  {  
    LogicalSegmentAllocator LogicalSegmentAllocator;
    LogicalSegmentAllocator.logAllocation();

    set<string> resumeVRs;
    set<string> suspendVRs;
    string currentVR;
    vector<string> currentLS;
    uint16 currentPriority;
    time_t currentStart;
    time_t currentStop;
    double currentBW;
    
    currentVR       =vrs[0];
    currentLS       =logicalSegments1;
    currentPriority =priorities[0];
    currentStart    =startTimes[0];
    currentStop     =stopTimes[0];
    currentBW       =requiredBandwidth[0];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(!LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);

    currentVR=vrs[0];
    LOG_INFO(formatString("Deallocating VR %s",currentVR.c_str()));
    LogicalSegmentAllocator.deallocateVirtualRoute(currentVR,lsFreeBandWidths,resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);
  
    currentVR=vrs[0];
    LOG_INFO(formatString("Deallocating VR %s",currentVR.c_str()));
    LogicalSegmentAllocator.deallocateVirtualRoute(currentVR,lsFreeBandWidths,resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);
  
    currentVR       =vrs[0];
    currentLS       =logicalSegments2;
    currentPriority =priorities[0];
    currentStart    =startTimes[0];
    currentStop     =stopTimes[0];
    currentBW       =requiredBandwidth[0];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(!LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);
  
    currentVR       =vrs[0];
    currentLS       =logicalSegments2;
    currentPriority =priorities[0];
    currentStart    =startTimes[1];
    currentStop     =stopTimes[1];
    currentBW       =requiredBandwidth[0];
    LOG_INFO(formatString("Reallocating VR %s: %d logicalSegment(s), %d,%s,%s,%f",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(!LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);
  
    currentVR       =vrs[1];
    currentLS       =logicalSegments1;
    currentPriority =priorities[1];
    currentStart    =startTimes[1];
    currentStop     =stopTimes[1];
    currentBW       =requiredBandwidth[1];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f using overlapping times, same priority",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(!LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);
    
    currentVR       =vrs[2];
    currentLS       =logicalSegments1;
    currentPriority =priorities[2];
    currentStart    =startTimes[0];
    currentStop     =stopTimes[0];
    currentBW       =requiredBandwidth[2];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f non-overlapping ",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(!LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);
    
    currentVR       =vrs[2];
    currentLS       =logicalSegments1;
    currentPriority =priorities[2];
    currentStart    =startTimes[1];
    currentStop     =stopTimes[1];
    currentBW       =requiredBandwidth[2];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f overlapping times. failure expected",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);
    
    currentVR       =vrs[3];
    currentLS       =logicalSegments1;
    currentPriority =priorities[3];
    currentStart    =startTimes[1];
    currentStop     =stopTimes[1];
    currentBW       =requiredBandwidth[3];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f overlapping times. failure expected",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LogicalSegmentAllocator.logAllocation();
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);
    
    currentVR=vrs[0];
    LOG_INFO(formatString("Deallocating VR %s",currentVR.c_str()));
    LogicalSegmentAllocator.deallocateVirtualRoute(currentVR,lsFreeBandWidths,resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);
    currentVR=vrs[1];
    LOG_INFO(formatString("Deallocating VR %s",currentVR.c_str()));
    LogicalSegmentAllocator.deallocateVirtualRoute(currentVR,lsFreeBandWidths,resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);
    currentVR=vrs[2];
    LOG_INFO(formatString("Deallocating VR %s",currentVR.c_str()));
    LogicalSegmentAllocator.deallocateVirtualRoute(currentVR,lsFreeBandWidths,resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);
    currentVR=vrs[3];
    LOG_INFO(formatString("Deallocating VR %s",currentVR.c_str()));
    LogicalSegmentAllocator.deallocateVirtualRoute(currentVR,lsFreeBandWidths,resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);
  
    currentVR       =vrs[0];
    currentLS       =logicalSegments1;
    currentPriority =priorities[0];
    currentStart    =startTimes[0];
    currentStop     =stopTimes[0];
    currentBW       =requiredBandwidth[0];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(!LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);

    currentVR       =vrs[1];
    currentLS       =logicalSegments1;
    currentPriority =priorities[1];
    currentStart    =startTimes[1];
    currentStop     =stopTimes[1];
    currentBW       =requiredBandwidth[1];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f non-overlapping times",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(!LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);

    currentVR       =vrs[2];
    currentLS       =logicalSegments1;
    currentPriority =priorities[2];
    currentStart    =startTimes[2];
    currentStop     =stopTimes[2];
    currentBW       =requiredBandwidth[2];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f non-overlapping times",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(!LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);

    currentVR       =vrs[3];
    currentLS       =logicalSegments1;
    currentPriority =priorities[3];
    currentStart    =startTimes[3];
    currentStop     =stopTimes[3];
    currentBW       =requiredBandwidth[3];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f overlapping times",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(!LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);

    currentVR       =vrs[4];
    currentLS       =logicalSegments2;
    currentPriority =priorities[4];
    currentStart    =startTimes[4];
    currentStop     =stopTimes[4];
    currentBW       =requiredBandwidth[4];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f overlapping times",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(!LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);

    currentVR       =vrs[5];
    currentLS       =logicalSegments2;
    currentPriority =priorities[5];
    currentStart    =startTimes[5];
    currentStop     =stopTimes[5];
    currentBW       =requiredBandwidth[5];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f  overlapping times. This should fail",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);

    currentVR       =vrs[6];
    currentLS       =logicalSegments2;
    currentPriority =priorities[6];
    currentStart    =startTimes[6];
    currentStop     =stopTimes[6];
    currentBW       =requiredBandwidth[6];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f overlapping times. This should fail",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    LOG_INFO("The same allocation, grouped by VR:");
    LogicalSegmentAllocator.logAllocation(true);
    printResumeSuspendVRs(resumeVRs,suspendVRs);


    currentVR       =vrs[6];
    currentLS       =logicalSegments2;
    currentPriority =priorities[7];
    currentStart    =startTimes[6];
    currentStop     =stopTimes[6];
    currentBW       =requiredBandwidth[6];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f  overlapping times with HIGH priority. Let's see what gets suspended...",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(!LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    LOG_INFO("The same allocation, grouped by VR:");
    LogicalSegmentAllocator.logAllocation(true);
    printResumeSuspendVRs(resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logSuspendedAllocation();

    currentVR=vrs[6];
    LOG_INFO(formatString("Deallocating the HIGH priority VR %s. Let's see what gets resumed...",currentVR.c_str()));
    LogicalSegmentAllocator.deallocateVirtualRoute(currentVR,lsFreeBandWidths,resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logSuspendedAllocation();


    currentVR       =vrs[5];
    currentLS       =logicalSegments2;
    currentPriority =priorities[7];
    currentStart    =startTimes[4];
    currentStop     =stopTimes[4];
    currentBW       =requiredBandwidth[7];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f overlapping times with HIGH priority. Failure expected because there is no VR that can be replaced",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    LOG_INFO("The same allocation, grouped by VR:");
    LogicalSegmentAllocator.logAllocation(true);
    printResumeSuspendVRs(resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logSuspendedAllocation();


    currentVR       =vrs[6];
    currentLS       =logicalSegments2;
    currentPriority =priorities[8];
    currentStart    =startTimes[3];
    currentStop     =stopTimes[3];
    currentBW       =requiredBandwidth[8];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f overlapping times with HIGH priority. Let's see what gets suspended...",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(!LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    LOG_INFO("The same allocation, grouped by VR:");
    LogicalSegmentAllocator.logAllocation(true);
    printResumeSuspendVRs(resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logSuspendedAllocation();

    currentVR=vrs[5];
    LOG_INFO(formatString("Deallocating the HIGH priority VR %s. Let's see what gets resumed...",currentVR.c_str()));
    LogicalSegmentAllocator.deallocateVirtualRoute(currentVR,lsFreeBandWidths,resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logSuspendedAllocation();

    currentVR=vrs[6];
    LOG_INFO(formatString("Deallocating the HIGH priority VR %s. Let's see what gets resumed...",currentVR.c_str()));
    LogicalSegmentAllocator.deallocateVirtualRoute(currentVR,lsFreeBandWidths,resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logSuspendedAllocation();

    currentVR       =vrs[5];
    currentLS       =logicalSegments2;
    currentPriority =priorities[7];
    currentStart    =startTimes[3];
    currentStop     =stopTimes[3];
    currentBW       =requiredBandwidth[7];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f overlapping times with HIGH priority. Let's see what gets suspended...",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(!LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    LOG_INFO("The same allocation, grouped by VR:");
    LogicalSegmentAllocator.logAllocation(true);
    printResumeSuspendVRs(resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logSuspendedAllocation();

    currentVR       =vrs[6];
    currentLS       =logicalSegments2;
    currentPriority =priorities[8];
    currentStart    =startTimes[3];
    currentStop     =stopTimes[3];
    currentBW       =requiredBandwidth[8];
    LOG_INFO(formatString("Allocating VR %s: %d logicalSegment(s), %d,%s,%s,%f overlapping times with HIGH priority. Let's see what gets suspended...",currentVR.c_str(),currentLS.size(),currentPriority,formatTime(currentStart).c_str(),formatTime(currentStop).c_str(),currentBW));
    if(!LogicalSegmentAllocator.allocateVirtualRoute(currentVR,lsFreeBandWidths,currentLS,currentPriority,currentStart,currentStop,currentBW,resumeVRs,suspendVRs))
    {
      LOG_FATAL("test failed");
      exit(-1);
    }
    LogicalSegmentAllocator.logAllocation();
    LOG_INFO("The same allocation, grouped by VR:");
    LogicalSegmentAllocator.logAllocation(true);
    printResumeSuspendVRs(resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logSuspendedAllocation();

    currentVR=vrs[6];
    LOG_INFO(formatString("Deallocating the HIGH priority VR %s. Let's see what gets resumed...",currentVR.c_str()));
    LogicalSegmentAllocator.deallocateVirtualRoute(currentVR,lsFreeBandWidths,resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logSuspendedAllocation();

    currentVR=vrs[5];
    LOG_INFO(formatString("Deallocating the HIGH priority VR %s. Let's see what gets resumed...",currentVR.c_str()));
    LogicalSegmentAllocator.deallocateVirtualRoute(currentVR,lsFreeBandWidths,resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logAllocation();
    printResumeSuspendVRs(resumeVRs,suspendVRs);
    LogicalSegmentAllocator.logSuspendedAllocation();
  }

  {
    vector<string> allvrs;
    vector<string> alllogicalSegments;
    LogicalSegmentAllocator::TLogicalSegmentBandwidth allFreeBandWidths;
    vector<uint16> allpriorities;
    vector<time_t> allstartTimes;
    vector<time_t> allstopTimes;
    double requiredBandwidth(30.0);
    LogicalSegmentAllocator::TLogicalSegmentBandwidth lsBandWidthsDelta;

    for(int i=0;i<100;i++)
    {
      char tempStr[10];
      sprintf(tempStr,"FieldCN_RS%03d",i);
      alllogicalSegments.push_back(string(tempStr));
      allFreeBandWidths[string(tempStr)] = 1024.0;

      sprintf(tempStr,"VR%03d",i);
      allvrs.push_back(string(tempStr));

      allpriorities.push_back(i);

      allstartTimes.push_back(APLUtilities::getUTCtime() + 1000 + i*10);
      allstopTimes.push_back (APLUtilities::getUTCtime() + 1000 + i*10+5);
    }
    allvrs.push_back(string("VR100"));
    
    LogicalSegmentAllocator LogicalSegmentAllocator;
    LogicalSegmentAllocator.logAllocation();

    set<string> resumeVRs;
    set<string> suspendVRs;
    string currentVR;

    printResumeSuspendVRs(resumeVRs,suspendVRs);
    
    LOG_INFO("and now something big");
    
    struct timeval beginTv;
    struct timeval endTv;
    double t1,t2,timediff;
    bool success;

    for(int t=0;t<5;t++)
    {
      gettimeofday(&beginTv,0);
      success=LogicalSegmentAllocator.allocateVirtualRoute(allvrs[t],allFreeBandWidths,alllogicalSegments,allpriorities[t],allstartTimes[t],allstopTimes[t],requiredBandwidth,resumeVRs,suspendVRs);
      gettimeofday(&endTv,0);
      t1 = beginTv.tv_sec + ((double)beginTv.tv_usec/1000000);
      t2 = endTv.tv_sec + ((double)endTv.tv_usec/1000000);
      timediff = t2-t1;
      LOG_INFO(formatString("It took me (%f - %f = ) %f to allocate it all",t2,t1,timediff));
      if(!success)
      {
        LOG_FATAL("test failed");
        exit(-1);
      }
    }
    
    gettimeofday(&beginTv,0);
    success=LogicalSegmentAllocator.allocateVirtualRoute(allvrs[100],lsFreeBandWidths,alllogicalSegments,allpriorities[0],allstartTimes[0],allstopTimes[0],requiredBandwidth,resumeVRs,suspendVRs);
    gettimeofday(&endTv,0);
    t1 = beginTv.tv_sec + ((double)beginTv.tv_usec/1000000);
    t2 = endTv.tv_sec + ((double)endTv.tv_usec/1000000);
    timediff = t2-t1;
    LOG_INFO(formatString("It took me (%f - %f = ) %f to conclude that allocation is not possible",t2,t1,timediff));
    if(success)
    {
      LOG_FATAL("test failed");
      exit(-1);
    }

    for(int t=0;t<5;t++)
    {
      gettimeofday(&beginTv,0);
      LogicalSegmentAllocator.deallocateVirtualRoute(allvrs[t],lsFreeBandWidths,resumeVRs,suspendVRs);
      gettimeofday(&endTv,0);
      t1 = beginTv.tv_sec + ((double)beginTv.tv_usec/1000000);
      t2 = endTv.tv_sec + ((double)endTv.tv_usec/1000000);
      timediff = t2-t1;
      LOG_INFO(formatString("It took me (%f - %f = ) %f to deallocate one VR",t2,t1,timediff));
    }    

  }
  return retval;
}

