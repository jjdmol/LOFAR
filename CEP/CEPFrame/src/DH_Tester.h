//  DH_Tester.h:
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
//  Revision 1.12  2002/05/03 11:21:31  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.11  2002/03/14 14:18:56  wierenga
//  system include before local includes
//
//  Revision 1.10  2002/03/01 08:27:56  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.9  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.8  2001/09/21 12:19:02  gvd
//  Added make functions to WH classes to fix memory leaks
//
//  Revision 1.7  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.6  2001/03/01 13:15:47  gvd
//  Added type argument in DataHolder constructor which is used in
//  the connect functions to check if the DH types match
//  Improved the simulator parser
//  Improved documentation
//
//  Revision 1.5  2001/02/05 14:53:05  loose
//  Added GPL headers
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_DH_TESTER_H
#define BASESIM_DH_TESTER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Common/lofar_complex.h>
#include "BaseSim/DataHolder.h"
#include "BaseSim/BaseSim.h"

/**
   This class is a simple DataHolder test class for the program Tester.
*/

class DH_Tester: public DataHolder
{
public:
  typedef complex<float> DataBufferType;

  explicit DH_Tester (const string& name);
  virtual ~DH_Tester();

  void setCounter (int counter);
  int getCounter() const;

  DataBufferType* getBuffer();
  const DataBufferType* getBuffer() const;

protected:
  class DataPacket: public DataHolder::DataPacket
  {
  public:
    DataPacket() : itsCounter(0) {};
    DataBufferType itsBuffer[10];
    int itsCounter;
  };

private:
  /// Forbid copy constructor.
  DH_Tester (const DH_Tester&);
  /// Forbid assignment.
  DH_Tester& operator= (const DH_Tester&);


  DataPacket itsDataPacket;
};


inline void DH_Tester::setCounter (int counter)
  { itsDataPacket.itsCounter = counter; }

inline int DH_Tester::getCounter() const
  { return itsDataPacket.itsCounter; }

inline DH_Tester::DataBufferType* DH_Tester::getBuffer()
  { return itsDataPacket.itsBuffer; }

inline const DH_Tester::DataBufferType* DH_Tester::getBuffer() const
  { return itsDataPacket.itsBuffer; }


#endif 
