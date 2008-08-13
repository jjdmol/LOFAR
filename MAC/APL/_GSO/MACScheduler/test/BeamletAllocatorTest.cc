//#  BeamletAllocatorTest.cc: Main entry for the BeamletAllocator test
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
#include "../src/BeamletAllocator.h"

using namespace LOFAR;
using namespace LOFAR::GSO;
using namespace LOFAR::APLCommon;

void printResumeSuspendVIs(
  map<string, BeamletAllocator::TStationBeamletAllocation> resumeVIs,
  set<string> suspendVIs)
{
  map<string, BeamletAllocator::TStationBeamletAllocation>::iterator itR;
  set<string>::iterator itS;
  stringstream logStream;
  logStream << "ResumeVIs: ";
  for(itR=resumeVIs.begin();itR!=resumeVIs.end();++itR)
  {
    logStream << itR->first.c_str() << ", ";
  }
  logStream << endl;
  LOG_INFO(formatString("%s",logStream.str().c_str()));
  
  logStream.str(string(""));
  logStream << "suspendVIs: ";
  for(itS=suspendVIs.begin();itS!=suspendVIs.end();++itS)
  {
    logStream << itS->c_str() << ", ";
  }
  logStream << endl;
  LOG_INFO(formatString("%s",logStream.str().c_str()));
}
int main(int /*argc*/, char** /*argv*/)
{
  int retval=0;
  
  INIT_LOGGER("./BeamletAllocatorTest.log_prop");   

  vector<string> stations1;
  stations1.push_back(string("STS1"));
  
  vector<string> stations2;
  stations2.push_back(string("STS1"));
  stations2.push_back(string("STS2"));
  
  vector<string> stations3;
  stations3.push_back(string("STS1"));
  stations3.push_back(string("STS2"));
  stations3.push_back(string("STS3"));

  vector<string> stations4;
  stations4.push_back(string("STS1"));
  stations4.push_back(string("STS2"));
  stations4.push_back(string("STS3"));
  stations4.push_back(string("STS4"));
  
  vector<string> vis;
  vis.push_back(string("VI1"));
  vis.push_back(string("VI2"));
  vis.push_back(string("VI3"));
  vis.push_back(string("VI4"));
  vis.push_back(string("VI5"));
  vis.push_back(string("VI6"));
  vis.push_back(string("VI7"));
  
  vector<uint16> priorities;
  priorities.push_back(100);
  priorities.push_back(100);
  priorities.push_back(100);
  priorities.push_back(100);
  priorities.push_back(100);
  priorities.push_back(100);
  priorities.push_back(100);
  priorities.push_back(10);
  priorities.push_back(5);
  
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
  
  vector<int16> subbands;
  for(int i=0;i<10;i++)
    subbands.push_back(i);
  
  {  
    BeamletAllocator beamletAllocator(25);
    beamletAllocator.logAllocation();

    BeamletAllocator::TStationBeamletAllocation beamlets;
    map<string, BeamletAllocator::TStationBeamletAllocation> resumeVIs;
    set<string> suspendVIs;
    string currentVI;

    currentVI=vis[0];
    LOG_INFO(formatString("Allocating VI %s: 10 subbands for 1 station",currentVI.c_str()));
    if(!beamletAllocator.allocateBeamlets(currentVI,priorities[0],stations1,startTimes[0],stopTimes[0],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);

    currentVI=vis[0];
    LOG_INFO(formatString("Deallocating VI %s",currentVI.c_str()));
    beamletAllocator.deallocateBeamlets(currentVI,resumeVIs,suspendVIs);
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
  
    currentVI=vis[0];
    LOG_INFO(formatString("Deallocating VI %s",currentVI.c_str()));
    beamletAllocator.deallocateBeamlets(currentVI,resumeVIs,suspendVIs);
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
  
    currentVI=vis[0];
    LOG_INFO(formatString("Allocating VI %s: 10 subbands for 2 stations",currentVI.c_str()));
    if(!beamletAllocator.allocateBeamlets(currentVI,priorities[0],stations2,startTimes[0],stopTimes[0],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
  
    currentVI=vis[0];
    LOG_INFO(formatString("Reallocating VI %s: 10 subbands for 2 stations",currentVI.c_str()));
    if(!beamletAllocator.allocateBeamlets(currentVI,priorities[0],stations2,startTimes[1],stopTimes[1],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
  
    currentVI=vis[1];
    LOG_INFO(formatString("Allocating VI %s: 10 subbands for 1 station using overlapping times",currentVI.c_str()));
    if(!beamletAllocator.allocateBeamlets(currentVI,priorities[1],stations1,startTimes[1],stopTimes[1],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    
    currentVI=vis[2];
    LOG_INFO(formatString("Allocating VI %s: 10 subbands for 1 station at non-overlapping times",currentVI.c_str()));
    if(!beamletAllocator.allocateBeamlets(currentVI,priorities[2],stations1,startTimes[0],stopTimes[0],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    
    currentVI=vis[2];
    LOG_INFO(formatString("Reallocating VI %s: 10 subbands for 1 station at overlapping times. This must fail because not enough beamlets are available",currentVI.c_str()));
    if(beamletAllocator.allocateBeamlets(currentVI,priorities[2],stations1,startTimes[1],stopTimes[1],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    
    currentVI=vis[3];
    LOG_INFO(formatString("Allocating VI %s: 10 subbands for 1 station using overlapping times. This must fail because not enough beamlets are available",currentVI.c_str()));
    if(beamletAllocator.allocateBeamlets(currentVI,priorities[3],stations1,startTimes[1],stopTimes[1],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    
    currentVI=vis[0];
    LOG_INFO(formatString("Deallocating VI %s",currentVI.c_str()));
    beamletAllocator.deallocateBeamlets(currentVI,resumeVIs,suspendVIs);
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    currentVI=vis[1];
    LOG_INFO(formatString("Deallocating VI %s",currentVI.c_str()));
    beamletAllocator.deallocateBeamlets(currentVI,resumeVIs,suspendVIs);
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    currentVI=vis[2];
    LOG_INFO(formatString("Deallocating VI %s",currentVI.c_str()));
    beamletAllocator.deallocateBeamlets(currentVI,resumeVIs,suspendVIs);
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    currentVI=vis[3];
    LOG_INFO(formatString("Deallocating VI %s",currentVI.c_str()));
    beamletAllocator.deallocateBeamlets(currentVI,resumeVIs,suspendVIs);
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
  
    currentVI=vis[0];
    LOG_INFO(formatString("Allocating VI %s: 10 subbands for 1 station",currentVI.c_str()));
    if(!beamletAllocator.allocateBeamlets(currentVI,priorities[0],stations1,startTimes[0],stopTimes[0],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    currentVI=vis[1];
    LOG_INFO(formatString("Allocating VI %s: 10 subbands for 1 station at non-overlapping times",currentVI.c_str()));
    if(!beamletAllocator.allocateBeamlets(currentVI,priorities[1],stations1,startTimes[1],stopTimes[1],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    currentVI=vis[2];
    LOG_INFO(formatString("Allocating VI %s: 10 subbands for 1 station at non-overlapping times",currentVI.c_str()));
    if(!beamletAllocator.allocateBeamlets(currentVI,priorities[2],stations1,startTimes[2],stopTimes[2],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    currentVI=vis[3];
    LOG_INFO(formatString("Allocating VI %s: 10 subbands for 1 station at overlapping times",currentVI.c_str()));
    if(!beamletAllocator.allocateBeamlets(currentVI,priorities[3],stations1,startTimes[3],stopTimes[3],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    currentVI=vis[4];
    LOG_INFO(formatString("Allocating VI %s: 10 subbands for 2 stations at overlapping times",currentVI.c_str()));
    if(!beamletAllocator.allocateBeamlets(currentVI,priorities[4],stations2,startTimes[4],stopTimes[4],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    currentVI=vis[5];
    LOG_INFO(formatString("Allocating VI %s: 10 subbands for 2 stations at overlapping times. This should fail because station1 has not enough beamlets available",currentVI.c_str()));
    if(beamletAllocator.allocateBeamlets(currentVI,priorities[5],stations2,startTimes[5],stopTimes[5],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    currentVI=vis[6];
    LOG_INFO(formatString("Allocating VI %s: 10 subbands for 2 stations at overlapping times. This should fail because station1 has not enough beamlets available",currentVI.c_str()));
    if(beamletAllocator.allocateBeamlets(currentVI,priorities[6],stations2,startTimes[6],stopTimes[6],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    LOG_INFO("The same allocation, grouped by VI:");
    beamletAllocator.logAllocation(true);
    printResumeSuspendVIs(resumeVIs,suspendVIs);

    currentVI=vis[6];
    LOG_INFO(formatString("Allocating VI %s: 10 subbands for 2 stations at overlapping times with HIGH priority. Let's see what gets suspended...",currentVI.c_str()));
    if(!beamletAllocator.allocateBeamlets(currentVI,priorities[7],stations2,startTimes[4],stopTimes[4],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    LOG_INFO("The same allocation, grouped by VI:");
    beamletAllocator.logAllocation(true);
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    beamletAllocator.logSuspendedAllocation();

    currentVI=vis[6];
    LOG_INFO(formatString("Deallocating the HIGH priority VI %s. Let's see what gets resumed...",currentVI.c_str()));
    beamletAllocator.deallocateBeamlets(currentVI,resumeVIs,suspendVIs);
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    beamletAllocator.logSuspendedAllocation();

    currentVI=vis[5];
    LOG_INFO(formatString("Allocating VI %s: 10 subbands for 2 stations at overlapping times with HIGH priority. Let's see what gets suspended...",currentVI.c_str()));
    if(!beamletAllocator.allocateBeamlets(currentVI,priorities[7],stations2,startTimes[4],stopTimes[4],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    LOG_INFO("The same allocation, grouped by VI:");
    beamletAllocator.logAllocation(true);
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    beamletAllocator.logSuspendedAllocation();

    currentVI=vis[6];
    LOG_INFO(formatString("Allocating VI %s: 10 subbands for 2 stations at overlapping times with HIGH priority. Let's see what gets suspended...",currentVI.c_str()));
    if(!beamletAllocator.allocateBeamlets(currentVI,priorities[8],stations2,startTimes[4],stopTimes[4],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    LOG_INFO("The same allocation, grouped by VI:");
    beamletAllocator.logAllocation(true);
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    beamletAllocator.logSuspendedAllocation();

    currentVI=vis[5];
    LOG_INFO(formatString("Deallocating the HIGH priority VI %s. Let's see what gets resumed...",currentVI.c_str()));
    beamletAllocator.deallocateBeamlets(currentVI,resumeVIs,suspendVIs);
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    beamletAllocator.logSuspendedAllocation();

    currentVI=vis[6];
    LOG_INFO(formatString("Deallocating the HIGH priority VI %s. Let's see what gets resumed...",currentVI.c_str()));
    beamletAllocator.deallocateBeamlets(currentVI,resumeVIs,suspendVIs);
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    beamletAllocator.logSuspendedAllocation();

    currentVI=vis[5];
    LOG_INFO(formatString("Allocating VI %s: 10 subbands for 2 stations at overlapping times with HIGH priority. Let's see what gets suspended...",currentVI.c_str()));
    if(!beamletAllocator.allocateBeamlets(currentVI,priorities[7],stations2,startTimes[4],stopTimes[4],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    LOG_INFO("The same allocation, grouped by VI:");
    beamletAllocator.logAllocation(true);
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    beamletAllocator.logSuspendedAllocation();

    currentVI=vis[6];
    LOG_INFO(formatString("Allocating VI %s: 10 subbands for 2 stations at overlapping times with HIGH priority. Let's see what gets suspended...",currentVI.c_str()));
    if(!beamletAllocator.allocateBeamlets(currentVI,priorities[8],stations2,startTimes[4],stopTimes[4],subbands,beamlets,resumeVIs,suspendVIs))
    {
      LOG_FATAL("test failed");
      return -1;
    }
    beamletAllocator.logAllocation();
    LOG_INFO("The same allocation, grouped by VI:");
    beamletAllocator.logAllocation(true);
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    beamletAllocator.logSuspendedAllocation();

    currentVI=vis[6];
    LOG_INFO(formatString("Deallocating the HIGH priority VI %s. Let's see what gets resumed...",currentVI.c_str()));
    beamletAllocator.deallocateBeamlets(currentVI,resumeVIs,suspendVIs);
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    beamletAllocator.logSuspendedAllocation();

    currentVI=vis[5];
    LOG_INFO(formatString("Deallocating the HIGH priority VI %s. Let's see what gets resumed...",currentVI.c_str()));
    beamletAllocator.deallocateBeamlets(currentVI,resumeVIs,suspendVIs);
    beamletAllocator.logAllocation();
    printResumeSuspendVIs(resumeVIs,suspendVIs);
    beamletAllocator.logSuspendedAllocation();
  }

  {
    vector<int16> allsubbands;
    for(int i=0;i<200;i++)
      allsubbands.push_back(i);
    
    vector<string> allvis;
    vector<uint16> allpriorities;
    vector<string> allstations;
    vector<time_t> allstartTimes;
    vector<time_t> allstopTimes;
    for(int i=0;i<100;i++)
    {
      char tempStr[10];
      sprintf(tempStr,"STS%03d",i);
      allstations.push_back(string(tempStr));

      sprintf(tempStr,"VI%03d",i);
      allvis.push_back(string(tempStr));

      allpriorities.push_back(i);

      allstartTimes.push_back(APLUtilities::getUTCtime() + 1000 + i*10);
      allstopTimes.push_back (APLUtilities::getUTCtime() + 1000 + i*10+5);
    }
    allvis.push_back(string("VI100"));
    
    BeamletAllocator beamletAllocator(200);
    beamletAllocator.logAllocation();

    BeamletAllocator::TStationBeamletAllocation beamlets;
    map<string, BeamletAllocator::TStationBeamletAllocation> resumeVIs;
    set<string> suspendVIs;
    string currentVI;

    printResumeSuspendVIs(resumeVIs,suspendVIs);
    
    LOG_INFO("and now something big");
    
    struct timeval beginTv;
    struct timeval endTv;
    double t1,t2,timediff;
    bool success;

    for(int t=0;t<5;t++)
    {
      gettimeofday(&beginTv,0);
      success=beamletAllocator.allocateBeamlets(allvis[t],allpriorities[t],allstations,allstartTimes[t],allstopTimes[t],allsubbands,beamlets,resumeVIs,suspendVIs);
      gettimeofday(&endTv,0);
      t1 = beginTv.tv_sec + ((double)beginTv.tv_usec/1000000);
      t2 = endTv.tv_sec + ((double)endTv.tv_usec/1000000);
      timediff = t2-t1;
      LOG_INFO(formatString("It took me (%f - %f = ) %f to allocate it all",t2,t1,timediff));
      if(!success)
      {
        LOG_FATAL("test failed");
        return -1;
      }
    }
    
    gettimeofday(&beginTv,0);
    success=beamletAllocator.allocateBeamlets(allvis[100],allpriorities[0],allstations,allstartTimes[0],allstopTimes[0],allsubbands,beamlets,resumeVIs,suspendVIs);
    gettimeofday(&endTv,0);
    t1 = beginTv.tv_sec + ((double)beginTv.tv_usec/1000000);
    t2 = endTv.tv_sec + ((double)endTv.tv_usec/1000000);
    timediff = t2-t1;
    LOG_INFO(formatString("It took me (%f - %f = ) %f to conclude that allocation is not possible",t2,t1,timediff));
    if(success)
    {
      LOG_FATAL("test failed");
      return -1;
    }

    for(int t=0;t<5;t++)
    {
      gettimeofday(&beginTv,0);
      beamletAllocator.deallocateBeamlets(allvis[t],resumeVIs,suspendVIs);
      gettimeofday(&endTv,0);
      t1 = beginTv.tv_sec + ((double)beginTv.tv_usec/1000000);
      t2 = endTv.tv_sec + ((double)endTv.tv_usec/1000000);
      timediff = t2-t1;
      LOG_INFO(formatString("It took me (%f - %f = ) %f to deallocate one VI",t2,t1,timediff));
    }    

  }
  return retval;
}

