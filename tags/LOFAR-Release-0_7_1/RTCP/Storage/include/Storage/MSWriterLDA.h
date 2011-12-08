//  MSMriterDAL.h: implementation of MSWriter using the DAL to write HDF5
//
//  Copyright (C) 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id: MSWriterImpl.h 11891 2008-10-14 13:43:51Z gels $
//
//////////////////////////////////////////////////////////////////////


#ifndef LOFAR_STORAGE_MSWRITERLDA_H
#define LOFAR_STORAGE_MSWRITERLDA_H

#if defined HAVE_LDA && defined HAVE_HDF5
#define USE_LDA
#endif


//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>

#include <Interface/Parset.h>
#include <Interface/StreamableData.h>
#include <Storage/MSWriter.h>
#include <Storage/MSWriterFile.h>

#ifdef USE_LDA
#include <vector>
#endif

//# Forward declarations

namespace LOFAR
{

  namespace RTCP
  {
    template<typename T, unsigned DIM> class MSWriterLDA : public MSWriterFile
    {
    public:
      MSWriterLDA(const string &filename, const Parset &parset, OutputType outputType, unsigned fileno, bool isBigEndian);
      ~MSWriterLDA();
#ifdef USE_LDA
      virtual void write(StreamableData *data);
    private:
      const Transpose2 &itsTransposeLogic;
      const unsigned itsNrChannels;
      unsigned itsNrSamples;
      unsigned itsNextSeqNr;

      std::vector<T> itsZeroBlock; // block with zeros, the same size of StreamableData::samples
#endif
    };
  }
}

#endif
