/*
 * MACNavigatorInteraction.java
 *
 * Created on January 26, 2006, 10:16 AM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.util;

import java.io.*;
import org.apache.log4j.Logger;
import no.geosoft.cc.io.*;

/**
 *
 * @author blaakmeer
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
