//#  DH_RSP.cc: one line description
//#
//#  Copyright (C) 2006
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <CS1_Generator/DH_RSP.h>

namespace LOFAR {
  namespace CS1_Generator {

    DH_RSP::DH_RSP (const string& name,
                    const ACC::APS::ParameterSet pset)
                  : DataHolder       (name, "DH_RSP"),
                    itsEthernetFrame (0),
                    itsPSet          (pset)
    {
      itsBufSize         = EthernetFrame::getSize(itsPSet);
    }

    DH_RSP::DH_RSP(const DH_RSP& that)
    : DataHolder         (that),
      itsEthernetFrame   (0),
      itsBufSize         (that.itsBufSize),
      itsPSet            (that.itsPSet)
    {}

    DH_RSP::~DH_RSP()
    {}

    DataHolder* DH_RSP::clone() const
    {
      return new DH_RSP(*this);
    }

    void DH_RSP::init()
    {
      // Add the fields to the data definition.
      addField ("EthernetFrame", BlobField<char>(1, itsBufSize));
      // Create the data blob
      createDataBlock();
    }

    void DH_RSP::fillDataPointers()
    {
      itsFrame = new Frame(itsPSet, getData<char>("EthernetFrame"), itsBufSize);
      itsFrame->getData->clear();
    }
  } // namespace CS1_Generator
} // namespace LOFAR
