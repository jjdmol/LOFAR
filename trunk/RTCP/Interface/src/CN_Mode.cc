#include <lofar_config.h>

#include <Interface/CN_Mode.h>

namespace LOFAR {
namespace RTCP {

const struct CN_Mode::modeList CN_Mode::modeList[] = {
  // mode id, parset identifier, coherent?, nrStokes
  { CORRELATE,                 "Correlate",               true,  0 },
  { FILTER,                    "Filter",                  false, 0 },
  { COHERENT_COMPLEX_VOLTAGES, "CoherentComplexVoltages", true,  0 },
  { COHERENT_STOKES_I,         "CoherentStokesI",         true,  1 },
  { COHERENT_ALLSTOKES,        "CoherentAllStokes",       true,  4 },
  { INCOHERENT_STOKES_I,       "IncoherentStokesI",       false, 1 },
  { INCOHERENT_ALLSTOKES,      "IncoherentAllStokes",     false, 4 }
};

unsigned CN_Mode::nrModes()
{
  return sizeof modeList / sizeof modeList[0];
}

CN_Mode::CN_Mode()
{
  itsMarshalledData.mode = INVALID;
}


#if ! defined HAVE_BGP_CN

CN_Mode::CN_Mode(const Parset &ps)
{
  string name = ps.getModeName();

  itsMarshalledData.mode = INVALID;

  for (unsigned i = 0; i < nrModes(); i ++) {
    if (name == modeList[i].name) {
      itsMarshalledData.mode = modeList[i].mode;
      itsMarshalledData.isCoherent = modeList[i].isCoherent;
      itsMarshalledData.nrStokes = modeList[i].nrStokes;
      break;
    }
  }
}

#endif


std::string CN_Mode::getModeName()
{
  for( unsigned i = 0; i < nrModes(); i++ ) {
    if( itsMarshalledData.mode == modeList[i].mode ) {
      return modeList[i].name;
    }
  }

  return "invalid";
}

} // namespace RTCP
} // namespace LOFAR
