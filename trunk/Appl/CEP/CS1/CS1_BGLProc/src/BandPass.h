#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_BANDPASS_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_BANDPASS_H


namespace LOFAR {
namespace CS1 {

class BandPass {
  public:
			BandPass();
			~BandPass();

    const float		*correctionFactors() const;

  private:
    static const float	stationFilterConstants[65536];
    
    float		*factors;
};


inline const float *BandPass::correctionFactors() const
{
  return factors;
}

} // namespace CS1
} // namespace LOFAR

#endif
