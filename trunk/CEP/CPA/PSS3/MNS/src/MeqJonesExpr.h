//# MeqJonesExpr.h: The base class of a Jones matrix expression.
//#
//# Copyright (C) 2002
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

#if !defined(MNS_MEQJONESEXPR_H)
#define MNS_MEQJONESEXPR_H

//# Includes
#include <MNS/MeqResult.h>

//# Forward declarations
class MeqRequest;


// This class is the (abstract) base class for an expression.

class MeqJonesExpr
{
public:
  // The default constructor.
  MeqJonesExpr()
    {};

  virtual ~MeqJonesExpr();

  // Calculate the result of its members.
  virtual void calcResult (const MeqRequest&) = 0;

  // Get the result of the given element.
  const MeqResult& getResult11() const
    { return its11; }
  const MeqResult& getResult12() const
    { return its12; }
  const MeqResult& getResult21() const
    { return its21; }
  const MeqResult& getResult22() const
    { return its22; }

protected:
  // Set the result.
  void setResult11 (const MeqResult& result)
    { its11 = result; }
  void setResult12 (const MeqResult& result)
    { its12 = result; }
  void setResult21 (const MeqResult& result)
    { its21 = result; }
  void setResult22 (const MeqResult& result)
    { its22 = result; }

  // Get write access to the result.
  MeqResult& result11()
    { return its11; }
  MeqResult& result12()
    { return its12; }
  MeqResult& result21()
    { return its21; }
  MeqResult& result22()
    { return its22; }

private:
  MeqResult its11;
  MeqResult its12;
  MeqResult its21;
  MeqResult its22;
};


#endif
