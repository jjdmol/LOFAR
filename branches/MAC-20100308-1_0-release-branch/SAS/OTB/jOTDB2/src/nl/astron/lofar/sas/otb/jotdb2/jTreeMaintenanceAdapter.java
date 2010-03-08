//#  jTreeMaintenanceAdapter.java: The RMI adapter of the OTDB database.
//#
//#  Copyright (C) 2002-2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
  
package nl.astron.lofar.sas.otb.jotdb2;

import java.util.Vector;
import java.rmi.server.UnicastRemoteObject;
import java.rmi.RemoteException;
import org.apache.log4j.Logger;

public class jTreeMaintenanceAdapter extends UnicastRemoteObject implements jTreeMaintenanceInterface
{

   // Create a Log4J logger instance
   static Logger logger = Logger.getLogger(jTreeMaintenanceAdapter.class);

   // Constructor
   public jTreeMaintenanceAdapter (jTreeMaintenance adaptee) throws RemoteException
     {
	this.adaptee = adaptee;
     }
   
   

    //# --- PIC maintenance ---
    // Once in a while a new PIC tree will be loaded from PVSS which manages
    // the master PIC. The master PIC will be in a exported ASCII file, with
    // loadMasterFile this file can be added.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    public int loadMasterFile (String  filename) throws RemoteException {
        int anI;
        try {
            anI = adaptee.loadMasterFile(filename);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI loadMasterFile error",ex);
            throw anEx;            
        }
        return anI;            
    }

    //# --- VIC maintenance : Components ---
    // A VIC tree is build up from single components. The definitions of the
    // components can loaded from a file with this call
    public  int loadComponentFile (String filename,String forcedVersionNr, String forcedQualifier) throws RemoteException {
        int anI;
        try {
            anI = adaptee.loadComponentFile(filename,forcedVersionNr,forcedQualifier);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI loadComponentFile error",ex);
            throw anEx;            
        }
        return anI;              
    }

    // Find the top-components in the components table.
    public Vector<jVICnodeDef> getComponentList (String name , boolean topOnly) throws RemoteException {
        Vector<jVICnodeDef> aV=null;
        try {
            aV = adaptee.getComponentList(name,topOnly);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getComponentList error",ex);
            throw anEx;            
        }
        return aV;            
    }

    // Get the node definition of a VC node
    public jVICnodeDef getComponentNode(int aNodeID) throws RemoteException {
        jVICnodeDef aN=null;
        try {
            aN = adaptee.getComponentNode(aNodeID);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getComponentNode error",ex);
            throw anEx;            
        }
        return aN;              
    }

    // Get parameterlist of the component
    public Vector<jOTDBparam> getComponentParams(int aNodeID) throws RemoteException {
        Vector<jOTDBparam> aP=null;
        try {
            aP = adaptee.getComponentParams(aNodeID);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getComponentsParam error",ex);
            throw anEx;            
        }
        return aP;             
    }

    // Save new node or update the limits and description fields of the node
    public boolean  saveComponentNode(jVICnodeDef  aNode) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.saveComponentNode(aNode);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI saveComponentNode error",ex);
            throw anEx;            
        }
        return aB;            
    }

    // test if component is a top-component
    public boolean  isTopComponent(int aNodeID) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.isTopComponent(aNodeID);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI isTopComponent error",ex);
            throw anEx;            
        }
        return aB;              
    }

    // delete a component node
    public boolean deleteComponentNode(int  aNodeID) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.deleteComponentNode(aNodeID);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI deleteComponentNode error",ex);
            throw anEx;            
        }
        return aB;              
    }
    
    // get full Component Name
    public String  getFullComponentName(jVICnodeDef  aNode) throws RemoteException {
        String aS="";
        try {
            aS = adaptee.getFullComponentName(aNode);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getFullComponentName error",ex);
            throw anEx;            
        }
        return aS;              
    }

    //# --- VIC maintenance : Templates ---
    // From a component tree a template tree can be constructed. In a template
    // tree only the structure of the tree is created, there is no replication
    // of nodes on the same level.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    public int buildTemplateTree (int topNodeID, short aClassif) throws RemoteException {
        int anI;
        try {
            anI = adaptee.buildTemplateTree (topNodeID, aClassif);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI buildTemplateTree error",ex);
            throw anEx;            
        }
        return anI;
     }

    // Create a new OTDBtree record for an Template tree in the database
    // and return its treeID.
    public int newTemplateTree() throws RemoteException {
        int anI;
        try {
            anI = adaptee.newTemplateTree();
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI newTemplateTree error",ex);
            throw anEx;            
        }
        return anI;              
    }

    // Make a copy of an existing template tree.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    public int copyTemplateTree (int aTreeID) throws RemoteException {
        int anI;
        try {
            anI = adaptee.copyTemplateTree (aTreeID);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI copyTemplateTree error",ex);
            throw anEx;            
        }
        return anI;              
     }
   
    // Get a single node from the VIC template tree
    public jOTDBnode getNode (int aTreeID, int aNodeID) throws RemoteException {
        jOTDBnode aN=null;
        try {
            aN = adaptee.getNode (aTreeID, aNodeID);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getNode error",ex);
            throw anEx;            
        }
        return aN;              
     }

    // Get the parameter definition of a node
    public jOTDBparam getParam(int aTreeID,int aParamID) throws RemoteException {
        jOTDBparam aP=null;
        try {
            aP = adaptee.getParam(aTreeID,aParamID);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getParam error",ex);
            throw anEx;            
        }
        return aP;  
    }

    // Get the parameter definition of a node. Will recursively follow the
    // references in the limits field.
    public jOTDBparam getParam(jOTDBnode aNode) throws RemoteException {
        jOTDBparam aP=null;
        try {
            aP = adaptee.getParam(aNode);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getParam(OTDBnode) error",ex);
            throw anEx;            
        }
        return aP;          
    }
    
    // Save the parameter definition
    public boolean saveParam(jOTDBparam aParam) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.saveParam(aParam);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI saveParam error",ex);
            throw anEx;            
        }
        return aB;            
    }

    // Get a number of levels of children.
    public Vector getItemList (int aTreeID, int topNode, int depth) throws RemoteException {
        Vector aV=null;
        try {
            aV = adaptee.getItemList (aTreeID, topNode, depth);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getItemList error",ex);
            throw anEx;            
        }
        return aV;
     }
   
    // Get a list of nodes based on a namefragment. Use '%' as wildchar.
    public Vector getItemList (int aTreeID, String aNameFragment) throws RemoteException {
        Vector aV=null;
        try {
            aV = adaptee.getItemList (aTreeID, aNameFragment);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getItemList error",ex);
            throw anEx;            
        }
        return aV;            
     }
   
    // Duplicates the given node (and its parameters and children)
    // in the template database. The duplicate gets the new index.
    public int dupNode (int aTreeID, int orgNodeID, short newIndex) throws RemoteException {
        int anI;
        try {
            anI = adaptee.dupNode (aTreeID, orgNodeID, newIndex);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI dupNode error",ex);
            throw anEx;            
        }
        return anI;            
     }
   
    // Adds the given VIC Component under the given parent of a
    // template tree.
    public int addComponent (int compID,int treeID,int parentID, String newName) throws RemoteException {
        int anI;
        try {
            anI = adaptee.addComponent(compID,treeID,parentID, newName);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI addComponent error",ex);
            throw anEx;            
        }
        return anI;            
    }

    // Updates the (vector of) OTDBnodes to the database.
    public boolean saveNode (jOTDBnode aNode) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.saveNode (aNode);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI saveNode error",ex);
            throw anEx;            
        }
        return aB;            
            
     }
   
    public boolean saveNodeList (Vector aNodeList) throws RemoteException {
        boolean aB = false;
        try {
            aB = adaptee.saveNodeList (aNodeList);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI saveNodeList error",ex);
            throw anEx;            
        }
        return aB;
    }
   
    // Updates the (vector of) OTDBnodes to the database.
    public boolean deleteNode (jOTDBnode	aNode) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.deleteNode (aNode);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI deleteNode error",ex);
            throw anEx;            
        }
        return aB;            
     }
   
    public boolean deleteNodeList (Vector aNodeList) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.deleteNodeList (aNodeList);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI deleteNodeList error",ex);
            throw anEx;            
        }
        return aB;            
     }
   
    // Evaluate the constraints from a (sub)tree.
    public boolean checkTreeConstraints (int aTreeID, int topNode) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.checkTreeConstraints (aTreeID, topNode);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI checkTreeConstraints error",ex);
            throw anEx;            
        }
        return aB;            
     }
   
    //# --- VIC maintenance : Hierarchical trees ---
    // From a template tree a fully instanciated tree can be build.
    // Returns 0 on failure, otherwise the ID of the new tree is returned.
    public int instanciateTree (int baseTree) throws RemoteException {
        int anI;
        try {
            anI = adaptee.instanciateTree (baseTree);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI instantiateTree error",ex);
            throw anEx;            
        }
        return anI;
     }
   
    // Prune an instanciated tree to get loss of depricated values.
    public boolean pruneTree (int aTreeID, short pruningLevel) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.pruneTree (aTreeID, pruningLevel);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI pruneTree error",ex);
            throw anEx;            
        }
        return aB;            
     }

    // Export a VIC (sub)tree to a file. The user may choose in which format
    // the tree is exported: HTML, KeyValue List.
    public boolean exportTree (int aTreeID,int topItem,String filename,int outputFormat,boolean folded) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.exportTree(aTreeID,topItem,filename,outputFormat,folded);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI exportTree error",ex);
            throw anEx;            
        }
        return aB;            
    }
   
    //# --- Finally some general tree maintenance ---
    // Delete a tree (of any kind) from the database.
    public boolean deleteTree (int aTreeID) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.deleteTree (aTreeID);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI deleteTree error",ex);
            throw anEx;            
        }
        return aB;            
     }
   
    // Retrieve the topNode of any tree
    public jOTDBnode getTopNode (int aTreeID) throws RemoteException {
        jOTDBnode aN=null;
        try {
            aN = adaptee.getTopNode (aTreeID);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI getTopNode error",ex);
            throw anEx;            
        }
        return aN;            
     }
   
    public boolean setMomInfo(int aTreeID, int momID, String campaign) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.setMomInfo(aTreeID, momID, campaign);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI setMomInfo error",ex);
            throw anEx;            
        }
        return aB;            
    }  

    // Set the classification of any tree.
    public boolean setClassification (int aTreeID, short aClassification) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.setClassification (aTreeID, aClassification);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI setClassification error",ex);
            throw anEx;            
        }
        return aB;            
     }
   
    // Set the state of any tree. When changing the state of a tree all
    // constraints/validations for the current type must be fulfilled.
    // When errors occur these can be retrieved with the errorMsg function.
    public boolean setTreeState (int aTreeID, short aState) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.setTreeState (aTreeID, aState);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI setTreeState error",ex);
            throw anEx;            
        }
        return aB;
     }
   

    // Update the description of a tree.
    public boolean setDescription(int  aTreeID,String aDescription) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.setDescription( aTreeID,aDescription);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI setDescription error",ex);
            throw anEx;            
        }
        return aB;            
    }

    // Set the scheduling times of the tree
    public boolean setSchedule(int aTreeID, String aStartTime,String aStopTime) throws RemoteException {
        boolean aB=false;
        try {
            aB = adaptee.setSchedule(aTreeID,aStartTime,aStopTime);
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI setSchedule error",ex);
            throw anEx;            
        }
        return aB;
    }

    // Whenever an error occurs in one the OTDB functions the message can
    // be retrieved with this function.
    public String errorMsg() throws RemoteException {
        String aS=null;
        try {
            aS = adaptee.errorMsg();
        } catch (Exception ex) {
            logger.error(ex);
            RemoteException anEx=new RemoteException("JNI errorMsg error",ex);
            throw anEx;            
        }
        return aS;            
     }

   protected jTreeMaintenance adaptee;   
}
