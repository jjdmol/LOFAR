//# Condeq.h: Class to for a condition equation
//#
//# Copyright (C) 2003
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

#ifndef MEQ_CONDEQ_H
#define MEQ_CONDEQ_H
    
#include <MEQ/Node.h>
#include <MEQ/VellSet.h>

#pragma types #Meq::Condeq


namespace Meq {

class Request;


class Condeq : public Node
{
public:
  Condeq();

  virtual ~Condeq();

  // Get the result for the given request.
  virtual int getResult (Result::Ref &resref, const Request&,
			     bool newReq);

  // Returns the class TypeId
  virtual TypeId objectType() const;

  // Check the children after they have been resolved in class Node.
  // The order of the children is the order as given when the Node object
  // was created.
  virtual void checkChildren();
};


} // namespace Meq

#endif
