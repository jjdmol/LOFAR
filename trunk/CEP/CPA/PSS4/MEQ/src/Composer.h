//# Composer.h: Selects result planes from a result set
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

#ifndef MEQ_COMPOSER_H
#define MEQ_COMPOSER_H
    
#include <MEQ/Node.h>

#pragma aidgroup Meq
#pragma types #Meq::Composer

// The comments below are used to automatically generate a default
// init-record for the class 

//defrec begin MeqComposer
//  A MeqComposer concatenates the results of all its children 
//  into a single result.
//field: contagious_fail F
//  If true, then a fail in any child result causes the composer to generate
//  a complete fail -- i.e., a result composed entirely of fails.
//  If false (default), then fail vellsets from children are collected and 
//  passed along with valid vellsets.
//defrec end

namespace Meq {    


class Composer : public Node
{
  public:
    Composer ();
    virtual ~Composer ();

    virtual TypeId objectType() const
    { return TpMeqComposer; }
    
  protected:
    virtual int getResult (Result::Ref &resref, const Request& request, bool newReq);
    virtual void checkInitState (DataRecord &rec);
    virtual void setStateImpl (DataRecord &rec,bool initializing);
  
  private:
    bool contagious_fail;
};


} // namespace Meq

#endif
