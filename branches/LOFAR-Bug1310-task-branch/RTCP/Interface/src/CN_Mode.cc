#include <Interface/CN_Mode.h>

namespace LOFAR {
namespace RTCP {

const struct CN_Mode::modeList CN_Mode::modeList[] = {
  // mode id, parset identifier, output datatype, coherent?, nrStokes
  { CORRELATE,                 "Correlate",               CORRELATEDDATA, true,  0 },
  { FILTER,                    "Filter",                  FILTEREDDATA,   false, 0 },
  { COHERENT_COMPLEX_VOLTAGES, "CoherentComplexVoltages", PENCILBEAMDATA, true,  0 },
  { COHERENT_STOKES_I,         "CoherentStokesI",         STOKESDATA,     true,  1 },
  { COHERENT_ALLSTOKES,        "CoherentAllStokes",       STOKESDATA,     true,  4 },
  { INCOHERENT_STOKES_I,       "IncoherentStokesI",       STOKESDATA,     false, 1 },
  { INCOHERENT_ALLSTOKES,      "IncoherentAllStokes",     STOKESDATA,     false, 4 }
};

int CN_Mode::nrModes()
{
  return sizeof modeList / sizeof modeList[0];
}

CN_Mode::CN_Mode()
{
  itsMarshalledData.mode = INVALID;
}

CN_Mode::CN_Mode( std::string name )
{
  itsMarshalledData.mode = INVALID;

  for( unsigned i = 0; i < nrModes(); i++ ) {
    if( name == modeList[i].name ) {
      itsMarshalledData.mode = modeList[i].mode;
      itsMarshalledData.outputDataType = modeList[i].outputDataType;
      itsMarshalledData.isCoherent = modeList[i].isCoherent;
      itsMarshalledData.nrStokes = modeList[i].nrStokes;
      break;
    }
  }
}

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
