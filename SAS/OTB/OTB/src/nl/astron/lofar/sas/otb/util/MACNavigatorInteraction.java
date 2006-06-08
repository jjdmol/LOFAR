/*
 * MACNavigatorInteraction.java
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

import java.io.*;
import org.apache.log4j.Logger;
import no.geosoft.cc.io.*;

/**
 * Class to provide a way to interact with the MAC navigator
 *
 * @created 26-01-2006, 10:16
 * @author blaakmeer
 * @version $Id$
 * @updated
 */
public class MACNavigatorInteraction implements FileListener {
    
    // Create a Log4J logger instance
    private static Logger logger = Logger.getLogger(MACNavigatorInteraction.class);
    
    private static final String LOGIN_FILE = new String("login");
    private static final String SELECTION_FILE = new String("selection");
    
    // the file monitor that monitors files that are changed by the MAC Navigator
    private FileMonitor itsFileMonitor;
    private String      itsMACInteractionMonitorPath;
    private String      itsMACInteractionWritePath;

    /** Creates a new instance of MACNavigatorInteraction */
    public MACNavigatorInteraction(StorageLocation storageLocation) {
        itsMACInteractionMonitorPath = storageLocation.getMACInteractionMonitorPath();
        itsMACInteractionWritePath = storageLocation.getMACInteractionWritePath();
        
        // create the file monitor
        itsFileMonitor = new FileMonitor(10000);
        // add myself as a listener
        itsFileMonitor.addListener(this);
        // add the files to be monitored
        itsFileMonitor.addFile(new File(itsMACInteractionMonitorPath + "/" + SELECTION_FILE));
        
    }

    /**
    * Writes the selection to the file that is monitored by the MAC Navigator
    * 
    * @param selection  name of the selected node
    */
    public void setCurrentSelection(String selection) {
        FileOutputStream out; // declare a file output object
        PrintStream p; // declare a print stream object

        try
        {
            // Create a new file output stream
            out = new FileOutputStream(itsMACInteractionWritePath + "/" + SELECTION_FILE);

            // Connect print stream to the output stream
            p = new PrintStream( out );

            p.println (selection);

            p.close();
        }
        catch (Exception e)
        {
        System.err.println ("Error writing to file");
        }
        
    }
    
    /**
    * Writes the login information to the file that is monitored by the MAC Navigator
    * 
    * @param username  name of the user
    * @param password  password of the user
    */
    public void setCurrentUser(String username, String password) {
        FileOutputStream out; // declare a file output object
        PrintStream p; // declare a print stream object

        try
        {
            // Create a new file output stream
            out = new FileOutputStream(itsMACInteractionWritePath + "/" + LOGIN_FILE);

            // Connect print stream to the output stream
            p = new PrintStream( out );
            
            p.println (username);
            p.println (password); // should it be encrypted?

            p.close();
        }
        catch (Exception e)
        {
        System.err.println ("Error writing to file");
        }
        
    }
    
    /**
    * Called when one of the monitored files are created, deleted
    * or modified.
    * 
    * @param file  File which has been changed.
    */
    public void fileChanged (File file) {
        logger.debug("fileChanged: " + file.getPath());
    }
}
