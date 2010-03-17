/*
 * SharedVars.java
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
 * 
 */

package nl.astron.lofar.sas.otb;

import java.rmi.Naming;
import java.rmi.RemoteException;
import java.util.HashMap;
import nl.astron.lofar.java.cep.jparmfacade.jParmFacadeInterface;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.ParmDBConfigurationHelper;
import org.apache.log4j.Logger;

/**
 * Class to hold variables that will be used between panels in the mainframe.
 * This ensures that the Mainframe will stay package independend.
 *
 * Programmers that implement panels for other LOFAR packages can enter variables that need to be known
 * between different pannels here. The Mainfram has a method to get this Holder class and so the variabels 
 * will be accessible between packages.
 *
 * Please keep the file organised and clean, add vars and their accessors under a tag with the package name they belong to
 *
 * @created 12-04-2006, 13:37
 * @author coolen
 * @version $Id$
 * @updated
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
    // holds the treeState from the current tree
    private int                              itsTreeState=-1;
    // holds the OtdbRmi Object (RMI/JNI access for OTDB)
    private static OtdbRmi                   itsOtdbRmi;
    // holds the jParmFacade Object (JNI access for ParmDB)
    private static jParmFacadeInterface      itsjParmFacade;
    
    //LogParam
    private String itsLogParamStartTime="";
    private String itsLogParamEndTime = "";
    private boolean setLogParamMostRecent=false;
    private int itsLogParamLevel=0;
    
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
        setTreeState(aTreeID);
    }
    
    /** sets the Current TreeState */
    public void setTreeState(int aTreeID) {
        try {
            itsTreeState=OtdbRmi.getRemoteOTDB().getTreeInfo(aTreeID,false).state;
        } catch (RemoteException ex) {
            logger.debug("Exception during setTreeState(TreeID: "+aTreeID+")" );
            ex.printStackTrace();
        }
    }
    
    /** gets the Current TreeState */
    public int getTreeState() {
        return itsTreeState;
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
                //LOAD XML CONFIG FILE
                
                HashMap<String,String> serverConfig = ParmDBConfigurationHelper.getInstance().getParmDBServerInformation();
                
                String aRC="rmi://"+serverConfig.get("rmihostname");
                aRC += ":"+serverConfig.get("rmiport")+"/"+jParmFacadeInterface.SERVICENAME;
                
                itsjParmFacade = (jParmFacadeInterface) Naming.lookup(aRC);
                
            } catch (Throwable e) {
                logger.error("jParmFacade could not be loaded : "+e.getMessage(),e);
            }
        }
        return itsjParmFacade;
    }
    
    //LogParam
    public String getLogParamStartTime() {
        return itsLogParamStartTime;
    }

    public void setLogParamStartTime(String aTime){
        itsLogParamStartTime = aTime;
    }
    
    public String getLogParamEndTime() {
        return itsLogParamEndTime;
    }

    public void setLogParamEndTime(String aTime){
        itsLogParamEndTime = aTime;
    }
    
    public boolean getLogParamMostRecent() {
        return setLogParamMostRecent;
    }
    
    public void setLogParamMostRecent(boolean aMostRecent ) {
        setLogParamMostRecent = aMostRecent;
    }
    
    public int getLogParamLevel() {
        return itsLogParamLevel;
    }
    
    public void setLogParamLevel(int  aLevel ) {
        itsLogParamLevel = aLevel;
    }
    private int itsLevel=0;
    /**
     * Creates a new instance of SharedVars
     */
    public SharedVars(MainFrame mainFrame) {
        itsMainFrame = mainFrame;
        
        //
        // SAS
        //
        itsOtdbRmi = new OtdbRmi(mainFrame);
        
        //LogParam
        itsLogParamStartTime = "";
        itsLogParamEndTime = "";
        setLogParamMostRecent=false;
        itsLogParamLevel=0;
                
        
        
        
        itsCurrentTreeID=0;
        itsCurrentComponentID=0;
        itsTreeState=-1;
        
    }
    
}
