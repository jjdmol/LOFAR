/*
 * SharedVars.java
 *
 * Created on April 12, 2006, 1:37 PM
 *
 * Class to hold variables that will be used between panels in the mainframe.
 * This ensures that the Mainframe will stay package independend.
 *
 * Programmers that implement panels for other LOFAR packages can enter variables that need to be known
 * between different pannels here. The Mainfram has a method to get this Holder class and so the variabes 
 * will be accessible between packages.
 *
 * Please keep the file organised and cleam, add vars and their accessors under a tag with the package name they belong to
 * 
 */

package nl.astron.lofar.sas.otb;

import java.rmi.Naming;
import nl.astron.lofar.java.cep.jparmfacade.jParmFacadeInterface;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import org.apache.log4j.Logger;

/**
 *
 * @author coolen
 */




public class SharedVars {
    
    static Logger logger = Logger.getLogger(SharedVars.class);
    static String name = "SharedVars";
    
      
    
    private MainFrame itsMainFrame=null;
    
    
    /* 
     * PACKAGE SAS 
     */
    // holds the current Tree ID
    private int                              itsCurrentTreeID=0;
    // holds the current component ID
    private int                              itsCurrentComponentID=0;
    // holds the OtdbRmi Object (RMI/JNI access for OTDB)
    private static OtdbRmi                   itsOtdbRmi;
    // holds the jParmFacade Object (JNI access for ParmDB)
    private static jParmFacadeInterface      itsjParmFacade;
    
    /*
     * PACKAGE SAS
     */
        /** gets the Current TreeID */
    public int getTreeID() {
        return itsCurrentTreeID;
    }
    
    /** sets the Current TreeID */
    public void setTreeID(int aTreeID) {
        itsCurrentTreeID=aTreeID;
    }
    
    /** gets the Current ComponentID */
    public int getComponentID() {
        return itsCurrentComponentID;
    }
    
    /** sets the Current ComponentID */
    public void setComponentID(int anID) {
        itsCurrentComponentID=anID;
    }
    
    /** gets OTDBrmi
     * OTBrmi holds all JNI/RMI connections
     *
     * @return the OtdbRmi object
     */
    public static OtdbRmi getOTDBrmi() {
        return itsOtdbRmi;
    }
    
    public static jParmFacadeInterface getJParmFacade() {
        if(itsjParmFacade == null){
            try {
                String hostname = "localhost";
                int port = 10668;
                hostname=getOTDBrmi().RMIServerName;
                String aRC="rmi://"+hostname+":"+port+"/"+jParmFacadeInterface.SERVICENAME;
        
                itsjParmFacade = (jParmFacadeInterface) Naming.lookup (aRC);

             } catch (Throwable e) {
                logger.error("jParmFacade could not be loaded : "+e.getMessage(),e);
             } 
         }
        return itsjParmFacade;
        
    }
    
    /**
     * Creates a new instance of SharedVars
     */
    public SharedVars(MainFrame mainFrame) {
        itsMainFrame = mainFrame;
        
        //
        // SAS
        //
        itsOtdbRmi = new OtdbRmi(mainFrame);
                
        //ParmDB
        
        
                    
        itsCurrentTreeID=0;
        itsCurrentComponentID=0;

    }
    
}
