/*
 * TBBConfigPanel.java
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
import java.util.ArrayList;
import java.util.BitSet;
import java.util.Enumeration;
import java.util.ArrayList;
import javax.swing.JFileChooser;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.ListSelectionModel;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBtree;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.tablemodels.TBBConfigurationTableModel;
import org.apache.log4j.Logger;


/**
 * Panel for TBB specific configuration
 *
 * @author  Coolen
 *
 * Created on 21 november 2007, 10:53
 *
 * @version $Id$
 */


/**
 *
 * @author  Coolen
 */
public class TBBConfigPanel extends javax.swing.JPanel implements IViewPanel {
    
    static Logger logger = Logger.getLogger(TBBConfigPanel.class);
    static String name = "TBBConfigPanel";
    
    /** Creates new form TBBConfigPanel */
    public TBBConfigPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode=aNode;
        initialize();
        initPanel();
    }
    
    /** Creates new form TBBConfigPanel */
    public TBBConfigPanel() {
        initComponents();
        initialize();
    }
    
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
        } else {
            logger.error("No Mainframe supplied");
        }
    }
    
    public String getShortName() {
        return name;
    }
    
    public void setContent(Object anObject) {
        itsNode=(jOTDBnode)anObject;

        //fire up filling for TBBControlPanel
        this.tbbControlPanel.setMainFrame(this.itsMainFrame);
        this.tbbControlPanel.setContent(this.itsNode);
        jOTDBparam aParam=null;
        isInitialized=false;
        
        itsMainFrame.setHourglassCursor();
        try {
            
            //we need to get all the childs from this node.
            ArrayList<jOTDBnode> childs = new ArrayList(OtdbRmi.getRemoteMaintenance().getItemList(itsNode.treeID(), itsNode.nodeID(), 1));
            
            for (jOTDBnode aNode: childs) {
                aParam=null;
                
                String parentName=LofarUtils.keyName(aNode.name);
                
                // We need to keep all the nodes needed by this panel
                if (aNode.leaf) {
                    //we need to get all the childs from the following nodes as well.
                }else if (parentName.contains("TBBsetting")) {
                    // we also need to set the defaults in the inputfields
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }
            }
        } catch (RemoteException ex) {
            itsMainFrame.setNormalCursor();
            String aS="Error during getComponentParam: "+ ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            return;
        }
        
        initPanel();
        itsMainFrame.setNormalCursor();
    }
    
    public boolean isSingleton() {
        return false;
    }
    
    public JPanel getInstance() {
        return new TBBConfigPanel();
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
        switch (itsTreeType) {
            case "VHtree":
            //  Fill in menu as in the example above
            aMenuItem=new JMenuItem("Create ParSet File");
            aMenuItem.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    popupMenuHandler(evt);
                }
            });
            aMenuItem.setActionCommand("Create ParSet File");
            aPopupMenu.add(aMenuItem);
            
                aMenuItem=new JMenuItem("Create ParSetMeta File");
                aMenuItem.addActionListener(new java.awt.event.ActionListener() {
                    public void actionPerformed(java.awt.event.ActionEvent evt) {
                        popupMenuHandler(evt);
                    }
                });
                aMenuItem.setActionCommand("Create ParSetMeta File");
                aPopupMenu.add(aMenuItem);
            // For template trees
                break;
            case "VItemplate":
                break;
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
        switch (evt.getActionCommand()) {
            case "Create ParSet File":
                {
            logger.trace("Create ParSet File");
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
                            OtdbRmi.getRemoteMaintenance().exportTree(aTreeID,itsNode.nodeID(),aRemoteFileName);
                    
                    //obtain the remote file
                    byte[] dldata = OtdbRmi.getRemoteFileTrans().downloadFile(aRemoteFileName);
                            try (BufferedOutputStream output = new BufferedOutputStream(new FileOutputStream(aFile))) {
                    output.write(dldata,0,dldata.length);
                    output.flush();
                            }
                    logger.trace("File written to: " + aFile.getPath());
                } catch (RemoteException ex) {
                    String aS="exportTree failed : " + ex;
                    logger.error(aS);
                    LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                } catch (FileNotFoundException ex) {
                    String aS="Error during newPICTree creation: "+ ex;
                    logger.error(aS);
                    LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                } catch (IOException ex) {
                    String aS="Error during newPICTree creation: "+ ex;
                    logger.error(aS);
                    LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                }
            }
                    break;
        }
            case "Create ParSetMeta File":
                {
                    logger.trace("Create ParSetMeta File");
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
                            String aRemoteFileName="/tmp/"+aTreeID+"-"+itsNode.name+"_"+itsMainFrame.getUserAccount().getUserName()+".ParSetMeta";
                            
                            // write the parset
                            OtdbRmi.getRemoteMaintenance().exportResultTree(aTreeID,itsNode.nodeID(),aRemoteFileName);
                            
                            //obtain the remote file
                            byte[] dldata = OtdbRmi.getRemoteFileTrans().downloadFile(aRemoteFileName);
                    try (BufferedOutputStream output = new BufferedOutputStream(new FileOutputStream(aFile))) {
                        output.write(dldata,0,dldata.length);
                        output.flush();
                    }
                            logger.trace("File written to: " + aFile.getPath());
                        } catch (RemoteException ex) {
                            String aS="exportResultTree failed : " + ex;
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        } catch (FileNotFoundException ex) {
                            String aS="Error during newPICTree creation: "+ ex;
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        } catch (IOException ex) {
                            String aS="Error during newPICTree creation: "+ ex;
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        }
                    }
                    break;
                }
        }
    }
    
    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
        buttonPanel1.setButtonEnabled("Restore",enabled);
        buttonPanel1.setButtonEnabled("Apply",enabled);
        editConfigButton.setEnabled(enabled);
        deleteConfigButton.setEnabled(enabled);
        addConfigButton.setEnabled(enabled);
        cancelEditButton.setEnabled(enabled);
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
        buttonPanel1.setButtonVisible("Restore",visible);
        buttonPanel1.setButtonVisible("Apply",visible);
        editConfigButton.setVisible(visible);
        deleteConfigButton.setVisible(visible);
        addConfigButton.setVisible(visible);
        cancelEditButton.setVisible(visible);
    }
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
        inputOperatingMode.setEnabled(enabled);
        inputBaselevel.setEnabled(enabled);
        inputStartlevel.setEnabled(enabled);
        inputStoplevel.setEnabled(enabled);
        inputFilter.setEnabled(enabled);
        inputWindow.setEnabled(enabled);
        inputFilter0Coeff0.setEnabled(enabled);
        inputFilter0Coeff1.setEnabled(enabled);
        inputFilter0Coeff2.setEnabled(enabled);
        inputFilter0Coeff3.setEnabled(enabled);
        inputFilter1Coeff0.setEnabled(enabled);
        inputFilter1Coeff1.setEnabled(enabled);
        inputFilter1Coeff2.setEnabled(enabled);
        inputFilter1Coeff3.setEnabled(enabled);
        inputRCUs.setEnabled(enabled);
        inputSubbandList.setEnabled(enabled);
    }
    
    private void initialize() {
        buttonPanel1.addButton("Restore");
        buttonPanel1.setButtonIcon("Restore",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_undo.png")));
        buttonPanel1.setButtonToolTip("Restore","Restores the complete table to it's initial state");
        buttonPanel1.addButton("Apply");
        buttonPanel1.setButtonIcon("Apply",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_apply.png")));
        buttonPanel1.setButtonToolTip("Apply","Deleted all old TBBsetting instances from the Database and will write all new instances");
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
                String aS="TBBConfigPanel: Error getting treeInfo/treetype" + ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                itsTreeType="";
            }         } else {
            logger.error("ERROR:  no node given");
            }
        
        itsTBBConfigurationTableModel = new TBBConfigurationTableModel();
        TBBConfigurationPanel.setTableModel(itsTBBConfigurationTableModel);
        TBBConfigurationPanel.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        TBBConfigurationPanel.setColumnSize("mode",8);
        TBBConfigurationPanel.setColumnSize("trigger",8);
        TBBConfigurationPanel.setColumnSize("base",8);
        TBBConfigurationPanel.setColumnSize("start",8);
        TBBConfigurationPanel.setColumnSize("stop",8);
        TBBConfigurationPanel.setColumnSize("filter",8);
        TBBConfigurationPanel.setColumnSize("window",12);
        TBBConfigurationPanel.setColumnSize("F0C0",10);
        TBBConfigurationPanel.setColumnSize("F0C1",10);
        TBBConfigurationPanel.setColumnSize("F0C2",10);
        TBBConfigurationPanel.setColumnSize("F0C3",10);
        TBBConfigurationPanel.setColumnSize("F1C0",10);
        TBBConfigurationPanel.setColumnSize("F1C1",10);
        TBBConfigurationPanel.setColumnSize("F1C2",10);
        TBBConfigurationPanel.setColumnSize("F1C3",10);
        TBBConfigurationPanel.setColumnSize("RCUs",75);
        TBBConfigurationPanel.setColumnSize("Subbands",75);
        
        // set defaults
        // create initial RCUBitset
        // create initial table
        restore();

        // if VHTree disable buttons, VHTree is view only mode, no changes possible anymore...
        if (itsTreeType.equals("VHtree")) {
            this.setButtonsVisible(false);
            this.setAllEnabled(false);
        }
    }

    /**
     * Helper method that retrieves the child nodes for a given jOTDBnode,
     * and triggers setField() accordingly.
     * @param aNode the node to retrieve and display child data of.
     */
    private void retrieveAndDisplayChildDataForNode(jOTDBnode aNode){
        try {
            jOTDBparam aParam=null;
            // add original top node to list to be able to delete/change it later
            itsTBBsettings.add(aNode);
            ArrayList<jOTDBnode> HWchilds = new ArrayList(OtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1));
            // get all the params per child

            for (jOTDBnode aHWNode: HWchilds) {
                aParam=null;
                // We need to keep all the params needed by this panel
                if (aHWNode.leaf) {
                    aParam = OtdbRmi.getRemoteMaintenance().getParam(aHWNode);
                }
                setField(aNode,aParam,aHWNode); 
            }
            //first element should have been the default TBBsettings, so we can assume init of components should have been done
            isInitialized=true;
        } catch (RemoteException ex) {
            String aS="Error during retrieveAndDisplayChildDataForNode: "+ ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            return;
        }
    }
    
    /**
     * Fills the StringArrayLists with the values from the Database
     * Also does some base GUI settings on the Input fields
     * @param parent the parent node of the node to be displayed
     * @param aParam the parameter of the node to be displayed if applicable
     * @param aNode  the node to be displayed
     */
    private void setField(jOTDBnode parent,jOTDBparam aParam, jOTDBnode aNode) {
        
        String aKeyName = LofarUtils.keyName(aNode.name);
        String parentName = LofarUtils.keyName(String.valueOf(parent.name));
        boolean isRef = LofarUtils.isReference(aNode.limits);
        
        
        // Generic TBB
        logger.debug("setField for: "+ parentName + " - " + aNode.name);
        try {
            if (OtdbRmi.getRemoteTypes().getParamType(aParam.type).substring(0,1).equals("p")) {
               // Have to get new param because we need the unresolved limits field.
               aParam = OtdbRmi.getRemoteMaintenance().getParam(aNode.treeID(),aNode.paramDefID());                
            }
        } catch (RemoteException ex) {
            String aS="Error during getParam: "+ ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
        }
        switch (aKeyName) {
            case "operatingMode":
            // OperatingMode
            if (!isInitialized) {
               inputOperatingMode.setToolTipText(aParam.description);
               LofarUtils.setPopupComboChoices(inputOperatingMode,aParam.limits);
            }
            itsOperatingModes.add(aNode.limits);
                break;
            case "triggerMode":
            // TriggerMode
            if (!isInitialized) {
               inputTriggerMode.setToolTipText(aParam.description);
               LofarUtils.setPopupComboChoices(inputTriggerMode,aParam.limits);
            }
            itsTriggerModes.add(aNode.limits);
                break;
            case "baselevel":
            // Baselevel
            if (!isInitialized) {
               inputBaselevel.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsBaselevels.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsBaselevels.add(aNode.limits);
            }
                break;
            case "startlevel":
            // startlevel
            if (!isInitialized) {
               inputStartlevel.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsStartlevels.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsStartlevels.add(aNode.limits);
            }
                break;
            case "stoplevel":
            // stoplevel
            if (!isInitialized) {
               inputStoplevel.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsStoplevels.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsStoplevels.add(aNode.limits);
            }
                break;
            case "filter":
            // filter
            if (!isInitialized) {
               inputFilter.setToolTipText(aParam.description);
               LofarUtils.setPopupComboChoices(inputFilter,aParam.limits);
            }
            itsFilters.add(aNode.limits);
                break;
            case "window":
            // window
            if (!isInitialized) {
               inputWindow.setToolTipText(aParam.description);
               LofarUtils.setPopupComboChoices(inputWindow,aParam.limits);
            }
            itsWindows.add(aNode.limits);
                break;
            case "filter0_coeff0":
            // Coeff0
            if (!isInitialized) {
               inputFilter0Coeff0.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsFilter0Coeff0s.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsFilter0Coeff0s.add(aNode.limits);
            }
                break;
            case "filter0_coeff1":
            // Coeff1
            if (!isInitialized) {
               inputFilter0Coeff1.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsFilter0Coeff1s.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsFilter0Coeff1s.add(aNode.limits);
            }
                break;
            case "filter0_coeff2":
            // Coeff2
            if (!isInitialized) {
               inputFilter0Coeff2.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsFilter0Coeff2s.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsFilter0Coeff2s.add(aNode.limits);
            }
                break;
            case "filter0_coeff3":
            // Coeff3
            if (!isInitialized) {
               inputFilter0Coeff3.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsFilter0Coeff3s.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsFilter0Coeff3s.add(aNode.limits);
            }
                break;
            case "filter1_coeff0":
            // Coeff0
            if (!isInitialized) {
               inputFilter1Coeff0.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsFilter1Coeff0s.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsFilter1Coeff0s.add(aNode.limits);
            }
                break;
            case "filter1_coeff1":
            // Coeff1
            if (!isInitialized) {
               inputFilter1Coeff1.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsFilter1Coeff1s.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsFilter1Coeff1s.add(aNode.limits);
            }
                break;
            case "filter1_coeff2":
            // Coeff2
            if (!isInitialized) {
               inputFilter1Coeff2.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsFilter1Coeff2s.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsFilter1Coeff2s.add(aNode.limits);
            }
                break;
            case "filter1_coeff3":
            // Coeff3
            if (!isInitialized) {
               inputFilter1Coeff3.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsFilter1Coeff3s.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsFilter1Coeff3s.add(aNode.limits);
            }
                break;
            case "RCUs":
            // RCUs
            if (!isInitialized) {
               inputRCUs.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsRCUs.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsRCUs.add(aNode.limits);
            }
                break;
            case "subbandList":
            // SubbandList
            if (!isInitialized) {
               inputSubbandList.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsSubbandList.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsSubbandList.add(aNode.limits);
            }
                break;
        }
    }
    
    /** Restores all settings back to default input screens and the table
     */
    private void restore() {
        if (!itsTreeType.equals("VHtree")) {
            // set the default input fields back
            setDefaultInput();

            // create original RCU Bitset
            fillRCUBitset();
        }

        // set table back to initial values
        itsTBBConfigurationTableModel.fillTable(itsTreeType,itsOperatingModes,itsTriggerModes,itsBaselevels,itsStartlevels,itsStoplevels,itsFilters,itsWindows,itsFilter0Coeff0s,itsFilter0Coeff1s,itsFilter0Coeff2s,itsFilter0Coeff3s,itsFilter1Coeff0s,itsFilter1Coeff1s,itsFilter1Coeff2s,itsFilter1Coeff3s,itsRCUs,itsSubbandList);
        
        // also restore TBBControlPanel
        tbbControlPanel.restore();
    }
    
    
    private void setDefaultInput() {
        
        // if no entries in lists we can exit again
        if (itsOperatingModes.isEmpty()) {
            logger.error("ERROR setInputDefaults,  null entry found");
            return;
        }
        
        // defaultSettings Index
        int index=0;
          
        // OperatingMode
        if (!itsOperatingModes.get(index).equals("")) {
           inputOperatingMode.setSelectedItem(itsOperatingModes.get(index));
        }
        // TriggerMode
        if (!itsTriggerModes.get(index).equals("")) {
           inputTriggerMode.setSelectedItem(itsTriggerModes.get(index));
        }
        // Baselevel
         inputBaselevel.setText(itsBaselevels.get(index));
            
         // Startlevel
         inputStartlevel.setText(itsStartlevels.get(index));
 
         // Stoplevel
         inputStoplevel.setText(itsStoplevels.get(index));
            
         // Filter
         if (!itsFilters.get(index).equals("")) {
             inputFilter.setSelectedItem(itsFilters.get(index));
         }

         // Window
         if (!itsWindows.get(index).equals("")) {
            inputWindow.setSelectedItem(itsWindows.get(index));
         }
            
         // Coeff0
         inputFilter0Coeff0.setText(itsFilter0Coeff0s.get(index));

         // Coeff1
         inputFilter0Coeff1.setText(itsFilter0Coeff1s.get(index));

         // Coeff2
         inputFilter0Coeff2.setText(itsFilter0Coeff2s.get(index));

         // Coeff3
         inputFilter0Coeff3.setText(itsFilter0Coeff3s.get(index));

         // Coeff0
         inputFilter1Coeff0.setText(itsFilter1Coeff0s.get(index));

         // Coeff1
         inputFilter1Coeff1.setText(itsFilter1Coeff1s.get(index));

         // Coeff2
         inputFilter1Coeff2.setText(itsFilter1Coeff2s.get(index));

         // Coeff3
         inputFilter1Coeff3.setText(itsFilter1Coeff3s.get(index));
         // RCUs
         inputRCUs.setText(itsRCUs.get(index));
        
         // subbandList
         inputSubbandList.setText(itsSubbandList.get(index));
    }
    
    /** fill the RCU bitset to see what RCU's have been set. To be able to determine later if a given RCU is indeed free.
     */
    private void fillRCUBitset() {
        itsUsedRCUList.clear();
        for (int i=1;i<itsRCUs.size();i++) {
            BitSet aNewBitSet=rcuToBitSet(LofarUtils.expandedArrayString(itsRCUs.get(i)));
            
            // check if no duplication between the two bitsets
            if (itsUsedRCUList.intersects(aNewBitSet)) {
                String errorMsg = "ERROR:  This RCUList has RCUs defined that are allready used in a prior TBBsetting!!!!!  TBBsettingNr: "+i;
                JOptionPane.showMessageDialog(this,errorMsg,"RCUError",JOptionPane.ERROR_MESSAGE);
                logger.error(errorMsg );
                return;
            }
            
            // No intersection, both bitsets can be AND
            itsUsedRCUList.or(aNewBitSet);
        }
    }
    
    private BitSet rcuToBitSet(String aS) {
        
        BitSet aBitSet = new BitSet(192);
        
        if (aS==null || aS.length() <= 2) {
            return aBitSet;
        }
        //remove [] from string
        String rcus = aS.substring(aS.indexOf("[")+1,aS.lastIndexOf("]"));
        
        logger.debug("rcus found: "+rcus);
        // split into seperate rcu nr's
        String[] rculist=rcus.split("[,]");
        
        //fill bitset
        
        for (int j=0; j< rculist.length;j++) {
            int val;
            try {
                val = Integer.parseInt(rculist[j]);
                logger.debug("Setting bit "+val);
                aBitSet.set(val);
            } catch (NumberFormatException ex) {
                logger.error("Error converting rcu numbers");
                
            }
        }
        return aBitSet;
    }
    
    
    /** checks the given input
     */
    private boolean checkInput() {
        itsSavedRCUList =(BitSet)itsUsedRCUList.clone();
        boolean error=false;
        String errorMsg = "The following inputs are wrong: \n";
        
        // baselevel
        if (Integer.valueOf(inputBaselevel.getText())<1 || Integer.valueOf(inputBaselevel.getText())>4095) {
            error=true;
            errorMsg += "Baselevel \n";
        }
        
        // startlevel
        if (Integer.valueOf(inputStartlevel.getText())<1 || Integer.valueOf(inputStartlevel.getText())>15) {
            error=true;
            errorMsg += "Startlevel \n";
        }
        
        // stoplevel
        if (Integer.valueOf(inputStoplevel.getText())<1 || Integer.valueOf(inputStoplevel.getText())>15) {
            error=true;
            errorMsg += "Stoplevel \n";
        }
        
        
        // Coeff0
        if (Integer.valueOf(inputFilter0Coeff0.getText())<0 || Integer.valueOf(inputFilter0Coeff0.getText())>65535) {
            error=true;
            errorMsg += "Coeff0 \n";
        }
        
        // Coeff1
        if (Integer.valueOf(inputFilter0Coeff1.getText())<0 || Integer.valueOf(inputFilter0Coeff1.getText())>65535) {
            error=true;
            errorMsg += "Coeff1 \n";
        }
        
        // Coeff2
        if (Integer.valueOf(inputFilter0Coeff2.getText())<0 || Integer.valueOf(inputFilter0Coeff2.getText())>65535) {
            error=true;
            errorMsg += "Coeff2 \n";
        }
        
        // Coeff3
        if (Integer.valueOf(inputFilter0Coeff3.getText())<0 || Integer.valueOf(inputFilter0Coeff3.getText())>65535) {
            error=true;
            errorMsg += "Coeff3 \n";
        }
        
        
        if (!checkRCUs() ) {
            error=true;
            errorMsg += "(some) RCUs allready used earlier or invalid RCU inputString\n";
        }
        
        // subbandList
        if (inputSubbandList.getText().length() <=2) {
            error=true;
            errorMsg += "subbandList \n";
        }

        if (error) {
            // set back old daved RCUs
            itsUsedRCUList=(BitSet)itsSavedRCUList.clone();
            JOptionPane.showMessageDialog(this,errorMsg,"InputError",JOptionPane.ERROR_MESSAGE);
            return false;
        }
        
        return true;
    }
    
    // check if RCUs are spelled correctly and if RCU's are not used by other entries.'
    private boolean checkRCUs(){
        if (inputRCUs.getText().length() <=2) {
            return false;
        }
        
        BitSet aBitSet = this.rcuToBitSet(LofarUtils.expandedArrayString(inputRCUs.getText()));
        if(itsUsedRCUList.intersects(aBitSet)) {
            return false;
        } else {
            itsUsedRCUList.or(aBitSet);
        }
        return true;
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
            logger.error("Error: saveNode failed : " + ex);
        }
    }
    
    private void addConfig() {
        
        BitSet savedBitSet=itsUsedRCUList;
        
        BitSet oldRCUs = rcuToBitSet(LofarUtils.expandedArrayString(itsSavedRCUs));
        
        if (editting) {
            // remove old RCU's from list
            itsUsedRCUList.xor(oldRCUs);
        }
        
        if (!checkInput() ) {
            // if fault then reset old RCUs again
            if (editting) {
                itsUsedRCUList=savedBitSet;
            }
            return;
        }
        
        // check if we are editting an entry or adding a new entry
        String[] newRow = {inputOperatingMode.getSelectedItem().toString(),
                           inputTriggerMode.getSelectedItem().toString(),
                           inputBaselevel.getText(),
                           inputStartlevel.getText(),
                           inputStoplevel.getText(),
                           inputFilter.getSelectedItem().toString(),
                           inputWindow.getSelectedItem().toString(),
                           inputFilter0Coeff0.getText(),
                           inputFilter0Coeff1.getText(),
                           inputFilter0Coeff2.getText(),
                           inputFilter0Coeff3.getText(),
                           inputFilter1Coeff0.getText(),
                           inputFilter1Coeff1.getText(),
                           inputFilter1Coeff2.getText(),
                           inputFilter1Coeff3.getText(),
                           inputRCUs.getText(),
                           inputSubbandList.getText()
        };
        
        if (editting) {
            itsTBBConfigurationTableModel.updateRow(newRow,itsSelectedRow);
        } else {            
           itsTBBConfigurationTableModel.addRow(newRow);
        }
        
        // return to standard state
        if (editting) {
            //fill tabel with default Settings again
            setDefaultInput();
            // set editting = false
            editting=false;
        
            itsSavedRCUs = "";
        
            this.editConfigButton.setEnabled(false);
            this.deleteConfigButton.setEnabled(false);
        
            // set back to default Button visible
            addConfigButton.setText("Add Configuration");
            addConfigButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_new.png")));
            cancelEditButton.setVisible(false);
        } else {
            setDefaultInput();
        }
        
    }
  
    /** delete all old TBBsettings, and save new table back to the database
     */
    private boolean save() {

        if (itsTBBConfigurationTableModel.changed()) {
            int i=0;
            //delete all TBBsettings from the table (excluding the Default one);
        
            // Keep the 1st one, it's the default TBBsetting
            try {
                for (i=1; i< itsTBBsettings.size(); i++) {
                    OtdbRmi.getRemoteMaintenance().deleteNode(itsTBBsettings.get(i));
                }
            } catch (RemoteException ex) {
                logger.error("Error during deletion of defaultNode: "+ex);
                return false;
            }
        
            // now that all Nodes are deleted we should collect the tables input and create new TBBsettings to save to the database.
        
            itsTBBConfigurationTableModel.getTable(itsOperatingModes,itsTriggerModes,itsBaselevels,itsStartlevels,itsStoplevels,itsFilters,itsWindows,itsFilter0Coeff0s,itsFilter0Coeff1s,itsFilter0Coeff2s,itsFilter0Coeff3s,itsFilter1Coeff0s,itsFilter1Coeff1s,itsFilter1Coeff2s,itsFilter1Coeff3s,itsRCUs,itsSubbandList);
            itsTBBsettings.clear();
        
            try {
                // for all elements
                for (i=1; i < itsFilter0Coeff0s.size();i++) {
        
                    // make a dupnode from the default node, give it the next number in the count,get the elements and fill all values from the elements
                    // with the values from the set fields and save the elements again
                    //
                    // Duplicates the given node (and its parameters and children)
                    int aN = OtdbRmi.getRemoteMaintenance().dupNode(itsNode.treeID(),itsDefaultNode.nodeID(),(short)(i-1));
                    if (aN <= 0) {
                        logger.error("Something went wrong with duplicating tree no ("+i+") will try to save remainder");
                    } else {
                        // we got a new duplicate whos children need to be filled with the settings from the panel.
                        jOTDBnode aNode = OtdbRmi.getRemoteMaintenance().getNode(itsNode.treeID(),aN);
                        // store new duplicate in itsTBBsettings.
                        itsTBBsettings.add(aNode);

                        ArrayList<jOTDBnode> HWchilds = new ArrayList(OtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1));
                        // get all the params per child
                        for (jOTDBnode aHWNode: HWchilds) {
                            String aKeyName = LofarUtils.keyName(aHWNode.name);
                            switch (aKeyName) {
                                case "operatingMode":
                                aHWNode.limits=itsOperatingModes.get(i);
                                    break;
                                case "triggerMode":
                                aHWNode.limits=itsTriggerModes.get(i);
                                    break;
                                case "baselevel":
                                aHWNode.limits=itsBaselevels.get(i);
                                    break;
                                case "startlevel":
                                aHWNode.limits=itsStartlevels.get(i);
                                    break;
                                case "stoplevel":
                                aHWNode.limits=itsStoplevels.get(i);
                                    break;
                                case "filter":
                                aHWNode.limits=itsFilters.get(i);
                                    break;
                                case "window":
                                aHWNode.limits=itsWindows.get(i);
                                    break;
                                case "filter0_coeff0":
                                aHWNode.limits=itsFilter0Coeff0s.get(i);
                                    break;
                                case "filter0_coeff1":
                                aHWNode.limits=itsFilter0Coeff1s.get(i);
                                    break;
                                case "filter0_coeff2":
                                aHWNode.limits=itsFilter0Coeff2s.get(i);
                                    break;
                                case "filter0_coeff3":
                                aHWNode.limits=itsFilter0Coeff3s.get(i);
                                    break;
                                case "filter1_coeff0":
                                aHWNode.limits=itsFilter1Coeff0s.get(i);
                                    break;
                                case "filter1_coeff1":
                                aHWNode.limits=itsFilter1Coeff1s.get(i);
                                    break;
                                case "filter1_coeff2":
                                aHWNode.limits=itsFilter1Coeff2s.get(i);
                                    break;
                                case "filter1_coeff3":
                                aHWNode.limits=itsFilter1Coeff3s.get(i);
                                    break;
                                case "RCUs":
                                aHWNode.limits=itsRCUs.get(i);
                                    break;
                                case "subbandList":
                                aHWNode.limits=itsSubbandList.get(i);
                                    break;
                            }
                            saveNode(aHWNode);
                        }
                    }
                }

            
                // store new number of instances in baseSetting
                itsDefaultNode.instances=(short)(itsFilter0Coeff0s.size()-1); // - default at 0
                saveNode(itsDefaultNode);

                // reset all buttons, flags and tables to initial start position. So the panel now reflects the new, saved situation
                initPanel();
            
                itsMainFrame.setChanged("Template_Maintenance("+itsNode.treeID()+")" ,true);
                itsMainFrame.checkChanged("Template_Maintenance("+itsNode.treeID()+")");
            
            } catch (RemoteException ex) {
                logger.error("Error during duplication and save : " + ex);
                return false;
            }
        }
        return true;
    }
    
    /** set Input line to chosen row from table
     *
     * @param   selection   contains a String array with all settings
     */
    private void setInput(String[] selection) {
        // OperatingMode
        if (!selection[0].equals("")) {
           inputOperatingMode.setSelectedItem(selection[0]);
        }
        // TriggerMode
        inputTriggerMode.setSelectedItem(selection[1]);

        // Baselevel
        inputBaselevel.setText(selection[2]);
            
         // Startlevel
         inputStartlevel.setText(selection[3]);
 
         // Stoplevel
         inputStoplevel.setText(selection[4]);
            
         // Filter
         inputFilter.setSelectedItem(selection[5]);

         // Window
         if (!(selection[6]).equals("")) {
            inputWindow.setSelectedItem(selection[6]);
         }
            
         // Coeff0
         inputFilter0Coeff0.setText(selection[7]);

         // Coeff1
         inputFilter0Coeff1.setText(selection[8]);

         // Coeff2
         inputFilter0Coeff2.setText(selection[9]);

         // Coeff3
         inputFilter0Coeff3.setText(selection[10]);

         // Coeff0
         inputFilter1Coeff0.setText(selection[11]);

         // Coeff1
         inputFilter1Coeff1.setText(selection[12]);

         // Coeff2
         inputFilter1Coeff2.setText(selection[13]);

         // Coeff3
         inputFilter1Coeff3.setText(selection[14]);
         // RCUs
         inputRCUs.setText(selection[15]);

         // subbandList
         inputSubbandList.setText(selection[16]);
    }
        
    /** Edit chosen configuration.
     *
     * Keep in mind that the available rcu's in the old config chould be saved , they need to be
     * removed from the total list before adding the new ones.  This can best be done based on the editting flag in the
     * same method that checks the validity of the input. Because the original needs to stay intact untill the real data
     * is confirmed 
     */
    private void editConfig() {
        itsSelectedRow = TBBConfigurationPanel.getSelectedRow();
        String [] selection = itsTBBConfigurationTableModel.getSelection(itsSelectedRow);
        // if no row is selected, nothing to be done
        if (selection == null || selection[0].equals("")) {
            return;
        }
        
        // keep old RCUs
        itsSavedRCUs=selection[15];
        
        //fill table with entry to be editted
        // 0 in lists represent default
        setInput(selection);
        // set editting = true
        editting=true;
        this.editConfigButton.setEnabled(false);
        this.deleteConfigButton.setEnabled(false);
        
        // set back to default Button visible
        addConfigButton.setText("Apply Changes");
        buttonPanel1.setButtonIcon("Apply Changes",new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_save.png")));
        cancelEditButton.setVisible(true);
        
    }
    
    private void deleteConfig() {
        int row = TBBConfigurationPanel.getSelectedRow();
        // if removed then the old RCU's should be removed form the checklist also
        String oldRCUs = itsTBBConfigurationTableModel.getSelection(row)[15];
        BitSet rcuSet = rcuToBitSet(LofarUtils.expandedArrayString(oldRCUs));
        
        if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this configuration ?","Delete Configuration",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
            if (row > -1) {
                itsTBBConfigurationTableModel.removeRow(row);
                itsUsedRCUList.xor(rcuSet);
                // No selection anymore after delete, so buttons disabled again
                this.editConfigButton.setEnabled(false);
                this.deleteConfigButton.setEnabled(false);


            }
        }
        
    }
    
    
    
    
    
    
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jPanel1 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        jTabbedPane1 = new javax.swing.JTabbedPane();
        TBBSettingsPanel = new javax.swing.JPanel();
        labelOperatingMode = new javax.swing.JLabel();
        labelBaselevel = new javax.swing.JLabel();
        labelStartlevel = new javax.swing.JLabel();
        labelStoplevel = new javax.swing.JLabel();
        labelFilter = new javax.swing.JLabel();
        labelWindow = new javax.swing.JLabel();
        labelCoeff0 = new javax.swing.JLabel();
        labelCoeff1 = new javax.swing.JLabel();
        labelCoeff2 = new javax.swing.JLabel();
        labelCoeff3 = new javax.swing.JLabel();
        labelRCUs = new javax.swing.JLabel();
        inputOperatingMode = new javax.swing.JComboBox();
        inputBaselevel = new javax.swing.JTextField();
        inputStartlevel = new javax.swing.JTextField();
        inputStoplevel = new javax.swing.JTextField();
        inputWindow = new javax.swing.JComboBox();
        inputFilter0Coeff0 = new javax.swing.JTextField();
        inputFilter0Coeff1 = new javax.swing.JTextField();
        inputFilter0Coeff2 = new javax.swing.JTextField();
        inputFilter0Coeff3 = new javax.swing.JTextField();
        inputRCUs = new javax.swing.JTextField();
        limitsBaselevel = new javax.swing.JLabel();
        limitsStartlevel = new javax.swing.JLabel();
        limitsStoplevel = new javax.swing.JLabel();
        limitsCoeff0 = new javax.swing.JLabel();
        limitsCoeff1 = new javax.swing.JLabel();
        limitsCoeff2 = new javax.swing.JLabel();
        limitsCoeff3 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        editConfigButton = new javax.swing.JButton();
        addConfigButton = new javax.swing.JButton();
        deleteConfigButton = new javax.swing.JButton();
        cancelEditButton = new javax.swing.JButton();
        cancelEditButton.setVisible(false);
        labelSubbandList = new javax.swing.JLabel();
        inputSubbandList = new javax.swing.JTextField();
        labelTriggerMode = new javax.swing.JLabel();
        inputTriggerMode = new javax.swing.JComboBox();
        jLabel3 = new javax.swing.JLabel();
        jLabel4 = new javax.swing.JLabel();
        inputFilter1Coeff0 = new javax.swing.JTextField();
        inputFilter1Coeff1 = new javax.swing.JTextField();
        inputFilter1Coeff2 = new javax.swing.JTextField();
        inputFilter1Coeff3 = new javax.swing.JTextField();
        jScrollPane1 = new javax.swing.JScrollPane();
        TBBConfigurationPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        inputFilter = new javax.swing.JComboBox();
        TBBControlPanel = new javax.swing.JPanel();
        tbbControlPanel = new nl.astron.lofar.sas.otbcomponents.TBBControlPanel();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        setMinimumSize(new java.awt.Dimension(800, 400));
        setLayout(new java.awt.BorderLayout());

        jLabel1.setFont(new java.awt.Font("Tahoma", 1, 12));
        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("TBB");
        jLabel1.setBorder(javax.swing.BorderFactory.createEtchedBorder());

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, 1495, Short.MAX_VALUE)
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jLabel1, javax.swing.GroupLayout.PREFERRED_SIZE, 36, javax.swing.GroupLayout.PREFERRED_SIZE)
        );

        add(jPanel1, java.awt.BorderLayout.NORTH);

        TBBSettingsPanel.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        TBBSettingsPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        labelOperatingMode.setText("operating Mode:");
        TBBSettingsPanel.add(labelOperatingMode, new org.netbeans.lib.awtextra.AbsoluteConstraints(12, 16, 100, -1));

        labelBaselevel.setText("baselevel:");
        TBBSettingsPanel.add(labelBaselevel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 80, 89, -1));
        labelBaselevel.getAccessibleContext().setAccessibleName("labelBaselevel");

        labelStartlevel.setText("startlevel:");
        TBBSettingsPanel.add(labelStartlevel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 110, 89, -1));

        labelStoplevel.setText("stoplevel:");
        TBBSettingsPanel.add(labelStoplevel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 140, 89, -1));

        labelFilter.setText("filter:");
        TBBSettingsPanel.add(labelFilter, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 170, 89, -1));

        labelWindow.setText("window:");
        TBBSettingsPanel.add(labelWindow, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 200, 89, -1));

        labelCoeff0.setText("coeff0:");
        TBBSettingsPanel.add(labelCoeff0, new org.netbeans.lib.awtextra.AbsoluteConstraints(340, 30, 84, -1));

        labelCoeff1.setText("coeff1:");
        TBBSettingsPanel.add(labelCoeff1, new org.netbeans.lib.awtextra.AbsoluteConstraints(340, 60, 84, -1));

        labelCoeff2.setText("coeff2:");
        TBBSettingsPanel.add(labelCoeff2, new org.netbeans.lib.awtextra.AbsoluteConstraints(340, 90, 67, -1));

        labelCoeff3.setText("coeff3:");
        TBBSettingsPanel.add(labelCoeff3, new org.netbeans.lib.awtextra.AbsoluteConstraints(340, 120, 84, -1));

        labelRCUs.setText("RCUs:");
        TBBSettingsPanel.add(labelRCUs, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 220, 84, -1));

        inputOperatingMode.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Transient Detection", "Subband Data", " " }));
        inputOperatingMode.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(inputOperatingMode, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 10, 133, -1));

        inputBaselevel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(inputBaselevel, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 70, 138, 20));

        inputStartlevel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(inputStartlevel, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 100, 138, -1));

        inputStoplevel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(inputStoplevel, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 130, 138, -1));

        inputWindow.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "16B", "64B", "256B", "1K" }));
        inputWindow.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(inputWindow, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 190, 138, -1));

        inputFilter0Coeff0.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(inputFilter0Coeff0, new org.netbeans.lib.awtextra.AbsoluteConstraints(440, 30, 140, -1));

        inputFilter0Coeff1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(inputFilter0Coeff1, new org.netbeans.lib.awtextra.AbsoluteConstraints(440, 60, 140, -1));

        inputFilter0Coeff2.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(inputFilter0Coeff2, new org.netbeans.lib.awtextra.AbsoluteConstraints(440, 90, 140, -1));

        inputFilter0Coeff3.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(inputFilter0Coeff3, new org.netbeans.lib.awtextra.AbsoluteConstraints(440, 120, 140, -1));

        inputRCUs.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(inputRCUs, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 220, 990, -1));

        limitsBaselevel.setText("(4-127)");
        TBBSettingsPanel.add(limitsBaselevel, new org.netbeans.lib.awtextra.AbsoluteConstraints(260, 70, 62, -1));

        limitsStartlevel.setText("(1-15)");
        TBBSettingsPanel.add(limitsStartlevel, new org.netbeans.lib.awtextra.AbsoluteConstraints(260, 100, 62, -1));

        limitsStoplevel.setText("(1-15)");
        TBBSettingsPanel.add(limitsStoplevel, new org.netbeans.lib.awtextra.AbsoluteConstraints(260, 130, 62, -1));

        limitsCoeff0.setText("(0-65535)");
        TBBSettingsPanel.add(limitsCoeff0, new org.netbeans.lib.awtextra.AbsoluteConstraints(800, 30, 100, -1));

        limitsCoeff1.setText("(0-65535)");
        TBBSettingsPanel.add(limitsCoeff1, new org.netbeans.lib.awtextra.AbsoluteConstraints(800, 60, 120, -1));

        limitsCoeff2.setText("(0-65535)");
        TBBSettingsPanel.add(limitsCoeff2, new org.netbeans.lib.awtextra.AbsoluteConstraints(800, 90, 130, 20));

        limitsCoeff3.setText("(0-65535)");
        TBBSettingsPanel.add(limitsCoeff3, new org.netbeans.lib.awtextra.AbsoluteConstraints(800, 120, 130, -1));

        jLabel2.setText("Current configurations");
        TBBSettingsPanel.add(jLabel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(540, 320, 156, -1));

        editConfigButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_edit.gif"))); // NOI18N
        editConfigButton.setText("Edit Configuration");
        editConfigButton.setToolTipText("edit the selected configuration from the table");
        editConfigButton.setEnabled(false);
        editConfigButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                editConfigButtonActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(editConfigButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 620, -1, -1));

        addConfigButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_new.png"))); // NOI18N
        addConfigButton.setText("Apply Configuration");
        addConfigButton.setToolTipText("add composed Configuration to table");
        addConfigButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addConfigButtonActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(addConfigButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 290, -1, -1));

        deleteConfigButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_delete.png"))); // NOI18N
        deleteConfigButton.setText("Delete Configuration");
        deleteConfigButton.setToolTipText("delete the selected configuration from the table");
        deleteConfigButton.setEnabled(false);
        deleteConfigButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                deleteConfigButtonActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(deleteConfigButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(180, 620, -1, -1));

        cancelEditButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_cancel.png"))); // NOI18N
        cancelEditButton.setText("Cancel edit Configuration");
        cancelEditButton.setToolTipText("sets Configuration  entries back to default");
        cancelEditButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                cancelEditButtonActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(cancelEditButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(200, 290, -1, -1));

        labelSubbandList.setText("subbands:");
        TBBSettingsPanel.add(labelSubbandList, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 250, 84, -1));

        inputSubbandList.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(inputSubbandList, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 250, 990, -1));

        labelTriggerMode.setText("triggerMode:");
        TBBSettingsPanel.add(labelTriggerMode, new org.netbeans.lib.awtextra.AbsoluteConstraints(12, 42, 89, -1));

        inputTriggerMode.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "One-shot normal", "Continues normal", "One-shot external", "Continue external", "" }));
        inputTriggerMode.setSelectedItem("Continues normal");
        inputTriggerMode.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputTriggerModeinputActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(inputTriggerMode, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 40, 133, -1));

        jLabel3.setText("Filter 0");
        TBBSettingsPanel.add(jLabel3, new org.netbeans.lib.awtextra.AbsoluteConstraints(490, 10, -1, -1));

        jLabel4.setText("Filter 1");
        TBBSettingsPanel.add(jLabel4, new org.netbeans.lib.awtextra.AbsoluteConstraints(680, 10, -1, -1));

        inputFilter1Coeff0.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputFilter1Coeff0inputActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(inputFilter1Coeff0, new org.netbeans.lib.awtextra.AbsoluteConstraints(630, 30, 140, -1));

        inputFilter1Coeff1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputFilter1Coeff1inputActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(inputFilter1Coeff1, new org.netbeans.lib.awtextra.AbsoluteConstraints(630, 60, 140, -1));

        inputFilter1Coeff2.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputFilter1Coeff2inputActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(inputFilter1Coeff2, new org.netbeans.lib.awtextra.AbsoluteConstraints(630, 90, 140, -1));

        inputFilter1Coeff3.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputFilter1Coeff3inputActionPerformed(evt);
            }
        });
        TBBSettingsPanel.add(inputFilter1Coeff3, new org.netbeans.lib.awtextra.AbsoluteConstraints(630, 120, 140, -1));

        TBBConfigurationPanel.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                TBBConfigurationPanelMouseClicked(evt);
            }
        });
        jScrollPane1.setViewportView(TBBConfigurationPanel);

        TBBSettingsPanel.add(jScrollPane1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 350, 1090, 250));

        TBBSettingsPanel.add(inputFilter, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 160, 140, -1));

        jTabbedPane1.addTab("TBB-settings", TBBSettingsPanel);

        javax.swing.GroupLayout TBBControlPanelLayout = new javax.swing.GroupLayout(TBBControlPanel);
        TBBControlPanel.setLayout(TBBControlPanelLayout);
        TBBControlPanelLayout.setHorizontalGroup(
            TBBControlPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(TBBControlPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(tbbControlPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(296, Short.MAX_VALUE))
        );
        TBBControlPanelLayout.setVerticalGroup(
            TBBControlPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(TBBControlPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(tbbControlPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("TBBControl", TBBControlPanel);

        add(jTabbedPane1, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });
        add(buttonPanel1, java.awt.BorderLayout.SOUTH);
    }// </editor-fold>//GEN-END:initComponents

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        switch (evt.getActionCommand()) {
            case "Apply":
            itsMainFrame.setHourglassCursor();
                save();
            // save the input from the AntennaConfig Panel
            this.tbbControlPanel.saveInput();
            // reset all buttons, flags and tables to initial start position. So the panel now reflects the new, saved situation
            initPanel();
            itsMainFrame.setNormalCursor();
                break;
            case "Restore":
            itsMainFrame.setHourglassCursor();
            restore();
                itsMainFrame.setNormalCursor();
                break;
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    private void cancelEditButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_cancelEditButtonActionPerformed
        //fill tabel with default Settings again
       setDefaultInput();
        // set editting = false
        editting=false;
        
        itsSavedRCUs = "";
        
        this.editConfigButton.setEnabled(true);
        this.deleteConfigButton.setEnabled(true);
        
        // set back to default Button visible
        addConfigButton.setText("Add Configuration");
        addConfigButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_new.png")));
        cancelEditButton.setVisible(false);
    }//GEN-LAST:event_cancelEditButtonActionPerformed
    
    private void TBBConfigurationPanelMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_TBBConfigurationPanelMouseClicked
        editConfigButton.setEnabled(true);
        deleteConfigButton.setEnabled(true);
        
    }//GEN-LAST:event_TBBConfigurationPanelMouseClicked
    
    private void inputActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputActionPerformed
        addConfigButton.setEnabled(true);
    }//GEN-LAST:event_inputActionPerformed
    
    private void deleteConfigButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteConfigButtonActionPerformed
        deleteConfig();
    }//GEN-LAST:event_deleteConfigButtonActionPerformed
    
    private void editConfigButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_editConfigButtonActionPerformed
        editConfig();
    }//GEN-LAST:event_editConfigButtonActionPerformed
    
    private void addConfigButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addConfigButtonActionPerformed
        addConfig();
    }//GEN-LAST:event_addConfigButtonActionPerformed

    private void inputTriggerModeinputActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputTriggerModeinputActionPerformed
        // TODO add your handling code here:
}//GEN-LAST:event_inputTriggerModeinputActionPerformed

    private void inputFilter1Coeff0inputActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputFilter1Coeff0inputActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_inputFilter1Coeff0inputActionPerformed

    private void inputFilter1Coeff1inputActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputFilter1Coeff1inputActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_inputFilter1Coeff1inputActionPerformed

    private void inputFilter1Coeff2inputActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputFilter1Coeff2inputActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_inputFilter1Coeff2inputActionPerformed

    private void inputFilter1Coeff3inputActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputFilter1Coeff3inputActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_inputFilter1Coeff3inputActionPerformed
    
    private jOTDBnode                  itsNode        = null;
    private jOTDBnode                  itsDefaultNode = null;
    private MainFrame                  itsMainFrame   = null;
    private String                     itsTreeType    = "";
    private JFileChooser               fc             = null;
    private TBBConfigurationTableModel itsTBBConfigurationTableModel = null;
    
    // each rcu has its bit in the bitset
    private BitSet   itsUsedRCUList = new BitSet(192);
    private BitSet   itsSavedRCUList = new BitSet(192);
    private boolean  editting = false;
    private boolean  isInitialized=false;
    private int      itsSelectedRow = -1;
    private String   itsSavedRCUs = "";
    

    
    // TBBsettings
    private ArrayList<jOTDBnode> itsTBBsettings = new ArrayList<>();
    // All TBBsetting nodes
    private ArrayList<String>    itsOperatingModes = new ArrayList<>();
    private ArrayList<String>    itsTriggerModes = new ArrayList<>();
    private ArrayList<String>    itsBaselevels= new ArrayList<>();
    private ArrayList<String>    itsStartlevels= new ArrayList<>();
    private ArrayList<String>    itsStoplevels= new ArrayList<>();
    private ArrayList<String>    itsFilters= new ArrayList<>();
    private ArrayList<String>    itsWindows= new ArrayList<>();
    private ArrayList<String>    itsFilter0Coeff0s= new ArrayList<>();
    private ArrayList<String>    itsFilter0Coeff1s= new ArrayList<>();
    private ArrayList<String>    itsFilter0Coeff2s= new ArrayList<>();
    private ArrayList<String>    itsFilter0Coeff3s= new ArrayList<>();
    private ArrayList<String>    itsFilter1Coeff0s= new ArrayList<>();
    private ArrayList<String>    itsFilter1Coeff1s= new ArrayList<>();
    private ArrayList<String>    itsFilter1Coeff2s= new ArrayList<>();
    private ArrayList<String>    itsFilter1Coeff3s= new ArrayList<>();
    private ArrayList<String>    itsRCUs= new ArrayList<>();
    private ArrayList<String>    itsSubbandList= new ArrayList<>();
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.sas.otbcomponents.TablePanel TBBConfigurationPanel;
    private javax.swing.JPanel TBBControlPanel;
    private javax.swing.JPanel TBBSettingsPanel;
    private javax.swing.JButton addConfigButton;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JButton cancelEditButton;
    private javax.swing.JButton deleteConfigButton;
    private javax.swing.JButton editConfigButton;
    private javax.swing.JTextField inputBaselevel;
    private javax.swing.JComboBox inputFilter;
    private javax.swing.JTextField inputFilter0Coeff0;
    private javax.swing.JTextField inputFilter0Coeff1;
    private javax.swing.JTextField inputFilter0Coeff2;
    private javax.swing.JTextField inputFilter0Coeff3;
    private javax.swing.JTextField inputFilter1Coeff0;
    private javax.swing.JTextField inputFilter1Coeff1;
    private javax.swing.JTextField inputFilter1Coeff2;
    private javax.swing.JTextField inputFilter1Coeff3;
    private javax.swing.JComboBox inputOperatingMode;
    private javax.swing.JTextField inputRCUs;
    private javax.swing.JTextField inputStartlevel;
    private javax.swing.JTextField inputStoplevel;
    private javax.swing.JTextField inputSubbandList;
    private javax.swing.JComboBox inputTriggerMode;
    private javax.swing.JComboBox inputWindow;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.JLabel labelBaselevel;
    private javax.swing.JLabel labelCoeff0;
    private javax.swing.JLabel labelCoeff1;
    private javax.swing.JLabel labelCoeff2;
    private javax.swing.JLabel labelCoeff3;
    private javax.swing.JLabel labelFilter;
    private javax.swing.JLabel labelOperatingMode;
    private javax.swing.JLabel labelRCUs;
    private javax.swing.JLabel labelStartlevel;
    private javax.swing.JLabel labelStoplevel;
    private javax.swing.JLabel labelSubbandList;
    private javax.swing.JLabel labelTriggerMode;
    private javax.swing.JLabel labelWindow;
    private javax.swing.JLabel limitsBaselevel;
    private javax.swing.JLabel limitsCoeff0;
    private javax.swing.JLabel limitsCoeff1;
    private javax.swing.JLabel limitsCoeff2;
    private javax.swing.JLabel limitsCoeff3;
    private javax.swing.JLabel limitsStartlevel;
    private javax.swing.JLabel limitsStoplevel;
    private nl.astron.lofar.sas.otbcomponents.TBBControlPanel tbbControlPanel;
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
        myListenerList.add(java.awt.event.ActionListener.class, listener);
    }
    
    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeActionListener(java.awt.event.ActionListener listener) {
        
        myListenerList.remove(java.awt.event.ActionListener.class, listener);
    }
    
    /**
     * Notifies all registered listeners about the event.
     *
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {
        
        if (myListenerList == null) return;
        Object[] listeners = myListenerList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed(event);
            }
        }
    }
    
}
