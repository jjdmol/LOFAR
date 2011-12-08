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
import java.io.File;
import java.util.Iterator;
import java.util.Map;
import nl.astron.lofar.sas.otb.exceptions.NoServerConnectionException;
import nl.astron.lofar.sas.otb.exceptions.NotLoggedInException;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import sun.misc.Signal;
import sun.misc.SignalHandler;

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
    static MainFrame itsMainFrame = null;
    private static boolean running = true;
    
    public static void init() {
    Runtime.getRuntime().addShutdownHook(new Thread() {
            @Override
      public void run() {
        System.out.println("Shutting down OTB");
      }
    });

    SignalHandler handler = new SignalHandler () {
            @Override
      public void handle(Signal sig) {
        if (running) {
          running = false;
          System.out.println("Signal " + sig);
          System.out.println("Shutting down connections to the database...");
          if (itsMainFrame != null)  itsMainFrame.exit();
        } else {
          // only on the second attempt do we exit
          System.out.println(" database connection shutdown interrupted!");
          System.exit(0);
        }
      }
    };
    Signal.handle(new Signal("INT"), handler);
    Signal.handle(new Signal("TERM"), handler);
  }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] argv) {
        init();
        try {
            String logConfig = "OTB.log_prop";
            String server    = "sas001";
            String port      = "10199";
            String database  = "LOFAR_2";
            String user      = "observer";

            boolean errs     = false;
            GetOptDesc options[] = {
                new GetOptDesc('s', "server", true),
                new GetOptDesc('p', "port", true),
                new GetOptDesc('l', "logfile", true),
                new GetOptDesc('d', "database", true),
                new GetOptDesc('u', "user", true),
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
                    case 'd':
                        database = (String)optionsFound.get(key);
                        break;
                    case 'u':
                        user = (String)optionsFound.get(key);
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
                System.err.println("Usage: OTB.jar [-s server] [-p port] [-d database] [-u username] [-l logFile] [-h]");
            }         

            File f = new File(logConfig);
            if (f.exists()) {
                PropertyConfigurator.configure(logConfig);
            } else {
                logConfig = File.separator+"opt"+File.separator+"sas"+File.separator+logConfig;
                f = new File(logConfig);
                if (f.exists()) {
                    PropertyConfigurator.configure(logConfig);
                } else {
                    logger.error("OTB.log_prop not found.");
                }
            }
            logger.info("OTB started");

            try {
               itsMainFrame = new MainFrame(server,port,database,user);

               GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
               Rectangle screenRect = ge.getMaximumWindowBounds();
               itsMainFrame.setSize(screenRect.getSize());

               itsMainFrame.setVisible(true);
            }
            catch(NoServerConnectionException ex ) {
                String aS="ex";
                logger.error(aS);
            }
            catch (NotLoggedInException ex ) {
                logger.error(ex);
            }
        }
        catch(Exception e) {
            // catch all exceptions and create a fatal error message, including 
            // a stack trace.
            logger.fatal("Fatal exception, OTB halted",e);
        }
    }
}
