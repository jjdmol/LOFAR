#ifndef LOFAR_CNPROC_FLAGGER_HISTORY_H
#define LOFAR_CNPROC_FLAGGER_HISTORY_H


#define HISTORY_SIZE 64
#define MIN_HISTORY_SIZE 4 // at least 1, max HISTORY_SIZE
#define FLAG_WITH_INTEGRATED_HISTORY_POWERS 0

#if FLAG_WITH_INTEGRATED_HISTORY_POWERS
#include <Interface/MultiDimArray.h>
#endif

namespace LOFAR {
namespace RTCP {

class FlaggerHistory {
  unsigned itsSize;
  unsigned itsCurrent;
  float itsMedianSum;

  std::vector<float> itsMedians;

#if FLAG_WITH_INTEGRATED_HISTORY_POWERS
  MultiDimArray<float,2> itsPowers; // history of powers, [HISTORY_SIZE][itsNrChannels]
  std::vector<float> itsIntegratedPowers; // the sum of all powers in itsPowers [itsNrChannels]
#endif

public:

FlaggerHistory() : itsSize(0), itsCurrent(0), itsMedianSum(0.0f) {
    itsMedians.resize(HISTORY_SIZE);
    memset(&itsMedians[0], 0, HISTORY_SIZE * sizeof(float));
  }

  void add(float median, std::vector<float>& powers) { // we have to copy powers
#if FLAG_WITH_INTEGRATED_HISTORY_POWERS
    unsigned nrChannels = powers.size();
    if(itsSize == 0) {
      itsPowers.resize(boost::extents[HISTORY_SIZE][nrChannels]);
      itsIntegratedPowers.resize(nrChannels);
      memset(&itsIntegratedPowers[0], 0, nrChannels * sizeof(float));
      memset(&itsPowers[0][0], 0, HISTORY_SIZE * nrChannels * sizeof(float));
    }
#else
    (void) powers; // prevent compiler warning
#endif

    if (itsSize >= HISTORY_SIZE) { // we are overwriting an old element
      itsMedianSum -= itsMedians[itsCurrent];

#if FLAG_WITH_INTEGRATED_HISTORY_POWERS
      for(unsigned c=0; c<nrChannels; c++) {
	itsIntegratedPowers[c] -= itsPowers[itsCurrent][c];
      }
#endif
    } else {
      itsSize++;
    }
    itsMedians[itsCurrent] = median;
    itsMedianSum += median;

#if FLAG_WITH_INTEGRATED_HISTORY_POWERS
    for(unsigned c=0; c<nrChannels; c++) {
      itsPowers[itsCurrent][c] = powers[c];
      itsIntegratedPowers[itsCurrent] += powers[c];
    }
#endif

    itsCurrent++;
    if(itsCurrent >= HISTORY_SIZE) itsCurrent = 0;

#if 0
    std::cout << "HISTORY(" << itsSize << "): ";
    for(int i=0; i<HISTORY_SIZE; i++) {
	    std::cout << itsMedians[i] << " ";
    }
    std::cout << std::endl;
#endif
  }

  float getMeanMedian() {
    if (itsSize == 0) {
      return 0.0f;
    }
    return itsMedianSum / itsSize;
  }

  float getStdDevOfMedians() {
    if (itsSize == 0) {
      return 0.0f;
    }

    float stdDev = 0.0f;
    float meanMedian = getMeanMedian();

    for (unsigned i = 0; i < itsSize; i++) {
      float diff = itsMedians[i] - meanMedian;
      stdDev += diff * diff;
    }
    stdDev /= itsSize;
    stdDev = (float) sqrt(stdDev);
    return stdDev;
  }

  unsigned getSize() {
    return itsSize;
  }

#if FLAG_WITH_INTEGRATED_HISTORY_POWERS
  std::vector<float>& getIntegratedPowers() {
    return itsIntegratedPowers;
  }
#endif

}; // end of FlaggerHistory
  

} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_CNPROC_FLAGGER_HISTORY_H
