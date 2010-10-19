/*
 * OlapPanel.java
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

package nl.astron.lofar.sas.otbcomponents.userpanels;

import java.awt.Component;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.rmi.RemoteException;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.JFileChooser;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import org.apache.log4j.Logger;

/**
 * Panel for OLAP specific configuration
 *
 * @author  Coolen
 *
 * Created on 19 april 2007, 20:54
 *
 * @version $Id$
 */
public class OlapPanel extends javax.swing.JPanel implements IViewPanel{

    static Logger logger = Logger.getLogger(OlapPanel.class);    
    static String name = "OlapPanel";
    
    /** Creates new form OlapPanel */
    public OlapPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode=aNode;
        initialize();
        initPanel();
    }
    
    /** Creates new form BeanForm */
    public OlapPanel() {
        initComponents();
        initialize();
    }   
    
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
        } else {
            logger.debug("No Mainframe supplied");
        }
    }
    
    public String getShortName() {
        return name;
    }
    
    public void setContent(Object anObject) { 
        itsNode=(jOTDBnode)anObject;
        jOTDBparam aParam=null;
        try {
           
            //we need to get all the childs from this node.    
            Vector childs = OtdbRmi.getRemoteMaintenance().getItemList(itsNode.treeID(), itsNode.nodeID(), 1);
            
            // get all the params per child
            Enumeration e = childs.elements();
            while( e.hasMoreElements()  ) {
                aParam=null;
            
                jOTDBnode aNode = (jOTDBnode)e.nextElement();
                        
                // We need to keep all the nodes needed by this panel
                // if the node is a leaf we need to get the pointed to value via Param.
                if (aNode.leaf) {
                    aParam = OtdbRmi.getRemoteMaintenance().getParam(aNode);
                    setField(itsNode,aParam,aNode);
                    
                                    
                //we need to get all the childs from the following nodes as well.
                }else if (LofarUtils.keyName(aNode.name).equals("CNProc")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("DelayComp")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("OLAP_Conn")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("IONProc")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("StorageProc")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }
            }
        } catch (RemoteException ex) {
            logger.debug("Error during getComponentParam: "+ ex);
            return;
        }
        
        initPanel();
    }
    
    public boolean isSingleton() {
        return false;
    }
    
    public JPanel getInstance() {
        return new OlapPanel();
    }
    public boolean hasPopupMenu() {
        return true;
    }    
    
    /** create popup menu for this panel
     *
     *  // build up the menu
     *  aPopupMenu= new JPopupMenu();
     *  aMenuItem=new JMenuItem("Choice 1");        
     *  aMenuItem.addActionListener(new java.awt.event.ActionListener() {
     *      public void actionPerformed(java.awt.event.ActionEvent evt) {
     *          popupMenuHandler(evt);
     *      }
     *  });
     *  aMenuItem.setActionCommand("Choice 1");
     *  aPopupMenu.add(aMenuItem);
     *  aPopupMenu.setOpaque(true);
     *
     *
     *  aPopupMenu.show(aComponent, x, y );        
     */
    public void createPopupMenu(Component aComponent,int x, int y) {
        JPopupMenu aPopupMenu=null;
        JMenuItem  aMenuItem=null;
        
        aPopupMenu= new JPopupMenu();
        // For VIC trees
        if (itsTreeType.equals("VHtree")) {
            //  Fill in menu as in the example above
            aMenuItem=new JMenuItem("Create ParSet File");        
            aMenuItem.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    popupMenuHandler(evt);
                }
            });
            aMenuItem.setActionCommand("Create ParSet File");
            aPopupMenu.add(aMenuItem);
            
        // For template trees
        } else if (itsTreeType.equals("VItemplate")) {
                
        }
        
        aPopupMenu.setOpaque(true);
        aPopupMenu.show(aComponent, x, y );       
    }
    
    /** handles the choice from the popupmenu 
     *
     * depending on the choices that are possible for this panel perform the action for it
     *
     *      if (evt.getActionCommand().equals("Choice 1")) {
     *          perform action
     *      }  
     */
    public void popupMenuHandler(java.awt.event.ActionEvent evt) {
         if (evt.getActionCommand().equals("Create ParSet File")) {
            logger.debug("Create ParSet File");
            int aTreeID=itsMainFrame.getSharedVars().getTreeID();
            if (fc == null) {
                fc = new JFileChooser();
                fc.setApproveButtonText("Apply");
            }
            // try to get a new filename to write the parsetfile to
            if (fc.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
                try {
                    File aFile = fc.getSelectedFile();
                    
                    // create filename that can be used at the remote site    
                    String aRemoteFileName="/tmp/"+aTreeID+"-"+itsNode.name+"_"+itsMainFrame.getUserAccount().getUserName()+".ParSet";
                    
                    // write the parset
                    OtdbRmi.getRemoteMaintenance().exportTree(aTreeID,itsNode.nodeID(),aRemoteFileName,2,false); 
                    
                    //obtain the remote file
                    byte[] dldata = OtdbRmi.getRemoteFileTrans().downloadFile(aRemoteFileName);

                    BufferedOutputStream output = new BufferedOutputStream(new FileOutputStream(aFile));
                    output.write(dldata,0,dldata.length);
                    output.flush();
                    output.close();
                    logger.debug("File written to: " + aFile.getPath());
                } catch (RemoteException ex) {
                    logger.debug("exportTree failed : " + ex);
                } catch (FileNotFoundException ex) {
                    logger.debug("Error during newPICTree creation: "+ ex);
                } catch (IOException ex) {
                    logger.debug("Error during newPICTree creation: "+ ex);
                }
            }
        }       
    } 
    
    /** 
     * Helper method that retrieves the child nodes for a given jOTDBnode, 
     * and triggers setField() accordingly.
     * @param aNode the node to retrieve and display child data of.
     */
    private void retrieveAndDisplayChildDataForNode(jOTDBnode aNode){
        jOTDBparam aParam=null;
        try {
            Vector HWchilds = OtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1);
            // get all the params per child
            Enumeration e1 = HWchilds.elements();
            while( e1.hasMoreElements()  ) {
                
                jOTDBnode aHWNode = (jOTDBnode)e1.nextElement();
                aParam=null;
                // We need to keep all the params needed by this panel
                if (aHWNode.leaf) {
                    aParam = OtdbRmi.getRemoteMaintenance().getParam(aHWNode);
                }
                setField(aNode,aParam,aHWNode);
            }
        } catch (RemoteException ex) {
            logger.debug("Error during retrieveAndDisplayChildDataForNode: "+ ex);
            return;
        }
    }
    
    /**
     * Sets the different fields in the GUI, using the names of the nodes provided
     * @param parent the parent node of the node to be displayed
     * @param aParam the parameter of the node to be displayed if applicable
     * @param aNode  the node to be displayed
     */
    private void setField(jOTDBnode parent,jOTDBparam aParam, jOTDBnode aNode) {
        if (aParam==null) {
            return;
        }
        boolean isRef = LofarUtils.isReference(aNode.limits);
        String aKeyName = LofarUtils.keyName(aNode.name);
        String parentName = LofarUtils.keyName(String.valueOf(parent.name));
        /* Set's the different fields in the GUI */

        // Generic OLAP
        if (aParam==null) {
            return;
        }
        logger.debug("setField for: "+ aNode.name);
        try {
            if (OtdbRmi.getRemoteTypes().getParamType(aParam.type).substring(0,1).equals("p")) {
               // Have to get new param because we need the unresolved limits field.
               aParam = OtdbRmi.getRemoteMaintenance().getParam(aNode.treeID(),aNode.paramDefID());                
            }
        } catch (RemoteException ex) {
            logger.debug("Error during getParam: "+ ex);
        }
        
        if(parentName.equals("CNProc")){
            // OLAP-CNProc params
            if (aKeyName.equals("integrationSteps")) {
                inputCNIntegrationSteps.setToolTipText(aParam.description);
                itsCNProcIntegrationSteps=aNode;
                if (isRef && aParam != null) {
                    inputCNIntegrationSteps.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputCNIntegrationSteps.setText(aNode.limits);
                }
            } else if (aKeyName.equals("nrPPFTaps")) {
                inputNrPPFTaps.setToolTipText(aParam.description);
                itsNrPPFTaps=aNode;
                if (isRef && aParam != null) {
                    inputNrPPFTaps.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputNrPPFTaps.setText(aNode.limits);
                }
            } else if (aKeyName.equals("coresPerPset")) {        
                inputCoresPerPset.setToolTipText(aParam.description);
                itsCoresPerPset=aNode;
                if (isRef && aParam != null) {
                    inputCoresPerPset.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputCoresPerPset.setText(aNode.limits);
                }
            } else if (aKeyName.equals("partition")) {
                inputPartition.setToolTipText(aParam.description);
                itsPartition=aNode;
                if (isRef && aParam != null) {
                    inputPartition.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputPartition.setText(aNode.limits);
                }
            }
            
        } else if(parentName.equals("DelayComp")){       
            // OLAP DelayComp params

            if (aKeyName.equals("nrCalcDelays")) {        
                inputNrCalcDelays.setToolTipText(aParam.description);
                itsNrCalcDelays=aNode;
                if (isRef && aParam != null) {
                    inputNrCalcDelays.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputNrCalcDelays.setText(aNode.limits);
                }
            } else if (aKeyName.equals("positionType")) {
                inputPositionType.setToolTipText(aParam.description);
                itsPositionType=aNode;
                if (isRef && aParam != null) {
                    inputPositionType.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputPositionType.setText(aNode.limits);
                }
            }
            
        } else if(parentName.equals("OLAP_Conn")){
            // OLAP OLAP_Conn params
            if (aKeyName.equals("IONProc_Storage_Ports")) {
                inputIONProcStoragePorts.setToolTipText(aParam.description);
                itsCNProcStoragePorts=aNode;
                if (isRef && aParam != null) {
                    inputIONProcStoragePorts.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputIONProcStoragePorts.setText(aNode.limits);
                }
            } else if (aKeyName.equals("IONProc_CNProc_Transport")) {        
                inputIONProcCNProcTransport.setToolTipText(aParam.description);
                LofarUtils.setPopupComboChoices(inputIONProcCNProcTransport,aParam.limits);
                if (!aNode.limits.equals("")) {
                    inputIONProcCNProcTransport.setSelectedItem(aNode.limits);
                }
                itsIONProcCNProcTransport=aNode;
            } else if (aKeyName.equals("rawDataOutputs")) {
                inputRawDataOutputs.setToolTipText(aParam.description);
                itsRawDataOutputs=aNode;
                if (isRef && aParam != null) {
                    inputRawDataOutputs.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputRawDataOutputs.setText(aNode.limits);
                }
           }
            
        } else if(parentName.equals("IONProc")){       
            // OLAP IONProc params
            if (aKeyName.equals("integrationSteps")) {
                inputIONProcIntegrationSteps.setToolTipText(aParam.description);
                itsIONProcIntegrationSteps=aNode;
                if (isRef && aParam != null) {
                    inputIONProcIntegrationSteps.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputIONProcIntegrationSteps.setText(aNode.limits);
                }
            }
            
        } else if(parentName.equals("StorageProc")){       
            // OLAP StorageProc params
            if (aKeyName.equals("subbandsPerMS")) {
                inputSubbandsPerMS.setToolTipText(aParam.description);
                itsSubbandsPerMS=aNode;
                if (isRef && aParam != null) {
                    inputSubbandsPerMS.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputSubbandsPerMS.setText(aNode.limits);
                }
            }
        } else if(parentName.equals("OLAP")){
            // Olap Specific parameters
            if (aKeyName.equals("EPAHeaderSize")) {
                inputEPAHeaderSize.setToolTipText(aParam.description);
                itsEPAHeaderSize=aNode;
                if (isRef && aParam != null) {
                    inputEPAHeaderSize.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputEPAHeaderSize.setText(aNode.limits);
                }
            } else if (aKeyName.equals("IPHeaderSize")) {
                inputIPHeaderSize.setToolTipText(aParam.description);
                itsIPHeaderSize=aNode;
                if (isRef && aParam != null) {
                    inputIPHeaderSize.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputIPHeaderSize.setText(aNode.limits);
                }
            } else if (aKeyName.equals("maxNetworkDelay")) {
                inputMaxNetworkDelay.setToolTipText(aParam.description);
                itsMaxNetworkDelay=aNode;
                if (isRef && aParam != null) {
                    inputMaxNetworkDelay.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputMaxNetworkDelay.setText(aNode.limits);
                }
            } else if (aKeyName.equals("nrSubbandsPerFrame")) {
                inputNrSubbandsPerFrame.setToolTipText(aParam.description);
                itsNrSubbandsPerFrame=aNode;
                if (isRef && aParam != null) {
                    inputNrSubbandsPerFrame.setText(aNode.limits);
                    subbandsPerFrameDerefText.setText(aParam.limits);
                } else {
                    inputNrSubbandsPerFrame.setText(aNode.limits);
                }
            } else if (aKeyName.equals("delayCompensation")) {
                inputDelayCompensation.setToolTipText(aParam.description);
                itsDelayCompensation=aNode;
                boolean aSelection = false;
                if (isRef && aParam != null) {
                    if (aParam.limits.equals("true")||aParam.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                } else {
                    if (aNode.limits.equals("true")||aNode.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                }
                inputDelayCompensation.setSelected(aSelection);
            } else if (aKeyName.equals("nrBitsPerSample")) {
                inputNrBitsPerSample.setToolTipText(aParam.description);
                itsNrBitsPerSample=aNode;
                if (isRef && aParam != null) {
                    inputNrBitsPerSample.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputNrBitsPerSample.setText(aNode.limits);
                }
            } else if (aKeyName.equals("nrSecondsOfBuffer")) {
                inputNrSecondsOfBuffer.setToolTipText(aParam.description);
                itsNrSecondsOfBuffer=aNode;
                if (isRef && aParam != null) {
                    inputNrSecondsOfBuffer.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputNrSecondsOfBuffer.setText(aNode.limits);
                }
            } else if (aKeyName.equals("nrTimesInFrame")) {
                inputNrTimesInFrame.setToolTipText(aParam.description);
                itsNrTimesInFrame=aNode;
                if (isRef && aParam != null) {
                    inputNrTimesInFrame.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputNrTimesInFrame.setText(aNode.limits);
                }

            } else if (aKeyName.equals("storageStationNames")) {
                inputStorageStationNames.setToolTipText(aParam.description);
                itsStorageStationNames=aNode;
                if (isRef && aParam != null) {
                    inputStorageStationNames.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputStorageStationNames.setText(aNode.limits);
                }
            } else if (aKeyName.equals("realTime")) {
                inputRealTime.setToolTipText(aParam.description);
                itsRealTime=aNode;
                boolean aSelection = false;
                if (isRef && aParam != null) {
                    if (aParam.limits.equals("true")||aParam.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                } else {
                    if (aNode.limits.equals("true")||aNode.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                }
                inputRealTime.setSelected(aSelection);
            } else if (aKeyName.equals("correctBandPass")) {
                inputCorrectBandPass.setToolTipText(aParam.description);
                itsCorrectBandPass=aNode;
                boolean aSelection = false;
                if (isRef && aParam != null) {
                    if (aParam.limits.equals("true")||aParam.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                } else {
                    if (aNode.limits.equals("true")||aNode.limits.equals("TRUE")) {
                        aSelection = true;
                    }
                }
                inputCorrectBandPass.setSelected(aSelection);
            }
        }   
    }
    
    private void restore() {
      boolean aB=false;

      // Olap Specific parameters
      inputEPAHeaderSize.setText(itsEPAHeaderSize.limits);
      inputIPHeaderSize.setText(itsIPHeaderSize.limits);
      aB=false;
      if (itsDelayCompensation.limits.equals("true")||itsDelayCompensation.limits.equals("TRUE")) {
          aB=true;
      }
      inputDelayCompensation.setSelected(aB);
      inputNrBitsPerSample.setText(itsNrBitsPerSample.limits);
      inputNrSecondsOfBuffer.setText(itsNrSecondsOfBuffer.limits);
      inputNrTimesInFrame.setText(itsNrTimesInFrame.limits);
      inputStorageStationNames.setText(itsStorageStationNames.limits);
      aB=false;
      if (itsRealTime.limits.equals("true")||itsRealTime.limits.equals("TRUE")) {
          aB=true;
      }
      inputRealTime.setSelected(aB);
      aB=false;
      if (itsCorrectBandPass.limits.equals("true")||itsCorrectBandPass.limits.equals("TRUE")) {
          aB=true;
      }
      inputCorrectBandPass.setSelected(aB);
      inputMaxNetworkDelay.setText(itsMaxNetworkDelay.limits);
      inputNrSubbandsPerFrame.setText(itsNrSubbandsPerFrame.limits);

      //OLAP-CNProc      
      inputCNIntegrationSteps.setText(itsCNProcIntegrationSteps.limits);
      inputNrPPFTaps.setText(itsNrPPFTaps.limits);
      inputCoresPerPset.setText(itsCoresPerPset.limits);
      inputPartition.setText(itsPartition.limits);
      
      //OLAP-DelayComp
      inputPositionType.setText(itsPositionType.limits);
      inputNrCalcDelays.setText(itsNrCalcDelays.limits);
      
      //OLAP IONProc
      inputIONProcIntegrationSteps.setText(itsIONProcIntegrationSteps.limits);
     
      //OLAP StorageProc
      inputSubbandsPerMS.setText(itsSubbandsPerMS.limits);
      
      //OLAP-OLAP_Conn
      inputIONProcStoragePorts.setText(itsCNProcStoragePorts.limits);
      inputIONProcCNProcTransport.setSelectedItem(itsIONProcCNProcTransport.limits);
      inputRawDataOutputs.setText(itsRawDataOutputs.limits);
    }
     
    private void initialize() {
        buttonPanel1.addButton("Restore");
        buttonPanel1.addButton("Apply");
    }
    
    private void initPanel() {
        // check access
        UserAccount userAccount = itsMainFrame.getUserAccount();

        // for now:
        setAllEnabled(true);
        
        if(userAccount.isAdministrator()) {
            // enable/disable certain controls
        }
        if(userAccount.isAstronomer()) {
            // enable/disable certain controls
        }
        if(userAccount.isInstrumentScientist()) {
            // enable/disable certain controls
        }
        

         if (itsNode != null) {
            try {
                //figure out the caller
                jOTDBtree aTree = OtdbRmi.getRemoteOTDB().getTreeInfo(itsNode.treeID(),false);
                itsTreeType=OtdbRmi.getTreeType().get(aTree.type);
            } catch (RemoteException ex) {
                logger.debug("OlapPanel: Error getting treeInfo/treetype" + ex);
                itsTreeType="";
            }         } else {
            logger.debug("ERROR:  no node given");
        }
    }   
    
 
    /** saves the given node back to the database
     */
    private void saveNode(jOTDBnode aNode) {
        if (aNode == null) {
            return;
        }
        try {
            OtdbRmi.getRemoteMaintenance().saveNode(aNode); 
        } catch (RemoteException ex) {
            logger.debug("Error: saveNode failed : " + ex);
        } 
    }   
    
   
    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
    }        
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
    }
    
    private void saveInput() {
        boolean hasChanged = false;

        // Generic OLAP       
        if (itsEPAHeaderSize != null && !inputEPAHeaderSize.getText().equals(itsEPAHeaderSize.limits)) {
            itsEPAHeaderSize.limits = inputEPAHeaderSize.getText();
            saveNode(itsEPAHeaderSize);
        }
        if (itsIPHeaderSize != null && !inputIPHeaderSize.getText().equals(itsIPHeaderSize.limits)) {
            itsIPHeaderSize.limits = inputIPHeaderSize.getText();
            saveNode(itsIPHeaderSize);
        }    
        if ((!inputDelayCompensation.isSelected() &  
                (itsDelayCompensation.limits.equals("TRUE") ||itsDelayCompensation.limits.equals("true") )) ||
            (inputDelayCompensation.isSelected() &  
                (itsDelayCompensation.limits.equals("FALSE") ||itsDelayCompensation.limits.equals("false") )))        
        {  
            String delay="true";
            if (!inputDelayCompensation.isSelected()) {
                delay="false";
            }
            itsDelayCompensation.limits = delay;
            saveNode(itsDelayCompensation);
        }
        if (itsIPHeaderSize != null && !inputIPHeaderSize.getText().equals(itsIPHeaderSize.limits)) {
            itsIPHeaderSize.limits = inputIPHeaderSize.getText();
            saveNode(itsIPHeaderSize);
        }
        if (itsMaxNetworkDelay != null && !inputMaxNetworkDelay.getText().equals(itsMaxNetworkDelay.limits)) {
            itsMaxNetworkDelay.limits = inputMaxNetworkDelay.getText();
            saveNode(itsMaxNetworkDelay);
        }
        if (itsNrSubbandsPerFrame != null && !inputNrSubbandsPerFrame.getText().equals(itsNrSubbandsPerFrame.limits)) {
            itsNrSubbandsPerFrame.limits = inputNrSubbandsPerFrame.getText();
            saveNode(itsNrSubbandsPerFrame);
        }      
        if (itsNrBitsPerSample != null && !inputNrBitsPerSample.getText().equals(itsNrBitsPerSample.limits)) {
            itsNrBitsPerSample.limits = inputNrBitsPerSample.getText();
            saveNode(itsNrBitsPerSample);
        }
        if (itsNrSecondsOfBuffer != null && !inputNrSecondsOfBuffer.getText().equals(itsNrSecondsOfBuffer.limits)) {
            itsNrSecondsOfBuffer.limits = inputNrSecondsOfBuffer.getText();
            saveNode(itsNrSecondsOfBuffer);
        }
        if (itsNrTimesInFrame != null && !inputNrTimesInFrame.getText().equals(itsNrTimesInFrame.limits)) {
            itsNrTimesInFrame.limits = inputNrTimesInFrame.getText();
            saveNode(itsNrTimesInFrame);
        }
        if (itsStorageStationNames != null && !inputStorageStationNames.getText().equals(itsStorageStationNames.limits)) {
            itsStorageStationNames.limits = inputStorageStationNames.getText();
            saveNode(itsStorageStationNames);
        }
        if ((!inputRealTime.isSelected() &  
                (itsRealTime.limits.equals("TRUE") ||itsRealTime.limits.equals("true") )) ||
            (inputRealTime.isSelected() &  
                (itsRealTime.limits.equals("FALSE") ||itsRealTime.limits.equals("false") )))        
        {  
            String rt="true";
            if (!inputRealTime.isSelected()) {
                rt="false";
            }
            itsRealTime.limits = rt;
            saveNode(itsRealTime);
        }
        if ((!inputCorrectBandPass.isSelected() &  
                (itsCorrectBandPass.limits.equals("TRUE") ||itsCorrectBandPass.limits.equals("true") )) ||
            (inputCorrectBandPass.isSelected() &  
                (itsCorrectBandPass.limits.equals("FALSE") ||itsCorrectBandPass.limits.equals("false") )))        
        {  
            String bp="true";
            if (!inputCorrectBandPass.isSelected()) {
                bp="false";
            }
            itsCorrectBandPass.limits = bp;
            saveNode(itsCorrectBandPass);
        }

        
        // OLAP-CNProc
        if (itsCNProcIntegrationSteps != null && !inputCNIntegrationSteps.getText().equals(itsCNProcIntegrationSteps.limits)) {
            itsCNProcIntegrationSteps.limits = inputCNIntegrationSteps.getText();
            saveNode(itsCNProcIntegrationSteps);
        }
        if (itsNrPPFTaps != null && !inputNrPPFTaps.getText().equals(itsNrPPFTaps.limits)) {
            itsNrPPFTaps.limits = inputNrPPFTaps.getText();
            saveNode(itsNrPPFTaps);
        }
        if (itsCoresPerPset != null && !inputCoresPerPset.getText().equals(itsCoresPerPset.limits)) {
            itsCoresPerPset.limits = inputCoresPerPset.getText();
            saveNode(itsCoresPerPset);
        }
        if (itsPartition != null && !inputPartition.getText().equals(itsPartition.limits)) {
            itsPartition.limits = inputPartition.getText();
            saveNode(itsPartition);
        }

        //Olap-DelayComp
        if (itsNrCalcDelays!= null && !inputNrCalcDelays.getText().equals(itsNrCalcDelays.limits)) {
            itsNrCalcDelays.limits = inputNrCalcDelays.getText();
            saveNode(itsNrCalcDelays);
        }
        if (itsPositionType != null && !inputPositionType.getText().equals(itsPositionType.limits)) {
            itsPositionType.limits = inputPositionType.getText();
            saveNode(itsPositionType);
        }
        
        // OLAP-OLAP_Conn
        if (itsCNProcStoragePorts != null && !inputIONProcStoragePorts.getText().equals(itsCNProcStoragePorts.limits)) {
            itsCNProcStoragePorts.limits = inputIONProcStoragePorts.getText();
            saveNode(itsCNProcStoragePorts);
        }
        if (itsIONProcCNProcTransport!= null && !inputIONProcCNProcTransport.getSelectedItem().toString().equals(itsIONProcCNProcTransport.limits)) {  
            itsIONProcCNProcTransport.limits = inputIONProcCNProcTransport.getSelectedItem().toString();
            saveNode(itsIONProcCNProcTransport);
        }
        if (itsRawDataOutputs != null && !inputRawDataOutputs.getText().equals(itsRawDataOutputs.limits)) {
            itsRawDataOutputs.limits = inputRawDataOutputs.getText();
            saveNode(itsRawDataOutputs);
        }


        // OLAP-IONProc
        if (itsIONProcIntegrationSteps != null && !inputIONProcIntegrationSteps.getText().equals(itsIONProcIntegrationSteps.limits)) {
            itsIONProcIntegrationSteps.limits = inputIONProcIntegrationSteps.getText();
            saveNode(itsIONProcIntegrationSteps);
        }

        //OLAP-StorageProc
        if (itsSubbandsPerMS != null && !inputSubbandsPerMS.getText().equals(itsSubbandsPerMS.limits)) {
            itsSubbandsPerMS.limits = inputSubbandsPerMS.getText();
            saveNode(itsSubbandsPerMS);
        }
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jPanel9 = new javax.swing.JPanel();
        jPanel1 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        jScrollPane1 = new javax.swing.JScrollPane();
        jPanel2 = new javax.swing.JPanel();
        jPanel4 = new javax.swing.JPanel();
        jPanel8 = new javax.swing.JPanel();
        labelEPAHeaderSize = new javax.swing.JLabel();
        inputEPAHeaderSize = new javax.swing.JTextField();
        labelIPHeaderSize = new javax.swing.JLabel();
        inputIPHeaderSize = new javax.swing.JTextField();
        labelNrBitsPerSample = new javax.swing.JLabel();
        inputNrBitsPerSample = new javax.swing.JTextField();
        labelNrTimesInFrame = new javax.swing.JLabel();
        inputNrTimesInFrame = new javax.swing.JTextField();
        labelDelayCompensation = new javax.swing.JLabel();
        inputDelayCompensation = new javax.swing.JCheckBox();
        javax.swing.JLabel labelNrSecondsOfBuffer = new javax.swing.JLabel();
        inputNrSecondsOfBuffer = new javax.swing.JTextField();
        labelStorageStationNames = new javax.swing.JLabel();
        inputStorageStationNames = new javax.swing.JTextField();
        labelMaxNetworkDelay = new javax.swing.JLabel();
        inputMaxNetworkDelay = new javax.swing.JTextField();
        labelNrSubbandsPerFrame = new javax.swing.JLabel();
        inputNrSubbandsPerFrame = new javax.swing.JTextField();
        labelRealTime = new javax.swing.JLabel();
        inputRealTime = new javax.swing.JCheckBox();
        labelCorrectBandPass = new javax.swing.JLabel();
        inputCorrectBandPass = new javax.swing.JCheckBox();
        subbandsPerFrameDerefText = new javax.swing.JTextField();
        jPanel10 = new javax.swing.JPanel();
        labelIONProcIntegrationSteps = new javax.swing.JLabel();
        inputIONProcIntegrationSteps = new javax.swing.JTextField();
        jPanel6 = new javax.swing.JPanel();
        labelIONProcCNProcTransport = new javax.swing.JLabel();
        inputIONProcCNProcTransport = new javax.swing.JComboBox();
        labelIONProcStoragePorts = new javax.swing.JLabel();
        inputIONProcStoragePorts = new javax.swing.JTextField();
        labelRawDataOutputs = new javax.swing.JLabel();
        inputRawDataOutputs = new javax.swing.JTextField();
        jPanel7 = new javax.swing.JPanel();
        labelNrPPFTaps = new javax.swing.JLabel();
        inputNrPPFTaps = new javax.swing.JTextField();
        labelPartition = new javax.swing.JLabel();
        labelCNIntegrationSteps = new javax.swing.JLabel();
        inputCNIntegrationSteps = new javax.swing.JTextField();
        labelCoresPerPset = new javax.swing.JLabel();
        inputCoresPerPset = new javax.swing.JTextField();
        inputPartition = new javax.swing.JTextField();
        jPanel11 = new javax.swing.JPanel();
        labelSubbandsPerMs = new javax.swing.JLabel();
        inputSubbandsPerMS = new javax.swing.JTextField();
        jPanel5 = new javax.swing.JPanel();
        labelNrCalcDelays = new javax.swing.JLabel();
        labelPositionType = new javax.swing.JLabel();
        inputNrCalcDelays = new javax.swing.JTextField();
        inputPositionType = new javax.swing.JTextField();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        javax.swing.GroupLayout jPanel9Layout = new javax.swing.GroupLayout(jPanel9);
        jPanel9.setLayout(jPanel9Layout);
        jPanel9Layout.setHorizontalGroup(
            jPanel9Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 100, Short.MAX_VALUE)
        );
        jPanel9Layout.setVerticalGroup(
            jPanel9Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 100, Short.MAX_VALUE)
        );

        setMinimumSize(new java.awt.Dimension(800, 600));
        setLayout(new java.awt.BorderLayout());

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder(""));
        jPanel1.setPreferredSize(new java.awt.Dimension(100, 25));
        jPanel1.setLayout(new java.awt.BorderLayout());

        jLabel1.setFont(new java.awt.Font("Tahoma", 1, 11));
        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("OLAP Details");
        jPanel1.add(jLabel1, java.awt.BorderLayout.CENTER);

        add(jPanel1, java.awt.BorderLayout.NORTH);

        jPanel2.setBorder(javax.swing.BorderFactory.createTitledBorder(""));

        jPanel8.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Olap", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N

        labelEPAHeaderSize.setText("EPA Header Size:");

        labelIPHeaderSize.setText("IP Header Size:");

        labelNrBitsPerSample.setText("# Bits per Sample:");

        labelNrTimesInFrame.setText("# Times In Frame:");

        labelDelayCompensation.setText("Delay Compensation:");

        inputDelayCompensation.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputDelayCompensation.setMargin(new java.awt.Insets(0, 0, 0, 0));

        labelNrSecondsOfBuffer.setText("# SecondsOfBuffer:");

        labelStorageStationNames.setText("storageStationNames:");

        labelMaxNetworkDelay.setText("Max Network Delay:");

        labelNrSubbandsPerFrame.setText("# SubbandsPerFrame:");

        inputNrSubbandsPerFrame.setToolTipText("");

        labelRealTime.setText("RealTime");

        inputRealTime.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputRealTime.setMargin(new java.awt.Insets(0, 0, 0, 0));

        labelCorrectBandPass.setText("Correct BandPass");

        inputCorrectBandPass.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        inputCorrectBandPass.setMargin(new java.awt.Insets(0, 0, 0, 0));

        subbandsPerFrameDerefText.setEditable(false);
        subbandsPerFrameDerefText.setEnabled(false);

        javax.swing.GroupLayout jPanel8Layout = new javax.swing.GroupLayout(jPanel8);
        jPanel8.setLayout(jPanel8Layout);
        jPanel8Layout.setHorizontalGroup(
            jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                        .addComponent(labelStorageStationNames, javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(labelDelayCompensation, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(labelNrBitsPerSample, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(labelNrTimesInFrame, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(labelIPHeaderSize, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 124, Short.MAX_VALUE))
                    .addComponent(labelRealTime, javax.swing.GroupLayout.DEFAULT_SIZE, 124, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                                .addComponent(inputNrTimesInFrame, javax.swing.GroupLayout.Alignment.LEADING)
                                .addComponent(inputNrBitsPerSample, javax.swing.GroupLayout.Alignment.LEADING)
                                .addGroup(jPanel8Layout.createSequentialGroup()
                                    .addComponent(inputRealTime)
                                    .addGap(18, 18, 18)
                                    .addComponent(labelCorrectBandPass, javax.swing.GroupLayout.PREFERRED_SIZE, 105, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                                    .addComponent(inputCorrectBandPass))
                                .addComponent(inputIPHeaderSize, javax.swing.GroupLayout.Alignment.LEADING))
                            .addComponent(inputDelayCompensation))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(labelNrSubbandsPerFrame, javax.swing.GroupLayout.DEFAULT_SIZE, 148, Short.MAX_VALUE)
                            .addComponent(labelMaxNetworkDelay, javax.swing.GroupLayout.DEFAULT_SIZE, 148, Short.MAX_VALUE)
                            .addComponent(labelEPAHeaderSize, javax.swing.GroupLayout.DEFAULT_SIZE, 144, Short.MAX_VALUE)
                            .addComponent(labelNrSecondsOfBuffer, javax.swing.GroupLayout.DEFAULT_SIZE, 148, Short.MAX_VALUE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(inputMaxNetworkDelay)
                            .addComponent(inputNrSecondsOfBuffer)
                            .addComponent(inputEPAHeaderSize, javax.swing.GroupLayout.DEFAULT_SIZE, 170, Short.MAX_VALUE)
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addComponent(inputNrSubbandsPerFrame, javax.swing.GroupLayout.PREFERRED_SIZE, 130, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(subbandsPerFrameDerefText, javax.swing.GroupLayout.PREFERRED_SIZE, 37, javax.swing.GroupLayout.PREFERRED_SIZE))))
                    .addComponent(inputStorageStationNames, javax.swing.GroupLayout.DEFAULT_SIZE, 484, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel8Layout.setVerticalGroup(
            jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelIPHeaderSize)
                    .addComponent(inputIPHeaderSize, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelEPAHeaderSize)
                    .addComponent(inputEPAHeaderSize, javax.swing.GroupLayout.PREFERRED_SIZE, 20, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(6, 6, 6)
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelNrTimesInFrame)
                    .addComponent(inputNrTimesInFrame, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelNrSecondsOfBuffer)
                    .addComponent(inputNrSecondsOfBuffer, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelNrBitsPerSample)
                    .addComponent(inputNrBitsPerSample, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelMaxNetworkDelay)
                    .addComponent(inputMaxNetworkDelay, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addGroup(jPanel8Layout.createSequentialGroup()
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                            .addComponent(inputDelayCompensation, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                                .addComponent(labelNrSubbandsPerFrame)
                                .addComponent(inputNrSubbandsPerFrame, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addComponent(subbandsPerFrameDerefText, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addComponent(labelDelayCompensation))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(labelRealTime)
                            .addGroup(jPanel8Layout.createSequentialGroup()
                                .addGap(1, 1, 1)
                                .addComponent(inputRealTime, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE))))
                    .addComponent(labelCorrectBandPass)
                    .addComponent(inputCorrectBandPass, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(15, 15, 15)
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelStorageStationNames)
                    .addComponent(inputStorageStationNames, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
        );

        jPanel10.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "ION Proc", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N

        labelIONProcIntegrationSteps.setText("Integration Steps:");

        javax.swing.GroupLayout jPanel10Layout = new javax.swing.GroupLayout(jPanel10);
        jPanel10.setLayout(jPanel10Layout);
        jPanel10Layout.setHorizontalGroup(
            jPanel10Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel10Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(labelIONProcIntegrationSteps)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(inputIONProcIntegrationSteps, javax.swing.GroupLayout.PREFERRED_SIZE, 73, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(60, Short.MAX_VALUE))
        );
        jPanel10Layout.setVerticalGroup(
            jPanel10Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel10Layout.createSequentialGroup()
                .addGroup(jPanel10Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelIONProcIntegrationSteps)
                    .addComponent(inputIONProcIntegrationSteps, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(133, Short.MAX_VALUE))
        );

        jPanel6.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Olap Conn", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N
        jPanel6.setToolTipText("Olap Conn");

        labelIONProcCNProcTransport.setText("IONProc->CNProc Transport:");

        inputIONProcCNProcTransport.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));

        labelIONProcStoragePorts.setText("IONProc->Storage Ports:");

        labelRawDataOutputs.setText("rawDataOutputs");

        javax.swing.GroupLayout jPanel6Layout = new javax.swing.GroupLayout(jPanel6);
        jPanel6.setLayout(jPanel6Layout);
        jPanel6Layout.setHorizontalGroup(
            jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel6Layout.createSequentialGroup()
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(labelRawDataOutputs, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelIONProcStoragePorts, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(labelIONProcCNProcTransport, javax.swing.GroupLayout.DEFAULT_SIZE, 155, Short.MAX_VALUE))
                .addGap(26, 26, 26)
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(inputRawDataOutputs)
                    .addComponent(inputIONProcStoragePorts)
                    .addComponent(inputIONProcCNProcTransport, 0, 197, Short.MAX_VALUE))
                .addContainerGap(240, Short.MAX_VALUE))
        );
        jPanel6Layout.setVerticalGroup(
            jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel6Layout.createSequentialGroup()
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelIONProcCNProcTransport)
                    .addComponent(inputIONProcCNProcTransport, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelIONProcStoragePorts)
                    .addComponent(inputIONProcStoragePorts, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel6Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelRawDataOutputs)
                    .addComponent(inputRawDataOutputs, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(36, 36, 36))
        );

        jPanel7.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "CN Proc", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N
        jPanel7.setToolTipText("BGLProc");

        labelNrPPFTaps.setText("# PPFTaps:");

        labelPartition.setText("Partition");

        labelCNIntegrationSteps.setText("Integration Steps:");

        labelCoresPerPset.setText("# Cores per Pset:");

        javax.swing.GroupLayout jPanel7Layout = new javax.swing.GroupLayout(jPanel7);
        jPanel7.setLayout(jPanel7Layout);
        jPanel7Layout.setHorizontalGroup(
            jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel7Layout.createSequentialGroup()
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(labelNrPPFTaps, javax.swing.GroupLayout.DEFAULT_SIZE, 88, Short.MAX_VALUE)
                    .addComponent(labelPartition, javax.swing.GroupLayout.DEFAULT_SIZE, 88, Short.MAX_VALUE)
                    .addComponent(labelCNIntegrationSteps)
                    .addComponent(labelCoresPerPset))
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel7Layout.createSequentialGroup()
                        .addGap(3, 3, 3)
                        .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(inputCNIntegrationSteps, javax.swing.GroupLayout.DEFAULT_SIZE, 134, Short.MAX_VALUE)
                            .addComponent(inputPartition, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 134, Short.MAX_VALUE)
                            .addComponent(inputNrPPFTaps, javax.swing.GroupLayout.DEFAULT_SIZE, 134, Short.MAX_VALUE)))
                    .addGroup(jPanel7Layout.createSequentialGroup()
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(inputCoresPerPset, javax.swing.GroupLayout.DEFAULT_SIZE, 133, Short.MAX_VALUE)))
                .addContainerGap())
        );
        jPanel7Layout.setVerticalGroup(
            jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel7Layout.createSequentialGroup()
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(inputNrPPFTaps, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(labelNrPPFTaps))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelPartition)
                    .addComponent(inputPartition, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelCNIntegrationSteps)
                    .addComponent(inputCNIntegrationSteps, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelCoresPerPset)
                    .addComponent(inputCoresPerPset, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
        );

        jPanel11.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Storage Proc", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N

        labelSubbandsPerMs.setText("#subbands per MS:");

        javax.swing.GroupLayout jPanel11Layout = new javax.swing.GroupLayout(jPanel11);
        jPanel11.setLayout(jPanel11Layout);
        jPanel11Layout.setHorizontalGroup(
            jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel11Layout.createSequentialGroup()
                .addComponent(labelSubbandsPerMs)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(inputSubbandsPerMS, javax.swing.GroupLayout.PREFERRED_SIZE, 125, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(6, Short.MAX_VALUE))
        );
        jPanel11Layout.setVerticalGroup(
            jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel11Layout.createSequentialGroup()
                .addGroup(jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelSubbandsPerMs)
                    .addComponent(inputSubbandsPerMS, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(37, 37, 37))
        );

        jPanel5.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Delay Compensation", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N
        jPanel5.setToolTipText("Delay Compensation");

        labelNrCalcDelays.setText("# Calc. Delays");

        labelPositionType.setText("PositionType:");

        javax.swing.GroupLayout jPanel5Layout = new javax.swing.GroupLayout(jPanel5);
        jPanel5.setLayout(jPanel5Layout);
        jPanel5Layout.setHorizontalGroup(
            jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel5Layout.createSequentialGroup()
                .addGroup(jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(labelNrCalcDelays)
                    .addComponent(labelPositionType))
                .addGap(112, 112, 112)
                .addGroup(jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(inputPositionType)
                    .addComponent(inputNrCalcDelays, javax.swing.GroupLayout.DEFAULT_SIZE, 194, Short.MAX_VALUE))
                .addContainerGap(242, Short.MAX_VALUE))
        );
        jPanel5Layout.setVerticalGroup(
            jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel5Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelNrCalcDelays)
                    .addComponent(inputNrCalcDelays, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelPositionType)
                    .addComponent(inputPositionType, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
        );

        javax.swing.GroupLayout jPanel4Layout = new javax.swing.GroupLayout(jPanel4);
        jPanel4.setLayout(jPanel4Layout);
        jPanel4Layout.setHorizontalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel4Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jPanel6, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel8, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel5, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addGap(18, 18, 18)
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(jPanel11, 0, 251, Short.MAX_VALUE)
                    .addComponent(jPanel7, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel10, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel4Layout.setVerticalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel4Layout.createSequentialGroup()
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jPanel10, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jPanel8, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(jPanel7, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel6, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jPanel11, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jPanel5, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addComponent(jPanel4, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(4319, Short.MAX_VALUE))
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addComponent(jPanel4, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(2931, Short.MAX_VALUE))
        );

        jScrollPane1.setViewportView(jPanel2);

        add(jScrollPane1, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });
        add(buttonPanel1, java.awt.BorderLayout.SOUTH);
    }// </editor-fold>//GEN-END:initComponents

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        if(evt.getActionCommand().equals("Apply")) {
            saveInput();
        } else if(evt.getActionCommand().equals("Restore")) {
            restore();
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    private jOTDBnode    itsNode      = null;
    private MainFrame    itsMainFrame = null;
    private String       itsTreeType  = "";
    private JFileChooser fc           = null;  
    
    //Olap specific parameters
    private jOTDBnode itsEPAHeaderSize=null;
    private jOTDBnode itsIPHeaderSize=null;
    private jOTDBnode itsDelayCompensation=null;
    private jOTDBnode itsNrBitsPerSample=null;
    private jOTDBnode itsNrSecondsOfBuffer=null;
    private jOTDBnode itsNrTimesInFrame=null;
    private jOTDBnode itsStorageStationNames=null;
    private jOTDBnode itsMaxNetworkDelay=null;
    private jOTDBnode itsNrSubbandsPerFrame=null;
    private jOTDBnode itsRealTime=null;
    private jOTDBnode itsCorrectBandPass=null;

    
    // OLAP-CNProc parameters
    private jOTDBnode itsCNProcIntegrationSteps=null;
    private jOTDBnode itsNrPPFTaps=null;
    private jOTDBnode itsPartition=null;
    private jOTDBnode itsCoresPerPset=null;

    
    // OLAP-DelayComp parameters
    private jOTDBnode itsNrCalcDelays=null;
    private jOTDBnode itsPositionType=null;
    
    // OLAP-Conn parameters
    private jOTDBnode itsCNProcStoragePorts=null;
    private jOTDBnode itsIONProcCNProcTransport=null;
    private jOTDBnode itsRawDataOutputs=null;
   
    // OLAP-StorageProc parameters
    private jOTDBnode itsSubbandsPerMS=null;
    
    // OLAP-IONProc parameters
    private jOTDBnode itsIONProcIntegrationSteps=null;
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JTextField inputCNIntegrationSteps;
    private javax.swing.JTextField inputCoresPerPset;
    private javax.swing.JCheckBox inputCorrectBandPass;
    private javax.swing.JCheckBox inputDelayCompensation;
    private javax.swing.JTextField inputEPAHeaderSize;
    private javax.swing.JComboBox inputIONProcCNProcTransport;
    private javax.swing.JTextField inputIONProcIntegrationSteps;
    private javax.swing.JTextField inputIONProcStoragePorts;
    private javax.swing.JTextField inputIPHeaderSize;
    private javax.swing.JTextField inputMaxNetworkDelay;
    private javax.swing.JTextField inputNrBitsPerSample;
    private javax.swing.JTextField inputNrCalcDelays;
    private javax.swing.JTextField inputNrPPFTaps;
    private javax.swing.JTextField inputNrSecondsOfBuffer;
    private javax.swing.JTextField inputNrSubbandsPerFrame;
    private javax.swing.JTextField inputNrTimesInFrame;
    private javax.swing.JTextField inputPartition;
    private javax.swing.JTextField inputPositionType;
    private javax.swing.JTextField inputRawDataOutputs;
    private javax.swing.JCheckBox inputRealTime;
    private javax.swing.JTextField inputStorageStationNames;
    private javax.swing.JTextField inputSubbandsPerMS;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel10;
    private javax.swing.JPanel jPanel11;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanel5;
    private javax.swing.JPanel jPanel6;
    private javax.swing.JPanel jPanel7;
    private javax.swing.JPanel jPanel8;
    private javax.swing.JPanel jPanel9;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JLabel labelCNIntegrationSteps;
    private javax.swing.JLabel labelCoresPerPset;
    private javax.swing.JLabel labelCorrectBandPass;
    private javax.swing.JLabel labelDelayCompensation;
    private javax.swing.JLabel labelEPAHeaderSize;
    private javax.swing.JLabel labelIONProcCNProcTransport;
    private javax.swing.JLabel labelIONProcIntegrationSteps;
    private javax.swing.JLabel labelIONProcStoragePorts;
    private javax.swing.JLabel labelIPHeaderSize;
    private javax.swing.JLabel labelMaxNetworkDelay;
    private javax.swing.JLabel labelNrBitsPerSample;
    private javax.swing.JLabel labelNrCalcDelays;
    private javax.swing.JLabel labelNrPPFTaps;
    private javax.swing.JLabel labelNrSubbandsPerFrame;
    private javax.swing.JLabel labelNrTimesInFrame;
    private javax.swing.JLabel labelPartition;
    private javax.swing.JLabel labelPositionType;
    private javax.swing.JLabel labelRawDataOutputs;
    private javax.swing.JLabel labelRealTime;
    private javax.swing.JLabel labelStorageStationNames;
    private javax.swing.JLabel labelSubbandsPerMs;
    private javax.swing.JTextField subbandsPerFrameDerefText;
    // End of variables declaration//GEN-END:variables
 
    /**
     * Utility field used by event firing mechanism.
     */
    private javax.swing.event.EventListenerList myListenerList =  null;

    /**
     * Registers ActionListener to receive events.
     * @param listener The listener to register.
     */
    public synchronized void addActionListener(java.awt.event.ActionListener listener) {

        if (myListenerList == null ) {
            myListenerList = new javax.swing.event.EventListenerList();
        }
        myListenerList.add (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeActionListener(java.awt.event.ActionListener listener) {

        myListenerList.remove (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Notifies all registered listeners about the event.
     * 
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {

        if (myListenerList == null) return;
        Object[] listeners = myListenerList.getListenerList ();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed (event);
            }
        }
    } 
}
