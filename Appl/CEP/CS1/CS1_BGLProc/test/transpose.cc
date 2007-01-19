//#  transpose.cc: test transpose on BG/L torus
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

#include <lofar_config.h>

#include <APS/ParameterSet.h>
#include <Common/lofar_complex.h>
#include <Common/Timer.h>
#include <PLC/ACCmain.h>
#include <tinyCEP/ApplicationHolderController.h>
#include <tinyCEP/TinyApplicationHolder.h>
#include <tinyCEP/WorkHolder.h>
#include <Transport/BGLConnection.h>
#include <Transport/DataHolder.h>
#include <Transport/TH_MPI.h>

#include <bglpersonality.h>
#include <rts.h>

#include <exception>
#include <iostream>
#include <string>
#include <vector>


namespace LOFAR {
namespace CS1 {


class DH_Station : public DataHolder
{
  public:
    typedef i4complex SampleType;

    DH_Station(const string &name, const ACC::APS::ParameterSet &ps);

    DataHolder	 *clone() const;
    virtual void init();
    virtual void fillDataPointers();

  private:
    SampleType	 *itsSamples;
    unsigned	 itsNrSamples;
};


DH_Station::DH_Station(const string &name, const ACC::APS::ParameterSet &ps)
:
  DataHolder(name, "DH_Station")
{
  itsNrSamples = ps.getUint32("Observation.NSubbandSamples") * ps.getUint32("Observation.NPolarisations");
}


DataHolder *DH_Station::clone() const
{
  return new DH_Station(*this);
}


void DH_Station::init()
{
  addField("Samples", BlobField<uint8>(1, itsNrSamples * sizeof(SampleType)), 32);
  createDataBlock(); // calls fillDataPointers
}


void DH_Station::fillDataPointers()
{
  itsSamples = (SampleType *) getData<uint8>("Samples");
}


class Position
{
  public:
    Position(unsigned x, unsigned y, unsigned z, unsigned t);
    Position(unsigned rank);

    unsigned		rank() const;
    unsigned		psetNumber() const;
    Position		psetBase() const;
    static Position	psetBase(unsigned psetNumber);
    Position		positionInPset(unsigned index) const;
    unsigned		indexInPset() const;

    unsigned		x, y, z, t;
    static unsigned	xSize, ySize, zSize;
};


unsigned Position::xSize, Position::ySize, Position::zSize;


Position::Position(unsigned x, unsigned y, unsigned z, unsigned t)
:
  x(x), y(y), z(z), t(t)
{
}


Position::Position(unsigned rank)
{
  if (rts_coordinatesForRank(rank, &x, &y, &z, &t) != 0) {
    cerr << "error calling rts_coordinatesForRank" << endl;
    exit(1);
  }
}


unsigned Position::rank() const
{
  unsigned rank, numProcs;

  if (rts_rankForCoordinates(x, y, z, t, &rank, &numProcs) != 0) {
    cerr << "error calling rts_rankForCoordinates" << endl;
    exit(1);
  }

  return rank;
}


unsigned Position::psetNumber() const
{
  return (x / 2) + (xSize / 2) * ((y / 2) + (ySize / 2) * (z / 2));
}


Position Position::psetBase() const
{
  return Position(x & ~1, y & ~1, z & ~1, 0);
}


Position Position::psetBase(unsigned psetNumber)
{
  return Position(2 * (psetNumber % (xSize / 2)), 
		  2 * (psetNumber / (xSize / 2) % (ySize / 2)),
		  2 * (psetNumber / (xSize / 2) / (ySize / 2)),
		  0);
}


Position Position::positionInPset(unsigned index) const
{
  Position base = psetBase();
  return Position(base.x + index % 2, base.y + index / 2 % 2, base.z + index / 4 % 2, base.t + index / 8 % 2);
}


unsigned Position::indexInPset() const
{
  return (x % 2) + 2 * (y % 2) + 4 * (z % 2) + 8 * (t % 2);
}




class WH_Transpose : public WorkHolder
{
  public:
    WH_Transpose(const string &name, const ACC::APS::ParameterSet &ps, unsigned rank);

    virtual void preprocess();
    virtual void process();

  private:
    virtual WorkHolder		 *make(const string &name);

    void			 sendToAll();
    void			 receiveFromAll();

    const ACC::APS::ParameterSet &itsParamSet;
    unsigned			 itsNrStations;
    unsigned			 itsCoreNumber, itsPsetNumber, itsPsetIndex;
    unsigned			 itsPhase;
};


WH_Transpose::WH_Transpose(const string &name, const ACC::APS::ParameterSet &ps, unsigned rank)
:
  WorkHolder(ps.getUint32("Observation.NStations"),
	     ps.getUint32("Observation.NStations"),
	     name, string("WH_Transpose")),
  itsParamSet(ps),
  itsNrStations(ps.getUint32("Observation.NStations")),
  itsCoreNumber(rank),
  itsPsetNumber(Position(rank).psetNumber()),
  itsPsetIndex(Position(rank).indexInPset()),
  itsPhase(itsPsetIndex)
{
  TinyDataManager &dm = getDataManager();

  for (unsigned i = 0; i < itsNrStations; i ++) {
    dm.addInDataHolder(i, new DH_Station("input", ps));
    dm.addOutDataHolder(i, new DH_Station("input", ps));
  }
}


void WH_Transpose::preprocess()
{
}


void WH_Transpose::process()
{
  NSTimer	 transposeTimer("transpose", itsCoreNumber == 0);
  static NSTimer totalTimer("total", itsCoreNumber == 0);

  TH_MPI::synchroniseAllProcesses();
  totalTimer.start();
  transposeTimer.start();

  if (itsPhase == 2)
    receiveFromAll();

  if (itsPhase == 1)
    sendToAll();

  TH_MPI::synchroniseAllProcesses();
  transposeTimer.stop();
  totalTimer.stop();

  ++ itsPhase, itsPhase %= 16;
}


void WH_Transpose::sendToAll()
{
  unsigned station = itsPsetNumber, i = 0;

  do {
    unsigned psetIndex = (itsPsetIndex + 1) % 16;
    unsigned dest      = Position::psetBase(station).positionInPset(psetIndex).rank();
    getDataManager().getOutHolder(i);
    ((unsigned *) getDataManager().getOutHolder(i)->getDataPtr())[100] = itsCoreNumber;
    //clog << itsCoreNumber << " sends message to " << dest << ", size = " << getDataManager().getOutHolder(i)->getDataSize() << endl;
    //NSTimer timer("send timer", true);
    //timer.start();
    getDataManager().readyWithOutHolder(i);
    //timer.stop();
  } while (++ i, ++ station, station %= itsNrStations, station != itsPsetNumber);
}

void WH_Transpose::receiveFromAll()
{
  unsigned station = itsPsetNumber, i = 0;

  do {
    unsigned psetIndex = (itsPsetIndex - 1) % 16;
    unsigned source    = Position::psetBase(station).positionInPset(psetIndex).rank();
    getDataManager().getInHolder(i);
    getDataManager().readyWithInHolder(i);
    //clog << itsCoreNumber << " received message from " << source << ", value = " << ((unsigned *) getDataManager().getInHolder(i)->getDataPtr())[100] << endl;
  } while (++ i, ++ station, station %= itsNrStations, station != itsPsetNumber);
}


WorkHolder *WH_Transpose::make(const string &name)
{
  return new WH_Transpose(name, itsParamSet, itsCoreNumber);
}




class WH_Idle : public WorkHolder
{
  public:
    WH_Idle(const string &name);

    virtual void process();

  private:
    virtual WorkHolder *make(const string &name);
};


WH_Idle::WH_Idle(const string &name)
:
  WorkHolder(0, 0, name, string("idle"))
{
}


WorkHolder *WH_Idle::make(const string &name)
{
  return new WH_Idle(name);
}


void WH_Idle::process()
{
  TH_MPI::synchroniseAllProcesses();
  TH_MPI::synchroniseAllProcesses();
}



class AH_Transpose : public TinyApplicationHolder
{
  public:
    virtual void define(const KeyValueMap &);
    virtual void init();
    virtual void run(int nsteps);

  private:
    vector<WorkHolder *> itsWHs;
};


void AH_Transpose::define(const KeyValueMap &)
{
  struct BGLPersonality personality;

  if (rts_get_personality(&personality, sizeof personality) != 0) {
    cout << "could not get personality" << endl;
    exit(1);
  }

  Position::xSize = personality.getXsize();
  Position::ySize = personality.getYsize();
  Position::zSize = personality.getZsize();

  //clog << itsCoreNumber << " at (" << personality.getXcoord() << ',' << personality.getYcoord() << ',' << personality.getZcoord() << "), phase = " << itsPhase << endl;

  unsigned nrStations = itsParamSet.getUint32("Observation.NStations");
  unsigned nrNodes    = TH_MPI::getNumberOfNodes();

  if (16 * nrStations > nrNodes) {
    if (TH_MPI::getCurrentRank() == 0)
      cerr << "Too many stations for number of nodes" << endl;

    exit(1);
  }

  for (int rank = 0; rank < nrNodes; rank ++) {
    WorkHolder *wh = Position(rank).psetNumber() < nrStations ?
      (WorkHolder *) new WH_Transpose("WH_Transpose", itsParamSet, rank) :
      (WorkHolder *) new WH_Idle("WH_Idle");
    wh->runOnNode(rank);
    itsWHs.push_back(wh);
  }

  for (unsigned sourcePset = 0; sourcePset < nrStations; sourcePset ++) {
    for (unsigned sourcePsetIndex = 0; sourcePsetIndex < 16; sourcePsetIndex ++) {
      for (unsigned destPset = 0; destPset < nrStations; destPset ++) {
	unsigned	destPsetIndex = (sourcePsetIndex + 1) % 16;
	unsigned	source	      = Position::psetBase(sourcePset).positionInPset(sourcePsetIndex).rank();
	unsigned	dest	      = Position::psetBase(destPset).positionInPset(destPsetIndex).rank();
	unsigned	channel	      = (destPset - sourcePset + nrStations) % nrStations; // unsigned modulo

	TH_MPI		*th	      = new TH_MPI(source, dest);
	TinyDataManager &sourceDM     = itsWHs[source]->getDataManager();
	TinyDataManager &destDM       = itsWHs[dest]->getDataManager();
	BGLConnection	*connection   = new BGLConnection("mpi", sourceDM.getGeneralOutHolder(channel), destDM.getGeneralInHolder(channel), th);

	sourceDM.setOutConnection(channel, connection);
	sourceDM.setAutoTriggerOut(channel, false);
	destDM.setInConnection(channel, connection);
	destDM.setAutoTriggerIn(channel, false);
      }
    }
  }
}


void AH_Transpose::init()
{
  for (unsigned i = 0; i < itsWHs.size(); i ++)
    itsWHs[i]->basePreprocess();
}


void AH_Transpose::run(int nsteps)
{
  for (int i = 0; i < nsteps; i ++)
    for (unsigned j = 0; j < itsWHs.size(); j ++)
      itsWHs[j]->baseProcess();
}


} // namespace CS1
} // namespace LOFAR

using namespace LOFAR;
using namespace LOFAR::CS1;

int main(int argc, char **argv)
{
  int retval;

  try {
    AH_Transpose myAH;
    ApplicationHolderController myAHController(myAH, 1); //listen to ACC every 1 runs
    retval = ACC::PLC::ACCmain(argc, argv, &myAHController);
  } catch (Exception &e) {
    std::cerr << "Caught exception: " << e.what() << endl;
    retval = 1;
  } catch (std::exception &e) {
    std::cerr << "Caught exception: " << e.what() << endl;
    retval = 1;
  }

  return retval;
}
