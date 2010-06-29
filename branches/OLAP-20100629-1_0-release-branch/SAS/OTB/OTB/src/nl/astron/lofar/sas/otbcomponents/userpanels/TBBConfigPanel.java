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
import java.util.BitSet;
import java.util.Enumeration;
import java.util.Vector;
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
        jOTDBparam aParam=null;
        isInitialized=false;
        
        itsMainFrame.setHourglassCursor();
        try {
            
            //we need to get all the childs from this node.
            Vector childs = OtdbRmi.getRemoteMaintenance().getItemList(itsNode.treeID(), itsNode.nodeID(), 1);
            
            // first element is the default Node we should keep it.
            itsDefaultNode = (jOTDBnode)childs.elementAt(0);
            
            // get all the params per child
            Enumeration e = childs.elements();
            while( e.hasMoreElements()  ) {
                aParam=null;
                
                jOTDBnode aNode = (jOTDBnode)e.nextElement();
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
            logger.error("Error during getComponentParam: "+ ex);
            itsMainFrame.setNormalCursor();
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
                    logger.error("exportTree failed : " + ex);
                } catch (FileNotFoundException ex) {
                    logger.error("Error during newPICTree creation: "+ ex);
                } catch (IOException ex) {
                    logger.error("Error during newPICTree creation: "+ ex);
                }
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
        inputCoeff0.setEnabled(enabled);
        inputCoeff1.setEnabled(enabled);
        inputCoeff2.setEnabled(enabled);
        inputCoeff3.setEnabled(enabled);
        inputRCUs.setEnabled(enabled);
        inputSubbandList.setEnabled(enabled);
    }
    
    private void initialize() {
        buttonPanel1.addButton("Restore");
        buttonPanel1.setButtonToolTip("Restore","Restores the complete table to it's initial state");
        buttonPanel1.addButton("Apply");
        buttonPanel1.setButtonToolTip("Apply","Deleted all old TBBsetting instances from the Database and will write all new instances");
        buttonPanel1.setButtonEnabled("Restore",false);
        buttonPanel1.setButtonEnabled("Apply",false);
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
                logger.error("TBBConfigPanel: Error getting treeInfo/treetype" + ex);
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
        TBBConfigurationPanel.setColumnSize("C0",10);
        TBBConfigurationPanel.setColumnSize("C1",10);
        TBBConfigurationPanel.setColumnSize("C2",10);
        TBBConfigurationPanel.setColumnSize("C3",10);
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
            //first element should have been the default TBBsettings, so we can assume init of components should have been done
            isInitialized=true;
        } catch (RemoteException ex) {
            logger.error("Error during retrieveAndDisplayChildDataForNode: "+ ex);
            return;
        }
    }
    
    /**
     * Fills the StringVectors with the values from the Database
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
            logger.error("Error during getParam: "+ ex);
        }
        
        if (aKeyName.equals("operatingMode")) {
            // OperatingMode
            if (!isInitialized) {
               inputOperatingMode.setToolTipText(aParam.description);
               LofarUtils.setPopupComboChoices(inputOperatingMode,aParam.limits);
            }
            itsOperatingModes.add(aNode.limits);
            
        } else if (aKeyName.equals("triggerMode")) {
            // TriggerMode
            if (!isInitialized) {
               inputTriggerMode.setToolTipText(aParam.description);
               LofarUtils.setPopupComboChoices(inputTriggerMode,aParam.limits);
            }
            itsTriggerModes.add(aNode.limits);
        } else if (aKeyName.equals("baselevel")) {
            // Baselevel
            if (!isInitialized) {
               inputBaselevel.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsBaselevels.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsBaselevels.add(aNode.limits);
            }
            
        } else if (aKeyName.equals("startlevel")) {
            // startlevel
            if (!isInitialized) {
               inputStartlevel.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsStartlevels.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsStartlevels.add(aNode.limits);
            }

         } else if (aKeyName.equals("stoplevel")) {
            // stoplevel
            if (!isInitialized) {
               inputStoplevel.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsStoplevels.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsStoplevels.add(aNode.limits);
            }

        } else if (aKeyName.equals("filter")) {
            // filter
            if (!isInitialized) {
               inputFilter.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsFilters.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsFilters.add(aNode.limits);
            }
            
        } else if (aKeyName.equals("window")) {
            // window
            if (!isInitialized) {
               inputWindow.setToolTipText(aParam.description);
               LofarUtils.setPopupComboChoices(inputWindow,aParam.limits);
            }
            itsWindows.add(aNode.limits);

        } else if (aKeyName.equals("coeff0")) {
            // Coeff0
            if (!isInitialized) {
               inputCoeff0.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsCoeff0s.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsCoeff0s.add(aNode.limits);
            }
            
        } else if (aKeyName.equals("coeff1")) {
            // Coeff1
            if (!isInitialized) {
               inputCoeff1.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsCoeff1s.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsCoeff1s.add(aNode.limits);
            }
            
        } else if (aKeyName.equals("coeff2")) {
            // Coeff2
            if (!isInitialized) {
               inputCoeff2.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsCoeff2s.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsCoeff2s.add(aNode.limits);
            }
            
        } else if (aKeyName.equals("coeff3")) {
            // Coeff3
            if (!isInitialized) {
               inputCoeff3.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsCoeff3s.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsCoeff3s.add(aNode.limits);
            }
            
        } else if (aKeyName.equals("RCUs")) {
            // RCUs
            if (!isInitialized) {
               inputRCUs.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsRCUs.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsRCUs.add(aNode.limits);
            }
        } else if (aKeyName.equals("subbandList")) {
            // SubbandList
            if (!isInitialized) {
               inputSubbandList.setToolTipText(aParam.description);
            }
            if (isRef && aParam != null) {
                itsSubbandList.add(aNode.limits + " : " + aParam.limits);
            } else {
                itsSubbandList.add(aNode.limits);
            }
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
        itsTBBConfigurationTableModel.fillTable(itsTreeType,itsOperatingModes,itsTriggerModes,itsBaselevels,itsStartlevels,itsStoplevels,itsFilters,itsWindows,itsCoeff0s,itsCoeff1s,itsCoeff2s,itsCoeff3s,itsRCUs,itsSubbandList);
        
        buttonPanel1.setButtonEnabled("Restore",false);
        buttonPanel1.setButtonEnabled("Apply",false);
    }
    
    
    private void setDefaultInput() {
        
        // if no entries in lists we can exit again
        if (itsOperatingModes.size() == 0) {
            logger.error("ERROR setInputDefaults,  null entry found");
        }
        
        // defaultSettings Index
        int index=0;
          
        // OperatingMode
        if (!itsOperatingModes.elementAt(index).equals("")) {
           inputOperatingMode.setSelectedItem(itsOperatingModes.elementAt(index));
        }
        // TriggerMode
        if (!itsTriggerModes.elementAt(index).equals("")) {
           inputTriggerMode.setSelectedItem(itsTriggerModes.elementAt(index));
        }
        // Baselevel
         inputBaselevel.setText(itsBaselevels.elementAt(index));
            
         // Startlevel
         inputStartlevel.setText(itsStartlevels.elementAt(index));
 
         // Stoplevel
         inputStoplevel.setText(itsStoplevels.elementAt(index));
            
         // Filter
         inputFilter.setText(itsFilters.elementAt(index));

         // Window
         if (!itsWindows.elementAt(index).equals("")) {
            inputWindow.setSelectedItem(itsWindows.elementAt(index));
         }
            
         // Coeff0
         inputCoeff0.setText(itsCoeff0s.elementAt(index));

         // Coeff1
         inputCoeff1.setText(itsCoeff1s.elementAt(index));

         // Coeff2
         inputCoeff2.setText(itsCoeff2s.elementAt(index));

         // Coeff3
         inputCoeff3.setText(itsCoeff3s.elementAt(index));

         // RCUs
         inputRCUs.setText(itsRCUs.elementAt(index));
        
         // subbandList
         inputSubbandList.setText(itsSubbandList.elementAt(index));
    }
    
    /** fill the RCU bitset to see what RCU's have been set. To be able to determine later if a given RCU is indeed free.
     */
    private void fillRCUBitset() {
        itsUsedRCUList.clear();
        for (int i=1;i<itsRCUs.size();i++) {
            BitSet aNewBitSet=rcuToBitSet(LofarUtils.expandedArrayString(itsRCUs.elementAt(i)));
            
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
        
        // filter
        if (Integer.valueOf(inputFilter.getText())<0 || Integer.valueOf(inputFilter.getText())>255) {
            error=true;
            errorMsg += "Filter \n";
        }
        
        // Coeff0
        if (Integer.valueOf(inputCoeff0.getText())<0 || Integer.valueOf(inputCoeff0.getText())>65535) {
            error=true;
            errorMsg += "Coeff0 \n";
        }
        
        // Coeff1
        if (Integer.valueOf(inputCoeff1.getText())<0 || Integer.valueOf(inputCoeff1.getText())>65535) {
            error=true;
            errorMsg += "Coeff1 \n";
        }
        
        // Coeff2
        if (Integer.valueOf(inputCoeff2.getText())<0 || Integer.valueOf(inputCoeff2.getText())>65535) {
            error=true;
            errorMsg += "Coeff2 \n";
        }
        
        // Coeff3
        if (Integer.valueOf(inputCoeff3.getText())<0 || Integer.valueOf(inputCoeff3.getText())>65535) {
            error=true;
            errorMsg += "Coeff3 \n";
        }
        
        // RCUs
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
                           inputFilter.getText(),
                           inputWindow.getSelectedItem().toString(),
                           inputCoeff0.getText(),
                           inputCoeff1.getText(),
                           inputCoeff2.getText(),
                           inputCoeff3.getText(),
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
            cancelEditButton.setVisible(false);            
        } else {
            setDefaultInput();
        }
        
        // something obviously changed, so enable restore and save buttons
        buttonPanel1.setButtonEnabled("Restore",true);
        buttonPanel1.setButtonEnabled("Apply",true);

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
                    OtdbRmi.getRemoteMaintenance().deleteNode(itsTBBsettings.elementAt(i));
                }
            } catch (RemoteException ex) {
                logger.error("Error during deletion of defaultNode: "+ex);
                return false;
            }
        
            // now that all Nodes are deleted we should collect the tables input and create new TBBsettings to save to the database.
        
            itsTBBConfigurationTableModel.getTable(itsOperatingModes,itsTriggerModes,itsBaselevels,itsStartlevels,itsStoplevels,itsFilters,itsWindows,itsCoeff0s,itsCoeff1s,itsCoeff2s,itsCoeff3s,itsRCUs,itsSubbandList);
            itsTBBsettings.clear();
        
            try {
                // for all elements
                for (i=1; i < itsCoeff0s.size();i++) {
        
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

                        Vector HWchilds = OtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1);
                        // get all the params per child
                        Enumeration e1 = HWchilds.elements();
                        while( e1.hasMoreElements()  ) {
                            jOTDBnode aHWNode = (jOTDBnode)e1.nextElement();
                            String aKeyName = LofarUtils.keyName(aHWNode.name);
                            if (aKeyName.equals("operatingMode")) {
                                aHWNode.limits=itsOperatingModes.elementAt(i);
                            } else if (aKeyName.equals("triggerMode")) {
                                aHWNode.limits=itsTriggerModes.elementAt(i);
                            } else if (aKeyName.equals("baselevel")) {
                                aHWNode.limits=itsBaselevels.elementAt(i);
                            } else if (aKeyName.equals("startlevel")) {
                                aHWNode.limits=itsStartlevels.elementAt(i);
                            } else if (aKeyName.equals("stoplevel")) {
                                aHWNode.limits=itsStoplevels.elementAt(i);
                            } else if (aKeyName.equals("filter")) {
                                aHWNode.limits=itsFilters.elementAt(i);
                            } else if (aKeyName.equals("window")) {
                                aHWNode.limits=itsWindows.elementAt(i);
                            } else if (aKeyName.equals("coeff0")) {
                                aHWNode.limits=itsCoeff0s.elementAt(i);
                            } else if (aKeyName.equals("coeff1")) {
                                aHWNode.limits=itsCoeff1s.elementAt(i);
                            } else if (aKeyName.equals("coeff2")) {
                                aHWNode.limits=itsCoeff2s.elementAt(i);
                            } else if (aKeyName.equals("coeff3")) {
                                aHWNode.limits=itsCoeff3s.elementAt(i);
                            } else if (aKeyName.equals("RCUs")) {
                                aHWNode.limits=itsRCUs.elementAt(i);
                            } else if (aKeyName.equals("subbandList")) {
                                aHWNode.limits=itsSubbandList.elementAt(i);
                            }
                            saveNode(aHWNode);
                        }
                    }
                }

            
                // store new number of instances in baseSetting
                itsDefaultNode.instances=(short)(itsCoeff0s.size()-1); // - default at 0
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
         inputFilter.setText(selection[5]);

         // Window
         if (!(selection[6]).equals("")) {
            inputWindow.setSelectedItem(selection[6]);
         }
            
         // Coeff0
         inputCoeff0.setText(selection[7]);

         // Coeff1
         inputCoeff1.setText(selection[8]);

         // Coeff2
         inputCoeff2.setText(selection[9]);

         // Coeff3
         inputCoeff3.setText(selection[10]);

         // RCUs
         inputRCUs.setText(selection[11]);

         // subbandList
         inputSubbandList.setText(selection[12]);
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
        itsSavedRCUs=selection[11];
        
        //fill table with entry to be editted
        // 0 in lists represent default
        setInput(selection);
        // set editting = true
        editting=true;
        this.editConfigButton.setEnabled(false);
        this.deleteConfigButton.setEnabled(false);
        
        // set back to default Button visible
        addConfigButton.setText("Apply Changes");
        cancelEditButton.setVisible(true);
        
    }
    
    private void deleteConfig() {
        int row = TBBConfigurationPanel.getSelectedRow();
        // if removed then the old RCU's should be removed form the checklist also
        String oldRCUs = itsTBBConfigurationTableModel.getSelection(row)[11];
        BitSet rcuSet = rcuToBitSet(LofarUtils.expandedArrayString(oldRCUs));
        
        if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this configuration ?","Delete Configuration",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
            if (row > -1) {
                itsTBBConfigurationTableModel.removeRow(row);
                itsUsedRCUList.xor(rcuSet);
                // No selection anymore after delete, so buttons disabled again
                this.editConfigButton.setEnabled(false);
                this.deleteConfigButton.setEnabled(false);

                // something obviously changed, so enable restore and save buttons
                buttonPanel1.setButtonEnabled("Restore",true);
                buttonPanel1.setButtonEnabled("Apply",true);

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
        jPanel2 = new javax.swing.JPanel();
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
        inputFilter = new javax.swing.JTextField();
        inputWindow = new javax.swing.JComboBox();
        inputCoeff0 = new javax.swing.JTextField();
        inputCoeff1 = new javax.swing.JTextField();
        inputCoeff2 = new javax.swing.JTextField();
        inputCoeff3 = new javax.swing.JTextField();
        inputRCUs = new javax.swing.JTextField();
        limitsBaselevel = new javax.swing.JLabel();
        limitsStartlevel = new javax.swing.JLabel();
        limitsStoplevel = new javax.swing.JLabel();
        limitsFilter = new javax.swing.JLabel();
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
        TBBConfigurationPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        labelSubbandList = new javax.swing.JLabel();
        inputSubbandList = new javax.swing.JTextField();
        labelTriggerMode = new javax.swing.JLabel();
        inputTriggerMode = new javax.swing.JComboBox();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        setMinimumSize(new java.awt.Dimension(800, 400));
        setLayout(new java.awt.BorderLayout());

        jLabel1.setFont(new java.awt.Font("Tahoma", 1, 12));
        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("Configure TBB settings");
        jLabel1.setBorder(javax.swing.BorderFactory.createEtchedBorder());

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, 1169, Short.MAX_VALUE)
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addComponent(jLabel1, javax.swing.GroupLayout.PREFERRED_SIZE, 36, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        add(jPanel1, java.awt.BorderLayout.NORTH);

        jPanel2.setBorder(javax.swing.BorderFactory.createEtchedBorder());

        labelOperatingMode.setText("operating Mode:");

        labelBaselevel.setText("baselevel:");

        labelStartlevel.setText("startlevel:");

        labelStoplevel.setText("stoplevel:");

        labelFilter.setText("filter:");

        labelWindow.setText("window:");

        labelCoeff0.setText("coeff0:");

        labelCoeff1.setText("coeff1:");

        labelCoeff2.setText("coeff2:");

        labelCoeff3.setText("coeff3:");

        labelRCUs.setText("RCUs:");

        inputOperatingMode.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Transient Detection", "Subband Data", " " }));
        inputOperatingMode.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });

        inputBaselevel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });

        inputStartlevel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });

        inputStoplevel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });

        inputFilter.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });

        inputWindow.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "16B", "64B", "256B", "1K" }));
        inputWindow.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });

        inputCoeff0.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });

        inputCoeff1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });

        inputCoeff2.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });

        inputCoeff3.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });

        inputRCUs.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });

        limitsBaselevel.setText("(1-4095)");

        limitsStartlevel.setText("(1-15)");

        limitsStoplevel.setText("(1-15)");

        limitsFilter.setText("(1-255)");

        limitsCoeff0.setText("(0-65535)");

        limitsCoeff1.setText("(0-65535)");

        limitsCoeff2.setText("(0-65535)");

        limitsCoeff3.setText("(0-65535)");

        jLabel2.setText("Current configurations");

        editConfigButton.setText("Edit Configuration");
        editConfigButton.setToolTipText("edit the selected configuration from the table");
        editConfigButton.setEnabled(false);
        editConfigButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                editConfigButtonActionPerformed(evt);
            }
        });

        addConfigButton.setText("Apply Configuration");
        addConfigButton.setToolTipText("add composed Configuration to table");
        addConfigButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addConfigButtonActionPerformed(evt);
            }
        });

        deleteConfigButton.setText("Delete Configuration");
        deleteConfigButton.setToolTipText("delete the selected configuration from the table");
        deleteConfigButton.setEnabled(false);
        deleteConfigButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                deleteConfigButtonActionPerformed(evt);
            }
        });

        cancelEditButton.setText("Cancel edit Configuration");
        cancelEditButton.setToolTipText("sets Configuration  entries back to default");
        cancelEditButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                cancelEditButtonActionPerformed(evt);
            }
        });

        TBBConfigurationPanel.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                TBBConfigurationPanelMouseClicked(evt);
            }
        });

        labelSubbandList.setText("subbands:");

        inputSubbandList.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputActionPerformed(evt);
            }
        });

        labelTriggerMode.setText("triggerMode:");

        inputTriggerMode.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "One-shot normal", "Continues normal", "One-shot external", "Continue external", "" }));
        inputTriggerMode.setSelectedItem("Continues normal");
        inputTriggerMode.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputTriggerModeinputActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(labelCoeff2, javax.swing.GroupLayout.PREFERRED_SIZE, 67, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(labelBaselevel, javax.swing.GroupLayout.DEFAULT_SIZE, 84, Short.MAX_VALUE)
                            .addComponent(labelOperatingMode, javax.swing.GroupLayout.DEFAULT_SIZE, 84, Short.MAX_VALUE)
                            .addComponent(labelTriggerMode, javax.swing.GroupLayout.DEFAULT_SIZE, 84, Short.MAX_VALUE)
                            .addComponent(labelStartlevel, javax.swing.GroupLayout.DEFAULT_SIZE, 84, Short.MAX_VALUE)
                            .addComponent(labelFilter, javax.swing.GroupLayout.DEFAULT_SIZE, 84, Short.MAX_VALUE)
                            .addComponent(labelWindow, javax.swing.GroupLayout.DEFAULT_SIZE, 84, Short.MAX_VALUE)
                            .addComponent(labelStoplevel, javax.swing.GroupLayout.DEFAULT_SIZE, 84, Short.MAX_VALUE)
                            .addComponent(labelCoeff0, javax.swing.GroupLayout.DEFAULT_SIZE, 84, Short.MAX_VALUE)
                            .addComponent(labelCoeff1, javax.swing.GroupLayout.DEFAULT_SIZE, 84, Short.MAX_VALUE)
                            .addComponent(labelCoeff3, javax.swing.GroupLayout.DEFAULT_SIZE, 84, Short.MAX_VALUE)
                            .addComponent(labelRCUs, javax.swing.GroupLayout.DEFAULT_SIZE, 84, Short.MAX_VALUE)
                            .addComponent(labelSubbandList, javax.swing.GroupLayout.DEFAULT_SIZE, 84, Short.MAX_VALUE))
                        .addGap(10, 10, 10)
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel2Layout.createSequentialGroup()
                                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(inputStartlevel, javax.swing.GroupLayout.DEFAULT_SIZE, 146, Short.MAX_VALUE)
                                    .addComponent(inputBaselevel, javax.swing.GroupLayout.DEFAULT_SIZE, 146, Short.MAX_VALUE)
                                    .addComponent(inputStoplevel, javax.swing.GroupLayout.DEFAULT_SIZE, 146, Short.MAX_VALUE)
                                    .addComponent(inputFilter, javax.swing.GroupLayout.DEFAULT_SIZE, 146, Short.MAX_VALUE)
                                    .addComponent(inputWindow, 0, 146, Short.MAX_VALUE)
                                    .addComponent(inputCoeff0, javax.swing.GroupLayout.DEFAULT_SIZE, 146, Short.MAX_VALUE)
                                    .addComponent(inputCoeff1, javax.swing.GroupLayout.DEFAULT_SIZE, 146, Short.MAX_VALUE)
                                    .addComponent(inputTriggerMode, javax.swing.GroupLayout.PREFERRED_SIZE, 133, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addComponent(inputOperatingMode, javax.swing.GroupLayout.PREFERRED_SIZE, 133, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addComponent(inputCoeff2, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 146, Short.MAX_VALUE)
                                    .addComponent(inputCoeff3, javax.swing.GroupLayout.DEFAULT_SIZE, 146, Short.MAX_VALUE))
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addGroup(jPanel2Layout.createSequentialGroup()
                                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                                            .addComponent(limitsCoeff2, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 59, Short.MAX_VALUE)
                                            .addComponent(limitsCoeff1, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 59, Short.MAX_VALUE)
                                            .addComponent(limitsCoeff0, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 59, Short.MAX_VALUE)
                                            .addComponent(limitsBaselevel, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 59, Short.MAX_VALUE)
                                            .addComponent(limitsStartlevel, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 59, Short.MAX_VALUE)
                                            .addComponent(limitsStoplevel, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 59, Short.MAX_VALUE)
                                            .addComponent(limitsFilter, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 59, Short.MAX_VALUE))
                                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                                            .addGroup(jPanel2Layout.createSequentialGroup()
                                                .addGap(310, 310, 310)
                                                .addComponent(jLabel2, javax.swing.GroupLayout.PREFERRED_SIZE, 156, javax.swing.GroupLayout.PREFERRED_SIZE)
                                                .addGap(322, 322, 322))
                                            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel2Layout.createSequentialGroup()
                                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                                    .addGroup(jPanel2Layout.createSequentialGroup()
                                                        .addComponent(editConfigButton)
                                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                                        .addComponent(deleteConfigButton)
                                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                                        .addComponent(addConfigButton)
                                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                                        .addComponent(cancelEditButton))
                                                    .addComponent(TBBConfigurationPanel, javax.swing.GroupLayout.PREFERRED_SIZE, 830, javax.swing.GroupLayout.PREFERRED_SIZE))
                                                .addGap(18, 18, 18))))
                                    .addComponent(limitsCoeff3)))
                            .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                                .addComponent(inputSubbandList, javax.swing.GroupLayout.Alignment.LEADING)
                                .addComponent(inputRCUs, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 1040, Short.MAX_VALUE)))))
                .addContainerGap())
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel2)
                    .addComponent(labelOperatingMode)
                    .addComponent(inputOperatingMode, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addGap(6, 6, 6)
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                            .addGroup(jPanel2Layout.createSequentialGroup()
                                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                                    .addComponent(labelTriggerMode)
                                    .addComponent(inputTriggerMode, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                                    .addComponent(labelBaselevel)
                                    .addComponent(inputBaselevel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                            .addComponent(limitsBaselevel))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(labelStartlevel)
                            .addComponent(inputStartlevel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(limitsStartlevel))
                        .addGap(6, 6, 6)
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(inputStoplevel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(labelStoplevel)
                            .addComponent(limitsStoplevel))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(labelFilter)
                            .addComponent(inputFilter, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(limitsFilter))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(labelWindow)
                            .addComponent(inputWindow, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(limitsCoeff0)
                            .addComponent(inputCoeff0, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(labelCoeff0))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(labelCoeff1)
                            .addComponent(inputCoeff1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(limitsCoeff1)))
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(TBBConfigurationPanel, javax.swing.GroupLayout.PREFERRED_SIZE, 209, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addGap(12, 12, 12)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelCoeff2)
                    .addComponent(inputCoeff2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(limitsCoeff2)
                    .addComponent(editConfigButton)
                    .addComponent(deleteConfigButton)
                    .addComponent(addConfigButton)
                    .addComponent(cancelEditButton))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelCoeff3)
                    .addComponent(inputCoeff3, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(limitsCoeff3))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelRCUs)
                    .addComponent(inputRCUs, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelSubbandList)
                    .addComponent(inputSubbandList, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(23, Short.MAX_VALUE))
        );

        labelBaselevel.getAccessibleContext().setAccessibleName("labelBaselevel");

        add(jPanel2, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });
        add(buttonPanel1, java.awt.BorderLayout.SOUTH);
    }// </editor-fold>//GEN-END:initComponents

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        if(evt.getActionCommand().equals("Apply")) {
            if (JOptionPane.showConfirmDialog(this,"This will throw away all old TBBsettings from the database and rewrite the new ones. Are you sure you want to do this ","Write new configurations",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
                save();
            }
        } else if(evt.getActionCommand().equals("Restore")) {
            if (JOptionPane.showConfirmDialog(this,"This will throw away all changes and restore the original settings. Are you sure you want to do this ","Restore old configuration",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
                restore();
            }
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
    
    private jOTDBnode                  itsNode        = null;
    private jOTDBnode                  itsDefaultNode = null;
    private MainFrame                  itsMainFrame   = null;
    private String                     itsTreeType    = "";
    private JFileChooser               fc             = null;
    private TBBConfigurationTableModel itsTBBConfigurationTableModel = null;
    
    // each rcu has its bit in the bitset
    private BitSet   itsUsedRCUList = new BitSet(192);
    private boolean  editting = false;
    private boolean  isInitialized=false;
    private int      itsSelectedRow = -1;
    private String   itsSavedRCUs = "";
    

    
    // TBBsettings
    private Vector<jOTDBnode> itsTBBsettings = new Vector<jOTDBnode>();
    // All TBBsetting nodes
    private Vector<String>    itsOperatingModes = new Vector<String>();
    private Vector<String>    itsTriggerModes = new Vector<String>();
    private Vector<String>    itsBaselevels= new Vector<String>();
    private Vector<String>    itsStartlevels= new Vector<String>();
    private Vector<String>    itsStoplevels= new Vector<String>();
    private Vector<String>    itsFilters= new Vector<String>();
    private Vector<String>    itsWindows= new Vector<String>();
    private Vector<String>    itsCoeff0s= new Vector<String>();
    private Vector<String>    itsCoeff1s= new Vector<String>();
    private Vector<String>    itsCoeff2s= new Vector<String>();
    private Vector<String>    itsCoeff3s= new Vector<String>();
    private Vector<String>    itsRCUs= new Vector<String>();
    private Vector<String>    itsSubbandList= new Vector<String>();
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.sas.otbcomponents.TablePanel TBBConfigurationPanel;
    private javax.swing.JButton addConfigButton;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JButton cancelEditButton;
    private javax.swing.JButton deleteConfigButton;
    private javax.swing.JButton editConfigButton;
    private javax.swing.JTextField inputBaselevel;
    private javax.swing.JTextField inputCoeff0;
    private javax.swing.JTextField inputCoeff1;
    private javax.swing.JTextField inputCoeff2;
    private javax.swing.JTextField inputCoeff3;
    private javax.swing.JTextField inputFilter;
    private javax.swing.JComboBox inputOperatingMode;
    private javax.swing.JTextField inputRCUs;
    private javax.swing.JTextField inputStartlevel;
    private javax.swing.JTextField inputStoplevel;
    private javax.swing.JTextField inputSubbandList;
    private javax.swing.JComboBox inputTriggerMode;
    private javax.swing.JComboBox inputWindow;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
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
    private javax.swing.JLabel limitsFilter;
    private javax.swing.JLabel limitsStartlevel;
    private javax.swing.JLabel limitsStoplevel;
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
