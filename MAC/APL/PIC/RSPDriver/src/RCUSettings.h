//#  -*- mode: c++ -*-
//#
//#  RCUSettings.h: RCU control information
//#
//#  Copyright (C) 2002-2004
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

#ifndef RCUSETTINGS_H_
#define RCUSETTINGS_H_

#include <complex>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace RSP_Protocol
{
  class RCUSettings
  {
  public:
    /**
     * Setting bitfield for an RCU.
     */
    typedef struct RCURegisterType
    {
      uint8 lba_enable:1;
      uint8 hba_enable:1;
      uint8 bandsel:1;
      uint8 filsel_a:1;
      uint8 filsel_b:1;
      uint8 vl_enable:1;
      uint8 vh_enable:1;
      uint8 vddvcc_en:1;
    };

    /**
     * Constructors for a RCUSettings object.
     * Currently the tv_usec part is always set to 0 irrespective
     * of the value passed in.
     */
    RCUSettings() { }
	  
    /* Destructor for RCUSettings. */
    virtual ~RCUSettings() {}

    /* get reference settings array */
    blitz::Array<RCURegisterType, 1>& operator()();

  public:
    /*@{*/
    /**
     * marshalling methods
     */
    unsigned int getSize();
    unsigned int pack  (void* buffer);
    unsigned int unpack(void *buffer);
    /*@}*/

  private:
    blitz::Array<RCURegisterType, 1> m_registers;
  };
  
  inline blitz::Array<RCUSettings::RCURegisterType, 1>& RCUSettings::operator()() { return m_registers; }

};
     
#endif /* RCUSETTINGS_H_ */
