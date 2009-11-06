//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Transpose.h>

#include <Common/Timer.h>
#include <Interface/CN_Mapping.h>
#include <Interface/PrintVector.h>

#include <cassert>
#include <map>
#include <set>


namespace LOFAR {
namespace RTCP {

#if defined HAVE_MPI

static NSTimer transposeTimer("transpose()", true);


template <typename SAMPLE_TYPE> std::vector<MPI_Comm> Transpose<SAMPLE_TYPE>::allTransposeGroups;


template <typename SAMPLE_TYPE> Transpose<SAMPLE_TYPE>::Transpose(bool isTransposeInput, bool isTransposeOutput, unsigned myCore)
:
  itsIsTransposeInput(isTransposeInput),
  itsIsTransposeOutput(isTransposeOutput),
  itsTransposeGroup(allTransposeGroups[myCore])
{
}


template <typename SAMPLE_TYPE> Transpose<SAMPLE_TYPE>::~Transpose()
{
}


#if defined HAVE_BGL || defined HAVE_BGP

template <typename SAMPLE_TYPE> unsigned Transpose<SAMPLE_TYPE>::remapOnTree(unsigned pset, unsigned core, const std::vector<unsigned> &psetNumbers)
{
  core = CN_Mapping::mapCoreOnPset(core, pset);

  for (unsigned rank = 0;; rank ++)
    if (psetNumbers[rank] == pset && core -- == 0)
      return rank;
}


template <typename SAMPLE_TYPE> void Transpose<SAMPLE_TYPE>::getMPIgroups(unsigned nrCoresPerPset, const LocationInfo &locationInfo, const std::vector<unsigned> &inputPsets, const std::vector<unsigned> &outputPsets)
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

    for (std::set<unsigned>::const_iterator pset = psets.begin(); pset != psets.end(); pset ++) {
      ranks.push_back(locationInfo.remapOnTree(*pset, core));
    }

    if (locationInfo.rank() == 0) {
      std::clog << "Transpose :: group " << core << " contains cores " << ranks << std::endl;
    }

    if (MPI_Group_incl(all, ranks.size(), &ranks[0], &group) != MPI_SUCCESS) {
      std::cerr << "MPI_Group_incl() failed" << std::endl;
      exit(1);
    }

    if (MPI_Comm_create(MPI_COMM_WORLD, group, &allTransposeGroups[core]) != MPI_SUCCESS) {
      std::cerr << "MPI_Comm_create() failed" << std::endl;
      exit(1);
    }

    if (MPI_Group_free(&group) != MPI_SUCCESS) {
      std::cerr << "MPI_Group_free() failed" << std::endl;
      exit(1);
    }
  }
}

#endif


template <typename SAMPLE_TYPE> void Transpose<SAMPLE_TYPE>::setupTransposeParams(const LocationInfo &locationInfo, const std::vector<unsigned> &inputPsets, const std::vector<unsigned> &outputPsets, InputData<SAMPLE_TYPE> *inputData, TransposedData<SAMPLE_TYPE> *transposedData)
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

  if (locationInfo.rank() == 0)
    for (std::map<unsigned, unsigned>::const_iterator it = psetToGroupIndex.begin(); it != psetToGroupIndex.end(); it ++)
      std::clog << "pset " << it->first << " maps to group index " << it->second << std::endl;

  itsTransposeParams.send.counts.resize(nrPsetsUsed, 0);
  itsTransposeParams.send.displacements.resize(nrPsetsUsed);
  itsTransposeParams.receive.counts.resize(nrPsetsUsed, 0);
  itsTransposeParams.receive.displacements.resize(nrPsetsUsed);
  itsTransposeParams.receive.psetIndex.resize(nrPsetsUsed);

  itsTransposeMetaParams.send.counts.resize(nrPsetsUsed, 0);
  itsTransposeMetaParams.send.displacements.resize(nrPsetsUsed);
  itsTransposeMetaParams.receive.counts.resize(nrPsetsUsed, 0);
  itsTransposeMetaParams.receive.displacements.resize(nrPsetsUsed);

  if (itsIsTransposeInput)
    for (unsigned psetIndex = 0; psetIndex < outputPsets.size(); psetIndex ++) {
      unsigned pset  = outputPsets[psetIndex];
      unsigned index = psetToGroupIndex[pset];

      const boost::detail::multi_array::sub_array<SAMPLE_TYPE, 2> &slice = inputData->samples[psetIndex];

      itsTransposeParams.send.counts[index] = slice.num_elements() * sizeof(SAMPLE_TYPE);
      itsTransposeParams.send.displacements[index] = reinterpret_cast<const char *>(slice.origin()) - reinterpret_cast<const char *>(inputData->samples.origin());

      itsTransposeMetaParams.send.counts[index] = sizeof(SubbandMetaData);
      itsTransposeMetaParams.send.displacements[index] = reinterpret_cast<const char *>(&inputData->metaData[psetIndex]) - reinterpret_cast<const char *>(&inputData->metaData[0]);
    }

  if (itsIsTransposeOutput)
    for (unsigned psetIndex = 0; psetIndex < inputPsets.size(); psetIndex ++) {
      unsigned pset  = inputPsets[psetIndex];
      unsigned index = psetToGroupIndex[pset];
      const boost::detail::multi_array::sub_array<SAMPLE_TYPE, 2> &slice = transposedData->samples[psetIndex];

      itsTransposeParams.receive.counts[index] = slice.num_elements() * sizeof(SAMPLE_TYPE);
      itsTransposeParams.receive.displacements[index] = reinterpret_cast<const char *>(slice.origin()) - reinterpret_cast<const char *>(transposedData->samples.origin());
      itsTransposeParams.receive.psetIndex[index] = psetIndex;

      itsTransposeMetaParams.receive.counts[index] = sizeof(SubbandMetaData);
      itsTransposeMetaParams.receive.displacements[index] = reinterpret_cast<const char *>(&transposedData->metaData[psetIndex]) - reinterpret_cast<const char *>(&transposedData->metaData[0]);
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


template <typename SAMPLE_TYPE> void Transpose<SAMPLE_TYPE>::transpose(const InputData<SAMPLE_TYPE> *inputData, TransposedData<SAMPLE_TYPE> *transposedData)
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


template <typename SAMPLE_TYPE> void Transpose<SAMPLE_TYPE>::transposeMetaData(const InputData<SAMPLE_TYPE> *inputData, TransposedData<SAMPLE_TYPE> *transposedData)
{
#if 0
  // no need to marshall itsInputMetaData; it has not been unmarshalled
  // after reading from ION
#endif

  if (MPI_Alltoallv(
	itsIsTransposeInput ? (void *) &inputData->metaData[0] : 0,
	&itsTransposeMetaParams.send.counts[0],
	&itsTransposeMetaParams.send.displacements[0],
	MPI_BYTE,
	itsIsTransposeOutput ? &transposedData->metaData[0] : 0,
	&itsTransposeMetaParams.receive.counts[0],
	&itsTransposeMetaParams.receive.displacements[0],
	MPI_BYTE,
	itsTransposeGroup) != MPI_SUCCESS)
  {
    std::cerr << "MPI_Alltoallv() failed" << std::endl;
    exit(1);
  }
  
#if 0
  if (itsIsTransposeOutput)
    for (unsigned station = 0; station < transposedData->metaData.size(); station ++)
      transposedData->metaData[station].unmarshall();
#endif
}

template class Transpose<i4complex>;
template class Transpose<i8complex>;
template class Transpose<i16complex>;

#endif // HAVE_MPI



} // namespace RTCP
} // namespace LOFAR
