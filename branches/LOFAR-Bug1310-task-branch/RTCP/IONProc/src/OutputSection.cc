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

#include <ION_Allocator.h>
#include <OutputSection.h>

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
      std::clog << "subband " << subbandNumber << " written to null:" << std::endl;
      itsStreamsToStorage.push_back(new NullStream);
    } else if (connectionType == "TCP") {
      std::string    server = itsParset->storageHostName(prefix + "_ServerHosts", subbandNumber);
      //unsigned short port   = boost::lexical_cast<unsigned short>(ps->getPortsOf(prefix)[storagePortIndex]);
      unsigned short port   = boost::lexical_cast<unsigned short>(itsParset->getPortsOf(prefix)[subbandNumber]);

      std::clog << "subband " << subbandNumber << " written to tcp:" << server << ':' << port << std::endl;
      itsStreamsToStorage.push_back(new SocketStream(server.c_str(), port, SocketStream::TCP, SocketStream::Client));
    } else if (connectionType == "FILE") {
      std::string filename = itsParset->getString(prefix + "_BaseFileName") + '.' +
		      boost::lexical_cast<std::string>(storageHostIndex) + '.' +
		      boost::lexical_cast<std::string>(subbandNumber);
		      //boost::lexical_cast<std::string>(storagePortIndex);

      std::clog << "subband " << subbandNumber << " written to file:" << filename << std::endl;
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
  itsNrIntegrationSteps     = ps->IONintegrationSteps();
  itsCurrentIntegrationStep = 0;
  itsSequenceNumber	    = 0;

  itsDroppedCount.resize(itsNrSubbandsPerPset);

  connectToStorage();

  itsTmpSum = newDataHolder( *ps, hugeMemoryAllocator );

  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++)
    itsSums.push_back(newDataHolder( *ps, hugeMemoryAllocator ));

  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++)
    itsOutputThreads.push_back(new OutputThread(itsStreamsToStorage[subband], *ps));
}


void OutputSection::droppingData(unsigned subband)
{
  if (itsDroppedCount[subband] ++ == 0) {
    unsigned subbandNumber = itsPsetNumber * itsNrSubbandsPerPset + subband;
    std::clog << "Warning: dropping data for subband " << subbandNumber << std::endl;
  }
}


void OutputSection::notDroppingData(unsigned subband)
{
  if (itsDroppedCount[subband] > 0) {
    unsigned subbandNumber = itsPsetNumber * itsNrSubbandsPerPset + subband;
    std::clog << "Warning: dropped " << itsDroppedCount[subband] << " integration times for subband " << subbandNumber << std::endl;
    itsDroppedCount[subband] = 0;
  }
}


void OutputSection::process()
{
  bool firstTime = itsCurrentIntegrationStep == 0;
  bool lastTime  = itsCurrentIntegrationStep == itsNrIntegrationSteps - 1;

  if( itsNrIntegrationSteps > 1 && !itsTmpSum->isIntegratable() )
  {
    // not integratable, so don't try
    std::clog << "Warning: not integrating received data because data type is not integratable" << std::endl;
    lastTime = firstTime = true;
  }

  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++) {
    // TODO: make sure that there are more free buffers than subbandsPerPset

    unsigned inputChannel = CN_Mapping::mapCoreOnPset(itsCurrentComputeCore, itsPsetNumber);

    if (lastTime) {
      if (itsOutputThreads[subband]->itsFreeQueue.empty()) {
	droppingData(subband);
	itsTmpSum->read(itsStreamsFromCNs[inputChannel], false);
      } else {
	notDroppingData(subband);
	StreamableData *data = itsOutputThreads[subband]->itsFreeQueue.remove();
      
	data->read(itsStreamsFromCNs[inputChannel], false);

	if (!firstTime)
	  *data += *itsSums[subband];

	data->sequenceNumber = itsSequenceNumber;
	itsOutputThreads[subband]->itsSendQueue.append(data);
      }
    } else if (firstTime) {
      itsSums[subband]->read(itsStreamsFromCNs[inputChannel], false);
    } else {
      itsTmpSum->read(itsStreamsFromCNs[inputChannel], false);
      *itsSums[subband] += *itsTmpSum;
    }

    if (++ itsCurrentComputeCore == itsNrComputeCores)
      itsCurrentComputeCore = 0;
  }

  if (++ itsCurrentIntegrationStep == itsNrIntegrationSteps) {
    itsCurrentIntegrationStep = 0;
    ++ itsSequenceNumber;
  }
}


void OutputSection::postprocess()
{
  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++) {
    notDroppingData(subband); // for final warning message
    itsOutputThreads[subband]->itsSendQueue.append(0); // 0 indicates that no more messages will be sent
    delete itsOutputThreads[subband];
    delete itsSums[subband];
    delete itsStreamsToStorage[subband];
  }

  itsOutputThreads.clear();
  itsSums.clear();
  itsStreamsToStorage.clear();
  itsDroppedCount.clear();

  delete itsTmpSum;
  itsTmpSum = 0;
}

}
}
