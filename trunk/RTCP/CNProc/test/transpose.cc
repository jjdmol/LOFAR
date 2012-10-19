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

#if defined HAVE_MPI
#include <Common/ParameterSet.h>
#include <Common/lofar_complex.h>
#include <Common/Timer.h>
#include <PLC/ACCmain.h>

#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include <boost/multi_array.hpp>


#define SIMULATE_PSETS

#define NR_POLARIZATIONS 2


namespace LOFAR {
namespace RTCP {


class DH_RSP : public DataHolder
{
  public:
    typedef i4complex SampleType;

    DH_RSP(const string &name, const ParameterSet &ps);

    DataHolder	 *clone() const;
    virtual void init();
    virtual void fillDataPointers();

  private:
    SampleType	 *itsSamples;
    unsigned	 itsNrSamples;
};


DH_RSP::DH_RSP(const string &name, const ParameterSet &ps)
:
  DataHolder(name, "DH_RSP")
{
  itsNrSamples = ps.getUint32("Observation.NSubbandSamples") * ps.getUint32("Observation.NPolarisations");
}


DataHolder *DH_RSP::clone() const
{
  return new DH_RSP(*this);
}


void DH_RSP::init()
{
  addField("Samples", BlobField<uint8>(1, itsNrSamples * sizeof(SampleType)), 32);
  createDataBlock(); // calls fillDataPointers
}


void DH_RSP::fillDataPointers()
{
  itsSamples = (SampleType *) getData<uint8>("Samples");
}


class Position
{
  public:
    Position(unsigned x, unsigned y, unsigned z, unsigned t);
    Position(unsigned rank);

#if defined SIMULATE_PSETS
    static const unsigned	psetSize = 16;
#else
    static const unsigned	psetSize = 1;
#endif

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
#if defined SIMULATE_PSETS
  if (rts_coordinatesForRank(rank, &x, &y, &z, &t) != 0) {
    cerr << "error calling rts_coordinatesForRank" << endl;
    exit(1);
  }
#else
  x = rank;
  y = z = t = 0;
#endif
}


unsigned Position::rank() const
{
#if defined SIMULATE_PSETS
  unsigned rank, numProcs;

  if (rts_rankForCoordinates(x, y, z, t, &rank, &numProcs) != 0) {
    cerr << "error calling rts_rankForCoordinates" << endl;
    exit(1);
  }

  return rank;
#else
  return x;
#endif
}


unsigned Position::psetNumber() const
{
#if defined SIMULATE_PSETS
  return (x / 2) + (xSize / 2) * ((y / 2) + (ySize / 2) * (z / 2));
#else
  return x;
#endif
}


Position Position::psetBase() const
{
#if defined SIMULATE_PSETS
  return Position(x & ~1, y & ~1, z & ~1, 0);
#else
  return *this;
#endif
}


Position Position::psetBase(unsigned psetNumber)
{
#if defined SIMULATE_PSETS
  return Position(2 * (psetNumber % (xSize / 2)), 
		  2 * (psetNumber / (xSize / 2) % (ySize / 2)),
		  2 * (psetNumber / (xSize / 2) / (ySize / 2)),
		  0);
#else
  return Position(psetNumber);
#endif
}


Position Position::positionInPset(unsigned index) const
{
#if defined SIMULATE_PSETS
  Position base = psetBase();
  return Position(base.x + index % 2, base.y + index / 2 % 2, base.z + index / 4 % 2, base.t + index / 8 % 2);
#else
  return *this;
#endif
}


unsigned Position::indexInPset() const
{
#if defined SIMULATE_PSETS
  return (x % 2) + 2 * (y % 2) + 4 * (z % 2) + 8 * (t % 2);
#else
  return 0;
#endif
}




class WH_Transpose : public WorkHolder
{
  public:
    typedef i4complex SampleType;

    WH_Transpose(const string &name, const ParameterSet &ps, unsigned rank, MPI_Comm comm);

    virtual void preprocess();
    virtual void process();
    virtual void postprocess();

  private:
    virtual WorkHolder		 *make(const string &name);
    bool			 isInput() const, isOutput() const;

    void			 allToAll();

    const ParameterSet &itsParamSet;
    unsigned			 itsCoreNumber, itsPsetNumber, itsPsetIndex;
    unsigned			 itsNrStations, itsNrCorrelatorPsets, itsNrPsets;
    unsigned			 itsPhase;
    unsigned			 itsNrSamplesPerIntegration;

    boost::multi_array<SampleType, 3> *itsInData, *itsOutData;

    MPI_Comm			 itsMPIcomm;
};


WH_Transpose::WH_Transpose(const string &name, const ParameterSet &ps, unsigned rank, MPI_Comm communicator)
:
  WorkHolder(0, 0, name, string("WH_Transpose")),
  itsParamSet(ps),
  itsCoreNumber(rank),
  itsPsetNumber(Position(rank).psetNumber()),
  itsPsetIndex(Position(rank).indexInPset()),
  itsNrStations(ps.getUint32("Observation.NStations")),
  itsNrCorrelatorPsets(ps.getUint32("Observation.NSubbands") / ps.getUint32("General.SubbandsPerPset")),
  itsNrPsets(std::max(itsNrStations, itsNrCorrelatorPsets)),
  itsPhase(itsPsetIndex),
  itsNrSamplesPerIntegration(ps.getUint32("Observation.NSubbandSamples")),
  itsMPIcomm(communicator)
{
}


inline bool WH_Transpose::isInput() const
{
  return itsPsetNumber < itsNrStations;
}


inline bool WH_Transpose::isOutput() const
{
  return itsPsetNumber < itsNrCorrelatorPsets;
}


void WH_Transpose::preprocess()
{
  if (isInput())
    itsInData  = new boost::multi_array<SampleType, 3>(boost::extents[itsNrCorrelatorPsets][itsNrSamplesPerIntegration][NR_POLARIZATIONS]);

  if (isOutput())
    itsOutData = new boost::multi_array<SampleType, 3>(boost::extents[itsNrStations][itsNrSamplesPerIntegration][NR_POLARIZATIONS]);
}


void WH_Transpose::process()
{
  NSTimer	 transposeTimer("transpose", itsCoreNumber == 0);
  static NSTimer totalTimer("total", itsCoreNumber == 0);

  TH_MPI::synchroniseAllProcesses();
  totalTimer.start();
  transposeTimer.start();

  if (itsPhase == 0)
    allToAll();

  TH_MPI::synchroniseAllProcesses();
  transposeTimer.stop();
  totalTimer.stop();

  ++ itsPhase, itsPhase %= Position::psetSize;
}


void WH_Transpose::allToAll()
{
  int sendCounts[itsNrPsets], sendDisplacements[itsNrPsets];
  int receiveCounts[itsNrPsets], receiveDisplacements[itsNrPsets];

  for (unsigned pset = 0; pset < itsNrPsets; pset ++) {
    if (isInput() && pset < itsNrCorrelatorPsets) {
      sendCounts[pset] = (*itsInData)[pset].num_elements() * sizeof(SampleType);
      sendDisplacements[pset] = ((*itsInData)[pset].origin() - itsInData->origin()) / sizeof(SampleType);
    } else {
      sendCounts[pset] = 0;
      sendDisplacements[pset] = 0;
    }

    if (isOutput() && pset < itsNrStations) {
      receiveCounts[pset] = (*itsOutData)[pset].num_elements() * sizeof(SampleType);
      receiveDisplacements[pset] = ((*itsOutData)[pset].origin() - itsOutData->origin()) / sizeof(SampleType);
    } else {
      receiveCounts[pset] = 0;
      receiveDisplacements[pset] = 0;
    }
  }

  if (MPI_Alltoallv(isInput() ? itsInData->origin() : 0,
	sendCounts, sendDisplacements, MPI_BYTE,
	isOutput() ? itsOutData->origin() : 0,
	receiveCounts, receiveDisplacements, MPI_BYTE,
	itsMPIcomm) != MPI_SUCCESS)
  {
    std::cerr << "MPI_Alltoallv() failed" << std::endl;
    exit(1);
  }
}


void WH_Transpose::postprocess()
{
  if (isInput())
    delete itsInData;

  if (isOutput())
    delete itsOutData;
}

WorkHolder *WH_Transpose::make(const string &name)
{
  return new WH_Transpose(name, itsParamSet, itsCoreNumber, itsMPIcomm);
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
  Position::xSize = TH_MPI::getNumberOfNodes();

  unsigned nrStations = itsParamSet.getUint32("Observation.NStations");
  unsigned nrNodes    = TH_MPI::getNumberOfNodes();
  unsigned nrCorrelatorPsets = itsParamSet.getUint32("Observation.NSubbands") / itsParamSet.getUint32("General.SubbandsPerPset");

  if (Position::psetSize * nrStations > nrNodes) {
    if (TH_MPI::getCurrentRank() == 0)
      cerr << "Too many stations for number of nodes" << endl;

    exit(1);
  }

  if (Position::psetSize * nrCorrelatorPsets > nrNodes) {
    if (TH_MPI::getCurrentRank() == 0)
      cerr << "Too many subbands divided over too few psets" << endl;

    exit(1);
  }

  unsigned nrPsetsNeeded = std::max(nrStations, nrCorrelatorPsets);

  MPI_Group all, group;
  MPI_Comm  comms[Position::psetSize];

  if (MPI_Comm_group(MPI_COMM_WORLD, &all) != MPI_SUCCESS) {
    std::cerr << "MPI_Comm_group() failed" << std::endl;
    exit(1);
  }

  for (unsigned psetIndex = 0; psetIndex < Position::psetSize; psetIndex ++) {
    int ranks[nrPsetsNeeded];

    for (unsigned pset = 0; pset < nrPsetsNeeded; pset ++)
      ranks[pset] = Position::psetBase(pset).positionInPset(psetIndex).rank();

    if (MPI_Group_incl(all, nrPsetsNeeded, ranks, &group) != MPI_SUCCESS) {
      std::cerr << "MPI_Group_incl() failed" << std::endl;
      exit(1);
    }

    if (MPI_Comm_create(MPI_COMM_WORLD, group, &comms[psetIndex]) != MPI_SUCCESS) {
      std::cerr << "MPI_Comm_create() failed" << std::endl;
      exit(1);
    }
  }

  for (unsigned rank = 0; rank < nrNodes; rank ++) {
    WorkHolder *wh = Position(rank).psetNumber() < nrPsetsNeeded ?
      (WorkHolder *) new WH_Transpose("WH_Transpose", itsParamSet, rank, comms[Position(rank).indexInPset()]) :
      (WorkHolder *) new WH_Idle("WH_Idle");
    wh->runOnNode(rank);
    itsWHs.push_back(wh);
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


} // namespace RTCP
} // namespace LOFAR

using namespace LOFAR;
using namespace LOFAR::RTCP;

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

#else // !defined HAVE_MPI

int main()
{
  return 0;
}

#endif
