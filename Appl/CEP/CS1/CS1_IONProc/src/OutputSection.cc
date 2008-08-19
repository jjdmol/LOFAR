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

#include <CS1_Interface/BGL_Mapping.h>
#include <CS1_Interface/Allocator.h>
#include <OutputSection.h>

#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>

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
namespace CS1 {

OutputSection::OutputSection(unsigned psetNumber, const std::vector<Stream *> &streamsFromCNs)
:
  itsPsetNumber(psetNumber),
  itsStreamsFromCNs(streamsFromCNs)
{
}


OutputSection::~OutputSection()
{
}


void *OutputSection::sendThreadStub(void *arg)
{
  std::clog << "sendThread started" << std::endl;

  try {
    static_cast<OutputSection *>(arg)->sendThread();
  } catch (std::exception &ex) {
    std::cerr << "Caught std::exception: " << ex.what() << std::endl;
  } catch (...) {
    std::cerr << "Caught non-std::exception" << std::endl;
  }

  std::clog << "sendThread finished" << std::endl;
  return 0;
}


void OutputSection::sendThread()
{
  CorrelatedData *data;

  while ((data = itsSendQueue.remove()) != 0) {
    data->write(itsStreamToStorage);
    itsFreeQueue.append(data);
  }
}


void OutputSection::connectToStorage(const CS1_Parset *ps)
{
  unsigned myPsetIndex       = ps->outputPsetIndex(itsPsetNumber);
  unsigned nrPsetsPerStorage = ps->nrPsetsPerStorage();
  unsigned storageHostIndex  = myPsetIndex / nrPsetsPerStorage;
  unsigned storagePortIndex  = myPsetIndex % nrPsetsPerStorage;

  string   prefix	     = "OLAP.OLAP_Conn.BGLProc_Storage";
  string   connectionType    = ps->getString(prefix + "_Transport");

  if (connectionType == "NULL") {
    std::clog << "output section discards data to null:" << std::endl;
    itsStreamToStorage = new NullStream;
  } else if (connectionType == "TCP") {
    std::string    server = ps->getStringVector(prefix + "_ServerHosts")[storageHostIndex];
    unsigned short port   = boost::lexical_cast<unsigned short>(ps->getPortsOf(prefix)[storagePortIndex]);

    std::clog << "output section connects to tcp:" << server << ':' << port << std::endl;
    itsStreamToStorage = new SocketStream(server.c_str(), port, SocketStream::TCP, SocketStream::Client);
  } else if (connectionType == "FILE") {
    std::string filename = ps->getString(prefix + "_BaseFileName") + '.' +
		    boost::lexical_cast<std::string>(storageHostIndex) + '.' +
		    boost::lexical_cast<std::string>(storagePortIndex);

    std::clog << "output section write to file:" << filename << std::endl;
    itsStreamToStorage = new FileStream(filename.c_str(), 0666);
  } else {
    throw std::runtime_error("unsupported ION->Storage stream type");
  }
}


void OutputSection::preprocess(const CS1_Parset *ps)
{
  itsNrComputeCores	    = ps->nrCoresPerPset();
  itsCurrentComputeCore	    = 0;
  itsNrSubbandsPerPset	    = ps->nrSubbandsPerPset();
  itsNrIntegrationSteps     = ps->IONintegrationSteps();
  itsCurrentIntegrationStep = 0;

  connectToStorage(ps);

  unsigned nrBuffers   = itsNrSubbandsPerPset + 1 /* itsTmpSum */ + maxSendQueueSize;
  unsigned nrBaselines = ps->nrBaselines();
  unsigned arena       = 0;

  for (unsigned i = 0; i < nrBuffers; i ++)
    itsArenas.push_back(new MallocedArena(CorrelatedData::requiredSize(nrBaselines), 32));

  itsTmpSum = new CorrelatedData(*itsArenas[arena ++], nrBaselines);

  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++)
    itsVisibilitySums.push_back(new CorrelatedData(*itsArenas[arena ++], nrBaselines));

  for (unsigned i = 0; i < maxSendQueueSize; i ++)
    itsFreeQueue.append(new CorrelatedData(*itsArenas[arena ++], nrBaselines));

  if (pthread_create(&itsSendThread, 0, sendThreadStub, this) != 0)
    throw std::runtime_error("could not create send thread");
}


void OutputSection::process()
{
  bool firstTime = itsCurrentIntegrationStep == 0;
  bool lastTime  = itsCurrentIntegrationStep == itsNrIntegrationSteps - 1;

  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++) {
    // TODO: make sure that there are more free buffers than subbandsPerPset

    CorrelatedData *data   = lastTime ? itsFreeQueue.remove() : firstTime ? itsVisibilitySums[subband] : itsTmpSum;
  
    unsigned	   channel = BGL_Mapping::mapCoreOnPset(itsCurrentComputeCore, itsPsetNumber);

    data->read(itsStreamsFromCNs[channel]);

    if (!firstTime)
      if (lastTime)
	*data += *itsVisibilitySums[subband];
      else
	*itsVisibilitySums[subband] += *data;

    if (lastTime) {
#if 0
      for (unsigned ch = 1; ch < 256; ch ++)
	std::clog << std::setprecision(7) << ch << ' ' << abs(data->visibilities[0][ch][0][0]) << std::endl;
#endif

      itsSendQueue.append(data);
    }

    if (++ itsCurrentComputeCore == itsNrComputeCores)
      itsCurrentComputeCore = 0;
  }

  if (++ itsCurrentIntegrationStep == itsNrIntegrationSteps)
    itsCurrentIntegrationStep = 0;
}


void OutputSection::postprocess()
{
  itsSendQueue.append(0); // 0 indicates that no more messages will be sent

  if (pthread_join(itsSendThread, 0) != 0)
    throw std::runtime_error("could not join send thread");

  delete itsStreamToStorage; // closes stream, stopping the Storage section
  itsStreamToStorage = 0;

  for (unsigned subband = 0; subband < itsVisibilitySums.size(); subband ++)
    delete itsVisibilitySums[subband];

  itsVisibilitySums.resize(0);

  delete itsTmpSum;
  itsTmpSum = 0;

  for (unsigned i = 0; i < maxSendQueueSize; i ++)
    delete itsFreeQueue.remove();

  for (unsigned i = 0; i < itsArenas.size(); i ++)
    delete itsArenas[i];

  itsArenas.resize(0);
}

}
}
