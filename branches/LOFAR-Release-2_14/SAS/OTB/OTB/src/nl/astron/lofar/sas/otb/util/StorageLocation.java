/*
 * StorageLocation.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

/**
 * @created 26-01-2006, 10:55
 *
 * @author blaakmeer
 *
 * @version $Id$
 *
 * @updated
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
        String path = "/tmp/OTB/MAC2OTB/";
        if(itsOtdbRmi == null) {
            logger.info("Not using OTDB to get storage location");
        }
        return path;
    }
    
    public String getMACInteractionWritePath() {
        String path = "/tmp/OTB/OTB2MAC/";
        if(itsOtdbRmi == null) {
            logger.info("Not using OTDB to get storage location");
        }
        return path;
    }
    
}
