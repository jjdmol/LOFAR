//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Transpose.h>

#include <Common/Timer.h>
#include <Transport/TH_MPI.h>
#include <CS1_Interface/BGL_Mapping.h>
#include <CS1_Interface/PrintVector.h>

#if defined HAVE_BGL
#include <rts.h>
#endif

#include <cassert>
#include <map>
#include <set>


namespace LOFAR {
namespace CS1 {

#if defined HAVE_MPI

static NSTimer transposeTimer("transpose()", true);


std::vector<MPI_Comm> Transpose::allTransposeGroups;


Transpose::Transpose(bool isTransposeInput, bool isTransposeOutput, unsigned myCore, unsigned nrStations)
:
  itsIsTransposeInput(isTransposeInput),
  itsIsTransposeOutput(isTransposeOutput),
  itsNrStations(nrStations),
  itsTransposeGroup(allTransposeGroups[myCore])
{
}


Transpose::~Transpose()
{
}


#if defined HAVE_BGL

unsigned Transpose::remapOnTree(unsigned pset, unsigned core, const struct BGLPersonality &personality)
{
  unsigned psetXsize  = personality.getXpsetSize();
  unsigned psetYsize  = personality.getYpsetSize();
  unsigned psetZsize  = personality.getZpsetSize();

  unsigned psetXcount = personality.getXsize() / psetXsize;
  unsigned psetYcount = personality.getYsize() / psetYsize;
  unsigned psetZcount = personality.getZsize() / psetZsize;

  unsigned xOrigin    = pset			       % psetXcount * psetXsize;
  unsigned yOrigin    = pset / psetXcount	       % psetYcount * psetYsize;
  unsigned zOrigin    = pset / psetXcount / psetYcount % psetZcount * psetZsize;

  unsigned nodesPerPset = personality.numNodesInPset();

  unsigned numProcs, xOffset, yOffset, zOffset, node;

  core = BGL_Mapping::mapCoreOnPset(core, pset);
  personality.coordsForPsetRank(core % nodesPerPset, xOffset, yOffset, zOffset);

  unsigned x = xOrigin + xOffset - personality.xPsetOrigin();
  unsigned y = yOrigin + yOffset - personality.yPsetOrigin();
  unsigned z = zOrigin + zOffset - personality.zPsetOrigin();
  unsigned t = core / nodesPerPset;

  rts_rankForCoordinates(x, y, z, t, &node, &numProcs);

#if defined HAVE_MPI
  if (node >= (unsigned) TH_MPI::getNumberOfNodes()) {
    std::cerr << "not enough nodes allocated (node = " << node << ", TH_MPI::getNumberOfNodes() = " << TH_MPI::getNumberOfNodes() << std::endl;
    exit(1);
  }
#endif

  return node;
}


void Transpose::getMPIgroups(unsigned nrCoresPerPset, const struct BGLPersonality &personality, const std::vector<unsigned> &inputPsets, const std::vector<unsigned> &outputPsets)
{
  allTransposeGroups.resize(nrCoresPerPset);

  MPI_Group all, group;

  if (MPI_Comm_group(MPI_COMM_WORLD, &all) != MPI_SUCCESS) {
    std::cerr << "MPI_Comm_group() failed" << std::endl;
    exit(1);
  }

  std::set<unsigned> psets; // ordered list of all psets
  std::set_union(inputPsets.begin(), inputPsets.end(),
		 outputPsets.begin(), outputPsets.end(),
		 std::insert_iterator<std::set<unsigned> >(psets, psets.begin()));

  for (unsigned core = 0; core < nrCoresPerPset; core ++) {
    std::vector<int> ranks;

    for (std::set<unsigned>::const_iterator pset = psets.begin(); pset != psets.end(); pset ++)
      ranks.push_back(remapOnTree(*pset, core, personality));

    if (TH_MPI::getCurrentRank() == 0)
      std::clog << "group " << core << " contains cores " << ranks << std::endl;

    if (MPI_Group_incl(all, ranks.size(), &ranks[0], &group) != MPI_SUCCESS) {
      std::cerr << "MPI_Group_incl() failed" << std::endl;
      exit(1);
    }

    if (MPI_Comm_create(MPI_COMM_WORLD, group, &allTransposeGroups[core]) != MPI_SUCCESS) {
      std::cerr << "MPI_Comm_create() failed" << std::endl;
      exit(1);
    }

    if (MPI_Group_free(&group) != MPI_SUCCESS) {
      std::cerr << "MPI_Group_incl() failed" << std::endl;
      exit(1);
    }
  }
}

#endif


void Transpose::setupTransposeParams(const std::vector<unsigned> &inputPsets, const std::vector<unsigned> &outputPsets, InputData *inputData, TransposedData *transposedData)
{
  std::set<unsigned> psets; // ordered list of all psets
  std::set_union(inputPsets.begin(), inputPsets.end(),
		 outputPsets.begin(), outputPsets.end(),
		 std::insert_iterator<std::set<unsigned> >(psets, psets.begin()));

  unsigned		       nrPsetsUsed = psets.size();
  std::map<unsigned, unsigned> psetToGroupIndex;
  unsigned		       groupIndex  = 0;

  for (std::set<unsigned>::const_iterator pset = psets.begin(); pset != psets.end(); pset ++, groupIndex ++)
    psetToGroupIndex[*pset] = groupIndex;

  if (TH_MPI::getCurrentRank() == 0)
    for (std::map<unsigned, unsigned>::const_iterator it = psetToGroupIndex.begin(); it != psetToGroupIndex.end(); it ++)
      std::clog << "pset " << it->first << " maps to group index " << it->second << std::endl;

  itsTransposeParams.send.counts.resize(nrPsetsUsed, 0);
  itsTransposeParams.send.displacements.resize(nrPsetsUsed);
  itsTransposeParams.receive.counts.resize(nrPsetsUsed, 0);
  itsTransposeParams.receive.displacements.resize(nrPsetsUsed);
  itsTransposeMetaParams.send.counts.resize(nrPsetsUsed, 0);
  itsTransposeMetaParams.send.displacements.resize(nrPsetsUsed);
  itsTransposeMetaParams.receive.counts.resize(nrPsetsUsed, 0);
  itsTransposeMetaParams.receive.displacements.resize(nrPsetsUsed);

  itsOutputMetaData.resize(inputPsets.size());

  if (itsIsTransposeInput) {
    for (unsigned psetIndex = 0; psetIndex < outputPsets.size(); psetIndex ++) {
      unsigned pset  = outputPsets[psetIndex];
      unsigned index = psetToGroupIndex[pset];

      if (1 /* FIXME: psetIndex % itsCS1PS->nrRSPboardsPerStation() == 0 */) {
	const boost::detail::multi_array::sub_array<InputData::SampleType, 2> &slice = inputData->samples[psetIndex];

	itsTransposeParams.send.counts[index] = slice.num_elements() * sizeof(InputData::SampleType);
	itsTransposeParams.send.displacements[index] = reinterpret_cast<const char *>(slice.origin()) - reinterpret_cast<const char *>(inputData->samples.origin());

	itsTransposeMetaParams.send.counts[index] = sizeof itsInputMetaData;
	itsTransposeMetaParams.send.displacements[index] = 0;
      }
    }
  }

  if (itsIsTransposeOutput)
    for (unsigned psetIndex = 0; psetIndex < inputPsets.size(); psetIndex ++) {
      unsigned pset  = inputPsets[psetIndex];
      unsigned index = psetToGroupIndex[pset];
      const boost::detail::multi_array::sub_array<TransposedData::SampleType, 2> &slice = transposedData->samples[psetIndex];

      itsTransposeParams.receive.counts[index] = slice.num_elements() * sizeof(TransposedData::SampleType);
      itsTransposeParams.receive.displacements[index] = reinterpret_cast<const char *>(slice.origin()) - reinterpret_cast<const char *>(transposedData->samples.origin());

      itsTransposeMetaParams.receive.counts[index] = sizeof itsInputMetaData;
      itsTransposeMetaParams.receive.displacements[index] = psetIndex * sizeof itsInputMetaData;
    }

#if 0
if (itsIsTransposeInput) std::clog << "send_base: " << inputData->samples.origin() << std::endl;
std::clog << "send_counts:";
for (unsigned pset = 0; pset < nrPsetsUsed; pset ++)
std::clog << ' ' << itsTransposeParams.send.counts[pset];
std::clog << std::endl;
std::clog << "send_displacements:";
for (unsigned pset = 0; pset < nrPsetsUsed; pset ++)
std::clog << ' ' << itsTransposeParams.send.displacements[pset];
std::clog << std::endl;
if (itsIsTransposeOutput) std::clog << "receive_base: " << transposedData->samples.origin() << std::endl;
std::clog << "receive_counts:";
for (unsigned pset = 0; pset < nrPsetsUsed; pset ++)
std::clog << ' ' << itsTransposeParams.receive.counts[pset];
std::clog << std::endl;
std::clog << "receive_displacements:";
for (unsigned pset = 0; pset < nrPsetsUsed; pset ++)
std::clog << ' ' << itsTransposeParams.receive.displacements[pset];
std::clog << std::endl;
#endif

#if 0
std::clog << "meta send_counts:";
for (unsigned pset = 0; pset < nrPsetsUsed; pset ++)
std::clog << ' ' << itsTransposeMetaParams.send.counts[pset];
std::clog << std::endl;
std::clog << "meta send_displacements:";
for (unsigned pset = 0; pset < nrPsetsUsed; pset ++)
std::clog << ' ' << itsTransposeMetaParams.send.displacements[pset];
std::clog << std::endl;
std::clog << "meta receive_counts:";
for (unsigned pset = 0; pset < nrPsetsUsed; pset ++)
std::clog << ' ' << itsTransposeMetaParams.receive.counts[pset];
std::clog << std::endl;
std::clog << "meta receive_displacements:";
for (unsigned pset = 0; pset < nrPsetsUsed; pset ++)
std::clog << ' ' << itsTransposeMetaParams.receive.displacements[pset];
std::clog << std::endl;
#endif
}


void Transpose::transpose(const InputData *inputData, TransposedData *transposedData)
{
  if (MPI_Alltoallv(
	itsIsTransposeInput ? (void *) inputData->samples.origin() : 0,
	&itsTransposeParams.send.counts[0],
	&itsTransposeParams.send.displacements[0],
	MPI_BYTE,
	itsIsTransposeOutput ? transposedData->samples.origin() : 0,
	&itsTransposeParams.receive.counts[0],
	&itsTransposeParams.receive.displacements[0],
	MPI_BYTE,
	itsTransposeGroup) != MPI_SUCCESS)
  {
    std::cerr << "MPI_Alltoallv() failed" << std::endl;
    exit(1);
  }
}


void Transpose::transposeMetaData(/*const*/ InputData *inputData, TransposedData *transposedData)
{
  if (itsIsTransposeInput) {
    itsInputMetaData.delayAtBegin   = inputData->metaData.delayAtBegin();
    itsInputMetaData.delayAfterEnd  = inputData->metaData.delayAfterEnd();
    itsInputMetaData.alignmentShift = inputData->metaData.alignmentShift();
    assert(inputData->metaData.flags().marshall(&itsInputMetaData.flagsBuffer, sizeof itsInputMetaData.flagsBuffer) >= 0);
  }

  if (MPI_Alltoallv(
	&itsInputMetaData,
	&itsTransposeMetaParams.send.counts[0],
	&itsTransposeMetaParams.send.displacements[0],
	MPI_BYTE,
	&itsOutputMetaData[0],
	&itsTransposeMetaParams.receive.counts[0],
	&itsTransposeMetaParams.receive.displacements[0],
	MPI_BYTE,
	itsTransposeGroup) != MPI_SUCCESS)
  {
    std::cerr << "MPI_Alltoallv() failed" << std::endl;
    exit(1);
  }

  if (itsIsTransposeOutput) {
    for (unsigned station = 0; station < itsNrStations; station ++) {
      transposedData->delays[station].delayAtBegin  = itsOutputMetaData[station].delayAtBegin;
      transposedData->delays[station].delayAfterEnd = itsOutputMetaData[station].delayAfterEnd;
      transposedData->alignmentShifts[station]      = itsOutputMetaData[station].alignmentShift;
      transposedData->flags[station].unmarshall(itsOutputMetaData[station].flagsBuffer);
    }
  }
}

#endif

} // namespace CS1
} // namespace LOFAR
