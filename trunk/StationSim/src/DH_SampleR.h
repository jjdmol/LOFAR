//#  DH_SampleR.h: 
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

#ifndef STATIONSIM_DH_SAMPLER_H
#define STATIONSIM_DH_SAMPLER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <BaseSim/DataHolder.h>

/**
   This class is the DataHolder holding real sample data.
*/

class DH_SampleR: public DataHolder
{
public:
  typedef double BufferType;

  DH_SampleR (const string& name, unsigned int nx,
	      unsigned int ny);

  virtual ~DH_SampleR();

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get write access to the Buffer in the DataPacket.
  BufferType* getBuffer();
  /// Get read access to the Buffer in the DataPacket.
  const BufferType* getBuffer() const;

  /// Write the buffer into an ASCII file.
  virtual bool doFsWrite (ofstream&) const;

protected:
  // Definition of the DataPacket type.
  class DataPacket: public DataHolder::DataPacket
  {
  public:
    DataPacket()
      {};

    BufferType itsFill;           // to ensure alignment
  };

private:
  /// Forbid copy constructor.
  DH_SampleR (const DH_SampleR&);
  /// Forbid assignment.
  DH_SampleR& operator= (const DH_SampleR&);


  void*        itsDataPacket;
  BufferType*  itsBuffer;
  int          itsNx;
  int          itsNy;
};


inline DH_SampleR::BufferType* DH_SampleR::getBuffer()
  { return itsBuffer; }

inline const DH_SampleR::BufferType* DH_SampleR::getBuffer() const
  { return itsBuffer; }


#endif 
