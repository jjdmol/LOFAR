#ifndef LOFAR_CNPROC_PRE_CORRELATION_FLAGGER_H
#define LOFAR_CNPROC_PRE_CORRELATION_FLAGGER_H

#include <Flagger.h>
#include <Interface/FilteredData.h>

namespace LOFAR {
namespace RTCP {

enum PreCorrelationFlaggerType {
  PRE_FLAGGER_THRESHOLD,
  PRE_FLAGGER_INTEGRATED_THRESHOLD,
  PRE_FLAGGER_SUM_THRESHOLD,
  PRE_FLAGGER_INTEGRATED_SUM_THRESHOLD
};

class PreCorrelationFlagger : public Flagger {
  public:
  PreCorrelationFlagger(const Parset& parset, const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, float cutoffThreshold = 7.0f);

  void flag(FilteredData* filteredData);

  private:

  // Does simple thresholding.
  void integratingThresholdingFlagger(const MultiDimArray<float,2> &powers, vector<float> &integratedPowers, vector<bool> &integratedFlags);
  void integratingSumThresholdFlagger(const MultiDimArray<float,2> &powers, vector<float> &integratedPowers, vector<bool> &integratedFlags);

  void calculatePowers(unsigned station, unsigned pol, FilteredData* filteredData);
  void integratePowers(const MultiDimArray<float,2> &powers, vector<float> &integratedPowers);

  void wipeFlags();
  void applyFlags(FilteredData* filteredData, unsigned station);

  void flagSample(FilteredData* filteredData, unsigned channel, unsigned station, unsigned time);
  void wipeFlaggedSamples(FilteredData* filteredData);

  PreCorrelationFlaggerType getFlaggerType(std::string t);
  std::string getFlaggerTypeString(PreCorrelationFlaggerType t);
  std::string getFlaggerTypeString();

  const PreCorrelationFlaggerType itsFlaggerType;
  const unsigned itsNrSamplesPerIntegration;

  MultiDimArray<float,2> itsPowers; // [itsNrChannels][itsNrSamplesPerIntegration]
  vector<float> itsIntegratedPowers; // [itsNrChannels]

  MultiDimArray<bool,2> itsFlags; // [itsNrChannels][itsNrSamplesPerIntegration]
  vector<bool> itsIntegratedFlags; // [itsNrChannels]

};

} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_CNPROC_PRE_CORRELATION_FLAGGER_H
