//  DH_Example.h: Example DataHolder
//
//  Copyright (C) 2000, 2001
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
//  $Id$
//
//  $Log$
//  Revision 1.11  2002/05/03 11:21:31  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.10  2002/03/14 14:15:31  wierenga
//  system includes before local includes
//
//  Revision 1.9  2002/03/01 08:27:56  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.8  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.7  2001/09/24 14:04:08  gvd
//  Added preprocess and postprocess functions
//
//  Revision 1.6  2001/09/21 12:19:02  gvd
//  Added make functions to WH classes to fix memory leaks
//
//  Revision 1.5  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.4  2001/03/16 10:20:34  gvd
//  Updated comments and made buffer length dynamic
//
//  Revision 1.3  2001/03/01 13:15:47  gvd
//  Added type argument in DataHolder constructor which is used in
//  the connect functions to check if the DH types match
//  Improved the simulator parser
//  Improved documentation
//
//  Revision 1.2  2001/02/05 14:53:05  loose
//  Added GPL headers
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_DH_EXAMPLE_H
#define BASESIM_DH_EXAMPLE_H

#include <Common/lofar_complex.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/DataHolder.h"

/**
   This class is an example DataHolder which is only used in the
   Example programs.
*/

class DH_Example: public DataHolder
{
public:
  typedef complex<float> BufferType;

  explicit DH_Example (const string& name, unsigned int nbuffer = 10);

  virtual ~DH_Example();

  /// Set the Counter attribute in the DataPacket.
  void setCounter (int counter);
  /// Get the Counter attribute in the DataPacket.
  int getCounter() const;

  /// Aloocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Get write access to the Buffer in the DataPacket.
  BufferType* getBuffer();
  /// Get read access to the Buffer in the DataPacket.
  const BufferType* getBuffer() const;

protected:
  // Definition of the DataPacket type.
  class DataPacket: public DataHolder::DataPacket
  {
  public:
    DataPacket() : itsCounter(0) {};

    int itsCounter;
    BufferType itsFill;         // to ensure alignment
  };

private:
  /// Forbid copy constructor.
  DH_Example (const DH_Example&);
  /// Forbid assignment.
  DH_Example& operator= (const DH_Example&);


  DataPacket*  itsDataPacket;
  BufferType*  itsBuffer;
  unsigned int itsBufSize;
};


inline void DH_Example::setCounter (int counter)
  { itsDataPacket->itsCounter = counter; }

inline int DH_Example::getCounter() const
  { return itsDataPacket->itsCounter; }

inline DH_Example::BufferType* DH_Example::getBuffer()
  { return itsBuffer; }

inline const DH_Example::BufferType* DH_Example::getBuffer() const
  { return itsBuffer; }


#endif 
