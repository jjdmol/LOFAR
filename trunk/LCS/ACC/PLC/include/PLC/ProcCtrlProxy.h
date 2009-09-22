//# ProcCtrlProxy.h: Proxy for the ProcessControl class hierarchy.
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_PLC_PROCCTRLPROXY_H
#define LOFAR_PLC_PROCCTRLPROXY_H

// \file
// Proxy for the ProcessControl class hierarchy.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <Common/lofar_tribool.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
  //# Forward declarations
  class ParameterSet;

  namespace ACC
  {
    namespace PLC 
    {
      //# Forward declarations
      class ProcessControl;

      // \addtogroup PLC
      // @{

      // Proxy for the ProcessControl class hierarchy. This class provides
      // access to an instance of an implementation class of the
      // ProcessControl interface, henceforth called a PCImpl class.
      // 
      // This class is itself a base class for:
      // - ProcCtrlCmdLine, which provides a standard sequence of calls of the
      //   functions of ProcessControl;
      // - ProcCtrlRemote, which puts the process under control of ACC;
      //   i.e.\ ACC determines which functions of ProcessControl will be
      //   called when.
      class ProcCtrlProxy
      {
      public:
        // Destructor
        virtual ~ProcCtrlProxy();

        // "Implementation" of the ProcessControl methods. For detailed
        // documentation of these methods, please refer to the ProcessControl
        // class.
        // @{
        tribool define   ();
        tribool init     ();
        tribool run      ();
        tribool pause    (const string& condition);
        tribool release  ();
        tribool quit     ();
        tribool snapshot (const string& destination);
        tribool recover  (const string& source);
        tribool reinit   (const string& configID);
        string  askInfo  (const string& keylist);

		virtual void sendResultParameters(const string&		aKVList);

        bool inRunState() const;
        void setRunState();
        void clearRunState();
        // @}

      protected:
        // Constructor. Keep a pointer to the "real" Process Control object.
        //
        // \warning Make sure that the lifetime of the "real" Process Control
        // object exceeds that of the ProcCtrlProxy object. Otherwise,
        // ProcCtrlProxy will use a dangling pointer.
        ProcCtrlProxy(ProcessControl* aProcCtrl);

        // Start the process controller. Arguments can be passed in a generic
        // way, using a ParameterSet. This method must be implemented by the
        // derived classes.
        // \return 0 : success
        // \return 1 : failure
        virtual int operator()(const ParameterSet& arg) = 0;

      private:
        // Pointer to the "real" ProcessControl object.
        ProcessControl* itsProcCtrl;
      };

      // @}

    } // namespace PLC

  } // namespace ACC

} // namespace LOFAR

#endif
