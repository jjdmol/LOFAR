/*
 *  BBSStrategyPanel.java
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

package nl.astron.lofar.sas.otbcomponents.bbs;

import java.awt.Color;
import java.awt.Component;
import java.rmi.RemoteException;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.DefaultComboBoxModel;
import javax.swing.DefaultListModel;
import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTree;
import javax.swing.border.TitledBorder;
import javax.swing.tree.TreePath;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.treenodes.TreeNode;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStep;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStepDataManager;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStepInputDialog;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStepNode;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStepTreeManager;
import nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement.BBSStrategy;
import org.apache.log4j.Logger;

/**
 * OTB Component Panel for BBS Strategy and Steps configuration
 *
 * @author pompert
 * @version $Id$
 * @created 11-07-2006, 13:37
 */
public class BBSStrategyPanel extends javax.swing.JPanel implements IViewPanel{
    
    static Logger logger = Logger.getLogger(BBSStrategyPanel.class);
    static String name = "BBS Strategy";
            
    private jOTDBnode itsNode = null;
    private MainFrame  itsMainFrame;
    private Vector<jOTDBparam> itsParamList;
    
    // BBS Strategy parameters
    private jOTDBnode StrategySteps;
    private jOTDBnode StrategyStations;
    private jOTDBnode StrategyInputData;
    private jOTDBnode StrategyCorrelationSelection;
    private jOTDBnode StrategyCorrelationType;
    private jOTDBnode StrategyWDSFrequency;
    private jOTDBnode StrategyWDSTime;
    private jOTDBnode StrategyIntegrationFrequency;
    private jOTDBnode StrategyIntegrationTime;
    
    /**
     * Creates new BBSStrategyPanel instance using a given MainFrame instance and
     * the OTDBnode needed to fill the panel with correct data.
     *
     * @param  aMainFrame   the OTB instance
     * @param  aNode        the node to obtain the BBS Strategy information from
     * (should be the BBS.Strategy node in the component tree)
     */
    public BBSStrategyPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode=aNode;
        initialize();
        initPanel();
    }
    
    /**
     * Creates new BBSStrategyPanel instance
     */
    public BBSStrategyPanel() {
        initComponents();
        initialize();
        
    }
    /**
     * Sets the OTB MainFrame instance in this panel.
     * @param aMainFrame the MainFrame instance to associate with
     */
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
        } else {
            logger.debug("No Mainframe supplied");
        }
    }
    /**
     * Returns the friendly name of this panel
     * @return String the friendly name of this panel
     */
    public String getShortName() {
        return name;
    }
    /**
     * This method will attempt to fill this panel with a given jOTDBnode object.
     * <br><br>
     * <b>Important</b>: The jOTDBnode to be passed on should always be the 'BBS.Strategy' node.
     * @param anObject the BBS Strategy jOTDBnode to be displayed in the GUI.
     */
    public void setContent(Object anObject) {
        itsNode=(jOTDBnode)anObject;
        //it is assumed itsNode is the BBS.Strategy node.
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
                }else if (LofarUtils.keyName(aNode.name).equals("WorkDomainSize")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                    
                }else if (LofarUtils.keyName(aNode.name).equals("Integration")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                    
                }else if (LofarUtils.keyName(aNode.name).equals("Correlation")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }
            }
        } catch (RemoteException ex) {
            logger.debug("Error during getComponentParam: "+ ex);
            itsParamList=null;
            return;
        }
        
        initPanel();
        //reset all values
        restoreBBSStrategyPanel();
        
    }
    
    /**
     * This method allows the OTB to know if the BBSStrategyPanel should
     * only have one instance per OTB session. For now this method returns <i>false</i>
     * @return <i>true</i> - if the panel should be loaded only once, <i>false</i> - panel can
     * be instantiated more than once.
     */
    public boolean isSingleton() {
        return false;
    }
    /**
     * This method returns a BBSStrategyPanel instance
     * @return a new BBSStrategyPanel instance
     */
    public JPanel getInstance() {
        return new BBSStrategyPanel();
    }
    /**
     * This method tells the OTB if the BBSPanel
     * can show a popup menu object in the OTB.
     * Returns <i>false</i> for now.
     *
     * @return <i>true</i> - if the panel has a popup menu available,
     * <i>false</i> - if the panel does not have a popup menu available.
     */
    public boolean hasPopupMenu() {
        return false;
    }
    /**
     * Creates a popup menu for this panel in the OTB. Not used.
     */
    public void createPopupMenu(Component aComponent,int x, int y) {
    }
    /**
     * Handles the choice from the popupmenu in the OTB. Not used.
     */
    public void popupMenuHandler(java.awt.event.ActionEvent evt) {
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
     * Restores original Values in this panel, including the step tree
     */
    private void restoreBBSStrategyPanel() {
        this.inputDataText.setText(StrategyInputData.limits);
        this.wdsFrequencyText.setText(StrategyWDSFrequency.limits);
        this.wdsTimeText.setText(StrategyWDSTime.limits);
        wdsFrequencyText.setBackground(Color.WHITE);
        wdsTimeText.setBackground(Color.WHITE);
        
        
        this.integrationFrequencyText.setText(StrategyIntegrationFrequency.limits);
        this.integrationTimeText.setText(StrategyIntegrationTime.limits);
        integrationFrequencyText.setBackground(Color.WHITE);
        integrationTimeText.setBackground(Color.WHITE);
        
        this.correlationSelectionBox.setSelectedItem(StrategyCorrelationSelection.limits);
        LofarUtils.fillSelectionListFromString(correlationTypeList,StrategyCorrelationType.limits,true);
        if(StrategyStations.limits == null || StrategyStations.limits.equals("[]")){
            this.stationsUseAllCheckbox.setSelected(true);
            stationsList.setModel(new DefaultListModel());
        }else{
            this.stationsUseAllCheckbox.setSelected(false);
            LofarUtils.fillList(stationsList,StrategyStations.limits,true);
        }
//        addStationButton.setEnabled(false);
        
        //reload the strategy steps from the OTDB
        BBSStepDataManager.getInstance().generateStrategyFromOTDB();
        this.setupStepTree(StrategySteps);
    }
    /**
     * Adds the save button on the bottom of the form
     */
    
    private void initialize() {
        this.buttonPanel1.addButton("Apply Settings");
        this.stationsList.setModel(new DefaultListModel());
        this.modifyStationsCombobox.removeAllItems();
        this.modifyStationsCombobox.addItem("CS001");
        this.modifyStationsCombobox.addItem("CS008");
        this.modifyStationsCombobox.addItem("CS010");
        this.modifyStationsCombobox.addItem("CS016");
        
    }
     /**
     * Initialization method to be implemented when 
     * User Access is completely implemented in the OTB.
     */
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
    /**
     * Sets the different fields in the GUI, using the names of the nodes provided
     * @param parent the parent node of the node to be displayed
     * @param aParam the parameter of the node to be displayed if applicable
     * @param aNode  the node to be displayed
     */
    private void setField(jOTDBnode parent,jOTDBparam aParam, jOTDBnode aNode) {
        // OLAP_HW settings
        if (aParam==null) {
            return;
        }
        boolean isRef = LofarUtils.isReference(aNode.limits);
        String aKeyName = LofarUtils.keyName(aNode.name);
        String parentName = String.valueOf(parent.name);
        
        if(parentName.equals("Strategy")){
            //Setup step tree
            this.setupStepTree(parent);
            this.setupStepsList(BBSStepDataManager.getInstance().getStepNames());
            
            if (aKeyName.equals("InputData")) {
                this.inputDataText.setToolTipText(aParam.description);
                this.StrategyInputData=aNode;
                
                if (isRef && aParam != null) {
                    inputDataText.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputDataText.setText(aNode.limits);
                }
            }else if (aKeyName.equals("Stations")) {
                this.stationsList.setToolTipText(aParam.description);
                this.StrategyStations = aNode;
                
                //set the checkbox correctly when no stations are provided in the data
                if(StrategyStations.limits == null || StrategyStations.limits.equals("[]")){
                    this.stationsUseAllCheckbox.setSelected(true);
                    stationsList.setModel(new DefaultListModel());
                }else{
                    this.stationsUseAllCheckbox.setSelected(false);
                    TitledBorder aBorder = (TitledBorder)this.stationsPanel.getBorder();
                    if (isRef && aParam != null) {
                        aBorder.setTitle("Station Names (Referenced)");
                        LofarUtils.fillList(stationsList,aParam.limits,false);
                    } else {
                        aBorder.setTitle("Station Names");
                        LofarUtils.fillList(stationsList,aNode.limits,true);
                    }
                }
            }
        } else if(parentName.equals("WorkDomainSize")){
            if (aKeyName.equals("Freq")) {
                this.wdsFrequencyText.setToolTipText(aParam.description);
                this.StrategyWDSFrequency=aNode;
                
                if (isRef && aParam != null) {
                    wdsFrequencyText.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    wdsFrequencyText.setText(aNode.limits);
                }
            } else if (aKeyName.equals("Time")) {
                this.wdsTimeText.setToolTipText(aParam.description);
                this.StrategyWDSTime=aNode;
                
                if (isRef && aParam != null) {
                    wdsTimeText.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    wdsTimeText.setText(aNode.limits);
                }
            }
        } else if(parentName.equals("Integration")){
            if (aKeyName.equals("Freq")) {
                this.integrationFrequencyText.setToolTipText(aParam.description);
                this.StrategyIntegrationFrequency=aNode;
         
                if (isRef && aParam != null) {
                    integrationFrequencyText.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    integrationFrequencyText.setText(aNode.limits);
                }
            } else if (aKeyName.equals("Time")) {
                this.integrationTimeText.setToolTipText(aParam.description);
                this.StrategyIntegrationTime=aNode;
         
                if (isRef && aParam != null) {
                    integrationTimeText.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    integrationTimeText.setText(aNode.limits);
                }
            }
        } else if(parentName.equals("Correlation")){
            if (aKeyName.equals("Selection")) {
                this.correlationSelectionBox.setToolTipText(aParam.description);
                this.StrategyCorrelationSelection=aNode;
                this.correlationSelectionBox.setSelectedItem(aNode.limits);
                logger.trace("Correlation selection will be :"+this.correlationSelectionBox.getSelectedItem().toString());
            } else if (aKeyName.equals("Type")) {
                this.correlationTypeList.setToolTipText(aParam.description);
                this.StrategyCorrelationType=aNode;
                if (isRef && aParam != null) {
                    LofarUtils.fillSelectionListFromString(correlationTypeList,aParam.limits,false);
                } else {
                    LofarUtils.fillSelectionListFromString(correlationTypeList,aNode.limits,true);
                }
            }
        }
    }
    /** 
     * Saves the given node back to the OTDB database
     * @param aNode the node to be saved
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
    
    /** Enables/disables the buttons (unused)
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
    }
    
    /** Sets the buttons visible/invisible (unused)
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
    }
    /** Enables/disables the complete form (unused)
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
    }
    /**
     * Saves all BBS Strategy configuration nodes in the OTDB, should one or more have changed.
     */
    private void saveInput() {
        
        if (this.StrategyInputData != null && !this.inputDataText.getText().equals(StrategyInputData.limits)) {
            StrategyInputData.limits = inputDataText.getText();
            logger.trace("Variable BBS Strategy ("+StrategyInputData.name+"//"+StrategyInputData.treeID()+"//"+StrategyInputData.nodeID()+"//"+StrategyInputData.parentID()+"//"+StrategyInputData.paramDefID()+") from value ("+inputDataText.getText()+") updated to :"+StrategyInputData.limits);
            saveNode(StrategyInputData);
        }
        if (this.StrategyWDSFrequency != null && !this.wdsFrequencyText.getText().equals(StrategyWDSFrequency.limits)) {
            StrategyWDSFrequency.limits = wdsFrequencyText.getText();
            logger.trace("Variable BBS Strategy ("+StrategyWDSFrequency.name+"//"+StrategyWDSFrequency.treeID()+"//"+StrategyWDSFrequency.nodeID()+"//"+StrategyWDSFrequency.parentID()+"//"+StrategyWDSFrequency.paramDefID()+") updated to :"+StrategyWDSFrequency.limits);
            saveNode(StrategyWDSFrequency);
        }
        if (this.StrategyWDSTime != null && !this.wdsTimeText.getText().equals(StrategyWDSTime.limits)) {
            StrategyWDSTime.limits = wdsTimeText.getText();
            logger.trace("Variable BBS Strategy ("+StrategyWDSTime.name+"//"+StrategyWDSTime.treeID()+"//"+StrategyWDSTime.nodeID()+"//"+StrategyWDSTime.parentID()+"//"+StrategyWDSTime.paramDefID()+") updated to :"+StrategyWDSTime.limits);
            saveNode(StrategyWDSTime);
        }
        if (this.StrategyIntegrationFrequency != null && !this.integrationFrequencyText.getText().equals(StrategyIntegrationFrequency.limits)) {
            StrategyIntegrationFrequency.limits = integrationFrequencyText.getText();
            logger.trace("Variable BBS Strategy ("+StrategyIntegrationFrequency.name+"//"+StrategyIntegrationFrequency.treeID()+"//"+StrategyIntegrationFrequency.nodeID()+"//"+StrategyIntegrationFrequency.parentID()+"//"+StrategyIntegrationFrequency.paramDefID()+") updated to :"+StrategyIntegrationFrequency.limits);
            saveNode(StrategyIntegrationFrequency);
        }
        if (this.StrategyIntegrationTime != null && !this.integrationTimeText.getText().equals(StrategyIntegrationTime.limits)) {
            StrategyIntegrationTime.limits = integrationTimeText.getText();
            logger.trace("Variable BBS Strategy ("+StrategyIntegrationTime.name+"//"+StrategyIntegrationTime.treeID()+"//"+StrategyIntegrationTime.nodeID()+"//"+StrategyIntegrationTime.parentID()+"//"+StrategyIntegrationTime.paramDefID()+") updated to :"+StrategyIntegrationTime.limits);
            saveNode(StrategyIntegrationTime);
        }
        if (this.StrategyCorrelationSelection != null && !this.correlationSelectionBox.getSelectedItem().toString().equals(StrategyCorrelationSelection.limits)) {
            StrategyCorrelationSelection.limits = correlationSelectionBox.getSelectedItem().toString();
            logger.trace("Variable BBS Strategy ("+StrategyCorrelationSelection.name+"//"+StrategyCorrelationSelection.treeID()+"//"+StrategyCorrelationSelection.nodeID()+"//"+StrategyCorrelationSelection.parentID()+"//"+StrategyCorrelationSelection.paramDefID()+") updating to :"+StrategyCorrelationSelection.limits);
            saveNode(StrategyCorrelationSelection);
        }
        if (this.StrategyCorrelationType != null && !LofarUtils.createStringFromSelectionList(correlationTypeList,true).equals(StrategyCorrelationType.limits)) {
            StrategyCorrelationType.limits = LofarUtils.createStringFromSelectionList(correlationTypeList,true);
            logger.trace("Variable BBS Strategy ("+StrategyCorrelationType.name+"//"+StrategyCorrelationType.treeID()+"//"+StrategyCorrelationType.nodeID()+"//"+StrategyCorrelationType.parentID()+"//"+StrategyCorrelationType.paramDefID()+") updating to :"+StrategyCorrelationType.limits);
            saveNode(StrategyCorrelationType);
        }
        //clear the stations list if the use all stations checkbox is selected
        if(this.stationsUseAllCheckbox.isSelected()){
            stationsList.setModel(new DefaultListModel());
        }
        if (this.StrategyStations != null && !LofarUtils.createList(stationsList,true).equals(StrategyStations.limits)) {
            StrategyStations.limits = LofarUtils.createList(stationsList,true);
            logger.trace("Variable BBS Strategy ("+StrategyStations.name+"//"+StrategyStations.treeID()+"//"+StrategyStations.nodeID()+"//"+StrategyStations.parentID()+"//"+StrategyStations.paramDefID()+") updating to :"+StrategyStations.limits);
            saveNode(StrategyStations);
        }
    }
        
            
    /**
     * This helper method builds up the Step Tree that contains steps associated with the BBS Strategy
     *
     * @param strategyRootNode the BBS Strategy node needed to build the step tree
     */
    private void setupStepTree(jOTDBnode strategyRootNode){
        try {
            //Add steps that make up the strategy to the steps tree browser
            //fetch the BBS root Container node, which is the parent of the BBS Strategy node given in strategyRootNode
            Vector steps = OtdbRmi.getRemoteMaintenance().getItemList(strategyRootNode.treeID(), strategyRootNode.parentID(), 1);
            // get all the params per child
            Enumeration se = steps.elements();
            while( se.hasMoreElements()  ) {
                jOTDBnode aNode2 = (jOTDBnode)se.nextElement();
                
                if (aNode2.leaf) {
                //retrieve the BBS Step Container node, which holds all steps (BBS.Step)
                }else if (LofarUtils.keyName(aNode2.name).equals("Step")) {
                    //Add steps to tree
                    Object[] rootNodeArgs = new Object[3];
                    rootNodeArgs[0]= new String("Strategy Steps");
                    rootNodeArgs[1]=aNode2;
                    TreeNode newStepRootNode = BBSStepTreeManager.getInstance(itsMainFrame.getUserAccount()).getRootNode(rootNodeArgs);
                    this.stepsTreePanel.newRootNode(newStepRootNode);
                }
            }
            StrategySteps = strategyRootNode;
            this.stepsTreePanel.getTree().setSelectionRow(0);
            this.expandAll(this.stepsTreePanel.getTree(),true);
            this.stepsTreePanel.getTree().setSelectionRow(0);
            
        } catch (RemoteException ex) {
            logger.error("Strategy Step Tree could not be built.",ex);
        }
    }
    /**
     * Adds existings steps to the existing Step Combobox in the Step Tree sub-Panel.
     *
     * @param items the String items to be added to the combobox.
     */
    private void setupStepsList(Vector<String> items){
        DefaultComboBoxModel itsModel = new DefaultComboBoxModel();
        stepsList.setModel(itsModel);
        for(String anItem : items){
            itsModel.addElement(anItem);
        }
        stepsList.setModel(itsModel);
        if(items.size()<1){
            stepsList.setEnabled(false);
            this.addExistingStepButton.setEnabled(false);
        }else{
            stepsList.setEnabled(true);
            this.addExistingStepButton.setEnabled(true);
        }
    }
    
    /**
     * Helper method that is used to fully expand the step tree
     *
     * If expand is true, it expands all nodes in the tree.
     * Otherwise, collapses all nodes in the tree.
     *
     * @param tree The JTree to expand/collapse
     * @param expand true/false expand/collapse the tree.
     *
     */
   
    private void expandAll(JTree tree, boolean expand) {
        TreeNode root = (TreeNode)tree.getModel().getRoot();
        
        // Traverse tree from root
        expandAll(tree, new TreePath(root), expand);
    }
    /**
     * Helper method that recursively expands a given TreePath
     *
     * @param tree The JTree to expand/collapse
     * @param parent the TreePath to expand
     * @param expand true/false expand/collapse the treepath.
     */
    private void expandAll(JTree tree, TreePath parent, boolean expand) {
        // Traverse children
        TreeNode node = (TreeNode)parent.getLastPathComponent();
        if (node.getChildCount() >= 0) {
            for (Enumeration e=node.children(); e.hasMoreElements(); ) {
                TreeNode n = (TreeNode)e.nextElement();
                TreePath path = parent.pathByAddingChild(n);
                expandAll(tree, path, expand);
            }
        }
        
        // Expansion or collapse must be done bottom-up
        if (expand) {
            tree.expandPath(parent);
        } else {
            tree.collapsePath(parent);
        }
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {
        java.awt.GridBagConstraints gridBagConstraints;

        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();
        strategyScrollPane = new javax.swing.JScrollPane();
        strategyPanel = new javax.swing.JPanel();
        inputDataLabel = new javax.swing.JLabel();
        inputDataText = new javax.swing.JTextField();
        strategyRevertButton = new javax.swing.JButton();
        stationsPanel = new javax.swing.JPanel();
        stationsScrollPane = new javax.swing.JScrollPane();
        stationsList = new javax.swing.JList();
        stationsModPanel = new javax.swing.JPanel();
        stationsButtonPanel = new javax.swing.JPanel();
        deleteStationButton = new javax.swing.JButton();
        addStationButton = new javax.swing.JButton();
        modifyStationsCombobox = new javax.swing.JComboBox();
        stationsUseAllCheckbox = new javax.swing.JCheckBox();
        stepsPanel = new javax.swing.JPanel();
        stepsModsPanel = new javax.swing.JPanel();
        addStepButton = new javax.swing.JButton();
        modifyStepButton = new javax.swing.JButton();
        removeStepButton = new javax.swing.JButton();
        existingStepAddPanel = new javax.swing.JPanel();
        stepsList = new javax.swing.JComboBox();
        addExistingStepButton = new javax.swing.JButton();
        stepsTreePanel = new nl.astron.lofar.sas.otbcomponents.TreePanel();
        stepMovePanel = new javax.swing.JPanel();
        stepMoveUpButton = new javax.swing.JButton();
        stepMoveDownButton = new javax.swing.JButton();
        correlationPanel = new javax.swing.JPanel();
        correlationSelectionLabel = new javax.swing.JLabel();
        correlationSelectionBox = new javax.swing.JComboBox();
        correlationTypeLabel = new javax.swing.JLabel();
        correlationTypeScrollPane = new javax.swing.JScrollPane();
        correlationTypeList = new javax.swing.JList();
        workDomainSizePanel = new javax.swing.JPanel();
        wdsFrequencyLabel = new javax.swing.JLabel();
        wdsFrequencyText = new javax.swing.JTextField();
        wdsFrequencyUnitLabel = new javax.swing.JLabel();
        wdsTimeLabel = new javax.swing.JLabel();
        wdsTimeText = new javax.swing.JTextField();
        wdsTimeUnitLabel = new javax.swing.JLabel();
        integrationIntervalPanel = new javax.swing.JPanel();
        integrationFrequencyLabel = new javax.swing.JLabel();
        integrationFrequencyText = new javax.swing.JTextField();
        integrationFrequencyUnitLabel = new javax.swing.JLabel();
        integrationTimeLabel = new javax.swing.JLabel();
        integrationTimeText = new javax.swing.JTextField();
        integrationTimeUnitLabel = new javax.swing.JLabel();
        jLabel1 = new javax.swing.JLabel();

        setLayout(new java.awt.BorderLayout());

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });
        add(buttonPanel1, java.awt.BorderLayout.SOUTH);

        strategyPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        inputDataLabel.setText("Input Data Column:");
        strategyPanel.add(inputDataLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 30, -1, -1));

        inputDataText.setToolTipText("Name of the column in the measurement set that contains the input data");
        strategyPanel.add(inputDataText, new org.netbeans.lib.awtextra.AbsoluteConstraints(160, 30, 170, -1));

        strategyRevertButton.setText("Revert");
        strategyRevertButton.setToolTipText("Reverts the values shown in this screen to the values present in the database. WARNING: This also rebuilds the step tree and ignores any changes made to it!");
        strategyRevertButton.setMaximumSize(new java.awt.Dimension(100, 25));
        strategyRevertButton.setMinimumSize(new java.awt.Dimension(100, 25));
        strategyRevertButton.setPreferredSize(new java.awt.Dimension(100, 25));
        strategyRevertButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                strategyRevertButtonActionPerformed(evt);
            }
        });
        strategyPanel.add(strategyRevertButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 520, -1, -1));

        stationsPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Station Names"));
        stationsPanel.setToolTipText("Identifiers of the participating stations.");
        stationsPanel.setLayout(new java.awt.BorderLayout());

        stationsList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "1", "2", "3", "4", "5" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        stationsList.setToolTipText("Names of the participating stations.");
        stationsList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent evt) {
                stationsListValueChanged(evt);
            }
        });
        stationsScrollPane.setViewportView(stationsList);

        stationsPanel.add(stationsScrollPane, java.awt.BorderLayout.CENTER);

        stationsModPanel.setLayout(new java.awt.BorderLayout());

        stationsButtonPanel.setLayout(new java.awt.GridBagLayout());

        deleteStationButton.setText("Delete");
        deleteStationButton.setToolTipText("Remove the selected Station from the list");
        deleteStationButton.setEnabled(false);
        deleteStationButton.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteStationButton.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteStationButton.setPreferredSize(new java.awt.Dimension(65, 23));
        deleteStationButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                deleteStationButtonActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stationsButtonPanel.add(deleteStationButton, gridBagConstraints);

        addStationButton.setText("Add");
        addStationButton.setToolTipText("Add the station entered to the list");
        addStationButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addStationButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addStationButton.setPreferredSize(new java.awt.Dimension(60, 23));
        addStationButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addStationButtonActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stationsButtonPanel.add(addStationButton, gridBagConstraints);

        stationsModPanel.add(stationsButtonPanel, java.awt.BorderLayout.SOUTH);

        modifyStationsCombobox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        stationsModPanel.add(modifyStationsCombobox, java.awt.BorderLayout.CENTER);

        stationsPanel.add(stationsModPanel, java.awt.BorderLayout.SOUTH);

        stationsUseAllCheckbox.setText("Use all stations");
        stationsUseAllCheckbox.setToolTipText("Check this box to use all stations available in the observation");
        stationsUseAllCheckbox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        stationsUseAllCheckbox.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        stationsUseAllCheckbox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        stationsUseAllCheckbox.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                stationsUseAllCheckboxStateChanged(evt);
            }
        });
        stationsPanel.add(stationsUseAllCheckbox, java.awt.BorderLayout.NORTH);

        strategyPanel.add(stationsPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 150, 220, 330));

        stepsPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Steps"));
        stepsPanel.setToolTipText("");
        stepsPanel.setPreferredSize(new java.awt.Dimension(100, 100));
        stepsPanel.setLayout(new java.awt.BorderLayout());

        stepsModsPanel.setMinimumSize(new java.awt.Dimension(100, 40));
        stepsModsPanel.setPreferredSize(new java.awt.Dimension(100, 40));

        addStepButton.setText("Add");
        addStepButton.setToolTipText("Add a new Step to the structure");
        addStepButton.setEnabled(false);
        addStepButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addStepButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addStepButton.setPreferredSize(new java.awt.Dimension(60, 23));
        addStepButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addStepButtonActionPerformed(evt);
            }
        });

        modifyStepButton.setText("Edit");
        modifyStepButton.setToolTipText("Edit the selected step");
        modifyStepButton.setEnabled(false);
        modifyStepButton.setMaximumSize(new java.awt.Dimension(30, 25));
        modifyStepButton.setMinimumSize(new java.awt.Dimension(30, 25));
        modifyStepButton.setPreferredSize(new java.awt.Dimension(50, 23));
        modifyStepButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                modifyStepButtonActionPerformed(evt);
            }
        });

        removeStepButton.setText("Delete");
        removeStepButton.setToolTipText("Delete the selected step");
        removeStepButton.setEnabled(false);
        removeStepButton.setMaximumSize(new java.awt.Dimension(30, 25));
        removeStepButton.setMinimumSize(new java.awt.Dimension(30, 25));
        removeStepButton.setPreferredSize(new java.awt.Dimension(65, 23));
        removeStepButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                removeStepButtonActionPerformed(evt);
            }
        });

        existingStepAddPanel.setBorder(javax.swing.BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.RAISED));
        existingStepAddPanel.setLayout(new java.awt.GridBagLayout());

        stepsList.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        stepsList.setToolTipText("This box shows the already defined steps");
        stepsList.setEnabled(false);
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 10, 2, 2);
        existingStepAddPanel.add(stepsList, gridBagConstraints);

        addExistingStepButton.setText("Add");
        addExistingStepButton.setToolTipText("Add the selected existing step to the structure");
        addExistingStepButton.setEnabled(false);
        addExistingStepButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addExistingStepButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addExistingStepButton.setPreferredSize(new java.awt.Dimension(60, 23));
        addExistingStepButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addExistingStepButtonActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 10);
        existingStepAddPanel.add(addExistingStepButton, gridBagConstraints);

        org.jdesktop.layout.GroupLayout stepsModsPanelLayout = new org.jdesktop.layout.GroupLayout(stepsModsPanel);
        stepsModsPanel.setLayout(stepsModsPanelLayout);
        stepsModsPanelLayout.setHorizontalGroup(
            stepsModsPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(stepsModsPanelLayout.createSequentialGroup()
                .addContainerGap()
                .add(addStepButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(modifyStepButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(removeStepButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                .add(existingStepAddPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(33, 33, 33))
        );
        stepsModsPanelLayout.setVerticalGroup(
            stepsModsPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(stepsModsPanelLayout.createSequentialGroup()
                .add(stepsModsPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(stepsModsPanelLayout.createSequentialGroup()
                        .add(8, 8, 8)
                        .add(addStepButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(stepsModsPanelLayout.createSequentialGroup()
                        .add(8, 8, 8)
                        .add(stepsModsPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                            .add(modifyStepButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(removeStepButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            .add(existingStepAddPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 42, Short.MAX_VALUE)
        );

        stepsPanel.add(stepsModsPanel, java.awt.BorderLayout.SOUTH);

        stepsTreePanel.setTitle("Step Structure");
        stepsTreePanel.addTreeSelectionListener(new javax.swing.event.TreeSelectionListener() {
            public void valueChanged(javax.swing.event.TreeSelectionEvent evt) {
                stepsTreePanelValueChanged(evt);
            }
        });
        stepsPanel.add(stepsTreePanel, java.awt.BorderLayout.CENTER);

        stepMovePanel.setLayout(new java.awt.GridBagLayout());

        stepMoveUpButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/navigation/Up16.gif"))); // NOI18N
        stepMoveUpButton.setToolTipText("Move this step up in its parent collection");
        stepMoveUpButton.setEnabled(false);
        stepMoveUpButton.setMaximumSize(new java.awt.Dimension(30, 30));
        stepMoveUpButton.setMinimumSize(new java.awt.Dimension(30, 30));
        stepMoveUpButton.setPreferredSize(new java.awt.Dimension(30, 30));
        stepMoveUpButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                stepMoveUpButtonActionPerformed(evt);
            }
        });
        stepMovePanel.add(stepMoveUpButton, new java.awt.GridBagConstraints());

        stepMoveDownButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/navigation/Down16.gif"))); // NOI18N
        stepMoveDownButton.setToolTipText("Move this step down in its parent collection");
        stepMoveDownButton.setEnabled(false);
        stepMoveDownButton.setMaximumSize(new java.awt.Dimension(30, 30));
        stepMoveDownButton.setMinimumSize(new java.awt.Dimension(30, 30));
        stepMoveDownButton.setPreferredSize(new java.awt.Dimension(30, 30));
        stepMoveDownButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                stepMoveDownButtonActionPerformed(evt);
            }
        });
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        stepMovePanel.add(stepMoveDownButton, gridBagConstraints);

        stepsPanel.add(stepMovePanel, java.awt.BorderLayout.EAST);

        strategyPanel.add(stepsPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(250, 150, 400, 330));

        correlationPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Correlation"));
        correlationPanel.setToolTipText("Specifies which station correlations to use");
        correlationPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        correlationSelectionLabel.setText("Selection :");
        correlationPanel.add(correlationSelectionLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 40, -1, -1));

        correlationSelectionBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "AUTO", "CROSS", "ALL" }));
        correlationSelectionBox.setToolTipText("Specifies which station correlations to use");
        correlationPanel.add(correlationSelectionBox, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 60, 80, -1));

        correlationTypeLabel.setText("Type :");
        correlationPanel.add(correlationTypeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 20, -1, -1));

        correlationTypeList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "XX", "XY", "YX", "YY" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        correlationTypeList.setToolTipText("Specifies which station correlations to use");
        correlationTypeScrollPane.setViewportView(correlationTypeList);

        correlationPanel.add(correlationTypeScrollPane, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 40, 40, 80));

        strategyPanel.add(correlationPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(480, 10, 170, 130));

        workDomainSizePanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Work Domain Size"));
        workDomainSizePanel.setToolTipText("Size of the work domain in frequency and time. A work domain represents an amount of input data that is loaded into memory and processed as a single block. A large work domain size should reduce the overhead due to disk access.");
        workDomainSizePanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        wdsFrequencyLabel.setText("Frequency :");
        workDomainSizePanel.add(wdsFrequencyLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 20, -1, -1));

        wdsFrequencyText.setToolTipText("Size of the work domain in frequency");
        workDomainSizePanel.add(wdsFrequencyText, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 20, 80, -1));

        wdsFrequencyUnitLabel.setText("Hz");
        workDomainSizePanel.add(wdsFrequencyUnitLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 20, -1, -1));

        wdsTimeLabel.setText("Time :");
        workDomainSizePanel.add(wdsTimeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 50, -1, -1));

        wdsTimeText.setToolTipText("Size of the work work domain in time");
        workDomainSizePanel.add(wdsTimeText, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 50, 80, -1));

        wdsTimeUnitLabel.setText("s");
        workDomainSizePanel.add(wdsTimeUnitLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 50, 10, -1));

        strategyPanel.add(workDomainSizePanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 60, 220, 80));

        integrationIntervalPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Integration", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), java.awt.Color.lightGray));
        integrationIntervalPanel.setToolTipText("Cell size for integration. Not yet implemented.");
        integrationIntervalPanel.setEnabled(false);
        integrationIntervalPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        integrationFrequencyLabel.setText("Freq. Interval :");
        integrationFrequencyLabel.setEnabled(false);
        integrationIntervalPanel.add(integrationFrequencyLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, -1, -1));

        integrationFrequencyText.setEditable(false);
        integrationFrequencyText.setToolTipText("Frequency Interval in Hz");
        integrationFrequencyText.setEnabled(false);
        integrationIntervalPanel.add(integrationFrequencyText, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 20, 70, -1));

        integrationFrequencyUnitLabel.setText("Hz");
        integrationFrequencyUnitLabel.setEnabled(false);
        integrationIntervalPanel.add(integrationFrequencyUnitLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 20, -1, -1));

        integrationTimeLabel.setText("Time Interval :");
        integrationTimeLabel.setEnabled(false);
        integrationIntervalPanel.add(integrationTimeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 50, -1, -1));

        integrationTimeText.setEditable(false);
        integrationTimeText.setToolTipText("Time interval in seconds");
        integrationTimeText.setEnabled(false);
        integrationIntervalPanel.add(integrationTimeText, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 50, 70, -1));

        integrationTimeUnitLabel.setText("s");
        integrationTimeUnitLabel.setEnabled(false);
        integrationIntervalPanel.add(integrationTimeUnitLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 50, 10, -1));

        strategyPanel.add(integrationIntervalPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(250, 60, 220, 80));

        jLabel1.setFont(new java.awt.Font("Dialog", 1, 10));
        jLabel1.setForeground(java.awt.Color.gray);
        jLabel1.setText("Remember to press 'Apply Settings' if you like to save the strategy and its steps tree before you leave this screen.");
        strategyPanel.add(jLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 490, -1, -1));

        strategyScrollPane.setViewportView(strategyPanel);

        add(strategyScrollPane, java.awt.BorderLayout.CENTER);
    }// </editor-fold>//GEN-END:initComponents
    
    private void stepMoveDownButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_stepMoveDownButtonActionPerformed
        TreePath selectedPath = this.stepsTreePanel.getTree().getSelectionPath();
        if(selectedPath != null){
            TreeNode someBBSStepNode = (TreeNode)selectedPath.getLastPathComponent();
            BBSStep theStep = ((BBSStepNode)someBBSStepNode.getUserObject()).getContainedStep();
            int currentIndex = someBBSStepNode.getParent().getIndex(someBBSStepNode);
            if(theStep.hasParentStep()){
                BBSStepDataManager.getInstance().getStrategy().cascadingStepMove(theStep.getParentStep(),theStep,currentIndex,currentIndex+1);
                BBSStepDataManager.getInstance().assureBBSStepIsInCollection(theStep);
            }else{
                BBSStepDataManager.getInstance().getStrategy().cascadingStepMove(null,theStep,currentIndex,currentIndex+1);
            }
        }
        this.setupStepTree(StrategySteps);
    }//GEN-LAST:event_stepMoveDownButtonActionPerformed
    
    private void stepMoveUpButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_stepMoveUpButtonActionPerformed
        TreePath selectedPath = this.stepsTreePanel.getTree().getSelectionPath();
        if(selectedPath != null){
            TreeNode someBBSStepNode = (TreeNode)selectedPath.getLastPathComponent();
            BBSStep theStep = ((BBSStepNode)someBBSStepNode.getUserObject()).getContainedStep();
            int currentIndex = someBBSStepNode.getParent().getIndex(someBBSStepNode);
            if(theStep.hasParentStep()){
                BBSStepDataManager.getInstance().getStrategy().cascadingStepMove(theStep.getParentStep(),theStep,currentIndex,currentIndex-1);
                BBSStepDataManager.getInstance().assureBBSStepIsInCollection(theStep);
            }else{
                BBSStepDataManager.getInstance().getStrategy().cascadingStepMove(null,theStep,currentIndex,currentIndex-1);
            }
        }
        this.setupStepTree(StrategySteps);
    }//GEN-LAST:event_stepMoveUpButtonActionPerformed
    
    private void addExistingStepButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addExistingStepButtonActionPerformed
        TreePath selectedPath = this.stepsTreePanel.getTree().getSelectionPath();
        if(selectedPath != null){
            TreeNode someBBSStepNode = (TreeNode)selectedPath.getLastPathComponent();
            logger.trace("BBS Step Node to be supplied with child : "+someBBSStepNode.getName());
            if(!someBBSStepNode.getName().equals("Strategy Steps")){
                BBSStep theStep = ((BBSStepNode)someBBSStepNode.getUserObject()).getContainedStep();
                BBSStep newStep = BBSStepDataManager.getInstance().getBBSStep(theStep,stepsList.getSelectedItem().toString());
                BBSStepDataManager.getInstance().getStrategy().cascadingStepInsertion(theStep.getName(),newStep);
                BBSStepDataManager.getInstance().assureBBSStepIsInCollection(theStep);
                
            }else{
                BBSStep newStep = BBSStepDataManager.getInstance().getBBSStep(null,stepsList.getSelectedItem().toString());
                BBSStepDataManager.getInstance().getStrategy().cascadingStepInsertion(null,newStep);
            }
            this.setupStepTree(StrategySteps);
            this.setupStepsList(BBSStepDataManager.getInstance().getStepNames());
        }
    }//GEN-LAST:event_addExistingStepButtonActionPerformed
    
    private void removeStepButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_removeStepButtonActionPerformed
        TreePath selectedPath = this.stepsTreePanel.getTree().getSelectionPath();
        if(selectedPath != null){
            TreeNode someBBSStepNode = (TreeNode)selectedPath.getLastPathComponent();
            logger.trace("BBS Step to be deleted : "+someBBSStepNode.getName());
            
            
            if(someBBSStepNode.getName().equals("Strategy Steps")){
                
                String message = "Are you sure you want to delete all the steps from the BBS strategy?";
                String[] buttons = {"Yes","No"};
                int choice =  JOptionPane.showOptionDialog(this,message, "Please confirm", JOptionPane.DEFAULT_OPTION, JOptionPane.QUESTION_MESSAGE, null,buttons,buttons[0]);
                if(choice == 0){
                    //delete all steps
                    //BBSStepDataManager.getInstance().deleteAllSteps();
                    BBSStrategy theStrategy = BBSStepDataManager.getInstance().getStrategy();
                    theStrategy.removeAllChildSteps();
                    
                    this.setupStepTree(StrategySteps);
                    this.setupStepsList(BBSStepDataManager.getInstance().getStepNames());
                }
            } else if(!someBBSStepNode.getName().equals("Strategy Steps")){
                
                BBSStep theStep = ((BBSStepNode)someBBSStepNode.getUserObject()).getContainedStep();
                
                String message = "Are you sure you want to delete this step?";
                String[] buttons = {"Yes","No"};
                int choice =  JOptionPane.showOptionDialog(this,message, "Please confirm", JOptionPane.DEFAULT_OPTION, JOptionPane.QUESTION_MESSAGE, null,buttons,buttons[0]);
                if(choice == 0){
                    int currentIndex = someBBSStepNode.getParent().getIndex(someBBSStepNode);
                    //delete step from DB
                    if(theStep.getParentStep() != null){
                        BBSStepDataManager.getInstance().getStrategy().cascadingStepDeletion(theStep.getParentStep(),theStep,currentIndex);
                        BBSStepDataManager.getInstance().assureBBSStepIsInCollection(theStep);
                    }else{
                        BBSStepDataManager.getInstance().getStrategy().cascadingStepDeletion(null,theStep,currentIndex);
                    }
                    
                    this.setupStepTree(StrategySteps);
                    this.setupStepsList(BBSStepDataManager.getInstance().getStepNames());
                }
            }
        }
    }//GEN-LAST:event_removeStepButtonActionPerformed
    
    private void modifyStepButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_modifyStepButtonActionPerformed
        TreePath selectedPath = this.stepsTreePanel.getTree().getSelectionPath();
        if(selectedPath != null){
            TreeNode someBBSStepNode = (TreeNode)selectedPath.getLastPathComponent();
            logger.trace("BBS Step to be modified : "+someBBSStepNode.getName());
            if(!someBBSStepNode.getName().equals("Strategy Steps")){
                BBSStep theStep = ((BBSStepNode)someBBSStepNode.getUserObject()).getContainedStep();
                BBSStepInputDialog bbsStepDialog = new BBSStepInputDialog(this.itsMainFrame,true,theStep,null);
                bbsStepDialog.setTitle("Step Explorer - "+theStep.getName());
                bbsStepDialog.setVisible(true);
            }
            this.setupStepTree(StrategySteps);
            
        }
    }//GEN-LAST:event_modifyStepButtonActionPerformed
    
    private void stepsTreePanelValueChanged(javax.swing.event.TreeSelectionEvent evt) {//GEN-FIRST:event_stepsTreePanelValueChanged
        TreePath selectedPath = this.stepsTreePanel.getTree().getSelectionPath();
        if(selectedPath != null){
            TreeNode someBBSStepNode = (TreeNode)selectedPath.getLastPathComponent();
            //check if the root node was selected, which should not be editable
            if(!someBBSStepNode.getName().equals("Strategy Steps")){
                this.modifyStepButton.setEnabled(true);
                this.addStepButton.setEnabled(true);
                this.removeStepButton.setEnabled(true);
                //show a somewhat (not completely) limited list of steps that can be added to prevent infinite loops
                //remove all steps that are part of the step tree all the way to the strategy...
                Vector<String> items = BBSStepDataManager.getInstance().getStepNames();
                
                Object[] treeForThisNode = selectedPath.getPath();
                for(int i = 0; i < treeForThisNode.length; i++){
                    TreeNode aTreeNode = (TreeNode)treeForThisNode[i];
                    BBSStep aBBSStep = ((BBSStepNode)aTreeNode.getUserObject()).getContainedStep();
                    if(aBBSStep!=null){
                        if(items.contains(aBBSStep.getName())){
                            items.remove(aBBSStep.getName().toString());
                            
                            logger.trace("stepsTreePanelValueChanged() - removing "+aBBSStep.getName()+" from the list of add-able BBS steps");
                        }
                    }
                }
                this.setupStepsList(items);
                
                //move step up/down capability
                
                BBSStep theStep = ((BBSStepNode)someBBSStepNode.getUserObject()).getContainedStep();
                this.stepMoveUpButton.setEnabled(false);
                this.stepMoveDownButton.setEnabled(false);
                //step is part of another step
                if(theStep.hasParentStep()){
                    BBSStep parentStep = theStep.getParentStep();
                    int currentIndex = someBBSStepNode.getParent().getIndex(someBBSStepNode);
                    
                    if(currentIndex>=0 && currentIndex<parentStep.getChildSteps().size()-1){
                        this.stepMoveDownButton.setEnabled(true);
                    }
                    if(currentIndex>0 && currentIndex<=parentStep.getChildSteps().size()-1){
                        this.stepMoveUpButton.setEnabled(true);
                    }
                    //step is part of the strategy
                } else {
                    BBSStrategy theStrategy = BBSStepDataManager.getInstance().getStrategy();
                    int currentIndex = someBBSStepNode.getParent().getIndex(someBBSStepNode);
                    
                    if(currentIndex>=0 && currentIndex<theStrategy.getChildSteps().size()-1){
                        this.stepMoveDownButton.setEnabled(true);
                    }
                    if(currentIndex>0 && currentIndex<=theStrategy.getChildSteps().size()-1){
                        this.stepMoveUpButton.setEnabled(true);
                    }
                }
            }else{
                this.addStepButton.setEnabled(true);
                this.modifyStepButton.setEnabled(false);
                if(BBSStepDataManager.getInstance().getStrategy().hasChildSteps()){
                    this.removeStepButton.setEnabled(true);
                }
                this.stepMoveUpButton.setEnabled(false);
                this.stepMoveDownButton.setEnabled(false);
                //show full list of steps that can be added
                Vector<String> items = BBSStepDataManager.getInstance().getStepNames();
                this.setupStepsList(items);
            }
            
        }else{
            this.addStepButton.setEnabled(false);
            this.modifyStepButton.setEnabled(false);
            this.removeStepButton.setEnabled(false);
            this.stepsList.setEnabled(false);
            this.addExistingStepButton.setEnabled(false);
            this.stepMoveUpButton.setEnabled(false);
            this.stepMoveDownButton.setEnabled(false);
            
        }
    }//GEN-LAST:event_stepsTreePanelValueChanged
    
    private void addStepButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addStepButtonActionPerformed
        TreePath selectedPath = this.stepsTreePanel.getTree().getSelectionPath();
        if(selectedPath != null){
            TreeNode someBBSStepNode = (TreeNode)selectedPath.getLastPathComponent();
            logger.trace("BBS Step Node to be supplied with child : "+someBBSStepNode.getName());
            if(!someBBSStepNode.getName().equals("Strategy Steps")){
                BBSStep theStep = ((BBSStepNode)someBBSStepNode.getUserObject()).getContainedStep();
                BBSStepInputDialog bbsStepDialog = new BBSStepInputDialog(this.itsMainFrame,true,null,theStep);
                bbsStepDialog.setVisible(true);
            }else{
                BBSStepInputDialog bbsStepDialog = new BBSStepInputDialog(this.itsMainFrame,true,null,null);
                bbsStepDialog.setVisible(true);
            }
            this.setupStepTree(StrategySteps);
            this.setupStepsList(BBSStepDataManager.getInstance().getStepNames());
        }
    }//GEN-LAST:event_addStepButtonActionPerformed
        
    private void stationsListValueChanged(javax.swing.event.ListSelectionEvent evt) {//GEN-FIRST:event_stationsListValueChanged
        int[] selectedIndices = ((JList)evt.getSource()).getSelectedIndices();
        if(selectedIndices.length>0){
            this.deleteStationButton.setEnabled(true);
        }else{
            this.deleteStationButton.setEnabled(false);
        }
    }//GEN-LAST:event_stationsListValueChanged
    
    private void deleteStationButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteStationButtonActionPerformed
        DefaultListModel theStationModel = (DefaultListModel)stationsList.getModel();
        int[] selectedIndices = stationsList.getSelectedIndices();
        while(selectedIndices.length>0){
            theStationModel.remove(selectedIndices[0]);
            selectedIndices = stationsList.getSelectedIndices();
        }
        if(theStationModel.size()==0){
            this.deleteStationButton.setEnabled(false);
        }
    }//GEN-LAST:event_deleteStationButtonActionPerformed
    
    private void addStationButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addStationButtonActionPerformed
        String toBeAddedStation = (String)this.modifyStationsCombobox.getSelectedItem();
        DefaultListModel theStationModel = (DefaultListModel)stationsList.getModel();
        if(!theStationModel.contains(toBeAddedStation)){
            theStationModel.addElement(toBeAddedStation);
        }
    }//GEN-LAST:event_addStationButtonActionPerformed
    
    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        if(evt.getActionCommand().equals("Apply Settings")) {
            boolean warning = false;
            //perform input validation on the double values in the form
            String integrationTime = this.integrationTimeText.getText();
            String integrationFrequency = this.integrationFrequencyText.getText();
            String wdsTime = this.wdsTimeText.getText();
            String wdsFrequency = this.wdsFrequencyText.getText();
            
            if(!integrationTime.equals("")){
                try {
                    Double itime = Double.parseDouble(integrationTime);
                    integrationTimeText.setBackground(Color.WHITE);
                } catch (NumberFormatException ex) {
                    integrationTimeText.setBackground(Color.RED);
                    warning=true;
                }
            }else{
                integrationTimeText.setBackground(Color.RED);
                warning=true;
            }
            if(!integrationFrequency.equals("")){
                try {
                    Double itime = Double.parseDouble(integrationFrequency);
                    integrationFrequencyText.setBackground(Color.WHITE);
                } catch (NumberFormatException ex) {
                    warning=true;
                    integrationFrequencyText.setBackground(Color.RED);
                }
            }else{
                warning=true;
                integrationFrequencyText.setBackground(Color.RED);
            }
            if(!wdsFrequency.equals("")){
                try {
                    Double itime = Double.parseDouble(wdsFrequency);
                    wdsFrequencyText.setBackground(Color.WHITE);
                } catch (NumberFormatException ex) {
                    
                    wdsFrequencyText.setBackground(Color.RED);
                }
            }else{
                warning=true;
                wdsFrequencyText.setBackground(Color.RED);
            }
            if(!wdsTime.equals("")){
                try {
                    Double itime = Double.parseDouble(wdsTime);
                    wdsTimeText.setBackground(Color.WHITE);
                } catch (NumberFormatException ex) {
                    warning=true;
                    wdsTimeText.setBackground(Color.RED);
                }
            }else{
                warning=true;
                wdsTimeText.setBackground(Color.RED);
            }
            if(!warning){
                itsMainFrame.setHourglassCursor();
                saveInput();
                BBSStepDataManager.getInstance().persistStrategy();
                this.setupStepTree(StrategySteps);
                this.setupStepsList(BBSStepDataManager.getInstance().getStepNames());
                itsMainFrame.setNormalCursor();
            }
            
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    private void stationsUseAllCheckboxStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_stationsUseAllCheckboxStateChanged
        if(this.stationsUseAllCheckbox.isSelected()){
            this.stationsList.setBackground(Color.LIGHT_GRAY);
            this.stationsList.setEnabled(false);
            this.addStationButton.setEnabled(false);
            this.deleteStationButton.setEnabled(false);
            this.modifyStationsCombobox.setEnabled(false);
            
        }else{
            this.stationsList.setBackground(Color.WHITE);
            this.stationsList.setEnabled(true);
            this.addStationButton.setEnabled(true);
            this.modifyStationsCombobox.setEnabled(true);
        }
    }//GEN-LAST:event_stationsUseAllCheckboxStateChanged
    
    private void strategyRevertButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_strategyRevertButtonActionPerformed
        String message = "Are you sure you want to revert all strategy attributes, including the step tree?";
        message+="\n\nThis 'Revert' action will remove all changes you have made in the step tree since the last 'Save Settings' action!";
        String[] buttons = {"Yes","No"};
        int choice =  JOptionPane.showOptionDialog(this,message, "Please confirm", JOptionPane.DEFAULT_OPTION, JOptionPane.QUESTION_MESSAGE, null,buttons,buttons[0]);
        if(choice == 0){
            itsMainFrame.setHourglassCursor();
            this.restoreBBSStrategyPanel();
            itsMainFrame.setNormalCursor();
        }
    }//GEN-LAST:event_strategyRevertButtonActionPerformed
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton addExistingStepButton;
    private javax.swing.JButton addStationButton;
    private javax.swing.JButton addStepButton;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JPanel correlationPanel;
    private javax.swing.JComboBox correlationSelectionBox;
    private javax.swing.JLabel correlationSelectionLabel;
    private javax.swing.JLabel correlationTypeLabel;
    private javax.swing.JList correlationTypeList;
    private javax.swing.JScrollPane correlationTypeScrollPane;
    private javax.swing.JButton deleteStationButton;
    private javax.swing.JPanel existingStepAddPanel;
    private javax.swing.JLabel inputDataLabel;
    private javax.swing.JTextField inputDataText;
    private javax.swing.JLabel integrationFrequencyLabel;
    private javax.swing.JTextField integrationFrequencyText;
    private javax.swing.JLabel integrationFrequencyUnitLabel;
    private javax.swing.JPanel integrationIntervalPanel;
    private javax.swing.JLabel integrationTimeLabel;
    private javax.swing.JTextField integrationTimeText;
    private javax.swing.JLabel integrationTimeUnitLabel;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JComboBox modifyStationsCombobox;
    private javax.swing.JButton modifyStepButton;
    private javax.swing.JButton removeStepButton;
    private javax.swing.JPanel stationsButtonPanel;
    private javax.swing.JList stationsList;
    private javax.swing.JPanel stationsModPanel;
    private javax.swing.JPanel stationsPanel;
    private javax.swing.JScrollPane stationsScrollPane;
    private javax.swing.JCheckBox stationsUseAllCheckbox;
    private javax.swing.JButton stepMoveDownButton;
    private javax.swing.JPanel stepMovePanel;
    private javax.swing.JButton stepMoveUpButton;
    private javax.swing.JComboBox stepsList;
    private javax.swing.JPanel stepsModsPanel;
    private javax.swing.JPanel stepsPanel;
    private nl.astron.lofar.sas.otbcomponents.TreePanel stepsTreePanel;
    private javax.swing.JPanel strategyPanel;
    private javax.swing.JButton strategyRevertButton;
    private javax.swing.JScrollPane strategyScrollPane;
    private javax.swing.JLabel wdsFrequencyLabel;
    private javax.swing.JTextField wdsFrequencyText;
    private javax.swing.JLabel wdsFrequencyUnitLabel;
    private javax.swing.JLabel wdsTimeLabel;
    private javax.swing.JTextField wdsTimeText;
    private javax.swing.JLabel wdsTimeUnitLabel;
    private javax.swing.JPanel workDomainSizePanel;
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
