/*
 * UserAccount.java
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

import org.apache.log4j.Logger;
import nl.astron.lofar.sas.otb.exceptions.NoAccessException;

/**
 * @created 17-01-2006, 16:18
 *
 * @author coolen
 *
 * @version $Id$
 *
 * @updated
 */
public class UserAccount {
    
    // Create a Log4J logger instance
    static Logger logger = Logger.getLogger(UserAccount.class);

    // fields
    private OtdbRmi itsOtdbRmi;
    private int itsRoleMask;
    private String itsUserName;

    /**
     * Creates a new instance of UserAccount 
     */
    public UserAccount(OtdbRmi otdbrmi, String userName, String password) throws NoAccessException {
        itsOtdbRmi = otdbrmi;
        itsRoleMask = 0;
        itsUserName=userName;
        
        // For test purposes only
        if(userName.length() == 0) {
            throw new NoAccessException("No access");
        }
        
        // get rolemask from OTDB
    }

    /** Checks if the specified role is an administrator
      * 
      * @return true if the roleMask includes the Administrator role 
      */
    public boolean isAdministrator() {
        // access the OTDB and check if the rolemask includes the Administrator role
        if (itsUserName.equalsIgnoreCase("observer")) {
            return false;
        } else {
            return true;
        }
    }
    
    /** Checks if the specified role is an instrument scientist
      * 
      * @return true if the roleMask includes the InstrumentScientist role 
      */
    public boolean isInstrumentScientist() {
        // access the OTDB and check if the rolemask includes the InstrumentScientist role
        // for now this is default
        if (itsUserName.equalsIgnoreCase("observer")) {
            return false;
        } else {
            return true;
        }
    }
    
    /** Checks if the specified role is an astronomer
      * 
      * @return true if the roleMask includes the Astronomer role 
      */
    public boolean isAstronomer() {
        // access the OTDB and check if the rolemask includes the Astronomer role
        if (itsUserName.equalsIgnoreCase("observer")) {
            return false;
        } else {
            return true;
        }

    }
    
    /** Checks if the specified role is an administrator
      *
      * @return true if the roleMask includes the Administrator role
      */
    public boolean isObserver() {
        // access the OTDB and check if the rolemask includes the Administrator role
        // for now if userName == observer
        if (itsUserName.equalsIgnoreCase("observer")) {
            return true;
        } else {
            return false;
        }
    }

    /** Give back the name of the current user
     *
     * @return the name of the current user
     */
    public String getUserName() {
        return itsUserName;
    }
}
