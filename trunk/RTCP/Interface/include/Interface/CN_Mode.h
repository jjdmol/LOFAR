#ifndef LOFAR_INTERFACE_CN_MODE_H
#define LOFAR_INTERFACE_CN_MODE_H

#include <string>
#include <iostream>
#include <Stream/Stream.h>
#include <Interface/Parset.h>

namespace LOFAR {
namespace RTCP {

class CN_Mode
{
  public:
    enum OutputDataType {
      CORRELATEDDATA = 0,
      FILTEREDDATA,
      PENCILBEAMDATA,
      STOKESDATA,

      RAWDATA = -1
    };

    enum Mode {
      CORRELATE = 0,
      FILTER,
      COHERENT_COMPLEX_VOLTAGES,
      COHERENT_STOKES_I,
      COHERENT_ALLSTOKES,
      INCOHERENT_STOKES_I,
      INCOHERENT_ALLSTOKES,

      INVALID = -1
    };

    CN_Mode();
    CN_Mode( const Parset &ps );

    Mode mode() const;
    unsigned nrOutputs() const;
    OutputDataType outputDataType( unsigned output ) const;
    OutputDataType finalOutputDataType() const;

    bool isCoherent() const;
    unsigned nrStokes() const;

    std::string getModeName();
    std::string getOutputName( unsigned output ) const;

    void read(Stream *);
    void write(Stream *) const;

  private:
    struct modeList {
      Mode mode;
      std::string name;
      OutputDataType finalOutputDataType;
      bool isCoherent;
      unsigned nrStokes;
    } const static modeList[];

    static unsigned nrModes();

    struct {
      Mode mode;
      OutputDataType finalOutputDataType;
      bool isCoherent;
      unsigned nrStokes;

      unsigned outputIncoherentStokesI;
      unsigned nrOutputs;
    } itsMarshalledData;
};

inline CN_Mode::Mode CN_Mode::mode() const
{
  return itsMarshalledData.mode;
}

inline unsigned CN_Mode::nrOutputs() const
{
  return itsMarshalledData.nrOutputs;
}

inline CN_Mode::OutputDataType CN_Mode::outputDataType( unsigned output ) const
{
  if( itsMarshalledData.outputIncoherentStokesI ) {
    if( output == 0 ) {
      return CN_Mode::STOKESDATA;
    } else {
      output--;
    }
  }

  if( output == 0 ) {
    return finalOutputDataType();
  }

  return RAWDATA;
}

inline CN_Mode::OutputDataType CN_Mode::finalOutputDataType() const
{
  return itsMarshalledData.finalOutputDataType;
}

inline bool CN_Mode::isCoherent() const
{
  return itsMarshalledData.isCoherent;
}

inline unsigned CN_Mode::nrStokes() const
{
  return itsMarshalledData.nrStokes;
}

inline void CN_Mode::read( Stream *str )
{
  str->read(&itsMarshalledData, sizeof itsMarshalledData);
}

inline void CN_Mode::write( Stream *str ) const
{
  str->write(&itsMarshalledData, sizeof itsMarshalledData);
}

inline std::ostream& operator<<(std::ostream &str, CN_Mode &m)
{
  return str << m.getModeName();
}

} // namespace RTCP
} // namespace LOFAR

#endif

