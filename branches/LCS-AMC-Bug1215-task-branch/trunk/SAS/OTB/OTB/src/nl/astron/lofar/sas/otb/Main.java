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


import com.darwinsys.lang.GetOpt;
import com.darwinsys.lang.GetOptDesc;
import java.awt.GraphicsEnvironment;
import java.awt.Rectangle;
import java.util.Iterator;
import java.util.Map;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

/**
 * This is the Main class for the OTB framework.
 *
 * @created 11-01-2006, 9:11
 * @author coolen
 * @version $Id$
 * @updated 
 */
public class Main {
    
    
    static Logger logger = Logger.getLogger(Main.class);
        
    /**
     * @param args the command line arguments
     */
    public static void main(String[] argv) {
        try {
            String logConfig = "OTB.log_prop";
            String server    = "lofar17.astron.nl";
            String port      = "10199";
            boolean errs     = false;
            
            GetOptDesc options[] = {
                new GetOptDesc('s', "server", true),
                new GetOptDesc('p', "port", true),
                new GetOptDesc('l', "logfile", true),
                new GetOptDesc('h', "help", false)
            };
            
            GetOpt parser = new GetOpt(options);
            Map optionsFound = parser.parseArguments(argv);
            Iterator it = optionsFound.keySet().iterator();
            while (it.hasNext()) {
                String key = (String)it.next();
                char c = key.charAt(0);
                switch (c) {
                    case 's':
                        server = (String)optionsFound.get(key);
                        break;
                    case 'p':
                        port = (String)optionsFound.get(key);
                        break;
                    case 'l':
                        logConfig = (String)optionsFound.get(key);
                        break;
                    case 'h':
                        errs = true;
                        break;
                    case '?':
                        errs = true;
                        break;
                    default:
                        throw new IllegalStateException(
                                "Unexpected option character: "+ c);
                }
            }
            if (errs) {
                System.err.println("Usage: OTB.jar [-s server] [-p port] [-l logFile] [-h]");
            }         
                
            PropertyConfigurator.configure(logConfig);
            logger.info("OTB started");

            MainFrame aMainFrame = new MainFrame(server,port);

            GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
            Rectangle screenRect = ge.getMaximumWindowBounds();
            aMainFrame.setSize(screenRect.getSize());
            aMainFrame.setVisible(true);
        }
        catch(Exception e) {
            // catch all exceptions and create a fatal error message, including 
            // a stack trace.
            System.out.println("Error: "+ e);
            logger.fatal("Fatal exception, OTB halted",e);
        }
    }
}
