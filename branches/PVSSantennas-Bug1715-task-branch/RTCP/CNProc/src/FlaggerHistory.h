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

  float itsMeanMedian;

public:

  HistoryList() : itsSize(0), itsCurrent(0) {
    itsMedians.resize(HISTORY_SIZE);
  }

  void add(float median) {
    if (itsSize >= HISTORY_SIZE) { // we are overwriting an old element
      itsMeanMedian -= itsMedians[itsCurrent];
    } else {
	    itsSize++;
    }
    itsMedians[itsCurrent] = median;
    itsCurrent++;
    if(itsCurrent >= itsSize) itsCurrent = 0;

    itsMeanMedian += median;
  }

  float getMeanMedian() {
    if (itsSize == 0) {
      return 0.0f;
    }
    return itsMeanMedian / itsSize;
  }

  float getStdDevOfMedians() {
    float meanMedian = itsMeanMedian;
    float stdDev = 0.0f;
	  
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
