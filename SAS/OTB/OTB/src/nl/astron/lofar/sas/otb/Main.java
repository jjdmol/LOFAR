/*
 * Main.java
 *
 * Created on January 11, 2006, 9:11 AM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb;

import java.awt.GraphicsEnvironment;
import java.awt.Rectangle;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

/**
 *
 * @author blaakmeer
 */
public class Main {
    
    
    static Logger logger = Logger.getLogger(Main.class);
        
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        try {
            String logConfig = "OTB.log_prop";
            if(args.length > 0) {
                logConfig = args[0];
            }
                
            PropertyConfigurator.configure(logConfig);
            logger.info("OTB started");

            MainFrame aMainFrame = new MainFrame();

            GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
            Rectangle screenRect = ge.getMaximumWindowBounds();
            aMainFrame.setSize(screenRect.getSize());
            aMainFrame.setVisible(true);
        }
        catch(Exception e) {
            // catch all exceptions and create a fatal error message, including 
            // a stack trace.
            logger.fatal("Fatal exception, OTB halted",e);
        }
    }
}
