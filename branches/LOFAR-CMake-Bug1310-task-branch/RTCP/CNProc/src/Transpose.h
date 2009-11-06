#ifndef LOFAR_CNPROC_TRANSPOSE_H
#define LOFAR_CNPROC_TRANSPOSE_H

#include <AsyncCommunication.h>
#include <InputData.h>
#include <LocationInfo.h>
#include <TransposedData.h>
#include <Interface/SubbandMetaData.h>

#if defined HAVE_MPI
#define MPICH_IGNORE_CXX_SEEK
#include <mpi.h>
#endif

#if defined HAVE_BGL
#include <bglpersonality.h>
#endif

#include <vector>


namespace LOFAR {
namespace RTCP {

#if defined HAVE_MPI

template <typename SAMPLE_TYPE> class Transpose
{
  public:
    Transpose(bool isTransposeInput, bool isTransposeOutput, unsigned myCore);
    ~Transpose();

    void setupTransposeParams(const LocationInfo &, const std::vector<unsigned> &inputPsets, const std::vector<unsigned> &outputPsets, InputData<SAMPLE_TYPE> *, TransposedData<SAMPLE_TYPE> *);

#if defined HAVE_BGL || HAVE_BGP
    static void	getMPIgroups(unsigned nrCoresPerPset, const LocationInfo &, const std::vector<unsigned> &inputPsets, const std::vector<unsigned> &outputPsets);
    static unsigned remapOnTree(unsigned pset, unsigned core, const std::vector<unsigned> &psetNumbers);
#endif

    void transpose(const InputData<SAMPLE_TYPE> *, TransposedData<SAMPLE_TYPE> *);
    void transposeMetaData(const InputData<SAMPLE_TYPE> *, TransposedData<SAMPLE_TYPE> *);

 private:
    bool itsIsTransposeInput, itsIsTransposeOutput;

    // All cores at the same position within a pset form a group.  The
    // transpose is done between members of this group.
    struct {
      struct {
	std::vector<int> counts, displacements, psetIndex;
      } send, receive;
    } itsTransposeParams, itsTransposeMetaParams;

    MPI_Comm			 itsTransposeGroup;

    static std::vector<MPI_Comm> allTransposeGroups;
};

#endif // defined HAVE_MPI

} // namespace RTCP
} // namespace LOFAR

#endif
