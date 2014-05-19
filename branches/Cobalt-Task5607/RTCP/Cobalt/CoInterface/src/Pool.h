//# Pool.h
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#ifndef LOFAR_COINTERFACE_POOL_H
#define LOFAR_COINTERFACE_POOL_H

#include <CoInterface/Queue.h>
#include <CoInterface/SmartPtr.h>

//#include <InputProc/Transpose/MPIUtil.h>

namespace LOFAR
{
  namespace Cobalt
  {
    // The pool operates using a free and a filled queue to cycle through buffers. Producers
    // move elements free->filled, and consumers move elements filled->free. By
    // wrapping the elements in a SmartPtr, memory leaks are prevented.
    template <typename T>
    struct Pool
    {
      typedef T element_type;

      Queue< SmartPtr<element_type> > free;
      Queue< SmartPtr<element_type> > filled;

      Pool(const std::string &name)
      :
        free(name + " [.free]"),
        filled(name + " [.filled]")
      {
      }
    };


    //template<typename SampleT>
    //struct MPIData
    //{
    //  size_t block;

    //  SmartPtr<char, SmartPtrMPI<char> > data;
    //  SmartPtr<char, SmartPtrMPI<char> > metaData;

    //  template<typename SampleT>
    //  void allocate(size_t nrStations, size_t nrBeamlets, size_t nrSamples);
   // };

  }
}

#endif
