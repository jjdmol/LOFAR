//# BBSProcessControl.h: Implementation of ACC/PLC ProcessControl class.
//#
//# Copyright (C) 2006
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

#ifndef LOFAR_BBSCONTROL_BBSPROCESSCONTROL_H
#define LOFAR_BBSCONTROL_BBSPROCESSCONTROL_H

// \file
// Implementation of ACC/PLC ProcessControl class

//# Includes
#include <PLC/ProcessControl.h>
#include <Common/lofar_smartptr.h>
#include <boost/logic/tribool_fwd.hpp>

namespace LOFAR
{
  //# Forward Declarations.

  namespace BBS
  {
    //# Forward Declarations.
    class BBSStrategy;
    class BBSStep;

    // \addtogroup BBS
    // @{

    class BBSProcessControl : public ACC::PLC::ProcessControl
    {
    public:
      BBSProcessControl();
      virtual ~BBSProcessControl();
      virtual boost::logic::tribool define();
      virtual boost::logic::tribool init();
      virtual boost::logic::tribool run();
      virtual boost::logic::tribool quit();
    private:
      // The operations below are not supported by BBSProcessControl.
      // @{
      virtual boost::logic::tribool pause(const string& condition);
      virtual boost::logic::tribool snapshot(const string& destination);
      virtual boost::logic::tribool recover(const string& source);
      virtual boost::logic::tribool reinit(const string& configID);
      virtual string                askInfo(const string& keylist);
      // @}

      // Our parameter set.
      ACC::APS::ParameterSet  itsParamSet;

      // The strategy that will be constructed from the parameter set.
      shared_ptr<BBSStrategy> itsStrategy;

      // Vector containing all the separate steps that this strategy consists
      // of in sequential order.
      vector<const BBSStep*>  itsSteps;

    };

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
