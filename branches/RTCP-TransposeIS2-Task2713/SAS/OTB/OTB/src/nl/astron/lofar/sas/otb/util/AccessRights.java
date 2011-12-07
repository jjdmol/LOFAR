/*
 *  AccessRights.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */ 

package nl.astron.lofar.sas.otb.util;

import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBparam;

/**
 * @created 24-01-2007, 12:48
 *
 * @author coolen
 *
 * @version $Id$
 *
 * @updated
 */
public class AccessRights {
    
    private MainFrame itsMainFrame; 
    
    /** Creates a new instance of AccessRights */
    public AccessRights() {
        itsMainFrame = null;
    }
    
    public void setMainFrame(MainFrame aMainFrame){
        itsMainFrame = aMainFrame;  
    }
    /**
     * Returns the accessrights for this parameter
     *
     * @param aParam  the parameter you want to check accessrights on
     * @return false if this param has no accessrights, otherwise true
     */
    public boolean isWritable(jOTDBparam aParam) {
        
        int aTreeState= itsMainFrame.getSharedVars().getTreeState();
        
        // if treeState == 0 all is allowed
        if (aTreeState == 0) {
            return true;
        }
        
        // if treeState > 300 Nothing is allowed
        if (aTreeState > 300) {
            return false;
        }
    
        short validation = aParam.valMoment;

        // all other cases (states between 0 and 300 inclusive)
        // if validation = 0 All is allowed, otherwise nothing is allowed
        if (validation == 0) {
            return true;
        } else {
            return false;
        }
    }
}
