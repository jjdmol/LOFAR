//# gcfnav-util.ctl
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
//# utility functions for the Navigator.
//#

#uses "nav_fw/gcf-logging.ctl"


global bool     ACTIVEX_SUPPORTED      = false;
global string   ACTIVEX_TREE_CTRL      = "NOT FlyTreeXCtrl.FlyTreeX";
global string   ACTIVEX_TREE_CTRL_NAME = "FlyTreeXCtrl1";
global string   LIST_TREE_CTRL_NAME    = "list";
global string   TAB_VIEWS_CTRL_NAME    = "TabViews";



//==================== miscellaneous environmental functions =====================

///////////////////////////////////////////////////////////////////////////
//
// Function ActiveXSupported
//  
// returns true if the panel contains the ActiveX tree control
//  
///////////////////////////////////////////////////////////////////////////
bool ActiveXSupported() { 
	return ACTIVEX_SUPPORTED;
}


///////////////////////////////////////////////////////////////////////////
//
// Function setActiveXSupported
//  
// sets the global variable that indicates if activeX is supported
//
///////////////////////////////////////////////////////////////////////////
void setActiveXSupported() {
  	idispatch activeXctrl = 0;
  	if (activeXctrl == 0) {
    	LOG_TRACE("I cannot create a COM object!? What the ....?? You must be running Linux or something.", "");
    	ACTIVEX_SUPPORTED = false;
  	}
  	else {
    	LOG_TRACE("I can create a COM object! ", activeXctrl);
    	releaseComObject(activeXctrl);
    	ACTIVEX_SUPPORTED = true;
  	}
}

///////////////////////////////////////////////////////////////////////////
//
// Function getTreeCtrlName
//  
// returns the name of the ActiveX tree control if activeX is supported, 
// returns the name of the emulated tree control otherwise
//  
///////////////////////////////////////////////////////////////////////////
string getTreeCtrlName() {
  	if (ActiveXSupported()) {
    	return ACTIVEX_TREE_CTRL_NAME;
  	}
 	else {
    	return LIST_TREE_CTRL_NAME;
  	}
}

///////////////////////////////////////////////////////////////////////////
//
// Function getTreeCtrl
//  
// returns the ActiveX tree control shape if activeX is supported, 
// returns the emulated tree control shape otherwise
//  
///////////////////////////////////////////////////////////////////////////
shape getTreeCtrl() {
  	return getShape(getTreeCtrlName());
}

///////////////////////////////////////////////////////////////////////////
//
// Function getTabCtrlName
//  
// returns the name of the tab control that contains the views
//  
///////////////////////////////////////////////////////////////////////////
string getTabCtrlName() {
  	return TAB_VIEWS_CTRL_NAME;
}

///////////////////////////////////////////////////////////////////////////
//
// Function getTabCtrl
//  
// returns the tab control shape
//  
///////////////////////////////////////////////////////////////////////////
shape getTabCtrl() {
  	return getShape(getTabCtrlName());
}



