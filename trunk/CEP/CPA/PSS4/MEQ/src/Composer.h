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
//  A MeqComposer concatenates the result sets of all its children into
//  a single result set.
//field: contagious_fail F
//  If true, then any fail result on any child causes the composer to generate
//  a complete fail -- i.e., a resultset composed entirely of fails.
//  If false (default), then fail results from children are collected and 
//  passed along as if they were normal results.
//defrec end

namespace Meq {    


class Composer : public Node
{
  public:
    Composer ();
    virtual ~Composer ();
    
    virtual void init (DataRecord::Ref::Xfer &initrec, Forest* frst);
    
    virtual void setState (const DataRecord &rec);
    
    virtual TypeId objectType() const
    { return TpMeqComposer; }
    
  protected:
    int getResult (Result::Ref &resref, const Request& request, bool newReq);
  
  private:
    bool contagious_fail;
};


} // namespace Meq

#endif
