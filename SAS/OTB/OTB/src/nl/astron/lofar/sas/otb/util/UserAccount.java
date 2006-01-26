/*
 * UserAccount.java
 *
 * Created on January 17, 2006, 4:18 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.util;

import org.apache.log4j.Logger;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.exceptions.NoAccessException;

/**
 *
 * @author blaakmeer
 */
public class UserAccount {
    
    // Create a Log4J logger instance
    static Logger logger = Logger.getLogger(UserAccount.class);

    // fields
    private OtdbRmi itsOtdbRmi;
    private int itsRoleMask;

    /**
     * Creates a new instance of UserAccount 
     */
    public UserAccount(OtdbRmi otdbrmi, String userName, String password) throws NoAccessException {
        itsOtdbRmi = otdbrmi;
        itsRoleMask = 0;
        
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
        return true;
    }
    
    /** Checks if the specified role is an instrument scientist
      * 
      * @return true if the roleMask includes the InstrumentScientist role 
      */
    public boolean isInstrumentScientist() {
        // access the OTDB and check if the rolemask includes the InstrumentScientist role
        return true;
    }
    
    /** Checks if the specified role is an astronomer
      * 
      * @return true if the roleMask includes the Astronomer role 
      */
    public boolean isAstronomer() {
        // access the OTDB and check if the rolemask includes the Astronomer role
        return true;
    }
}
