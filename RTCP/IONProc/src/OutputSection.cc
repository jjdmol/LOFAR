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
  itsStreamsFromCNs(streamsFromCNs)
{
}


void OutputSection::connectToStorage(const Parset *ps)
{
  unsigned myPsetIndex       = ps->outputPsetIndex(itsPsetNumber);
  unsigned nrPsetsPerStorage = ps->nrPsetsPerStorage();
  unsigned storageHostIndex  = myPsetIndex / nrPsetsPerStorage;
  //unsigned storagePortIndex  = myPsetIndex % nrPsetsPerStorage;

  string   prefix	     = "OLAP.OLAP_Conn.IONProc_Storage";
  string   connectionType    = ps->getString(prefix + "_Transport");

  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++) {
    unsigned subbandNumber = myPsetIndex * itsNrSubbandsPerPset + subband;

    if (connectionType == "NULL") {
      std::clog << "subband " << subbandNumber << " written to null:" << std::endl;
      itsStreamsToStorage.push_back(new NullStream);
    } else if (connectionType == "TCP") {
      std::string    server = ps->getStringVector(prefix + "_ServerHosts")[storageHostIndex];
      //unsigned short port   = boost::lexical_cast<unsigned short>(ps->getPortsOf(prefix)[storagePortIndex]);
      unsigned short port   = boost::lexical_cast<unsigned short>(ps->getPortsOf(prefix)[subbandNumber]);

      std::clog << "subband " << subbandNumber << " written to tcp:" << server << ':' << port << std::endl;
      itsStreamsToStorage.push_back(new SocketStream(server.c_str(), port, SocketStream::TCP, SocketStream::Client));
    } else if (connectionType == "FILE") {
      std::string filename = ps->getString(prefix + "_BaseFileName") + '.' +
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
  itsNrComputeCores	    = ps->nrCoresPerPset();
  itsCurrentComputeCore	    = 0;
  itsNrSubbandsPerPset	    = ps->nrSubbandsPerPset();
  itsNrIntegrationSteps     = ps->IONintegrationSteps();
  itsCurrentIntegrationStep = 0;

  connectToStorage(ps);

  unsigned nrBaselines = ps->nrBaselines();
  unsigned nrChannels  = ps->nrChannelsPerSubband();

  itsTmpSum = new CorrelatedData(nrBaselines, nrChannels, hugeMemoryAllocator);

  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++)
    itsVisibilitySums.push_back(new CorrelatedData(nrBaselines, nrChannels, hugeMemoryAllocator));

  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++)
    itsOutputThreads.push_back(new OutputThread(itsStreamsToStorage[subband], nrBaselines, nrChannels));
}


void OutputSection::process()
{
  bool firstTime = itsCurrentIntegrationStep == 0;
  bool lastTime  = itsCurrentIntegrationStep == itsNrIntegrationSteps - 1;

  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++) {
    // TODO: make sure that there are more free buffers than subbandsPerPset

    unsigned inputChannel = CN_Mapping::mapCoreOnPset(itsCurrentComputeCore, itsPsetNumber);

    if (lastTime) {
      CorrelatedData *data = itsOutputThreads[subband]->itsFreeQueue.remove();
    
      data->read(itsStreamsFromCNs[inputChannel]);

      if (!firstTime)
	*data += *itsVisibilitySums[subband];

      itsOutputThreads[subband]->itsSendQueue.append(data);
    } else if (firstTime) {
      itsVisibilitySums[subband]->read(itsStreamsFromCNs[inputChannel]);
    } else {
      itsTmpSum->read(itsStreamsFromCNs[inputChannel]);
      *itsVisibilitySums[subband] += *itsTmpSum;
    }

    if (++ itsCurrentComputeCore == itsNrComputeCores)
      itsCurrentComputeCore = 0;
  }

  if (++ itsCurrentIntegrationStep == itsNrIntegrationSteps)
    itsCurrentIntegrationStep = 0;
}


void OutputSection::postprocess()
{
  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++) {
    itsOutputThreads[subband]->itsSendQueue.append(0); // 0 indicates that no more messages will be sent
    delete itsOutputThreads[subband];
    delete itsVisibilitySums[subband];
    delete itsStreamsToStorage[subband];
  }

  itsOutputThreads.clear();
  itsVisibilitySums.clear();
  itsStreamsToStorage.clear();

  delete itsTmpSum;
  itsTmpSum = 0;
}

}
}
