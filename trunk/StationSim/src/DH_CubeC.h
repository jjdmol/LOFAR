//#  DH_CubeC.h: 
//#
//#  Copyright (C) 2002
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
//#

#ifndef STATIONSIM_DH_CUBEC_H
#define STATIONSIM_DH_CUBEC_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <BaseSim/DataHolder.h>
#include <Common/lofar_complex.h>

/**
   This class is the DataHolder holding real sample data.
*/

class DH_CubeC: public DataHolder
{
public:
  typedef complex <double> BufferType;

  DH_CubeC (const string& name, unsigned int nx, unsigned int ny, unsigned int nz);

  virtual ~DH_CubeC ();

  /// Allocate the buffers.
  virtual void preprocess ();

  /// Deallocate the buffers.
  virtual void postprocess ();

  /// Get write access to the Buffer in the DataPacket.
  BufferType* getBuffer ();
  /// Get read access to the Buffer in the DataPacket.
  const BufferType* getBuffer () const;

  /// Write the buffer into an ASCII file.
  virtual bool doFsWrite (ofstream&) const;

protected:
  // Definition of the DataPacket type.
  class DataPacket: public DataHolder::DataPacket
  {
  public:
    DataPacket ()
      {};

    BufferType itsFill;           // to ensure alignment
  };

private:
  /// Forbid copy constructor.
  DH_CubeC (const DH_CubeC&);
  /// Forbid assignment.
  DH_CubeC& operator= (const DH_CubeC&);

  void*        itsDataPacket;
  BufferType*  itsBuffer;
  int          itsNx;
  int          itsNy;
  int          itsNz;
};


inline DH_CubeC::BufferType* DH_CubeC::getBuffer ()
{ return itsBuffer; }

inline const DH_CubeC::BufferType* DH_CubeC::getBuffer () const
{ return itsBuffer; }


#endif 
