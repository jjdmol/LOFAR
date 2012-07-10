#ifndef LOFAR_CNPROC_FLAGGER_HISTORY_H
#define LOFAR_CNPROC_FLAGGER_HISTORY_H

#include <Interface/MultiDimArray.h>

#define HISTORY_SIZE 64
#define MIN_HISTORY_SIZE 4 // at least 1, max HISTORY_SIZE

namespace LOFAR {
namespace RTCP {

class FlaggerHistory {
  unsigned itsSize;
  unsigned itsCurrent;
  float itsMedianSum;

  MultiDimArray<float,2> itsPowers; // history of powers, [HISTORY_SIZE][itsNrChannels]
  std::vector<float> itsMedians;
  std::vector<float> itsIntegratedPowers; // the sum of all powers in itsPowers [itsNrChannels]

public:

FlaggerHistory() : itsSize(0), itsCurrent(0), itsMedianSum(0.0f) {
    itsMedians.resize(HISTORY_SIZE);
    for(unsigned i=0; i<HISTORY_SIZE; i++) {
      itsMedians[i] = 0.0f;
    }
  }

  void add(float median, std::vector<float>& powers) { // we have to copy powers
    unsigned nrChannels = powers.size();

    if(itsSize == 0) {
      itsPowers.resize(boost::extents[HISTORY_SIZE][nrChannels]);
      itsIntegratedPowers.resize(nrChannels);
      for(unsigned i=0;i<nrChannels; i++) {
	itsIntegratedPowers[i] = 0.0f;
      }
    
      for(unsigned i=0; i<HISTORY_SIZE; i++) {
	for(unsigned c=0;c<nrChannels; c++) {
	  itsPowers[i][c] = 0.0f;
	}
      }
    }

    if (itsSize >= HISTORY_SIZE) { // we are overwriting an old element
      itsMedianSum -= itsMedians[itsCurrent];
      for(unsigned c=0; c<nrChannels; c++) {
	itsIntegratedPowers[c] -= itsPowers[itsCurrent][c];
      }
    } else {
      itsSize++;
    }
    itsMedians[itsCurrent] = median;
    for(unsigned c=0; c<nrChannels; c++) {
      itsPowers[itsCurrent][c] = powers[c];
      itsIntegratedPowers[itsCurrent] += powers[c];
    }
    itsCurrent++;
    if(itsCurrent >= HISTORY_SIZE) itsCurrent = 0;

    itsMedianSum += median;
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

  std::vector<float>& getIntegratedPowers() {
    return itsIntegratedPowers;
  }
  

}; // end of FlaggerHistory
  

} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_CNPROC_FLAGGER_HISTORY_H
