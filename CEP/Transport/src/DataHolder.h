//# DataHolder.h: Abstract base class for the data holders
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

#ifndef LIBTRANSPORT_DATAHOLDER_H
#define LIBTRANSPORT_DATAHOLDER_H

#include <lofar_config.h>

//# Includes
#include <Common/lofar_string.h>
#include <libTransport/BaseDataHolder.h>

namespace LOFAR
{

//# Forward Declarations

/**
  Class DataHolder is the abstract base class for all data holders
  in the CEPFrame environment. Its main purpose is to offer a common interface
  to a class like WorkHolder. Apart from that it also offers some common
  functionality to the classes derived from it.

  DataHolder has an internal class called DataPacket. This class holds
  the data of a DataHolder class. A Class derived from DataHolder
  should also have an internal class to hold its data. That class should
  be derived from DataHolder::DataPacket.
  The basic DataPacket class offers some functions to set, get, and
  compare the timestamp of a data packet.

  The constructors of a class derived from DataHolder should always
  call the function setDataPacket in order to make their DataPacket object
  known to this base class.

  Prove \code list<table> \endcode or \<table\>.
  \code
    main()
    {
       list<table> l;
    }
  \endcode
*/

class DataHolder: public BaseDataHolder
{

public:
  /// Construct a DataHolder with a default name
  DataHolder (const string& name="aDataHolder",
	      const string& type="DH");
};


}
#endif
