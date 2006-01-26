/*
 * StorageLocation.java
 *
 * Created on January 26, 2006, 10:55 AM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.util;

import org.apache.log4j.Logger;

/**
 *
 * @author blaakmeer
 */
public class StorageLocation {
    
    // Create a Log4J logger instance
    static Logger logger = Logger.getLogger(StorageLocation.class);

    private OtdbRmi itsOtdbRmi; // OTDB reference
    
    /** Creates a new instance of StorageLocation */
    public StorageLocation(OtdbRmi otdbRmi) {
        itsOtdbRmi = otdbRmi;
    }
    
    public String getMACInteractionMonitorPath() {
        String path = new String("/tmp/OTB/MAC2OTB/");
        if(itsOtdbRmi == null) {
            logger.info("Not using OTDB to get storage location");
        }
        return path;
    }
    
    public String getMACInteractionWritePath() {
        String path = new String("/tmp/OTB/OTB2MAC/");
        if(itsOtdbRmi == null) {
            logger.info("Not using OTDB to get storage location");
        }
        return path;
    }
    
}
