#ifndef LOFAR_VISIBILITYRESAMPLERPROXY_H
#define LOFAR_VISIBILITYRESAMPLERPROXY_H

#include <synthesis/MeasurementComponents/VisibilityResamplerBase.h>

namespace LOFAR {
  class VisibilityResamplerProxy {
  public :
    VisibilityResamplerProxy() : itsVisibilityResampler(casa::CountedPtr<casa::VisibilityResamplerBase>()) {}
    VisibilityResamplerProxy(casa::CountedPtr<casa::VisibilityResamplerBase> vr) : itsVisibilityResampler(vr) {}
  protected:
    casa::CountedPtr<casa::VisibilityResamplerBase> itsVisibilityResampler;
  };
}
#endif
