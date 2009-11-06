#ifndef LOFAR_INTERFACE_CN_MODE_H
#define LOFAR_INTERFACE_CN_MODE_H

#include <string>
#include <iostream>
#include <Stream/Stream.h>

namespace LOFAR {
namespace RTCP {

class CN_Mode
{
  public:
    enum OutputDataType {
      CORRELATEDDATA = 0,
      FILTEREDDATA,
      PENCILBEAMDATA,
      STOKESDATA
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
    CN_Mode( std::string modeName );

    Mode &mode();
    OutputDataType &outputDataType();

    bool isCoherent() const;
    unsigned nrStokes() const;

    std::string getModeName();

    void read(Stream *);
    void write(Stream *) const;

  private:
    struct modeList {
      Mode mode;
      std::string name;
      OutputDataType outputDataType;
      bool isCoherent;
      unsigned nrStokes;
    } const static modeList[];

    static int nrModes();

    struct {
      Mode mode;
      OutputDataType outputDataType;
      bool isCoherent;
      unsigned nrStokes;
    } itsMarshalledData;
};

inline CN_Mode::Mode &CN_Mode::mode()
{
  return itsMarshalledData.mode;
}

inline CN_Mode::OutputDataType &CN_Mode::outputDataType()
{
  return itsMarshalledData.outputDataType;
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

