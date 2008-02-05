//  Composite.cc:
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
//
//////////////////////////////////////////////////////////////////////

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <CEPFrame/Composite.h>
#include <CEPFrame/NetworkBuilder.h>

namespace LOFAR
{

Composite::Composite()
  : Block(),
    itsComposite(0)
{}

Composite::Composite (int nrInputs,
		      int nrOutputs,
		      const string& name,
		      bool addNameSuffix)
  : Block()
{
  itsComposite = new CompositeRep (nrInputs, nrOutputs, name, addNameSuffix);
  itsRep = itsComposite;
}

Composite::Composite (int nrInputs,
		      int nrOutputs,
		      NetworkBuilder& aBuilder, 
		      const string& name,
		      bool addNameSuffix)
  : Block()
{
  itsComposite = new CompositeRep (nrInputs, nrOutputs, 
				   name, addNameSuffix);
  itsRep = itsComposite;
  aBuilder.buildNetwork (*this);
}

Composite::Composite (const Composite& that)
  : Block (that)
{
  itsComposite = that.itsComposite;
}

Composite::Composite (CompositeRep* rep)
: Block (rep)
{
  itsComposite = rep;
}

Composite& Composite::operator= (const Composite& that)
{
  if (this != &that) {
    Block::operator= (that);
    itsComposite = that.itsComposite;
  }
  return *this;
}

Composite::~Composite() 
{}

Composite* Composite::clone() const
{
  return new Composite (*this);
}

}




