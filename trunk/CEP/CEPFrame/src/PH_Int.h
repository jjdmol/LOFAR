//#  PH_Int.h: A simple ParamHolder containing an integer
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

#if !defined(PARAMS_PH_INT_H)
#define PARAMS_PH_INT_H

//# Includes
#include "CEPFrame/ParamHolder.h"

//# Forward Declarations


// Description of class.

class PH_Int : public ParamHolder
{
 public:
  explicit PH_Int(const string& name);

  virtual ~PH_Int();

  virtual ParamHolder* clone() const;

  virtual void preprocess();

  const int getValue() const;
  
  void setValue(int newvalue);

 protected:
  class ParamPacket: public ParamHolder::ParamPacket
  {
    public:
      ParamPacket() : itsValue(0) {};
      
      int itsValue;
  };

  PH_Int(const PH_Int&);  

  private:
  ParamPacket* itsParamPacket;

};


inline const int PH_Int::getValue() const
{
  return itsParamPacket->itsValue;
}

inline void PH_Int::setValue(int newvalue)
{
  itsParamPacket->itsValue = newvalue;
}

#endif
