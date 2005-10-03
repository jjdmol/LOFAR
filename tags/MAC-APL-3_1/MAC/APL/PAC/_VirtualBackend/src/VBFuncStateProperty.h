//#  VBFuncStateProperty.h: 
//#
//#  Copyright (C) 2002-2003
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

#ifndef VBFUNCSTATEPROPERTY_H
#define VBFUNCSTATEPROPERTY_H

#include <GCF/PAL/GCF_ExtProperty.h>
#include <GCF/GCF_PVChar.h>

namespace LOFAR 
{
  namespace AVB
  {
class VBQAnswer;

class VBFuncStateProperty : public GCF::PAL::GCFExtProperty
{
  public:
    VBFuncStateProperty (const string& propName, VBQAnswer& answer);
    virtual ~VBFuncStateProperty ();

    unsigned char getCurrentValue() const;
    void startMonitoring();

  private:
    void valueChanged (const GCF::Common::GCFPValue& value);
    void valueGet (const GCF::Common::GCFPValue& value);
    void subscriptionLost ();
    
  private:
    VBFuncStateProperty();

    // Don't allow copying this object.
    // <group>
    VBFuncStateProperty (const VBFuncStateProperty&);
    VBFuncStateProperty& operator= (const VBFuncStateProperty&);  
    // </group>

  private: // data members
    GCF::Common::GCFPVChar _value;
    
    ALLOC_TRACER_CONTEXT
};

inline VBFuncStateProperty::~VBFuncStateProperty ()
{
}

inline unsigned char VBFuncStateProperty::getCurrentValue() const
{
  return _value.getValue();
}

 } // namespace AVB
} // namespace LOFAR
#endif
