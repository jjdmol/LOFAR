//#  WH_Signal.cc: Emulate a station
//#
//#  Copyright (C) 2002-2005
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

// General includes
#include <Common/LofarLogger.h>

// Application specific includes
#include <WH_Signal.h>
// for timer
#include <signal.h>
#include <sys/time.h>
// for rand
#include <stdlib.h>
// for sleep (yield)
#include <boost/thread.hpp>

using namespace LOFAR;

int WH_Signal::theirNoRunningWHs = 0;
int WH_Signal::theirNoAlarms = 0;
bool WH_Signal::theirTimerSet = 0;

int16 randint16() {
  int32 value = rand() - RAND_MAX / 2;  
  // we need to return a int16 with 12 used bits
  // so divide by 2^20
  return value / 1048576 ;
};

WH_Signal::WH_Signal(const string& name, 
		     const int noOutputs,
		     const ParameterSet ps,
		     const SignalType st)
  : WorkHolder (0, noOutputs,
		name, 
		"WH_Signal"),
    itsPS(ps),
    itsSignalType(st)
{
  itsFrequency = ps.getDouble("Generator.OutputRate") / ps.getInt32("Input.NPacketsInFrame");
  
  for (int outDH = 0; outDH < noOutputs; outDH++) {
    getDataManager().addOutDataHolder(outDH, new DH_RSP("signal_out", itsPS));
  }  
}

WH_Signal::~WH_Signal() {
}

WorkHolder* WH_Signal::construct(const string& name,
				 const int noOutputs,
				 const ParameterSet ps,
				 const SignalType st)
{
  return new WH_Signal(name, noOutputs, ps, st);
}

WH_Signal* WH_Signal::make(const string& name)
{
  return new WH_Signal(name, itsNoutputs, itsPS, itsSignalType);
}

void WH_Signal::preprocess() 
{
  if (itsFrequency == 0) {
    // frequency was set to zero, which means output should go as fast as possible
    // we do that by setting theirNoAlarms to the number of runs
    theirNoAlarms += itsPS.getDouble("Generator.NoRuns");
  } else {
    if (!theirTimerSet) {
      sighandler_t ret = signal(SIGALRM, *WH_Signal::timerSignal);
      ASSERTSTR(ret != SIG_ERR, "WH_Signal couldn't set signal handler for timer");    
      struct itimerval value;
      memset (&value, 0, sizeof(itimerval));

      double time = 1/itsFrequency;    
      __time_t secs = floor(time);
      __time_t usecs = 1e6 * (time - secs);
      // this means 1MHz is the highest frequency
      if (secs + usecs == 0) usecs = 1;
      value.it_interval.tv_sec = secs;
      value.it_interval.tv_usec = usecs;
      value.it_value.tv_sec = secs;
      value.it_value.tv_usec = usecs;
      cout << "Setting timer interval to " << secs << "secs and " << usecs << "ms" << endl;

      setitimer(ITIMER_REAL, &value, 0);
      
      theirTimerSet = true;
    }
  }
  theirNoRunningWHs++;
}  

void WH_Signal::process() 
{
  EthernetFrame& myEthFrame = ((DH_RSP*)getDataManager().getOutHolder(0))->getEthernetFrame();
  myEthFrame.reset();

  for (int epap = 0; epap < myEthFrame.getNoPacketsInFrame(); epap++) {
    Header& header = myEthFrame.getHeader();
    header.setProtocol(43691);
    header.setSeqId(itsStamp.getSeqId());
    header.setBlockId(itsStamp.getBlockId());
    itsStamp++;

    if (itsSignalType == ZERO) {
      // do noting, zero's are already there
    } else if (itsSignalType == PATTERN) {
      RectMatrix<RSPDataType>& myMatrix = myEthFrame.getEpaPacket(epap).getMatrix();
      dimType sbDim = myMatrix.getDim("subband");
      dimType polDim = myMatrix.getDim("polarisation");
      RectMatrix<RSPDataType>::cursorType cursor = myMatrix.getCursor(0*sbDim);
      for (int sb = 0; sb < myMatrix.getNElemInDim(sbDim); myMatrix.moveCursor(&cursor, sbDim), sb++) {
	myMatrix.setValue(cursor, makei16complex(sb, itsStamp.getBlockId()));  // the real value is the beamlet number, the imag value is the slice count
// 	RectMatrix<RSPDataType>::cursorType polC = cursor;
// 	myMatrix.moveCursor(polC, polDim);
// 	myMatrix.setValue(polC, makei16complex(sb, itsStamp.getBlockId()));  // the real value is the beamlet number, the imag value is the slice count
	// the y polarisations are all 0
      }
    } else if (itsSignalType == RANDOM) {
      // create random data
      RectMatrix<RSPDataType>& myMatrix = myEthFrame.getEpaPacket(epap).getMatrix();
      dimType sbDim = myMatrix.getDim("subband");
      dimType pDim = myMatrix.getDim("polarisation");
      RectMatrix<RSPDataType>::cursorType cursor = myMatrix.getCursor(0*sbDim);
      RectMatrix<RSPDataType>::cursorType pcursor;
      RSPDataType value;
      for (int sb = 0; sb < myMatrix.getNElemInDim(sbDim); myMatrix.moveCursor(&cursor, sbDim), sb++) {
	myMatrix.setValue(cursor, makei16complex(randint16(), randint16()));
	pcursor = cursor;
	myMatrix.moveCursor(&pcursor, pDim);
	myMatrix.setValue(pcursor, makei16complex(randint16(), randint16()));
      }
    }
  }

  // copy to all outDHs
  EthernetFrame* dstFrame;
  for (int outDH = 1; outDH < itsNoutputs; outDH++) {dstFrame = &(((DH_RSP*)getDataManager().getOutHolder(outDH))->getEthernetFrame());
    DBGASSERTSTR(dstFrame->getPayloadSize() == myEthFrame.getPayloadSize(), "WH_Signal: sizes of eth frames are not equal");
    // cpy the complete ethernetframe
    memcpy(dstFrame->getPayloadp(), myEthFrame.getPayloadp(), myEthFrame.getPayloadSize());
  }

  int timerCount = 0;
  while (theirNoAlarms == 0) 
  {
    timerCount ++;
    // wait for alarm to go off
    boost::thread::yield();
  };
  cout<<"yield was called "<< timerCount << " times"<<endl;

  // we handled one alarm, so decrease it
  theirNoAlarms--;
}

void WH_Signal::postprocess() 
{
  theirNoRunningWHs--;
  if (theirNoRunningWHs == 0)
  {
    theirTimerSet = false;
    if (itsFrequency != 0) {
      // unset timer
      struct itimerval value;
      memset (&value, 0, sizeof(itimerval));
      setitimer(ITIMER_REAL, &value, 0);
      // remove sig handler
      sighandler_t ret = signal(SIGALRM, SIG_DFL);
      ASSERTSTR(ret != SIG_ERR, "WH_Signal couldn't unset signal handler for timer");    
    }
  }
}  

void WH_Signal::timerSignal(int sig)
{
  // set the number of frames that can be sent
  theirNoAlarms += theirNoRunningWHs;
}
  
