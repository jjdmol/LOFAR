//#  InputSection.h: Catch RSP ethernet frames and synchronize RSP inputs 
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_IONPROC_INPUTSECTION_H
#define LOFAR_IONPROC_INPUTSECTION_H

// \file
// Catch RSP ethernet frames and synchronize RSP inputs 

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Interface/Parset.h>
#include <Stream/Stream.h>
#include <BeamletBuffer.h>
#include <BeamletBufferToComputeNode.h>
#include <InputThread.h>
#include <LogThread.h>

#include <boost/multi_array.hpp>
#include <pthread.h>


namespace LOFAR {
namespace RTCP {

template <typename SAMPLE_TYPE> class InputSection {
  public:
    InputSection(const std::vector<Stream *> &cnStreams, unsigned psetNumber);
    ~InputSection();
  
    void			 preprocess(const Parset *);
    void			 process();
    void			 postprocess();
    
  private:
    void			 createInputStreams(const Parset *, const std::vector<Parset::StationRSPpair> &inputs);
    void			 createInputThreads(const Parset *);

    BeamletBufferToComputeNode<SAMPLE_TYPE> *itsBeamletBufferToComputeNode;

    std::vector<Stream *>	 itsInputStreams;
    const std::vector<Stream *>  &itsCNstreams;
    
    unsigned			 itsPsetNumber;
    unsigned			 itsNrRSPboards;
   
    std::vector<BeamletBuffer<SAMPLE_TYPE> *> itsBBuffers;
    double			 itsSampleRate, itsSampleDuration;

    LogThread				    *itsLogThread;
    std::vector<InputThread<SAMPLE_TYPE> *> itsInputThreads;
};

} // namespace RTCP
} // namespace LOFAR

#endif
