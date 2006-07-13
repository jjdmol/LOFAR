/*
 * OLAPConficPanel.java
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

package nl.astron.lofar.sas.otbcomponents;

import java.awt.Component;
import java.rmi.RemoteException;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.UserAccount;
import org.apache.log4j.Logger;

/**
 * Panel for OLAP specific configuration
 *
 * @created 17-05-2006, 13:47
 *
 * @author  coolen
 *
 * @version $Id$
 */
public class OLAPConfigPanel extends javax.swing.JPanel implements IViewPanel{
    
    static Logger logger = Logger.getLogger(OLAPConfigPanel.class);    
    static String name = "OLAPConfig";

   
    /** Creates new form BeanForm based upon aNode
     *
     * @params  aNode   node to obtain the info from
     *
     */
    public OLAPConfigPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode=aNode;
        initialize();
        initPanel();
    }
    
    /** Creates new form BeanForm */
    public OLAPConfigPanel() {
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
        jOTDBparam aRefParam=null;
        try {
            
            //we need to get all the childs from this node.    
            Vector childs = itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().getItemList(itsNode.treeID(), itsNode.nodeID(), 1);
            
            // get all the params per child
            Enumeration e = childs.elements();
            while( e.hasMoreElements()  ) {
                aParam=null;
                aRefParam=null;
            
                jOTDBnode aNode = (jOTDBnode)e.nextElement();
                        
                // We need to keep all the params needed by this panel
                if (aNode.leaf) {
                    aParam = itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().getParam(aNode);
                    setField(aParam,aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("OLAP_HW")) {
                    //we need to get all the childs from this node also.    
                    Vector HWchilds = itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1);
            
                    // get all the params per child
                    Enumeration e1 = HWchilds.elements();
                    while( e1.hasMoreElements()  ) {
                       
                        jOTDBnode aHWNode = (jOTDBnode)e1.nextElement();
                        
                        // We need to keep all the params needed by this panel
                        if (aHWNode.leaf) {
                            aParam = itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().getParam(aHWNode);
                        } else {
                            aParam = itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().getParam(aHWNode.treeID(),aHWNode.paramDefID()); 
                        }
                        setField(aParam,aHWNode);
                    }
                }
            }
        } catch (RemoteException ex) {
            logger.debug("Error during getComponentParam: "+ ex);
            itsParamList=null;
            return;
        }
        
        initPanel();
    }
    public boolean isSingleton() {
        return false;
    }
    
    public JPanel getInstance() {
        return new OLAPConfigPanel();
    }
    public boolean hasPopupMenu() {
        return false;
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
    }
    
    /** Restore original Values in OLAP_Hardware panel
     */
    private void restoreOLAP_HW() {
        AMCServerHostText.setText(itsAMCServerHost.limits);
        AMCServerPortText.setText(itsAMCServerPort.limits);
        DelayCompensationHostText.setText(itsDelayCompensationHost.limits);
        itsNewDelayCompensationPorts=itsDelayCompensationPorts.limits;
        InputClusterFENText.setText(itsInputClusterFEN.limits);
        itsNewInputBGLHosts=itsInputBGLHosts.limits;
        itsNewInputBGLPorts=itsInputBGLPorts.limits;
        StellaFENText.setText(itsStellaFEN.limits);
        StorageClusterFENText.setText(itsStorageClusterFEN.limits);
        itsNewBGLStorageHosts=itsBGLStorageHosts.limits;
        itsNewBGLStoragePorts=itsBGLStoragePorts.limits;
        partitionText.setText(itsPartition.limits);

    }
    
    /** Restore original Values in OLAP panel
     */
    private void restoreOLAP() {
        samplesToIntegrateText.setText(itsSamplesToIntegrate.limits);
        secondsToBufferText.setText(itsSecondsToBuffer.limits);
        useAMCServerCheckBox.setSelected(LofarUtils.StringToBoolean(itsUseAMCServer.limits));
        nodesPerCellText.setText(itsNodesPerCell.limits);
        subbandsPerCellText.setText(itsSubbandsPerCell.limits);
        ppfTapsText.setText(itsPpfTaps.limits);     
 }

    private void initialize() {
        buttonPanel1.addButton("Save Settings");
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
            // [TODO]
            // Fill from existing cfg needed ????
         } else {
            logger.debug("ERROR:  no node given");
        }
    }
    /* Set's the different fields in the GUI */
    private void setField(jOTDBparam aParam, jOTDBnode aNode) {
        // OLAP_HW settings
        if (aParam==null) {
            return;
        }
        boolean isRef = LofarUtils.isReference(aNode.limits);
        String aKeyName = LofarUtils.keyName(aParam.name);
        
        if (aKeyName.equals("AMCServerHost")) {
            AMCServerHostText.setToolTipText(aParam.description);
            if (isRef) {
                AMCServerHostText.setText(aParam.limits + " : " + aNode.limits);
                AMCServerHostText.setEnabled(false);
                itsAMCServerHost=null;
            } else {
                AMCServerHostText.setText(aParam.limits);
                AMCServerHostText.setEnabled(true);
                itsAMCServerHost=aParam;
            }
        } else if (aKeyName.equals("AMCServerPort")) {
            AMCServerPortText.setToolTipText(aParam.description);
            if (isRef) {
                AMCServerPortText.setText(aParam.limits + " : " + aNode.limits);
                AMCServerPortText.setEnabled(false);
                itsAMCServerPort=null;
            } else {
                AMCServerPortText.setText(aParam.limits);
                AMCServerPortText.setEnabled(true);
                itsAMCServerPort=aParam;
            }
        } else if (aKeyName.equals("DelayCompensationHost")) {
            DelayCompensationHostText.setToolTipText(aParam.description);
            if (isRef) {
                DelayCompensationHostText.setText(aParam.limits + " : " + aNode.limits);
                DelayCompensationHostText.setEnabled(false);
                itsDelayCompensationHost=null;
            } else {
                DelayCompensationHostText.setText(aParam.limits);
                DelayCompensationHostText.setEnabled(true);
                itsDelayCompensationHost=aParam;
            }
        } else if (aKeyName.equals("DelayCompensationPorts")) {
            itsDelayCompensationPorts=aParam;
        } else if (aKeyName.equals("InputClusterFEN")) {
            InputClusterFENText.setToolTipText(aParam.description);
            if (isRef) {
                InputClusterFENText.setText(aParam.limits + " : " + aNode.limits);
                InputClusterFENText.setEnabled(false);
                itsInputClusterFEN=null;
            } else {
                InputClusterFENText.setText(aParam.limits);
                InputClusterFENText.setEnabled(true);
                itsInputClusterFEN=aParam;
            }
        } else if (aKeyName.equals("InputBGLHosts")) {
            itsInputBGLHosts=aParam;
        } else if (aKeyName.equals("InputBGLPorts")) {
            itsInputBGLPorts=aParam;
        } else if (aKeyName.equals("StellaFEN")) {
            StellaFENText.setToolTipText(aParam.description);
            if (isRef) {
                StellaFENText.setText(aParam.limits + " : " + aNode.limits);
                StellaFENText.setEnabled(false);
                itsStellaFEN=null;
            } else {
                StellaFENText.setText(aParam.limits);
                StellaFENText.setEnabled(true);
                itsStellaFEN=aParam;
            }
        } else if (aKeyName.equals("StorageClusterFEN")) {
            StorageClusterFENText.setToolTipText(aParam.description);
            if (isRef) {
                StorageClusterFENText.setText(aParam.limits + " : " + aNode.limits);
                StorageClusterFENText.setEnabled(false);
                itsStorageClusterFEN=null;
            } else {
                StorageClusterFENText.setText(aParam.limits);
                StorageClusterFENText.setEnabled(true);
                itsStorageClusterFEN=aParam;
            }
        } else if (aKeyName.equals("BGLStorageHosts")) {
            itsBGLStorageHosts=aParam;
        } else if (aKeyName.equals("BGLStoragePorts")) {
            itsBGLStoragePorts=aParam;
        } else if (aKeyName.equals("partition")) {
            partitionText.setToolTipText(aParam.description);
            if (isRef) {
                partitionText.setText(aParam.limits + " : " + aNode.limits);
                partitionText.setEnabled(false);
                itsPartition=null;
            } else {
                partitionText.setToolTipText(aParam.description);
                partitionText.setText(aParam.limits);
                partitionText.setEnabled(true);
                itsPartition=aParam;
            }

        // OLAP Specific parameters    
        
        } else if (aKeyName.equals("samplesToIntegrate")) {
            samplesToIntegrateText.setToolTipText(aParam.description);
            if (isRef) {
                samplesToIntegrateText.setText(aParam.limits + " : " + aNode.limits);
                samplesToIntegrateText.setEnabled(false);
                itsSamplesToIntegrate=null;
            } else {
                samplesToIntegrateText.setText(aParam.limits);
                samplesToIntegrateText.setEnabled(true);
                itsSamplesToIntegrate=aParam;
            }
        } else if (aKeyName.equals("secondsToBuffer")) {
            secondsToBufferText.setToolTipText(aParam.description);
            if (isRef) {
                secondsToBufferText.setText(aParam.limits + " : " + aNode.limits);
                secondsToBufferText.setEnabled(false);
                itsSecondsToBuffer=null;
            } else {
                secondsToBufferText.setText(aParam.limits);
                secondsToBufferText.setEnabled(true);
                itsSecondsToBuffer=aParam;
            }
        } else if (aKeyName.equals("useAMCServer")) {
            useAMCServerCheckBox.setToolTipText(aParam.description);
            if (isRef) {
                useAMCServerCheckBox.setSelected(LofarUtils.StringToBoolean(aNode.limits));
                useAMCServerCheckBox.setEnabled(false);
                itsUseAMCServer=null;
            } else {
                useAMCServerCheckBox.setSelected(LofarUtils.StringToBoolean(aParam.limits));
                useAMCServerCheckBox.setEnabled(true);
                itsUseAMCServer=aParam;
            }
        } else if (aKeyName.equals("nodesPerCell")) {
            nodesPerCellText.setToolTipText(aParam.description);
            if (isRef) {
                nodesPerCellText.setText(aParam.limits + " : " + aNode.limits);
                nodesPerCellText.setEnabled(false);
                itsNodesPerCell=null;
            } else {
                nodesPerCellText.setText(aParam.limits);
                nodesPerCellText.setEnabled(true);
                itsNodesPerCell=aParam;
            }
        } else if (aKeyName.equals("subbandsPerCell")) {
            subbandsPerCellText.setToolTipText(aParam.description);
            if (isRef) {
                subbandsPerCellText.setText(aParam.limits + " : " + aNode.limits);
                subbandsPerCellText.setEnabled(false);
                itsSubbandsPerCell=null;
            } else {
                subbandsPerCellText.setText(aParam.limits);
                subbandsPerCellText.setEnabled(true);
                itsSubbandsPerCell=aParam;
            }
        } else if (aKeyName.equals("ppfTaps")) {
            ppfTapsText.setToolTipText(aParam.description);
            if (isRef) {
                ppfTapsText.setText(aParam.limits + " : " + aNode.limits);
                ppfTapsText.setEnabled(false);
                itsPpfTaps=null;
            } else {
                ppfTapsText.setText(aParam.limits);
                ppfTapsText.setEnabled(true);
                itsPpfTaps=aParam;
            }
         
         // Generic Settings used by OLAP
        } else if (aKeyName.equals("nrSubbands")) {
            nrSubbandsText.setToolTipText(aParam.description);
            if (isRef) {
                nrSubbandsText.setText(aParam.limits + " : " + aNode.limits);
                nrSubbandsText.setEnabled(false);
                itsNrSubbands=null;
            } else {
                nrSubbandsText.setText(aParam.limits);
                nrSubbandsText.setEnabled(true);
                itsNrSubbands=aParam;
            }
        } else if (aKeyName.equals("nrChannelsPerSubband")) {
            nrChannelsPerSubbandText.setToolTipText(aParam.description);
            if (isRef) {
                nrChannelsPerSubbandText.setText(aParam.limits + " : " + aNode.limits);
                nrChannelsPerSubbandText.setEnabled(false);
                itsNrChannelsPerSubband=null;
            } else {
                nrChannelsPerSubbandText.setText(aParam.limits);
                nrChannelsPerSubbandText.setEnabled(true);
                itsNrChannelsPerSubband=aParam;
            }
        } else if (aKeyName.equals("nrStations")) {
            nrStationsText.setToolTipText(aParam.description);
            if (isRef) {
                nrStationsText.setText(aParam.limits + " : " + aNode.limits);
                nrStationsText.setEnabled(false);
                itsNrStations=null;
            } else {
                nrStationsText.setText(aParam.limits);
                nrStationsText.setEnabled(true);
                itsNrStations=aParam;
            }
        } else if (aKeyName.equals("nrRSPBoards")) {
            nrRSPBoardsText.setToolTipText(aParam.description);
            if (isRef) {
                nrRSPBoardsText.setText(aParam.limits + " : " + aNode.limits);
                nrRSPBoardsText.setEnabled(false);
                itsNrRSPBoards=null;
            } else {
                nrRSPBoardsText.setText(aParam.limits);
                nrRSPBoardsText.setEnabled(true);
                itsNrRSPBoards=aParam;
            }
        } else if (aKeyName.equals("nrSamplesPerSecond")) {
            nrSamplesPerSecondText.setToolTipText(aParam.description);
            if (isRef) {
                nrSamplesPerSecondText.setText(aParam.limits + " : " + aNode.limits);
                nrSamplesPerSecondText.setEnabled(false);
                itsNrSamplesPerSecond=null;
            } else {
                nrSamplesPerSecondText.setText(aParam.limits);
                nrSamplesPerSecondText.setEnabled(true);
                itsNrSamplesPerSecond=aParam;
            }
        } else if (aKeyName.equals("MSName")) {
            MSNameText.setToolTipText(aParam.description);
            if (isRef) {
                MSNameText.setText(aParam.limits + " : " + aNode.limits);
                MSNameText.setEnabled(false);
                itsMSName=null;
            } else {
                MSNameText.setText(aParam.limits);
                MSNameText.setEnabled(true);
                itsMSName=aParam;
            }
        } else if (aKeyName.equals("nrSamplesPerEthFrame")) {
            nrSamplesPerEthFrameText.setToolTipText(aParam.description);
            if (isRef) {
                nrSamplesPerEthFrameText.setText(aParam.limits + " : " + aNode.limits);
                nrSamplesPerEthFrameText.setEnabled(false);
                itsNrSamplesPerEthFrame=null;
            } else {
                nrSamplesPerEthFrameText.setText(aParam.limits);
                nrSamplesPerEthFrameText.setEnabled(true);
                itsNrSamplesPerEthFrame=aParam;
            }
        } else if (aKeyName.equals("nyguistZone")) {
            nyguistZoneText.setToolTipText(aParam.description);
            if (isRef) {
                nyguistZoneText.setText(aParam.limits + " : " + aNode.limits);
                nyguistZoneText.setEnabled(false);
                itsNyguistZone=null;
            } else {
                nyguistZoneText.setText(aParam.limits);
                nyguistZoneText.setEnabled(true);
                itsNyguistZone=aParam;
            }
        } else if (aKeyName.equals("startTime")) {
            startTimeText.setToolTipText(aParam.description);
            if (isRef) {
                startTimeText.setText(aParam.limits + " : " + aNode.limits);
                startTimeText.setEnabled(false);
                itsStartTime=null;
            } else {
                startTimeText.setText(aParam.limits);
                startTimeText.setEnabled(true);
                itsStartTime=aParam;
            }
        } else if (aKeyName.equals("stopTime")) {
            stopTimeText.setToolTipText(aParam.description);
            if (isRef) {
                stopTimeText.setText(aParam.limits + " : " + aNode.limits);
                stopTimeText.setEnabled(false);
                itsStopTime=null;
            } else {
                stopTimeText.setText(aParam.limits);
                stopTimeText.setEnabled(true);
                itsStopTime=aParam;
            }
        } else if (aKeyName.equals("stsPosition")) {
            itsStsPosition=aParam;
        } else if (aKeyName.equals("RSPMACAddresses")) {
            itsRSPMACAddresses=aParam;
        } else if (aKeyName.equals("InputNodeMACAddresses")) {
            itsInputNodeMACAddresses=aParam;
        }
    }
    
    /** saves the given param back to the database
     */
    private void saveParam(jOTDBparam aParam) {
        if (aParam == null) {
            return;
        }
        try {
            itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().saveParam(aParam); 
        } catch (RemoteException ex) {
            logger.debug("Error: saveParam failed : " + ex);
        } 
    }
    
    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
        this.enableSpecificButtons(enabled);
        this.enableHardwareButtons(enabled);
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
        this.setSpecificButtonsVisible(visible);
        this.setHardwareButtonsVisible(visible);
    }

    

    private void enableSpecificButtons(boolean enabled) {
        this.SpecificRevertButton.setEnabled(enabled);
    }
    
    private void setSpecificButtonsVisible(boolean visible) {
        this.SpecificRevertButton.setVisible(visible);    }

    private void enableHardwareButtons(boolean enabled) {
        this.HardwareRevertButton.setEnabled(enabled);
    }
    
    private void setHardwareButtonsVisible(boolean visible) {
        this.HardwareRevertButton.setVisible(visible);
    }
    

    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
        enableSpecificButtons(enabled);
        enableHardwareButtons(enabled);
    }
    
    private void saveInput() {
        boolean hasChanged = false;
        
        // OLAP_HW
        
        if (itsAMCServerHost != null && !AMCServerHostText.equals(itsAMCServerHost.limits)) {  
            itsAMCServerHost.limits = AMCServerHostText.getText();
            saveParam(itsAMCServerHost);
        }
        if (itsAMCServerPort != null && !AMCServerPortText.equals(itsAMCServerPort.limits)) {
            itsAMCServerPort.limits=AMCServerPortText.getText();
            saveParam(itsAMCServerPort);
        }
        if (itsDelayCompensationHost != null && !DelayCompensationHostText.equals(itsDelayCompensationHost.limits)) {
            itsDelayCompensationHost.limits=DelayCompensationHostText.getText();
            saveParam(itsDelayCompensationHost);
        }
        if (itsDelayCompensationPorts != null &&
                !itsNewDelayCompensationPorts.equals(itsDelayCompensationPorts.limits) &&
                itsNewDelayCompensationPorts.length()>0) {
            itsDelayCompensationPorts.limits=itsNewDelayCompensationPorts;
            saveParam(itsDelayCompensationPorts);
        }
        if (itsInputClusterFEN != null && !InputClusterFENText.equals(itsInputClusterFEN.limits)) {
            itsInputClusterFEN.limits=InputClusterFENText.getText();
            saveParam(itsInputClusterFEN);
        }
        if (itsInputBGLHosts != null && 
                !itsNewInputBGLHosts.equals(itsInputBGLHosts.limits) &&
                itsNewInputBGLHosts.length()>0) {
            itsInputBGLHosts.limits=itsNewInputBGLHosts;
            saveParam(itsInputBGLHosts);
        }
        if (itsInputBGLPorts != null &&
                !itsNewInputBGLPorts.equals(itsInputBGLPorts.limits) &&
                itsNewInputBGLPorts.length()>0) {
            itsInputBGLPorts.limits=itsNewInputBGLPorts;
            saveParam(itsInputBGLPorts);
        }
        if (itsStellaFEN != null && !StellaFENText.equals(itsStellaFEN.limits)) {
            itsStellaFEN.limits=StellaFENText.getText();
            saveParam(itsStellaFEN);
        }
        if (itsStorageClusterFEN != null && !StorageClusterFENText.equals(itsStorageClusterFEN.limits)) {
            itsStorageClusterFEN.limits=StorageClusterFENText.getText();
            saveParam(itsStorageClusterFEN);
        }
        if (itsBGLStorageHosts != null && 
                !itsNewBGLStorageHosts.equals(itsBGLStorageHosts.limits) &&
                itsNewBGLStorageHosts.length()>0) {
            itsBGLStorageHosts.limits=itsNewBGLStorageHosts;
            saveParam(itsBGLStorageHosts);
        }
        if (itsBGLStoragePorts != null &&
                !itsNewBGLStoragePorts.equals(itsBGLStoragePorts.limits) &&
                itsNewBGLStoragePorts.length()>0) {
            itsBGLStoragePorts.limits=itsNewBGLStoragePorts;
            saveParam(itsBGLStoragePorts);
        }
        if (itsPartition != null && !partitionText.equals(itsPartition.limits)) {
            itsPartition.limits=partitionText.getText();
            saveParam(itsPartition);
        }
        
        // OLAP Specific parameters    
        
        if (itsSamplesToIntegrate != null && !samplesToIntegrateText.equals(itsSamplesToIntegrate.limits)) {
            itsSamplesToIntegrate.limits=samplesToIntegrateText.getText();
            saveParam(itsSamplesToIntegrate);
        }
        if (itsSecondsToBuffer != null && !secondsToBufferText.equals(itsSecondsToBuffer.limits)) {
            itsSecondsToBuffer.limits=secondsToBufferText.getText();
            saveParam(itsSecondsToBuffer);
        }
        if (itsUseAMCServer != null && !useAMCServerCheckBox.isSelected() == LofarUtils.StringToBoolean(itsUseAMCServer.limits)) {
            itsUseAMCServer.limits = LofarUtils.BooleanToString(useAMCServerCheckBox.isSelected());
            saveParam(itsUseAMCServer);
        }
        if (itsNodesPerCell != null && !nodesPerCellText.equals(itsNodesPerCell.limits)) {
            itsNodesPerCell.limits=nodesPerCellText.getText();
            saveParam(itsNodesPerCell);
        }
        if (itsSubbandsPerCell != null && !subbandsPerCellText.equals(itsSubbandsPerCell.limits)) {
            itsSubbandsPerCell.limits=subbandsPerCellText.getText();
            saveParam(itsSubbandsPerCell);
        }
        if (itsPpfTaps != null && !ppfTapsText.equals(itsPpfTaps.limits)) {
            itsPpfTaps.limits=ppfTapsText.getText();
            saveParam(itsPpfTaps);
        }
    }

    private void editList(String aType) {
        // Possible buttons so far :
        // DelayPorts
        // InputHosts
        // InputPorts
        // StorageHosts
        // StoragePorts
        
        // If the panel is still Visible it is probably not saved the last time
        // if so check if it has changed and if the calling button is not the same as the last time        
        if (listPanel.isVisible() && listPanel.isNew() && !listPanel.getTitle().equals(aType)) {
            if (JOptionPane.showConfirmDialog(this,"You didn't save your edit changes last time, are you sure you want to discard them ?",
                    "Discard list changes",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
                
                // Yes he is sure he wants to discard
            } else {
                return;
            }        
        }
        // in all other cases fill the edit pane with the info and set visible
        
        String aList="";
        if (aType.equals("DelayPorts")) {
            if (itsNewDelayCompensationPorts.length()<1) {
                aList=itsDelayCompensationPorts.limits;
            } else {
                aList=itsNewDelayCompensationPorts;
            }
        } else if (aType.equals("InputHosts")) {
            if (itsNewInputBGLHosts.length()<1) {
                aList=itsInputBGLHosts.limits;
            } else {
                aList=itsNewInputBGLHosts;
            }
            
        } else if (aType.equals("InputPorts")) {
            if (itsNewInputBGLPorts.length()<1) {
                aList=itsInputBGLPorts.limits;
            } else {
                aList=itsNewInputBGLPorts;
            }
            
        } else if (aType.equals("StorageHosts")) {
            if (itsNewBGLStorageHosts.length()<1) {
                aList=itsBGLStorageHosts.limits;
            } else {
                aList=itsNewBGLStorageHosts;
            }
            
        } else if (aType.equals("StoragePorts")) {
            if (itsNewBGLStoragePorts.length()<1) {
                aList=itsBGLStoragePorts.limits;
            } else {
                aList=itsNewBGLStorageHosts;
            }
            
        } else {
            logger.debug("Wrong ListType received: "+aType);
            return;
        }
        
        listPanel.setTitle(aType);
        listPanel.setList(aList);
        listPanel.validate();
        listPanel.setVisible(true);
    } 
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        jTabbedPane1 = new javax.swing.JTabbedPane();
        OLAPGenericPanel = new javax.swing.JPanel();
        jLabel13 = new javax.swing.JLabel();
        nrSubbandsText = new javax.swing.JTextField();
        jLabel14 = new javax.swing.JLabel();
        nrChannelsPerSubbandText = new javax.swing.JTextField();
        jLabel15 = new javax.swing.JLabel();
        nrStationsText = new javax.swing.JTextField();
        jLabel16 = new javax.swing.JLabel();
        nrRSPBoardsText = new javax.swing.JTextField();
        jLabel17 = new javax.swing.JLabel();
        nrSamplesPerSecondText = new javax.swing.JTextField();
        jLabel18 = new javax.swing.JLabel();
        MSNameText = new javax.swing.JTextField();
        jLabel19 = new javax.swing.JLabel();
        nrSamplesPerEthFrameText = new javax.swing.JTextField();
        jLabel20 = new javax.swing.JLabel();
        nyguistZoneText = new javax.swing.JTextField();
        jLabel21 = new javax.swing.JLabel();
        startTimeText = new javax.swing.JTextField();
        jLabel22 = new javax.swing.JLabel();
        stopTimeText = new javax.swing.JTextField();
        jLabel23 = new javax.swing.JLabel();
        viewStationPositions = new javax.swing.JButton();
        jLabel24 = new javax.swing.JLabel();
        jLabel25 = new javax.swing.JLabel();
        jButton1 = new javax.swing.JButton();
        viewInputMACAddresses = new javax.swing.JButton();
        OLAPSpecificPanel = new javax.swing.JPanel();
        NrSamplesLabel = new javax.swing.JLabel();
        samplesToIntegrateText = new javax.swing.JTextField();
        NrBufSecLabel = new javax.swing.JLabel();
        secondsToBufferText = new javax.swing.JTextField();
        UseAMCLabel = new javax.swing.JLabel();
        useAMCServerCheckBox = new javax.swing.JCheckBox();
        NrNodesCellLabel = new javax.swing.JLabel();
        nodesPerCellText = new javax.swing.JTextField();
        NrSubbandsCellLabel = new javax.swing.JLabel();
        subbandsPerCellText = new javax.swing.JTextField();
        NrFilterTabsLabel = new javax.swing.JLabel();
        ppfTapsText = new javax.swing.JTextField();
        SpecificRevertButton = new javax.swing.JButton();
        OLAPHardwarePanel = new javax.swing.JPanel();
        HardwareRevertButton = new javax.swing.JButton();
        jPanel1 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        AMCServerHostText = new javax.swing.JTextField();
        jLabel7 = new javax.swing.JLabel();
        AMCServerPortText = new javax.swing.JTextField();
        jPanel2 = new javax.swing.JPanel();
        jLabel5 = new javax.swing.JLabel();
        StellaFENText = new javax.swing.JTextField();
        jLabel6 = new javax.swing.JLabel();
        partitionText = new javax.swing.JTextField();
        jPanel3 = new javax.swing.JPanel();
        DelayCompensationHostText = new javax.swing.JTextField();
        DelayPorts = new javax.swing.JButton();
        jLabel2 = new javax.swing.JLabel();
        jLabel8 = new javax.swing.JLabel();
        jPanel4 = new javax.swing.JPanel();
        jLabel3 = new javax.swing.JLabel();
        InputClusterFENText = new javax.swing.JTextField();
        jLabel4 = new javax.swing.JLabel();
        jLabel9 = new javax.swing.JLabel();
        InputHosts = new javax.swing.JButton();
        InputPorts = new javax.swing.JButton();
        jPanel5 = new javax.swing.JPanel();
        jLabel12 = new javax.swing.JLabel();
        StorageClusterFENText = new javax.swing.JTextField();
        jLabel11 = new javax.swing.JLabel();
        jLabel10 = new javax.swing.JLabel();
        StorageHosts = new javax.swing.JButton();
        StoragePorts = new javax.swing.JButton();
        listPanel = new nl.astron.lofar.sas.otbcomponents.ListPanel();
        listPanel.setVisible(false);
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        setLayout(new java.awt.BorderLayout());

        jLabel13.setText("#Subbands:");

        jLabel14.setText("#Channels per Subband:");

        jLabel15.setText("#Stations:");

        jLabel16.setText("#RSPBoards:");

        jLabel17.setText("#SamplesPerSecond:");

        jLabel18.setText("MS Name:");

        jLabel19.setText("#Samples per EthernetFrame:");

        jLabel20.setText("Nyguist Zone:");

        jLabel21.setText("Starttime:");

        jLabel22.setText("Stoptime:");

        jLabel23.setText("StationPositions:");

        viewStationPositions.setText("View");

        jLabel24.setText("RSP MAC Addresses:");

        jLabel25.setText("Input Node MAC Addresses:");

        jButton1.setText("View");

        viewInputMACAddresses.setText("View");
        viewInputMACAddresses.setOpaque(false);

        org.jdesktop.layout.GroupLayout OLAPGenericPanelLayout = new org.jdesktop.layout.GroupLayout(OLAPGenericPanel);
        OLAPGenericPanel.setLayout(OLAPGenericPanelLayout);
        OLAPGenericPanelLayout.setHorizontalGroup(
            OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(OLAPGenericPanelLayout.createSequentialGroup()
                .addContainerGap()
                .add(OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jLabel13)
                    .add(jLabel14)
                    .add(jLabel15)
                    .add(jLabel17)
                    .add(jLabel16)
                    .add(jLabel19)
                    .add(jLabel18)
                    .add(jLabel20)
                    .add(jLabel21)
                    .add(jLabel22)
                    .add(jLabel23)
                    .add(jLabel25)
                    .add(jLabel24))
                .add(12, 12, 12)
                .add(OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                        .add(stopTimeText)
                        .add(startTimeText)
                        .add(MSNameText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 255, Short.MAX_VALUE)
                        .add(nyguistZoneText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 134, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .add(nrChannelsPerSubbandText)
                        .add(nrSubbandsText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 131, Short.MAX_VALUE)
                        .add(nrStationsText)
                        .add(nrSamplesPerEthFrameText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 133, Short.MAX_VALUE)
                        .add(nrRSPBoardsText)
                        .add(nrSamplesPerSecondText))
                    .add(viewStationPositions)
                    .add(viewInputMACAddresses)
                    .add(jButton1))
                .addContainerGap(1444, Short.MAX_VALUE))
        );

        OLAPGenericPanelLayout.linkSize(new java.awt.Component[] {MSNameText, nrChannelsPerSubbandText, nrRSPBoardsText, nrSamplesPerEthFrameText, nrSamplesPerSecondText, nrStationsText, nrSubbandsText, nyguistZoneText, startTimeText, stopTimeText}, org.jdesktop.layout.GroupLayout.HORIZONTAL);

        OLAPGenericPanelLayout.setVerticalGroup(
            OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(OLAPGenericPanelLayout.createSequentialGroup()
                .add(30, 30, 30)
                .add(OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel13)
                    .add(nrSubbandsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel14)
                    .add(nrChannelsPerSubbandText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(12, 12, 12)
                .add(OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel15)
                    .add(nrStationsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(12, 12, 12)
                .add(OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel16)
                    .add(nrRSPBoardsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel17)
                    .add(nrSamplesPerSecondText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(9, 9, 9)
                .add(OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel19)
                    .add(nrSamplesPerEthFrameText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(MSNameText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(jLabel18))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel20)
                    .add(nyguistZoneText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel21)
                    .add(startTimeText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel22)
                    .add(stopTimeText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel23)
                    .add(viewStationPositions, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 18, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel24)
                    .add(jButton1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 18, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel25)
                    .add(viewInputMACAddresses, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 18, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(321, Short.MAX_VALUE))
        );
        jTabbedPane1.addTab("Generic", OLAPGenericPanel);

        NrSamplesLabel.setText("#Samples to Integrate :");

        samplesToIntegrateText.setToolTipText("Nr of Samples to integrate");
        samplesToIntegrateText.setMaximumSize(new java.awt.Dimension(440, 19));
        samplesToIntegrateText.setMinimumSize(new java.awt.Dimension(440, 19));
        samplesToIntegrateText.setPreferredSize(new java.awt.Dimension(440, 19));

        NrBufSecLabel.setText("#Seconds to buffer :");

        secondsToBufferText.setToolTipText("Number of seconds that need 2 be buffered");
        secondsToBufferText.setMaximumSize(new java.awt.Dimension(200, 19));
        secondsToBufferText.setMinimumSize(new java.awt.Dimension(200, 19));
        secondsToBufferText.setPreferredSize(new java.awt.Dimension(200, 19));

        UseAMCLabel.setText("Use AMC server :");

        useAMCServerCheckBox.setToolTipText("Do you want to use an AMC server?");
        useAMCServerCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        useAMCServerCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));

        NrNodesCellLabel.setText("#Nodes/cell :");

        nodesPerCellText.setToolTipText("Number of Nodes per Cell");
        nodesPerCellText.setMaximumSize(new java.awt.Dimension(200, 19));
        nodesPerCellText.setMinimumSize(new java.awt.Dimension(200, 19));
        nodesPerCellText.setPreferredSize(new java.awt.Dimension(200, 19));

        NrSubbandsCellLabel.setText("#Subbands/cell :");

        subbandsPerCellText.setToolTipText("Number of Subbands per cell");
        subbandsPerCellText.setMaximumSize(new java.awt.Dimension(200, 19));
        subbandsPerCellText.setMinimumSize(new java.awt.Dimension(200, 19));
        subbandsPerCellText.setPreferredSize(new java.awt.Dimension(200, 19));

        NrFilterTabsLabel.setText("#Filter tabs :");

        ppfTapsText.setToolTipText("Number of Filter Tabs for PPFl");

        SpecificRevertButton.setText("Revert");
        SpecificRevertButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SpecificRevertButtonActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout OLAPSpecificPanelLayout = new org.jdesktop.layout.GroupLayout(OLAPSpecificPanel);
        OLAPSpecificPanel.setLayout(OLAPSpecificPanelLayout);
        OLAPSpecificPanelLayout.setHorizontalGroup(
            OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(OLAPSpecificPanelLayout.createSequentialGroup()
                .addContainerGap()
                .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(OLAPSpecificPanelLayout.createSequentialGroup()
                        .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(UseAMCLabel)
                            .add(NrNodesCellLabel)
                            .add(NrSubbandsCellLabel)
                            .add(NrFilterTabsLabel)
                            .add(NrBufSecLabel)
                            .add(NrSamplesLabel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 150, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                        .add(16, 16, 16)
                        .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(ppfTapsText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 234, Short.MAX_VALUE)
                            .add(useAMCServerCheckBox)
                            .add(nodesPerCellText, 0, 0, Short.MAX_VALUE)
                            .add(secondsToBufferText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 234, Short.MAX_VALUE)
                            .add(samplesToIntegrateText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 234, Short.MAX_VALUE)
                            .add(subbandsPerCellText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 234, Short.MAX_VALUE)))
                    .add(SpecificRevertButton))
                .addContainerGap(1456, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
        );
        OLAPSpecificPanelLayout.setVerticalGroup(
            OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, OLAPSpecificPanelLayout.createSequentialGroup()
                .addContainerGap()
                .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(samplesToIntegrateText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 19, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(NrSamplesLabel))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(NrBufSecLabel)
                    .add(secondsToBufferText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(UseAMCLabel)
                    .add(useAMCServerCheckBox))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(NrNodesCellLabel)
                    .add(nodesPerCellText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(NrSubbandsCellLabel)
                    .add(subbandsPerCellText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(NrFilterTabsLabel)
                    .add(ppfTapsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(44, 44, 44)
                .add(SpecificRevertButton)
                .add(307, 307, 307))
        );
        jTabbedPane1.addTab("OLAP Specific", OLAPSpecificPanel);

        HardwareRevertButton.setText("Revert");
        HardwareRevertButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                HardwareRevertButtonActionPerformed(evt);
            }
        });

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "AMC Server", javax.swing.border.TitledBorder.CENTER, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11)));
        jLabel1.setText("Host:");

        AMCServerHostText.setToolTipText("Give Machine where AMC server runs (hostname or IP address)");

        jLabel7.setText("Port:");

        AMCServerPortText.setToolTipText("Port to reach AMC Server");

        org.jdesktop.layout.GroupLayout jPanel1Layout = new org.jdesktop.layout.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jLabel1)
                    .add(jLabel7))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, 135, Short.MAX_VALUE)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(AMCServerHostText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 215, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(AMCServerPortText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel1Layout.createSequentialGroup()
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel1)
                    .add(AMCServerHostText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel7)
                    .add(AMCServerPortText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel2.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Stella", javax.swing.border.TitledBorder.CENTER, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11)));
        jLabel5.setText("Front End Node:");

        StellaFENText.setToolTipText("Cluster for Storage (Hostname or IP address)");

        jLabel6.setText("Partition for BGL_Processing:");

        partitionText.setToolTipText("Partition on which BGL_Processing runs.");

        org.jdesktop.layout.GroupLayout jPanel2Layout = new org.jdesktop.layout.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel2Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jLabel6)
                    .add(jLabel5))
                .add(21, 21, 21)
                .add(jPanel2Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(StellaFENText)
                    .add(partitionText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 212, Short.MAX_VALUE))
                .add(14, 14, 14))
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel2Layout.createSequentialGroup()
                .add(jPanel2Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel5)
                    .add(StellaFENText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel2Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel6)
                    .add(partitionText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel3.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Delay Compensation", javax.swing.border.TitledBorder.CENTER, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11)));
        DelayCompensationHostText.setToolTipText("Machine where DelayCompensation runs (Hostname or IP address)");

        DelayPorts.setText("Change/View");
        DelayPorts.setToolTipText("Change or view the list with portnumbers");
        DelayPorts.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                DelayPortsActionPerformed(evt);
            }
        });

        jLabel2.setText("Host:");

        jLabel8.setText("Ports:");

        org.jdesktop.layout.GroupLayout jPanel3Layout = new org.jdesktop.layout.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, jPanel3Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jLabel2)
                    .add(jLabel8))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, 134, Short.MAX_VALUE)
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(DelayPorts)
                    .add(DelayCompensationHostText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 213, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel3Layout.createSequentialGroup()
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(DelayCompensationHostText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(jLabel2))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(DelayPorts, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 18, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(jLabel8))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel4.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Input Cluster", javax.swing.border.TitledBorder.CENTER, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11)));
        jLabel3.setText("FEN :");

        InputClusterFENText.setToolTipText("Cluster for InputSection(Hostname or IP address)");

        jLabel4.setText("Hosts :");

        jLabel9.setText("Ports:");

        InputHosts.setText("Change/View");
        InputHosts.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                InputHostsActionPerformed(evt);
            }
        });

        InputPorts.setText("Change/View");
        InputPorts.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                InputPortsActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout jPanel4Layout = new org.jdesktop.layout.GroupLayout(jPanel4);
        jPanel4.setLayout(jPanel4Layout);
        jPanel4Layout.setHorizontalGroup(
            jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel4Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jLabel3)
                    .add(jLabel4)
                    .add(jLabel9))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, 125, Short.MAX_VALUE)
                .add(jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(InputHosts)
                    .add(InputClusterFENText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 217, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(InputPorts))
                .addContainerGap())
        );
        jPanel4Layout.setVerticalGroup(
            jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel4Layout.createSequentialGroup()
                .add(jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel3)
                    .add(InputClusterFENText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel4)
                    .add(InputHosts, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 18, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel9)
                    .add(InputPorts, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 18, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel5.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Storage Cluster", javax.swing.border.TitledBorder.CENTER, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11)));
        jLabel12.setText("FEN :");

        jLabel11.setText("Hosts :");

        jLabel10.setText("Ports:");

        StorageHosts.setText("Change/View");
        StorageHosts.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                StorageHostsActionPerformed(evt);
            }
        });

        StoragePorts.setText("Change/View");
        StoragePorts.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                StoragePortsActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout jPanel5Layout = new org.jdesktop.layout.GroupLayout(jPanel5);
        jPanel5.setLayout(jPanel5Layout);
        jPanel5Layout.setHorizontalGroup(
            jPanel5Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel5Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel5Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel5Layout.createSequentialGroup()
                        .add(jLabel12)
                        .add(112, 112, 112))
                    .add(jPanel5Layout.createSequentialGroup()
                        .add(jLabel10)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED))
                    .add(jPanel5Layout.createSequentialGroup()
                        .add(jLabel11, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 159, Short.MAX_VALUE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel5Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(StorageHosts)
                    .add(StorageClusterFENText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 217, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(StoragePorts))
                .addContainerGap())
        );
        jPanel5Layout.setVerticalGroup(
            jPanel5Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel5Layout.createSequentialGroup()
                .add(jPanel5Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel12)
                    .add(StorageClusterFENText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel5Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel11)
                    .add(StorageHosts, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 18, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel5Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel10)
                    .add(StoragePorts, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 18, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(24, Short.MAX_VALUE))
        );

        listPanel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                listPanelActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout OLAPHardwarePanelLayout = new org.jdesktop.layout.GroupLayout(OLAPHardwarePanel);
        OLAPHardwarePanel.setLayout(OLAPHardwarePanelLayout);
        OLAPHardwarePanelLayout.setHorizontalGroup(
            OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(OLAPHardwarePanelLayout.createSequentialGroup()
                .addContainerGap()
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(OLAPHardwarePanelLayout.createSequentialGroup()
                        .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(jPanel5, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .add(jPanel2, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 408, Short.MAX_VALUE)
                            .add(jPanel4, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .add(jPanel3, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .add(org.jdesktop.layout.GroupLayout.TRAILING, jPanel1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                        .add(16, 16, 16)
                        .add(listPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .add(954, 954, 954))
                    .add(HardwareRevertButton)))
        );
        OLAPHardwarePanelLayout.setVerticalGroup(
            OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(OLAPHardwarePanelLayout.createSequentialGroup()
                .add(24, 24, 24)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(listPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(OLAPHardwarePanelLayout.createSequentialGroup()
                        .add(jPanel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(jPanel3, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(jPanel4, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(jPanel2, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(jPanel5, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                .add(20, 20, 20)
                .add(HardwareRevertButton)
                .addContainerGap(131, Short.MAX_VALUE))
        );
        jTabbedPane1.addTab("OLAP Hardware", OLAPHardwarePanel);

        add(jTabbedPane1, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });

        add(buttonPanel1, java.awt.BorderLayout.SOUTH);

    }// </editor-fold>//GEN-END:initComponents

    private void listPanelActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_listPanelActionPerformed
        String aCommand = evt.getActionCommand();
        String aType = listPanel.getTitle();
        if (aType.equals("DelayPorts")) {
            itsNewDelayCompensationPorts=listPanel.getList();
        } else if (aType.equals("InputHosts")) {
            itsNewInputBGLHosts=listPanel.getList();
        } else if (aType.equals("InputPorts")) {
            itsNewInputBGLPorts=listPanel.getList();
        } else if (aType.equals("StorageHosts")) {
            itsNewBGLStorageHosts=listPanel.getList();
        } else if (aType.equals("StoragePorts")) {
            itsNewBGLStorageHosts=listPanel.getList();
        } else {
            logger.debug("Wrong ListType received: "+aType);
            return;
        }       
    }//GEN-LAST:event_listPanelActionPerformed

    private void StoragePortsActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_StoragePortsActionPerformed
        editList("StoragePorts");
    }//GEN-LAST:event_StoragePortsActionPerformed

    private void StorageHostsActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_StorageHostsActionPerformed
        editList("StorageHosts");
    }//GEN-LAST:event_StorageHostsActionPerformed

    private void InputPortsActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_InputPortsActionPerformed
        editList("InputPorts");
    }//GEN-LAST:event_InputPortsActionPerformed

    private void InputHostsActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_InputHostsActionPerformed
        editList("InputHosts");
    }//GEN-LAST:event_InputHostsActionPerformed

    private void DelayPortsActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DelayPortsActionPerformed
        editList("DelayPorts");
    }//GEN-LAST:event_DelayPortsActionPerformed

    private void HardwareRevertButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_HardwareRevertButtonActionPerformed
        restoreOLAP_HW();
    }//GEN-LAST:event_HardwareRevertButtonActionPerformed

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        if(evt.getActionCommand() == "Save Settings") {
            saveInput();
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed

    private void SpecificRevertButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SpecificRevertButtonActionPerformed
        restoreOLAP();
    }//GEN-LAST:event_SpecificRevertButtonActionPerformed
    
    private jOTDBnode itsNode = null;
    private MainFrame  itsMainFrame;
    private Vector<jOTDBparam> itsParamList;
    
    // Generic parameters
    private jOTDBparam itsRSPBoards;
    private jOTDBparam itsRSPBoardsPerStation;
            
    
    // OLAP Specific parameters
    private jOTDBparam itsSamplesToIntegrate;
    private jOTDBparam itsSecondsToBuffer;
    private jOTDBparam itsUseAMCServer;
    private jOTDBparam itsNodesPerCell;
    private jOTDBparam itsSubbandsPerCell;
    private jOTDBparam itsPpfTaps;
  
    
    // OLAP_HW parameters
    private jOTDBparam itsAMCServerHost=null;
    private jOTDBparam itsAMCServerPort=null;
    private jOTDBparam itsDelayCompensationHost=null;
    private jOTDBparam itsDelayCompensationPorts=null;
    private jOTDBparam itsInputClusterFEN=null;
    private jOTDBparam itsInputBGLHosts=null;
    private jOTDBparam itsInputBGLPorts=null;
    private jOTDBparam itsStellaFEN=null;
    private jOTDBparam itsBGLStorageHosts=null;
    private jOTDBparam itsBGLStoragePorts=null;
    private jOTDBparam itsStorageClusterFEN=null;
    private jOTDBparam itsPartition=null;
    
    private String itsNewDelayCompensationPorts = "";
    private String itsNewInputBGLHosts = "";
    private String itsNewInputBGLPorts = "";
    private String itsNewBGLStorageHosts = "";
    private String itsNewBGLStoragePorts = "";
    
    // OLAP Generic
    private jOTDBparam itsNrSubbands = null;
    private jOTDBparam itsNrChannelsPerSubband = null;
    private jOTDBparam itsNrStations = null;
    private jOTDBparam itsNrRSPBoards = null;
    private jOTDBparam itsNrSamplesPerSecond = null;
    private jOTDBparam itsMSName = null;
    private jOTDBparam itsNrSamplesPerEthFrame = null;
    private jOTDBparam itsNyguistZone = null;
    private jOTDBparam itsStartTime = null;
    private jOTDBparam itsStopTime = null;
    private jOTDBparam itsStsPosition = null;
    private jOTDBparam itsRSPMACAddresses = null;
    private jOTDBparam itsInputNodeMACAddresses = null;
    
    private String itsNewStsPositions = "";
    private String itsNewRSPMACAddresses = "";
    private String itsNewInputNodeMACAddresses = "";
    
    
    
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JTextField AMCServerHostText;
    private javax.swing.JTextField AMCServerPortText;
    private javax.swing.JTextField DelayCompensationHostText;
    private javax.swing.JButton DelayPorts;
    private javax.swing.JButton HardwareRevertButton;
    private javax.swing.JTextField InputClusterFENText;
    private javax.swing.JButton InputHosts;
    private javax.swing.JButton InputPorts;
    private javax.swing.JTextField MSNameText;
    private javax.swing.JLabel NrBufSecLabel;
    private javax.swing.JLabel NrFilterTabsLabel;
    private javax.swing.JLabel NrNodesCellLabel;
    private javax.swing.JLabel NrSamplesLabel;
    private javax.swing.JLabel NrSubbandsCellLabel;
    private javax.swing.JPanel OLAPGenericPanel;
    private javax.swing.JPanel OLAPHardwarePanel;
    private javax.swing.JPanel OLAPSpecificPanel;
    private javax.swing.JButton SpecificRevertButton;
    private javax.swing.JTextField StellaFENText;
    private javax.swing.JTextField StorageClusterFENText;
    private javax.swing.JButton StorageHosts;
    private javax.swing.JButton StoragePorts;
    private javax.swing.JLabel UseAMCLabel;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JButton jButton1;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel10;
    private javax.swing.JLabel jLabel11;
    private javax.swing.JLabel jLabel12;
    private javax.swing.JLabel jLabel13;
    private javax.swing.JLabel jLabel14;
    private javax.swing.JLabel jLabel15;
    private javax.swing.JLabel jLabel16;
    private javax.swing.JLabel jLabel17;
    private javax.swing.JLabel jLabel18;
    private javax.swing.JLabel jLabel19;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel20;
    private javax.swing.JLabel jLabel21;
    private javax.swing.JLabel jLabel22;
    private javax.swing.JLabel jLabel23;
    private javax.swing.JLabel jLabel24;
    private javax.swing.JLabel jLabel25;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JLabel jLabel8;
    private javax.swing.JLabel jLabel9;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanel5;
    private javax.swing.JTabbedPane jTabbedPane1;
    private nl.astron.lofar.sas.otbcomponents.ListPanel listPanel;
    private javax.swing.JTextField nodesPerCellText;
    private javax.swing.JTextField nrChannelsPerSubbandText;
    private javax.swing.JTextField nrRSPBoardsText;
    private javax.swing.JTextField nrSamplesPerEthFrameText;
    private javax.swing.JTextField nrSamplesPerSecondText;
    private javax.swing.JTextField nrStationsText;
    private javax.swing.JTextField nrSubbandsText;
    private javax.swing.JTextField nyguistZoneText;
    private javax.swing.JTextField partitionText;
    private javax.swing.JTextField ppfTapsText;
    private javax.swing.JTextField samplesToIntegrateText;
    private javax.swing.JTextField secondsToBufferText;
    private javax.swing.JTextField startTimeText;
    private javax.swing.JTextField stopTimeText;
    private javax.swing.JTextField subbandsPerCellText;
    private javax.swing.JCheckBox useAMCServerCheckBox;
    private javax.swing.JButton viewInputMACAddresses;
    private javax.swing.JButton viewStationPositions;
    // End of variables declaration//GEN-END:variables

    /**
     * Utility field used by event firing mechanism.
     */
    private javax.swing.event.EventListenerList listenerList =  null;

    /**
     * Registers ActionListener to receive events.
     * @param listener The listener to register.
     */
    public synchronized void addActionListener(java.awt.event.ActionListener listener) {

        if (listenerList == null ) {
            listenerList = new javax.swing.event.EventListenerList();
        }
        listenerList.add (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeActionListener(java.awt.event.ActionListener listener) {

        listenerList.remove (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Notifies all registered listeners about the event.
     * 
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {

        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList ();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed (event);
            }
        }
    }




    
}
