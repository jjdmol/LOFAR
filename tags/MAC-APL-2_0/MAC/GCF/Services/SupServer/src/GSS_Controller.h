//# GSS_Controller.h: the supervisory server for each ERTC board,
//#                        intermedier between ERTC controllers and LCU
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
//
//# $Id$

#ifndef GSS_CONTROLLER_H
#define GSS_CONTROLLER_H

#include <GCF/GCF_Task.h>
#include <GCF/GCF_TCPPort.h>
#include <Common/lofar_list.h>
#include <Common/lofar_string.h>

/**
 * 
 */

class GSSController : public GCFTask
{
	public:
		/**
		*  Constructor
		*  \param locName name of the hardware location where
		*  this instance will be started, this name must correspond with
		*  the name choosen in the mac.ns file (must always start with "SV")
		*/
		GSSController();
		~GSSController();

		/**
		* State methods
		*/
		GCFEvent::TResult initial_state(GCFEvent& e, GCFPortInterface& p);
		GCFEvent::TResult operational_state(GCFEvent& e, GCFPortInterface& p);

	private: // private methods
    bool findScope(string& scope);
    void sendMsgToPI(GCFEvent& e); 
    bool forwardMsgToPMLlite(GCFEvent& e, string& scope, char* logMsg);
    void replyMsgToPMLlite(GCFEvent& e, GCFPortInterface& p);
    

	private: // data members
    static const unsigned int MAX_NR_OF_CLIENTS = 64;

		GCFTCPPort* _supClientPorts[MAX_NR_OF_CLIENTS];
    typedef map<string /*scope*/, GCFPortInterface*> TScopeRegister;
    TScopeRegister    _scopeRegister;
    GCFTCPPort        _scPortProvider;
		GCFTCPPort       _propertyInterface;
};

#endif
