//# Constant.h: Real or complex constant
//#
//# Copyright (C) 2004
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

#ifndef MEQ_CONSTANT_H
#define MEQ_CONSTANT_H

//# Includes
#include <MEQ/Function.h>
#include <MEQ/Vells.h>
#include <Common/lofar_vector.h>

#pragma aidgroup Meq
#pragma types #Meq::Constant

// The comments below are used to automatically generate a default
// init-record for the class 

//defrec begin MeqConstant
//  Represents a constnat created on-the-fly.
//  A MeqConstant cannot have any children.
//field: value 0.0  
//  value - expected double/complex double scalar
//defrec end

namespace Meq {

class Constant: public Function
{
public:
  // Create a constant with the given value.
  explicit Constant (double value=0.);
  explicit Constant (const dcomplex& value);

  virtual ~Constant();

  virtual TypeId objectType() const
    { return TpMeqConstant; }

  // Get the requested result of the constant.
  virtual int getResult (Result::Ref& resref, 
                         const std::vector<Result::Ref>& childres,
                         const Request& req, bool newreq);

  // Initialize the object from the record.
  virtual void init (DataRecord::Ref::Xfer& initrec, Forest* frst);

  //## Standard debug info method
  virtual string sdebug (int detail = 1, const string& prefix = "",
			 const char* name = 0) const;

protected:
  // Set the state from the record.
  virtual void setStateImpl (DataRecord& rec, bool initializing);

private:
  Vells itsValue;
};


} // namespace Meq

#endif
