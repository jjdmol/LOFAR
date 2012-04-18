#ifndef LOFAR_RTCP_INTERFACE_OUTPUT_TYPES_H
#define LOFAR_RTCP_INTERFACE_OUTPUT_TYPES_H

namespace LOFAR {
namespace RTCP {

enum OutputType
{
  FILTERED_DATA = 1,
  CORRELATED_DATA,
  INCOHERENT_STOKES,
  BEAM_FORMED_DATA,
  COHERENT_STOKES,
  TRIGGER_DATA
};


const OutputType FIRST_OUTPUT_TYPE = static_cast<OutputType>(1);
const OutputType LAST_OUTPUT_TYPE  = static_cast<OutputType>(7); // exclusive


inline OutputType operator ++ (OutputType &outputType) // prefix ++
{
  return (outputType = static_cast<OutputType>(outputType + 1));
}


inline OutputType operator ++ (OutputType &outputType, int) // postfix ++
{
  return (outputType = static_cast<OutputType>(outputType + 1));
}

} // namespace RTCP
} // namespace LOFAR

#endif
