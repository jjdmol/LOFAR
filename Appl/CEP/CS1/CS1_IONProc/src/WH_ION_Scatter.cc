//#  WH_ION_Scatter.cc: Blue Gene processing for 1 second of sampled data
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

#include <CS1_IONProc/ION_Allocator.h>
#include <CS1_IONProc/WH_ION_Scatter.h>
#include <CS1_IONProc/TH_ZoidServer.h>

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


WH_ION_Scatter::WH_ION_Scatter(const string &name, const CS1_Parset *ps)
:
  WorkHolder(1, 0, name, "WH_ION_Scatter"),
  itsCurrentComputeNode(0),
  itsPS(ps)
{
  itsNrComputeNodes = ps->getUint32("OLAP.BGLProc.nodesPerPset");
  TinyDataManager &dm = getDataManager();
  DH_Subband *dh = new DH_Subband("input", ps);
  // prereserve some space for ExtraBlob, to avoid memory fragmentation
  dh->setMaxDataSize(ps->nrStations() * 1024, true);
  dh->setAllocationProperties(false, BlobStringType(false, ION_Allocator()));
  dm.addInDataHolder(0, dh);
  //dm.setAutoTriggerIn(0, false);
}


WH_ION_Scatter::~WH_ION_Scatter()
{
}


#if 0
WorkHolder* WH_ION_Scatter::construct(const string &name, const ACC::APS::ParameterSet &ps)
{
  return new WH_ION_Scatter(name, ps);
}
#endif


WH_ION_Scatter* WH_ION_Scatter::make(const string &name)
{
  return new WH_ION_Scatter(name, itsPS);
}


void WH_ION_Scatter::preprocess()
{
}


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


void WH_ION_Scatter::process()
{
  DH_Subband *dh = dynamic_cast<DH_Subband *>(getDataManager().getInHolder(0));
  dh->getExtraData();
  dh->fillExtraData();
  dh->pack();
  //std::clog.precision(7);
  //std::clog << getTime() << ": thread " << itsCurrentComputeNode << " received write right" << std::endl;
  TH_ZoidServer::theirTHs[itsCurrentComputeNode]->sendBlocking(dh->getDataPtr(), (dh->getDataSize() + 15) & ~15, 0, dh);
  //std::clog << getTime() << ": thread " << itsCurrentComputeNode << " releases write right" << std::endl;

  if (++ itsCurrentComputeNode == itsNrComputeNodes)
    itsCurrentComputeNode = 0;
}


void WH_ION_Scatter::postprocess()
{
}

}
}
