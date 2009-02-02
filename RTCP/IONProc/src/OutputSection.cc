//#  OutputSection.cc: Collects data from CNs and sends data to Storage
//#
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Interface/CN_Mapping.h>
#include <Interface/Allocator.h>
#include <Interface/Exceptions.h>
#include <Interface/StreamableData.h>
#include <Interface/CorrelatedData.h>
#include <Interface/DataHolder.h>

#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>

#include <IONProc/Lock.h>
#include <ION_Allocator.h>
#include <OutputSection.h>
#include <Scheduling.h>

#include <boost/lexical_cast.hpp>
#include <cstring>
#include <string>
#include <stdexcept>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


namespace LOFAR {
namespace RTCP {

OutputSection::OutputSection(unsigned psetNumber, const std::vector<Stream *> &streamsFromCNs)
:
  itsPsetNumber(psetNumber),
  itsParset(0),
  itsStreamsFromCNs(streamsFromCNs)
{
#if defined HAVE_BGP_ION
  doNotRunOnCore0();
  raisePriority();
#endif
}


void OutputSection::connectToStorage()
{
  unsigned myPsetIndex       = itsParset->outputPsetIndex(itsPsetNumber);
  unsigned nrPsetsPerStorage = itsParset->nrPsetsPerStorage();
  unsigned storageHostIndex  = myPsetIndex / nrPsetsPerStorage;
  //unsigned storagePortIndex  = myPsetIndex % nrPsetsPerStorage;

  string   prefix	     = "OLAP.OLAP_Conn.IONProc_Storage";
  string   connectionType    = itsParset->getString(prefix + "_Transport");

  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++) {
    unsigned subbandNumber = myPsetIndex * itsNrSubbandsPerPset + subband;

    if (connectionType == "NULL") {
      clog_logger("subband " << subbandNumber << " written to null:");
      itsStreamsToStorage.push_back(new NullStream);
    } else if (connectionType == "TCP") {
      std::string    server = itsParset->storageHostName(prefix + "_ServerHosts", subbandNumber);
      //unsigned short port   = boost::lexical_cast<unsigned short>(ps->getPortsOf(prefix)[storagePortIndex]);
      unsigned short port   = boost::lexical_cast<unsigned short>(itsParset->getPortsOf(prefix)[subbandNumber]);

      clog_logger("subband " << subbandNumber << " written to tcp:" << server << ':' << port);
      itsStreamsToStorage.push_back(new SocketStream(server.c_str(), port, SocketStream::TCP, SocketStream::Client));
    } else if (connectionType == "FILE") {
      std::string filename = itsParset->getString(prefix + "_BaseFileName") + '.' +
		      boost::lexical_cast<std::string>(storageHostIndex) + '.' +
		      boost::lexical_cast<std::string>(subbandNumber);
		      //boost::lexical_cast<std::string>(storagePortIndex);

      clog_logger("subband " << subbandNumber << " written to file:" << filename);
      itsStreamsToStorage.push_back(new FileStream(filename.c_str(), 0666));
    } else {
      THROW(IONProcException, "unsupported ION->Storage stream type");
    }
  }
}


void OutputSection::preprocess(const Parset *ps)
{
  itsParset                 = ps;
  itsNrComputeCores	    = ps->nrCoresPerPset();
  itsCurrentComputeCore	    = 0;
  itsNrSubbandsPerPset	    = ps->nrSubbandsPerPset();
  itsRealTime               = ps->realTime();
  itsMode                   = CN_Mode(*ps);

  itsDroppedCount.resize(itsNrSubbandsPerPset);
  itsCurrentIntegrationSteps.resize(itsMode.nrOutputs());
  itsSequenceNumbers.resize(itsMode.nrOutputs());

  connectToStorage();

  itsTmpSum = newDataHolders( *ps, hugeMemoryAllocator );

  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++)
    itsSums.push_back(newDataHolders( *ps, hugeMemoryAllocator ));

  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++)
    itsOutputThreads.push_back(new OutputThread(itsStreamsToStorage[subband], *ps));

  // only the last data set is integrated
  itsNrIntegrationSteps.resize(itsMode.nrOutputs());
  for (unsigned output = 0; output < itsMode.nrOutputs(); output ++) {
    unsigned steps = (output == itsMode.nrOutputs()-1) ? ps->IONintegrationSteps() : 1;

    if( steps > 1 && !(*itsTmpSum)[output]->isIntegratable() )
    {
      // not integratable, so don't try
      clog_logger("Warning: not integrating received data for output " << output << " because data type is not integratable");
      steps = 1;
    }

    itsNrIntegrationSteps[output] = steps;
  }
}


void OutputSection::droppingData(unsigned subband)
{
  if (itsDroppedCount[subband] ++ == 0) {
    unsigned subbandNumber = itsPsetNumber * itsNrSubbandsPerPset + subband;
    clog_logger("Warning: dropping data for subband " << subbandNumber);
  }
}


void OutputSection::notDroppingData(unsigned subband)
{
  if (itsDroppedCount[subband] > 0) {
    unsigned subbandNumber = itsPsetNumber * itsNrSubbandsPerPset + subband;
    clog_logger("Warning: dropped " << itsDroppedCount[subband] << " integration times for subband " << subbandNumber);
    itsDroppedCount[subband] = 0;
  }
}


void OutputSection::process()
{
  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++) {
    // TODO: make sure that there are more free buffers than subbandsPerPset

    unsigned inputChannel = CN_Mapping::mapCoreOnPset(itsCurrentComputeCore, itsPsetNumber);

    for (unsigned output = 0; output < itsMode.nrOutputs(); output ++) {
      bool firstTime = itsCurrentIntegrationSteps[output] == 0;
      bool lastTime  = itsCurrentIntegrationSteps[output] == itsNrIntegrationSteps[output] - 1;
      std::clog << "reading subband " << subband << " and output " << output << "(first,last)=" << firstTime << "," << lastTime << std::endl;

      if (lastTime) {
        if (itsRealTime && itsOutputThreads[subband]->itsFreeQueue[output].empty()) {
	  droppingData(subband);
          (*itsTmpSum)[output]->read(itsStreamsFromCNs[inputChannel], false);
        } else {
	  notDroppingData(subband);
	  StreamableData *data = itsOutputThreads[subband]->itsFreeQueue[output].remove();
      
	  data->read(itsStreamsFromCNs[inputChannel], false);

	  if (!firstTime)
	    *data += *(*itsSums[subband])[output];

	  data->sequenceNumber = itsSequenceNumbers[output];
	  itsOutputThreads[subband]->itsSendQueue[output].append(data);

          // report that data has been added to a send queue
	  itsOutputThreads[subband]->itsSendQueueActivity.append(output);
        }
      } else if (firstTime) {
        (*itsSums[subband])[output]->read(itsStreamsFromCNs[inputChannel], false);
      } else {
        (*itsTmpSum)[output]->read(itsStreamsFromCNs[inputChannel], false);
        *(*itsSums[subband])[output] += *(*itsTmpSum)[output];
      }
    }

    if (++ itsCurrentComputeCore == itsNrComputeCores)
      itsCurrentComputeCore = 0;
  }

  for (unsigned output = 0; output < itsMode.nrOutputs(); output ++) {
    if (++ itsCurrentIntegrationSteps[output] == itsNrIntegrationSteps[output] ) {
      itsCurrentIntegrationSteps[output] = 0;
      itsSequenceNumbers[output]++;
    }
  }
}


void OutputSection::postprocess()
{
  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++) {
    notDroppingData(subband); // for final warning message
    itsOutputThreads[subband]->itsSendQueueActivity.append(-1); // -1 indicates that no more messages will be sent

    for (unsigned output = 0; output < itsMode.nrOutputs(); output ++) {
      delete (*itsSums[subband])[output];
    }
    delete itsSums[subband];

    delete itsOutputThreads[subband];
    delete itsStreamsToStorage[subband];
  }


  itsOutputThreads.clear();
  itsSums.clear();
  itsStreamsToStorage.clear();
  itsDroppedCount.clear();
  itsNrIntegrationSteps.clear();
  itsCurrentIntegrationSteps.clear();
  itsSequenceNumbers.clear();

  for (unsigned output = 0; output < itsMode.nrOutputs(); output ++) {
    delete (*itsTmpSum)[output];
  }
  delete itsTmpSum;
  itsTmpSum = 0;
}

}
}
