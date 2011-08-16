#ifndef LOFAR_CNPROC_FLAGGER_HISTORY_H
#define LOFAR_CNPROC_FLAGGER_HISTORY_H

#include <Interface/MultiDimArray.h>

#define HISTORY_SIZE 64
#define MIN_HISTORY_SIZE HISTORY_SIZE

namespace LOFAR {
namespace RTCP {

class HistoryList {
  unsigned itsSize;
  unsigned itsCurrent;
  std::vector<float> itsMedians;

  float itsMedianSum;

public:

HistoryList() : itsSize(0), itsCurrent(0), itsMedianSum(0.0f) {
    itsMedians.resize(HISTORY_SIZE);

    for(int i=0; i<HISTORY_SIZE; i++) {
	    itsMedians[i] = 0.0f;
    }
  }

  void add(float median) {
    if (itsSize >= HISTORY_SIZE) { // we are overwriting an old element
      itsMedianSum -= itsMedians[itsCurrent];
    } else {
      itsSize++;
    }
    itsMedians[itsCurrent] = median;
    itsCurrent++;
    if(itsCurrent >= itsSize) itsCurrent = 0;

    itsMedianSum += median;
/*
    std::cout << "HISTORY: ";
    for(int i=0; i<HISTORY_SIZE; i++) {
	    std::cout << itsMedians[i] << " ";
    }
    std::cout << std::endl;
*/
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
      unsigned pos = (itsCurrent + i) % itsSize;
      float diff = itsMedians[pos] - meanMedian;
      stdDev += diff * diff;
    }
    stdDev /= itsSize;
    stdDev = (float) sqrt(stdDev);
    return stdDev;
  }

  unsigned getSize() {
    return itsSize;
  }

}; // end of HistoryList
  

} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_CNPROC_FLAGGER_HISTORY_H
