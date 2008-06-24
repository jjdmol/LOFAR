
#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_TRANSPOSE_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_TRANSPOSE_H

#include <InputData.h>
#include <LocationInfo.h>
#include <TransposedData.h>

#include <boost/multi_array.hpp>

#if defined HAVE_MPI
#define MPICH_IGNORE_CXX_SEEK
#include <mpi.h>
#endif

#if defined HAVE_BGL
#include <bglpersonality.h>
#endif

#include <vector>


namespace LOFAR {
namespace CS1 {

#if defined HAVE_MPI

class Transpose {
  public:
    Transpose(bool isTransposeInput, bool isTransposeOutput, unsigned myCore, unsigned nrStations,unsigned nrBeams);
    ~Transpose();

    void setupTransposeParams(const std::vector<unsigned> &inputPsets, const std::vector<unsigned> &outputPsets, InputData *, TransposedData *);

/* #if defined HAVE_BGL */
/*     static void	getMPIgroups(unsigned nrCoresPerPset, const BGLPersonality &, const std::vector<unsigned> &inputPsets, const std::vector<unsigned> &outputPsets); */
/*     static unsigned remapOnTree(unsigned pset, unsigned core, const struct BGLPersonality &); */
/* #elif defined HAVE_BGP */
#if defined HAVE_BGL || HAVE_BGP
    static void	getMPIgroups(unsigned nrCoresPerPset, const LocationInfo &, const std::vector<unsigned> &inputPsets, const std::vector<unsigned> &outputPsets);
    static unsigned remapOnTree(unsigned pset, unsigned core, const std::vector<unsigned> &psetNumbers);
#endif

    void transpose(const InputData *, TransposedData *);
    void transposeMetaData(/*const*/ InputData *, TransposedData *, const unsigned currentBeam);

  private:
    bool     itsIsTransposeInput, itsIsTransposeOutput;
    unsigned itsNrStations;
    unsigned itsNrBeams;

    // All cores at the same position within a pset form a group.  The
    // transpose is done between members of this group.
    struct {
      struct {
	std::vector<int> counts, displacements;
      } send, receive;
    } itsTransposeParams, itsTransposeMetaParams;

    struct metaData {
      float    delayAtBegin, delayAfterEnd;
      unsigned alignmentShift;
      char     flagsBuffer[132]; // enough for 16 flag ranges
    };

    boost::multi_array<struct metaData, 2> itsOutputMetaData; // [station][beam]

    std::vector<struct metaData> itsInputMetaData; // [beam]
    
    MPI_Comm			 itsTransposeGroup;

    static std::vector<MPI_Comm> allTransposeGroups;
};

#endif // defined HAVE_MPI

} // namespace CS1
} // namespace LOFAR

#endif
