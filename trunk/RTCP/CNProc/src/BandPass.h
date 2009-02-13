#ifndef LOFAR_CNPROC_BANDPASS_H
#define LOFAR_CNPROC_BANDPASS_H


namespace LOFAR {
namespace RTCP {

#define STATION_FILTER_LENGTH 16384 // Number of filter taps of the station filters.
#define STATION_FFT_SIZE 1024 // The size of the FFT that the station filter does.

class BandPass {
  public:
			BandPass(bool correct, unsigned nrChannels);
			~BandPass();

    const float		*correctionFactors() const;
    const float		*squaredCorrectionFactors() const;

  private:
    void		computeCorrectionFactors(unsigned nrChannels);

    static const float	stationFilterConstants[];
    
    float		*factors, *squaredFactors;
};


inline const float *BandPass::correctionFactors() const
{
  return factors;
}


inline const float *BandPass::squaredCorrectionFactors() const
{
  return squaredFactors;
}

} // namespace RTCP
} // namespace LOFAR

#endif
