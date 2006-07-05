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
                    aParam = itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().getParam(aNode.treeID(),aNode.paramDefID());
                    if (aParam != null && LofarUtils.isReference(aParam.limits)) {
                        aRefParam = itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().getParam(aNode);
                    }
                    setField(aParam,aRefParam);
                } else if (LofarUtils.keyName(aNode.name).equals("OLAP_HW")) {
                    //we need to get all the childs from this node also.    
                    Vector HWchilds = itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1);
            
                    // get all the params per child
                    Enumeration e1 = HWchilds.elements();
                    while( e1.hasMoreElements()  ) {
                       
                        jOTDBnode aHWNode = (jOTDBnode)e1.nextElement();
                        
                        // We need to keep all the params needed by this panel
                        if (aHWNode.leaf) {
                            aParam = itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().getParam(aHWNode.treeID(),aHWNode.paramDefID());
                            if (aParam != null && LofarUtils.isReference(aParam.limits)) {
                                aRefParam = itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().getParam(aHWNode);
                            }
                        }
                        setField(aParam,aRefParam);
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
        
        //  Fill in menu as in the example above        
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
        DelayCompensationPortsText.setText(itsDelayCompensationPorts.limits);
        InputClusterFENText.setText(itsInputClusterFEN.limits);
        InputBGLHostsText.setText(itsInputBGLHosts.limits);
        InputBGLPortsText.setText(itsInputBGLPorts.limits);
        StellaFENText.setText(itsStellaFEN.limits);
        StorageClusterFENText.setText(itsStorageClusterFEN.limits);
        BGLStorageHostsText.setText(itsBGLStorageHosts.limits);
        BGLStoragePortsText.setText(itsBGLStoragePorts.limits);
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
    private void setField(jOTDBparam aParam, jOTDBparam aRefParam) {
        // OLAP_HW settings
        if (aParam==null) {
            return;
        }
        if (LofarUtils.keyName(aParam.name).equals("AMCServerHost")) {
            if (aRefParam!=null) {
                AMCServerHostText.setToolTipText(aRefParam.description);
                AMCServerHostText.setText(aParam.limits + " : " + aRefParam.limits);
                AMCServerHostText.setEnabled(false);
                itsAMCServerHost=null;
            } else {
                AMCServerHostText.setToolTipText(aParam.description);
                AMCServerHostText.setText(aParam.limits);
                AMCServerHostText.setEnabled(true);
                itsAMCServerHost=aParam;
            }
        } else if (LofarUtils.keyName(aParam.name).equals("AMCServerPort")) {
            if (aRefParam!=null) {
                AMCServerPortText.setToolTipText(aRefParam.description);
                AMCServerPortText.setText(aParam.limits + " : " + aRefParam.limits);
                AMCServerPortText.setEnabled(false);
                itsAMCServerPort=null;
            } else {
                AMCServerPortText.setToolTipText(aParam.description);
                AMCServerPortText.setText(aParam.limits);
                AMCServerPortText.setEnabled(true);
                itsAMCServerPort=aParam;
            }
        } else if (LofarUtils.keyName(aParam.name).equals("DelayCompensationHost")) {
            if (aRefParam!=null) {
                DelayCompensationHostText.setToolTipText(aRefParam.description);
                DelayCompensationHostText.setText(aParam.limits + " : " + aRefParam.limits);
                DelayCompensationHostText.setEnabled(false);
                itsDelayCompensationHost=null;
            } else {
                DelayCompensationHostText.setToolTipText(aParam.description);
                DelayCompensationHostText.setText(aParam.limits);
                DelayCompensationHostText.setEnabled(true);
                itsDelayCompensationHost=aParam;
            }
        } else if (LofarUtils.keyName(aParam.name).equals("DelayCompensationPorts")) {
            if (aRefParam!=null) {
                DelayCompensationPortsText.setToolTipText(aRefParam.description);
                DelayCompensationPortsText.setText(aParam.limits + " : " + aRefParam.limits);
                DelayCompensationPortsText.setEnabled(false);
                itsDelayCompensationPorts=null;
            } else {
                DelayCompensationPortsText.setToolTipText(aParam.description);
                DelayCompensationPortsText.setText(aParam.limits);
                DelayCompensationPortsText.setEnabled(true);
                itsDelayCompensationPorts=aParam;
            }
        } else if (LofarUtils.keyName(aParam.name).equals("InputClusterFEN")) {
            if (aRefParam!=null) {
                InputClusterFENText.setToolTipText(aRefParam.description);
                InputClusterFENText.setText(aParam.limits + " : " + aRefParam.limits);
                InputClusterFENText.setEnabled(false);
                itsInputClusterFEN=null;
            } else {
                InputClusterFENText.setToolTipText(aParam.description);
                InputClusterFENText.setText(aParam.limits);
                InputClusterFENText.setEnabled(true);
                itsInputClusterFEN=aParam;
            }
        } else if (LofarUtils.keyName(aParam.name).equals("InputBGLHosts")) {
            if (aRefParam!=null) {
                InputBGLHostsText.setToolTipText(aRefParam.description);
                InputBGLHostsText.setText(aParam.limits + " : " + aRefParam.limits);
                InputBGLHostsText.setEnabled(false);
                itsInputBGLHosts=null;
            } else {
                InputBGLHostsText.setToolTipText(aParam.description);
                InputBGLHostsText.setText(aParam.limits);
                InputBGLHostsText.setEnabled(true);
                itsInputBGLHosts=aParam;
            }
        } else if (LofarUtils.keyName(aParam.name).equals("InputBGLPorts")) {
            if (aRefParam!=null) {
                InputBGLPortsText.setToolTipText(aRefParam.description);
                InputBGLPortsText.setText(aParam.limits + " : " + aRefParam.limits);
                InputBGLPortsText.setEnabled(false);
                itsInputBGLPorts=null;
            } else {
                InputBGLPortsText.setToolTipText(aParam.description);
                InputBGLPortsText.setText(aParam.limits);
                InputBGLPortsText.setEnabled(true);
                itsInputBGLPorts=aParam;
            }
        } else if (LofarUtils.keyName(aParam.name).equals("StellaFEN")) {
            if (aRefParam!=null) {
                StellaFENText.setToolTipText(aRefParam.description);
                StellaFENText.setText(aParam.limits + " : " + aRefParam.limits);
                StellaFENText.setEnabled(false);
                itsStellaFEN=null;
            } else {
                StellaFENText.setToolTipText(aParam.description);
                StellaFENText.setText(aParam.limits);
                StellaFENText.setEnabled(true);
                itsStellaFEN=aParam;
            }
        } else if (LofarUtils.keyName(aParam.name).equals("StorageClusterFEN")) {
            if (aRefParam!=null) {
                StorageClusterFENText.setToolTipText(aRefParam.description);
                StorageClusterFENText.setText(aParam.limits + " : " + aRefParam.limits);
                StorageClusterFENText.setEnabled(false);
                itsStorageClusterFEN=null;
            } else {
                StorageClusterFENText.setToolTipText(aParam.description);
                StorageClusterFENText.setText(aParam.limits);
                StorageClusterFENText.setEnabled(true);
                itsStorageClusterFEN=aParam;
            }
        } else if (LofarUtils.keyName(aParam.name).equals("BGLStorageHosts")) {
            if (aRefParam!=null) {
                BGLStorageHostsText.setToolTipText(aRefParam.description);
                BGLStorageHostsText.setText(aParam.limits + " : " + aRefParam.limits);
                BGLStorageHostsText.setEnabled(false);
                itsBGLStorageHosts=null;
            } else {
                BGLStorageHostsText.setToolTipText(aParam.description);
                BGLStorageHostsText.setText(aParam.limits);
                BGLStorageHostsText.setEnabled(true);
                itsBGLStorageHosts=aParam;
            }
        } else if (LofarUtils.keyName(aParam.name).equals("BGLStoragePorts")) {
            if (aRefParam!=null) {
                BGLStoragePortsText.setToolTipText(aRefParam.description);
                BGLStoragePortsText.setText(aParam.limits + " : " + aRefParam.limits);
                BGLStoragePortsText.setEnabled(false);
                itsBGLStoragePorts=null;
            } else {
                BGLStoragePortsText.setToolTipText(aParam.description);
                BGLStoragePortsText.setText(aParam.limits);
                BGLStoragePortsText.setEnabled(true);
                itsBGLStoragePorts=aParam;
            }
        } else if (LofarUtils.keyName(aParam.name).equals("partition")) {
            if (aRefParam!=null) {
                partitionText.setToolTipText(aRefParam.description);
                partitionText.setText(aParam.limits + " : " + aRefParam.limits);
                partitionText.setEnabled(false);
                itsPartition=null;
            } else {
                partitionText.setToolTipText(aParam.description);
                partitionText.setText(aParam.limits);
                partitionText.setEnabled(true);
                itsPartition=aParam;
            }

        // OLAP Specific parameters    
        
        } else if (LofarUtils.keyName(aParam.name).equals("samplesToIntegrate")) {
            if (aRefParam!=null) {
                samplesToIntegrateText.setToolTipText(aRefParam.description);
                samplesToIntegrateText.setText(aParam.limits + " : " + aRefParam.limits);
                samplesToIntegrateText.setEnabled(false);
                itsSamplesToIntegrate=null;
            } else {
                samplesToIntegrateText.setToolTipText(aParam.description);
                samplesToIntegrateText.setText(aParam.limits);
                samplesToIntegrateText.setEnabled(true);
                itsSamplesToIntegrate=aParam;
            }
        } else if (LofarUtils.keyName(aParam.name).equals("secondsToBuffer")) {
            if (aRefParam!=null) {
                secondsToBufferText.setToolTipText(aRefParam.description);
                secondsToBufferText.setText(aParam.limits + " : " + aRefParam.limits);
                secondsToBufferText.setEnabled(false);
                itsSecondsToBuffer=null;
            } else {
                secondsToBufferText.setToolTipText(aParam.description);
                secondsToBufferText.setText(aParam.limits);
                secondsToBufferText.setEnabled(true);
                itsSecondsToBuffer=aParam;
            }
        } else if (LofarUtils.keyName(aParam.name).equals("useAMCServer")) {
            if (aRefParam!=null) {
                useAMCServerCheckBox.setToolTipText(aRefParam.description);
                useAMCServerCheckBox.setSelected(LofarUtils.StringToBoolean(aRefParam.limits));
                useAMCServerCheckBox.setEnabled(false);
                itsUseAMCServer=null;
            } else {
                useAMCServerCheckBox.setToolTipText(aParam.description);
                useAMCServerCheckBox.setSelected(LofarUtils.StringToBoolean(aParam.limits));
                useAMCServerCheckBox.setEnabled(true);
                itsUseAMCServer=aParam;
            }
        } else if (LofarUtils.keyName(aParam.name).equals("nodesPerCell")) {
            if (aRefParam!=null) {
                nodesPerCellText.setToolTipText(aRefParam.description);
                nodesPerCellText.setText(aParam.limits + " : " + aRefParam.limits);
                nodesPerCellText.setEnabled(false);
                itsNodesPerCell=null;
            } else {
                nodesPerCellText.setToolTipText(aParam.description);
                nodesPerCellText.setText(aParam.limits);
                nodesPerCellText.setEnabled(true);
                itsNodesPerCell=aParam;
            }
        } else if (LofarUtils.keyName(aParam.name).equals("subbandsPerCell")) {
            if (aRefParam!=null) {
                subbandsPerCellText.setToolTipText(aRefParam.description);
                subbandsPerCellText.setText(aParam.limits + " : " + aRefParam.limits);
                subbandsPerCellText.setEnabled(false);
                itsSubbandsPerCell=null;
            } else {
                subbandsPerCellText.setToolTipText(aParam.description);
                subbandsPerCellText.setText(aParam.limits);
                subbandsPerCellText.setEnabled(true);
                itsSubbandsPerCell=aParam;
            }
        } else if (LofarUtils.keyName(aParam.name).equals("ppfTaps")) {
            if (aRefParam!=null) {
                ppfTapsText.setToolTipText(aRefParam.description);
                ppfTapsText.setText(aParam.limits + " : " + aRefParam.limits);
                ppfTapsText.setEnabled(false);
                itsPpfTaps=null;
            } else {
                ppfTapsText.setToolTipText(aParam.description);
                ppfTapsText.setText(aParam.limits);
                ppfTapsText.setEnabled(true);
                itsPpfTaps=aParam;
            }
            
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
        if (itsDelayCompensationPorts != null && !DelayCompensationPortsText.equals(itsDelayCompensationPorts.limits)) {
            itsDelayCompensationPorts.limits=DelayCompensationPortsText.getText();
            saveParam(itsDelayCompensationPorts);
        }
        if (itsInputClusterFEN != null && !InputClusterFENText.equals(itsInputClusterFEN.limits)) {
            itsInputClusterFEN.limits=InputClusterFENText.getText();
            saveParam(itsInputClusterFEN);
        }
        if (itsInputBGLHosts != null && !InputBGLHostsText.equals(itsInputBGLHosts.limits)) {
            itsInputBGLHosts.limits=InputBGLHostsText.getText();
            saveParam(itsInputBGLHosts);
        }
        if (itsInputBGLPorts != null && !InputBGLPortsText.equals(itsInputBGLPorts.limits)) {
            itsInputBGLPorts.limits=InputBGLPortsText.getText();
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
        if (itsBGLStorageHosts != null && !BGLStorageHostsText.equals(itsBGLStorageHosts.limits)) {
            itsBGLStorageHosts.limits=BGLStorageHostsText.getText();
            saveParam(itsBGLStorageHosts);
        }
        if (itsBGLStoragePorts != null && !BGLStoragePortsText.equals(itsBGLStoragePorts.limits)) {
            itsBGLStoragePorts.limits=BGLStoragePortsText.getText();
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

    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        jTabbedPane1 = new javax.swing.JTabbedPane();
        OLAPGenericPanel = new javax.swing.JPanel();
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
        jLabel1 = new javax.swing.JLabel();
        AMCServerHostText = new javax.swing.JTextField();
        jLabel2 = new javax.swing.JLabel();
        DelayCompensationHostText = new javax.swing.JTextField();
        jLabel3 = new javax.swing.JLabel();
        InputClusterFENText = new javax.swing.JTextField();
        jLabel4 = new javax.swing.JLabel();
        InputBGLHostsText = new javax.swing.JTextField();
        jLabel5 = new javax.swing.JLabel();
        StellaFENText = new javax.swing.JTextField();
        jLabel11 = new javax.swing.JLabel();
        BGLStorageHostsText = new javax.swing.JTextField();
        jLabel6 = new javax.swing.JLabel();
        partitionText = new javax.swing.JTextField();
        jLabel7 = new javax.swing.JLabel();
        AMCServerPortText = new javax.swing.JTextField();
        jLabel8 = new javax.swing.JLabel();
        DelayCompensationPortsText = new javax.swing.JTextField();
        jLabel9 = new javax.swing.JLabel();
        InputBGLPortsText = new javax.swing.JTextField();
        jLabel10 = new javax.swing.JLabel();
        BGLStoragePortsText = new javax.swing.JTextField();
        HardwareRevertButton = new javax.swing.JButton();
        jLabel12 = new javax.swing.JLabel();
        StorageClusterFENText = new javax.swing.JTextField();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        setLayout(new java.awt.BorderLayout());

        org.jdesktop.layout.GroupLayout OLAPGenericPanelLayout = new org.jdesktop.layout.GroupLayout(OLAPGenericPanel);
        OLAPGenericPanel.setLayout(OLAPGenericPanelLayout);
        OLAPGenericPanelLayout.setHorizontalGroup(
            OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(0, 838, Short.MAX_VALUE)
        );
        OLAPGenericPanelLayout.setVerticalGroup(
            OLAPGenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(0, 528, Short.MAX_VALUE)
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
                .addContainerGap(428, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
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

        jLabel1.setText("AMCServer Host:");

        AMCServerHostText.setToolTipText("Give Machine where AMC server runs (hostname or IP address)");

        jLabel2.setText("DelayCompensation Host:");

        DelayCompensationHostText.setToolTipText("Machine where DelayCompensation runs (Hostname or IP address)");

        jLabel3.setText("Input Cluster FEN :");

        InputClusterFENText.setToolTipText("Cluster for InputSection(Hostname or IP address)");

        jLabel4.setText("Input BGL Hosts :");

        InputBGLHostsText.setToolTipText("comma seperated list with all machinenames for InputSection");

        jLabel5.setText("Stella FEN:");

        StellaFENText.setToolTipText("Cluster for Storage (Hostname or IP address)");

        jLabel11.setText("BGL Storage Hosts :");

        BGLStorageHostsText.setToolTipText("comma seperated list with all machinenames for Storage");

        jLabel6.setText("Partition for BGL_Processing:");

        partitionText.setToolTipText("Partition on which BGL_Processing runs.");

        jLabel7.setText("AMC Server Port:");

        AMCServerPortText.setToolTipText("Port to reach AMC Server");

        jLabel8.setText("Delay Compensation Ports:");

        DelayCompensationPortsText.setToolTipText("Ports to use for Delay -> Input (Range like:  8000-8100 or 8000-  or 8000,8001,8003,8005) ");

        jLabel9.setText("Input BGL Ports:");

        InputBGLPortsText.setToolTipText("Ports to use for Input -> BGL_Proc (Range like:  8000-8100 or 8000-  or 8000,8001,8003,8005)");

        jLabel10.setText("BGL Storage Ports:");

        BGLStoragePortsText.setToolTipText("Ports to use for BGL_Proc -> Storage (Range like:  8000-8100 or 8000-  or 8000,8001,8003,8005)");

        HardwareRevertButton.setText("Revert");
        HardwareRevertButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                HardwareRevertButtonActionPerformed(evt);
            }
        });

        jLabel12.setText("StorageCluster FEN :");

        org.jdesktop.layout.GroupLayout OLAPHardwarePanelLayout = new org.jdesktop.layout.GroupLayout(OLAPHardwarePanel);
        OLAPHardwarePanel.setLayout(OLAPHardwarePanelLayout);
        OLAPHardwarePanelLayout.setHorizontalGroup(
            OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(OLAPHardwarePanelLayout.createSequentialGroup()
                .addContainerGap()
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                        .add(jLabel1)
                        .add(OLAPHardwarePanelLayout.createSequentialGroup()
                            .add(jLabel7)
                            .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED))
                        .add(OLAPHardwarePanelLayout.createSequentialGroup()
                            .add(jLabel8)
                            .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED))
                        .add(OLAPHardwarePanelLayout.createSequentialGroup()
                            .add(jLabel2)
                            .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED))
                        .add(OLAPHardwarePanelLayout.createSequentialGroup()
                            .add(jLabel3)
                            .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED))
                        .add(OLAPHardwarePanelLayout.createSequentialGroup()
                            .add(jLabel4)
                            .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED))
                        .add(OLAPHardwarePanelLayout.createSequentialGroup()
                            .add(jLabel9)
                            .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED))
                        .add(OLAPHardwarePanelLayout.createSequentialGroup()
                            .add(jLabel5)
                            .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED))
                        .add(OLAPHardwarePanelLayout.createSequentialGroup()
                            .add(jLabel12)
                            .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED))
                        .add(OLAPHardwarePanelLayout.createSequentialGroup()
                            .add(jLabel11, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 139, Short.MAX_VALUE)
                            .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)))
                    .add(OLAPHardwarePanelLayout.createSequentialGroup()
                        .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(jLabel6)
                            .add(jLabel10))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)))
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(OLAPHardwarePanelLayout.createSequentialGroup()
                        .add(AMCServerHostText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 200, Short.MAX_VALUE)
                        .add(742, 742, 742))
                    .add(OLAPHardwarePanelLayout.createSequentialGroup()
                        .add(AMCServerPortText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 200, Short.MAX_VALUE)
                        .add(742, 742, 742))
                    .add(OLAPHardwarePanelLayout.createSequentialGroup()
                        .add(DelayCompensationHostText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 200, Short.MAX_VALUE)
                        .add(742, 742, 742))
                    .add(OLAPHardwarePanelLayout.createSequentialGroup()
                        .add(DelayCompensationPortsText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 819, Short.MAX_VALUE)
                        .add(123, 123, 123))
                    .add(OLAPHardwarePanelLayout.createSequentialGroup()
                        .add(InputClusterFENText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 203, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .add(739, 739, 739))
                    .add(OLAPHardwarePanelLayout.createSequentialGroup()
                        .add(InputBGLHostsText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 819, Short.MAX_VALUE)
                        .add(123, 123, 123))
                    .add(OLAPHardwarePanelLayout.createSequentialGroup()
                        .add(InputBGLPortsText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 819, Short.MAX_VALUE)
                        .add(123, 123, 123))
                    .add(OLAPHardwarePanelLayout.createSequentialGroup()
                        .add(StellaFENText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 201, Short.MAX_VALUE)
                        .add(741, 741, 741))
                    .add(OLAPHardwarePanelLayout.createSequentialGroup()
                        .add(StorageClusterFENText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 201, Short.MAX_VALUE)
                        .add(741, 741, 741))
                    .add(OLAPHardwarePanelLayout.createSequentialGroup()
                        .add(partitionText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 206, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addContainerGap())
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, OLAPHardwarePanelLayout.createSequentialGroup()
                        .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                            .add(org.jdesktop.layout.GroupLayout.LEADING, BGLStoragePortsText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 819, Short.MAX_VALUE)
                            .add(BGLStorageHostsText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 819, Short.MAX_VALUE))
                        .add(123, 123, 123))))
            .add(OLAPHardwarePanelLayout.createSequentialGroup()
                .add(10, 10, 10)
                .add(HardwareRevertButton)
                .addContainerGap(1018, Short.MAX_VALUE))
        );
        OLAPHardwarePanelLayout.setVerticalGroup(
            OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(OLAPHardwarePanelLayout.createSequentialGroup()
                .add(32, 32, 32)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel1)
                    .add(AMCServerHostText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel7)
                    .add(AMCServerPortText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel2)
                    .add(DelayCompensationHostText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, jLabel8)
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, DelayCompensationPortsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel3)
                    .add(InputClusterFENText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel4)
                    .add(InputBGLHostsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel9)
                    .add(InputBGLPortsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel5)
                    .add(StellaFENText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel12)
                    .add(StorageClusterFENText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel11)
                    .add(BGLStorageHostsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel10)
                    .add(BGLStoragePortsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(jLabel6)
                    .add(partitionText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(39, 39, 39)
                .add(HardwareRevertButton)
                .addContainerGap(128, Short.MAX_VALUE))
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
    
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JTextField AMCServerHostText;
    private javax.swing.JTextField AMCServerPortText;
    private javax.swing.JTextField BGLStorageHostsText;
    private javax.swing.JTextField BGLStoragePortsText;
    private javax.swing.JTextField DelayCompensationHostText;
    private javax.swing.JTextField DelayCompensationPortsText;
    private javax.swing.JButton HardwareRevertButton;
    private javax.swing.JTextField InputBGLHostsText;
    private javax.swing.JTextField InputBGLPortsText;
    private javax.swing.JTextField InputClusterFENText;
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
    private javax.swing.JLabel UseAMCLabel;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel10;
    private javax.swing.JLabel jLabel11;
    private javax.swing.JLabel jLabel12;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JLabel jLabel8;
    private javax.swing.JLabel jLabel9;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.JTextField nodesPerCellText;
    private javax.swing.JTextField partitionText;
    private javax.swing.JTextField ppfTapsText;
    private javax.swing.JTextField samplesToIntegrateText;
    private javax.swing.JTextField secondsToBufferText;
    private javax.swing.JTextField subbandsPerCellText;
    private javax.swing.JCheckBox useAMCServerCheckBox;
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
