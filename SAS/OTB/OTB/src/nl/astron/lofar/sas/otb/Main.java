/*
 * Main.java
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
 *
 */

package nl.astron.lofar.sas.otb;

import java.awt.GraphicsEnvironment;
import java.awt.Rectangle;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

/**
 * This is the Main class for the OTB framework.
 *
 * @created 11-01-2006, 9:11
 * @author blaakmeer
 * @version $Id$
 * @updated
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
