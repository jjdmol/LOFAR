//#  DH_VarSize.h: DataHolder with one dimensional byte array that 
//#                  can grow its size (e.g. for performance measurements)
//#
//#  Copyright (C) 2000, 2001
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

#ifndef LOFAR_P2PERF_DH_VARSIZE_H
#define LOFAR_P2PERF_DH_VARSIZE_H

// \file
//  DataHolder with one dimensional byte array that can grow its size
//  (e.g. for performance measurements)

#include <Transport/DataHolder.h>

namespace LOFAR 
{
  /**
     This class is an data holder that has has a variable size.
     This class can be used in different ways: 
     -# Fixed size (this can be used to test the speed of sending fixed size
        packets)\n
        Set the initial size at creation time. At that moment the DH has a
        fixed size.
     -# Fixed but changing size (this can be used to measure the speed at
        different sizes in one run)\n
        Set the initial size at creation time. Use the method setFixedDataSize
        to change the size at runtime.  Make sure that the sending and
        receiving WH have the same fixed size.
     -# Variable size (this can be used to send packets of variable size; in
        this case the headers have to be send to)\n
        Set the initial size at zero; the DH now has a variable size and the
        headers will be send and received.  The method setVarDataSize can be
        used to change the size at run time.
  */

  class DH_VarSize: public LOFAR::DataHolder
  {
  public:
    DH_VarSize (const string& name, 
                unsigned int initialSize, 
                unsigned int maxSize = 135000000); //maximum size in bytes

    virtual ~DH_VarSize();

    DataHolder* clone() const;

    virtual void init();
  
    // allow changing of the fixed data size
    void setSpoofedDataSize(int dataSize);

    // overload getDataSize so we can control the amount of bytes that is actually sent
    virtual int getDataSize();

  protected:
    DH_VarSize (const DH_VarSize&);

  private:

    /// Forbid assignment.
    DH_VarSize& operator= (const DH_VarSize&);

    // pointer to a dataPacket object
  
    int itsMaxSpoofedDataSize;
    int itsSpoofedDataSize;
    int itsOverhead;
  };

  inline int DH_VarSize::getDataSize()
  {
    return itsSpoofedDataSize;
  }

} // namespace LOFAR

#endif 


  
