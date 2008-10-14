//# BGL_Configuration.h:
//#
//#  Copyright (C) 2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_INTERFACE_BGL_CONFIGURATION_H
#define LOFAR_INTERFACE_BGL_CONFIGURATION_H

#include <Stream/Stream.h>

#include <vector>


namespace LOFAR {
namespace RTCP {

class BGL_Configuration
{
  public:
    unsigned		  &nrStations();
    unsigned		  &nrBitsPerSample();
    unsigned		  &nrChannelsPerSubband();
    unsigned		  &nrSamplesPerIntegration();
    unsigned		  &nrSamplesToBGLProc();
    unsigned		  &nrUsedCoresPerPset();
    unsigned		  &nrSubbandsPerPset();
    bool		  &delayCompensation();
    bool		  &correctBandPass();
    double		  &sampleRate();
    std::vector<unsigned> &inputPsets(), &outputPsets(), &tabList();
    std::vector<double>	  &refFreqs();
    
    void		  read(Stream *);
    void		  write(Stream *);

    static const unsigned MAX_PSETS    = 64;
    static const unsigned MAX_SUBBANDS = 248;

  private:
    std::vector<unsigned> itsInputPsets, itsOutputPsets, itsTabList;
    std::vector<double>	  itsRefFreqs;

    struct MarshalledData
    {
      unsigned		  itsNrStations;
      unsigned		  itsNrBitsPerSample;
      unsigned		  itsNrChannelsPerSubband;
      unsigned		  itsNrSamplesPerIntegration;
      unsigned		  itsNrSamplesToBGLProc;
      unsigned		  itsNrUsedCoresPerPset;
      unsigned		  itsNrSubbandsPerPset;
      bool		  itsDelayCompensation;
      bool		  itsCorrectBandPass;
      double		  itsSampleRate;
      unsigned		  itsInputPsetsSize, itsOutputPsetsSize, itsTabListSize;
      unsigned		  itsRefFreqsSize;
      unsigned		  itsInputPsets[MAX_PSETS], itsOutputPsets[MAX_PSETS], itsTabList[MAX_PSETS];
      double		  itsRefFreqs[MAX_SUBBANDS];
    } itsMarshalledData;
};


inline unsigned &BGL_Configuration::nrStations()
{
  return itsMarshalledData.itsNrStations;
}

inline unsigned &BGL_Configuration::nrBitsPerSample()
{
  return itsMarshalledData.itsNrBitsPerSample;
}

inline unsigned &BGL_Configuration::nrChannelsPerSubband()
{
  return itsMarshalledData.itsNrChannelsPerSubband;
}

inline unsigned &BGL_Configuration::nrSamplesPerIntegration()
{
  return itsMarshalledData.itsNrSamplesPerIntegration;
}

inline unsigned &BGL_Configuration::nrSamplesToBGLProc()
{
  return itsMarshalledData.itsNrSamplesToBGLProc;
}

inline unsigned &BGL_Configuration::nrUsedCoresPerPset()
{
  return itsMarshalledData.itsNrUsedCoresPerPset;
}

inline unsigned &BGL_Configuration::nrSubbandsPerPset()
{
  return itsMarshalledData.itsNrSubbandsPerPset;
}

inline bool &BGL_Configuration::delayCompensation()
{
  return itsMarshalledData.itsDelayCompensation;
}

inline bool &BGL_Configuration::correctBandPass()
{
  return itsMarshalledData.itsCorrectBandPass;
}

inline double &BGL_Configuration::sampleRate()
{
  return itsMarshalledData.itsSampleRate;
}

inline std::vector<unsigned> &BGL_Configuration::inputPsets()
{
  return itsInputPsets;
}

inline std::vector<unsigned> &BGL_Configuration::outputPsets()
{
  return itsOutputPsets;
}

inline std::vector<unsigned> &BGL_Configuration::tabList()
{
  return itsTabList;
}

inline std::vector<double> & BGL_Configuration::refFreqs()
{
  return itsRefFreqs;
}

} // namespace RTCP
} // namespace LOFAR

#endif
