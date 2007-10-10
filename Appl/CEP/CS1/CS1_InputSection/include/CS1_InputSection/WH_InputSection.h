//#  WH_InputSection.h: Catch RSP ethernet frames and synchronize RSP inputs 
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_CS1_INPUTSECTION_WH_INPUTSECTION_H
#define LOFAR_CS1_INPUTSECTION_WH_INPUTSECTION_H

// \file
// Catch RSP ethernet frames and synchronize RSP inputs 

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <tinyCEP/WorkHolder.h>
#include <CS1_Interface/RSPTimeStamp.h>
#include <CS1_Interface/DH_Subband.h>
#include <Common/Timer.h>

#include <boost/thread.hpp>
#include <boost/multi_array.hpp>


namespace LOFAR 
{
  class NSTimer;
  class TransportHolder;

  namespace CS1 
  {

    // \addtogroup CS1_InputSection
    // @{

    //# Forward Declarations
    class BeamletBuffer;
    class InputThread;

    // This class is the workholder that receives data from the RSP boards
    // and distributes it per subband to the Blue Gene/L
    class WH_InputSection: public WorkHolder {
    public:
      typedef DH_Subband::SampleType SampleType;

      explicit WH_InputSection(const string &name, 
			   bool doInput,
			   bool doTranspose,
			   bool doOutput,
                           CS1_Parset *ps,
                           TransportHolder *inputTH,
			   unsigned stationNr,
			   unsigned nrInputChannels,
			   unsigned nrOutputChannels,
			   const std::vector<unsigned> &inputNodes,
			   const std::vector<unsigned> &outputNodes);
      virtual ~WH_InputSection();
    
      virtual WH_InputSection *make(const string &name);
     
      virtual void preprocess();
      virtual void process();
      virtual void postprocess();
      
    private:
      // Copying is not allowed
      WH_InputSection (const WH_InputSection &that);
      WH_InputSection& operator= (const WH_InputSection &that);

      void doInput(SparseSet<unsigned> &flags);
      void doOutput();

      void limitFlagsLength(SparseSet<unsigned> &flags);

      void transposeData();
      void transposeMetaData(const SparseSet<unsigned> &flags);
      
      //# Datamembers
      bool itsDelayCompensation;
      bool itsDoInput, itsDoTranspose, itsDoOutput;
      const std::vector<unsigned> &itsInputNodes, &itsOutputNodes;

      boost::multi_array<SampleType, 4> *itsInputData, *itsOutputData;

      struct metaData {
	float fineDelayAtBegin, fineDelayAfterEnd;
	char  flagsBuffer[132]; // enough for 16 flag ranges
      } *itsInputMetaData, *itsOutputMetaData;

      // writer thread
      InputThread *itsInputThreadObject;
      boost::thread *itsInputThread;

      TransportHolder *itsInputTH;
      uint itsStationNr;
      
      CS1_Parset *itsCS1PS;
      
      // synced stamp
      TimeStamp itsSyncedStamp;
     
      unsigned itsNSubbandsPerCell;
      unsigned itsNSamplesPerSec;
      unsigned itsNHistorySamples;
     
      BeamletBuffer *itsBBuffer;
      
      bool itsFirstRun;

      NSTimer itsPrePostTimer, itsProcessTimer, itsGetElemTimer;
      
      void	  startThread();
    
      //handle timer alarm
      static void timerSignal(int signal);    
      static bool signalReceived;
    };
    
    // @}

  } // namespace CS1
} // namespace LOFAR

#endif
