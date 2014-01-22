//# MSMriterDAL.h: implementation of MSWriter using the DAL to write HDF5
//# Copyright (C) 2011-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_STORAGE_MSWRITERDAL_H
#define LOFAR_STORAGE_MSWRITERDAL_H

#ifdef HAVE_DAL

//# Includes
#include <string>

#include <Common/LofarTypes.h>
#include <Common/ParameterSet.h>
#include <CoInterface/Parset.h>
#include <CoInterface/StreamableData.h>
#include "MSWriter.h"
#include "MSWriterFile.h"

namespace LOFAR
{

  namespace Cobalt
  {
    template<typename T, unsigned DIM>
    class MSWriterDAL : public MSWriterFile
    {
    public:
      MSWriterDAL(const std::string &filename, const Parset &parset, unsigned fileno);
      ~MSWriterDAL();
      virtual void write(StreamableData *data);
    private:
      const Parset &itsParset;
      unsigned itsNrChannels;
      unsigned itsNrSamples;
      unsigned itsNextSeqNr;
      unsigned itsFileNr;
      unsigned itsBlockSize; // the size of StreamableData::samples, in T
    };
  }
}

#endif // HAVE_DAL

#endif

