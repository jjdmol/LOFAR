//# DH_Vis.h: Vis DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef ONLINEPROTO_DH_VIS_H
#define ONLINEPROTO_DH_VIS_H


#include <lofar_config.h>

#include <Transport/DataHolder.h>
#include <Common/lofar_complex.h>

namespace LOFAR
{
class DH_Vis: public DataHolder
{
public:
  typedef complex<float> BufferType;

  explicit DH_Vis (const string& name);

  DH_Vis(const DH_Vis&);

  virtual ~DH_Vis();

  DataHolder* clone() const;


  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get write access to the Buffer in the DataPacket.
  BufferType* getBuffer();
  /// Get read access to the Buffer in the DataPacket.
  const BufferType* getBuffer() const;
  BufferType* getBufferElement(int station1, int station2, int channel);
  const int         getFBW() const;

  int nchannels;
  int nstations;

private:
  /// Forbid assignment.
  DH_Vis& operator= (const DH_Vis&);

  BufferType*  itsBuffer;    // array containing frequency spectrum.
  unsigned int itsBufSize;
  int          itsFBW; // number of frequency channels within this beamlet

  void fillDataPointers();
};

inline DH_Vis::BufferType* DH_Vis::getBuffer()
  { return itsBuffer; }

inline const DH_Vis::BufferType* DH_Vis::getBuffer() const
  { return itsBuffer; }

inline const int DH_Vis::getFBW() const
  { return itsFBW; }

}

#endif 
