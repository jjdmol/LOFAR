//#  WH_ION_Gather.cc: Blue Gene processing for 1 second of sampled data
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
#include <ION_Allocator.h>
#include <WH_ION_Gather.h>
#include <TH_ZoidServer.h>

#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


namespace LOFAR {
namespace CS1 {

WH_ION_Gather::WH_ION_Gather(const string &name, unsigned psetNumber, const CS1_Parset *ps)
:
  WorkHolder(0, 1, name, "WH_ION_Gather"),
  itsPsetNumber(psetNumber),
  itsPS(ps)
{
  itsTmpDH		    = 0;
  itsNrComputeCores	    = ps->nrCoresPerPset();
  itsCurrentComputeCore	    = 0;
  itsNrSubbandsPerPset	    = ps->nrSubbandsPerPset();
  itsCurrentSubband	    = 0;
  itsNrIntegrationSteps     = ps->IONintegrationSteps();
  itsCurrentIntegrationStep = 0;

  TinyDataManager &dm = getDataManager();
  DH_Visibilities *dh = new DH_Visibilities("output", ps);
  dh->setAllocationProperties(false, BlobStringType(false, ION_Allocator()));
  dm.addOutDataHolder(0, dh);
  dm.setAutoTriggerOut(0, false);

#if 0
  for (unsigned i = 0; i < itsNrComputeCores; i ++) {
    dm.addInDataHolder(i, new DH_Visibilities("input", ps));
    dm.setAutoTriggerIn(i, false);
  }
#endif
}


WH_ION_Gather::~WH_ION_Gather()
{
}


#if 0
WorkHolder* WH_ION_Gather::construct(const string &name, const ACC::APS::ParameterSet &ps)
{
  return new WH_ION_Gather(name, ps);
}
#endif


WH_ION_Gather* WH_ION_Gather::make(const string &name)
{
  return new WH_ION_Gather(name, itsPsetNumber, itsPS);
}


void WH_ION_Gather::preprocess()
{
  if (itsNrIntegrationSteps > 1)
    for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++) {
      DH_Visibilities *dh = new DH_Visibilities("sum", itsPS);
      dh->setAllocationProperties(false, BlobStringType(false, ION_Allocator()));
      dh->init();
      itsSumDHs.push_back(dh);
    }

  itsTmpDH = new DH_Visibilities("tmp", itsPS);
  itsTmpDH->setAllocationProperties(false, BlobStringType(false, ION_Allocator()));
  itsTmpDH->init();
}


#if 0
static double getTime()
{
  struct timeval tv;
  static double  first_time = 0.0;

  if (gettimeofday(&tv, 0) != 0) {
    perror("gettimeofday");
    tv.tv_sec = tv.tv_usec = 0;
  }

  double time = tv.tv_sec + tv.tv_usec / 1.0e6;

  if (first_time == 0)
    first_time = time;

  return time - first_time;
}
#endif


void WH_ION_Gather::process()
{
  bool firstTime = itsCurrentIntegrationStep == 0;
  bool lastTime  = itsCurrentIntegrationStep == itsNrIntegrationSteps - 1;

  //std::clog << "itsCurrentComputeCore = " << itsCurrentComputeCore << ", itsCurrentSubband = " << itsCurrentSubband << ", itsCurrentIntegrationStep = " << itsCurrentIntegrationStep << ", firstTime = " << firstTime << ", lastTime = " << lastTime << std::endl;
  DH_Visibilities *dh = lastTime ? dynamic_cast<DH_Visibilities *>(getDataManager().getOutHolder(0)) : firstTime ? itsSumDHs[itsCurrentSubband] : itsTmpDH;
  
  unsigned channel = BGL_Mapping::mapCoreOnPset(itsCurrentComputeCore, itsPsetNumber);
  //TH_ZoidServer::theirTHs[channel]->recvBlocking(dh->getDataPtr(), (dh->getDataSize() + 31) & ~31, 0, 0, dh);
  TH_ZoidServer::theirTHs[channel]->recvBlocking(dh->getVisibilities().origin(), dh->getVisibilities().num_elements() * sizeof(fcomplex), 0, 0, 0);
  TH_ZoidServer::theirTHs[channel]->recvBlocking(dh->getNrValidSamples().origin(), dh->getNrValidSamples().num_elements() * sizeof(unsigned short), 0, 0, 0);

  if (!firstTime)
    if (lastTime)
      *dh += *itsSumDHs[itsCurrentSubband];
    else
      *itsSumDHs[itsCurrentSubband] += *itsTmpDH;

  if (lastTime)
    getDataManager().readyWithOutHolder(0);

  if (++ itsCurrentComputeCore == itsNrComputeCores)
    itsCurrentComputeCore = 0;

  if (++ itsCurrentSubband == itsNrSubbandsPerPset) {
    itsCurrentSubband = 0;

    if (++ itsCurrentIntegrationStep == itsNrIntegrationSteps)
      itsCurrentIntegrationStep = 0;
  }
}


void WH_ION_Gather::postprocess()
{
  for (unsigned subband = 0; subband < itsSumDHs.size(); subband ++)
    delete itsSumDHs[subband];

  itsSumDHs.resize(0);
  delete itsTmpDH;
}

}
}
