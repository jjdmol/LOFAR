//# gcfpa-com.ctl
//#
//#  Copyright (C) 2002-2004
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

//#
//# Functions to send events to the PropertyAgent
//# 

#uses "gcf-logging.ctl"

////////////////////////////////////////////////////////////////////////////////
//
// Function sendEventToPA (message, systemName)
//
// Delivers the message to the PA by writing is to '<sys>:__gcfportAPI_DPAserver'
//
////////////////////////////////////////////////////////////////////////////////
void sendEventToPA(string msg, string destSysName) {
	LOG_DEBUG("sendEventToPA: ", msg, destSysName);

	sendEvent(destSysName + "__gcfportAPI_DPAserver.", msg);
}

////////////////////////////////////////////////////////////////////////////////
//
// Private Function sendEvent (dp, message)
//
// Write message as a blob to the given DP.
//
////////////////////////////////////////////////////////////////////////////////
void sendEvent(string dest, string msg) {
	LOG_DEBUG("Msg: ", msg);

	blob event;
	blobZero(event, strlen(msg) + 1); // "+ 1" workaround for known bug in CTRL impl.
	blobSetValue(event, 0, msg, strlen(msg));
	dpSet(dest, event);
}

