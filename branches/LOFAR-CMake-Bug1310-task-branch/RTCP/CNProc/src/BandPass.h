#ifndef LOFAR_CNPROC_BANDPASS_H
#define LOFAR_CNPROC_BANDPASS_H


namespace LOFAR {
namespace RTCP {

class BandPass {
  public:
			BandPass(bool correct, unsigned nrChannels);
			~BandPass();

    const float		*correctionFactors() const;

  private:
    void		computeCorrectionFactors(unsigned nrChannels);

    static const float	stationFilterConstants[65536];
    
    float		*factors;
};


inline const float *BandPass::correctionFactors() const
{
  return factors;
}

} // namespace RTCP
} // namespace LOFAR

#endif
