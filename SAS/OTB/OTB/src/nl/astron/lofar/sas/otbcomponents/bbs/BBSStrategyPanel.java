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
 * Panel for BBS strategy configuration
 * 
 * @author pompert
 * @version $Id$
 * @created 11-07-2006, 13:37
 */
public class BBSStrategyPanel extends javax.swing.JPanel implements IViewPanel{
    
    static Logger logger = Logger.getLogger(BBSStrategyPanel.class);
    static String name = "BBS Strategy";
    
    
    /** Creates new form BeanForm based upon aNode
     *
     * @params  aNode   node to obtain the info from
     *
     */
    public BBSStrategyPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode=aNode;
        initialize();
        initPanel();
    }
    
    /** Creates new form BeanForm */
    public BBSStrategyPanel() {
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
            Vector childs = itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().getItemList(itsNode.treeID(), itsNode.nodeID(), 1);
            
            // get all the params per child
            Enumeration e = childs.elements();
            while( e.hasMoreElements()  ) {
                aParam=null;
                jOTDBnode aNode = (jOTDBnode)e.nextElement();
                
                // We need to keep all the nodes needed by this panel
                // if the node is a leaf we need to get the pointed to value via Param.
                if (aNode.leaf) {
                    aParam = itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().getParam(aNode);
                    setField(aParam,aNode);
                    
                    //we need to get all the childs from the following nodes as well.
                }else if (LofarUtils.keyName(aNode.name).equals("ParmDB")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }else if (LofarUtils.keyName(aNode.name).equals("BBDB")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
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
        return new BBSStrategyPanel();
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
    /** Restore original Values in Detauks panel
     */
    private void retrieveAndDisplayChildDataForNode(jOTDBnode aNode){
        jOTDBparam aParam=null;
        try {
            Vector HWchilds = itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1);
            // get all the params per child
            Enumeration e1 = HWchilds.elements();
            while( e1.hasMoreElements()  ) {
                
                jOTDBnode aHWNode = (jOTDBnode)e1.nextElement();
                aParam=null;
                // We need to keep all the params needed by this panel
                if (aHWNode.leaf) {
                    aParam = itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().getParam(aHWNode);
                }
                setField(aParam,aHWNode);
            }
        } catch (RemoteException ex) {
            logger.debug("Error during retrieveAndDisplayChildDataForNode: "+ ex);
            return;
        }
    }
    
    private void restoreBBSStrategyPanel() {
        /*
        AMCServerHostText.setText(itsAMCServerHost.limits);
        StrategySteps;
        StrategySteps;
        StrategyInputData;
        StrategyCorrelationSelection;
        StrategyCorrelationType;
        StrategyWDSFrequency;
        StrategyWDSTime;
        StrategyIntegrationFrequency;
        StrategyIntegrationTime;
         */
    }
    
    
    private void initialize() {
        this.buttonPanel1.addButton("Save Settings");
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
        String aKeyName = LofarUtils.keyName(aNode.name);
        /*
        if (aKeyName.equals("DataSet")) {
            this.BBSDatasetText.setToolTipText(aParam.description);
            this.dataSet=aNode;
            
            if (isRef && aParam != null) {
                this.BBSDatasetDeRefText.setVisible(true);
                BBSDatasetText.setText(aNode.limits);
                BBSDatasetDeRefText.setText(aParam.limits);
            } else {
                BBSDatasetDeRefText.setVisible(false);
                BBSDatasetDeRefText.setText("");
                BBSDatasetText.setText(aNode.limits);
            }
        }else if (aKeyName.equals("DBName")) {
            this.BBDBDBNameText.setToolTipText(aParam.description);
            this.BBDBDBName=aNode;
            if (isRef && aParam != null) {
                BBDBDBNameText.setText(aNode.limits + " : " + aParam.limits);
            } else {
                BBDBDBNameText.setText(aNode.limits);
            }
        }else if (aKeyName.equals("Host")) {
            this.BBDBHostText.setToolTipText(aParam.description);
            this.BBDBHost=aNode;
            if (isRef && aParam != null) {
                BBDBHostText.setText(aNode.limits + " : " + aParam.limits);
            } else {
                BBDBHostText.setText(aNode.limits);
            }
        }else if (aKeyName.equals("Port")) {
            this.BBDBPortText.setToolTipText(aParam.description);
            this.BBDBPort=aNode;
            if (isRef && aParam != null) {
                BBDBPortText.setText(aNode.limits + " : " + aParam.limits);
            } else {
                BBDBPortText.setText(aNode.limits);
            }
        }else if (aKeyName.equals("UserName")) {
            this.BBDBDBUsernameText.setToolTipText(aParam.description);
            this.BBDBUsername=aNode;
            if (isRef && aParam != null) {
                BBDBDBUsernameText.setText(aNode.limits + " : " + aParam.limits);
            } else {
                BBDBDBUsernameText.setText(aNode.limits);
            }
        }else if (aKeyName.equals("PassWord")) {
            this.BBDBDBPasswordText.setToolTipText(aParam.description);
            this.BBDBPassword=aNode;
            if (isRef && aParam != null) {
                BBDBDBPasswordText.setText(aNode.limits + " : " + aParam.limits);
            } else {
                BBDBDBPasswordText.setText(aNode.limits);
            }
        }else if (aKeyName.equals("Instrument")) {
            this.ParmDBInstrumentText.setToolTipText(aParam.description);
            this.ParmDBInstrument=aNode;
            if (isRef && aParam != null) {
                ParmDBInstrumentText.setText(aNode.limits + " : " + aParam.limits);
            } else {
                ParmDBInstrumentText.setText(aNode.limits);
            }
        }else if (aKeyName.equals("LocalSky")) {
            this.ParmDBLocalSkyText.setToolTipText(aParam.description);
            this.ParmDBLocalSky=aNode;
            if (isRef && aParam != null) {
                ParmDBLocalSkyText.setText(aNode.limits + " : " + aParam.limits);
            } else {
                ParmDBLocalSkyText.setText(aNode.limits);
            }
        }*/
    }
    
    /** saves the given param back to the database
     */
    private void saveNode(jOTDBnode aNode) {
        if (aNode == null) {
            return;
        }
        try {
            itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().saveNode(aNode);
        } catch (RemoteException ex) {
            logger.debug("Error: saveNode failed : " + ex);
        }
    }
    
    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
        this.enableOverviewButtons(enabled);
        this.enableDetailButtons(enabled);
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
        this.setOverviewButtonsVisible(visible);
        this.setDetailsButtonsVisible(visible);
    }
    private void enableOverviewButtons(boolean enabled) {
        this.strategyRevertButton.setEnabled(enabled);
    }
    
    private void setOverviewButtonsVisible(boolean visible) {
        this.strategyRevertButton.setVisible(visible);
    }
    
    private void enableDetailButtons(boolean enabled) {
        this.strategyRevertButton.setEnabled(enabled);
    }
    
    private void setDetailsButtonsVisible(boolean visible) {
        this.strategyRevertButton.setVisible(visible);
    }
    
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
        enableOverviewButtons(enabled);
        enableDetailButtons(enabled);
    }
    
    private void saveInput() {
        /*
        if (this.dataSet != null && !this.BBSDatasetText.getText().equals(dataSet.limits)) {
            dataSet.limits = BBSDatasetText.getText();
            logger.trace("Variable BBS ("+dataSet.name+"//"+dataSet.treeID()+"//"+dataSet.nodeID()+"//"+dataSet.parentID()+"//"+dataSet.paramDefID()+") from value ("+BBSDatasetText.getText()+") updated to :"+dataSet.limits);
            saveNode(dataSet);
        } else if (this.BBDBHost != null && !this.BBDBHostText.getText().equals(BBDBHost.limits)) {
            BBDBHost.limits = BBDBHostText.getText();
            logger.trace("Variable BBS ("+BBDBHost.name+"//"+BBDBHost.treeID()+"//"+BBDBHost.nodeID()+"//"+BBDBHost.parentID()+"//"+BBDBHost.paramDefID()+") updated to :"+BBDBHost.limits);
            saveNode(BBDBHost);
        } else if (this.BBDBPort != null && !this.BBDBPortText.getText().equals(BBDBPort.limits)) {
            BBDBPort.limits = BBDBPortText.getText();
            logger.trace("Variable BBS ("+BBDBPort.name+"//"+BBDBPort.treeID()+"//"+BBDBPort.nodeID()+"//"+BBDBPort.parentID()+"//"+BBDBPort.paramDefID()+") updated to :"+BBDBPort.limits);
            saveNode(BBDBPort);
        } else if (this.BBDBDBName != null && !this.BBDBDBNameText.getText().equals(BBDBDBName.limits)) {
            BBDBDBName.limits = BBDBDBNameText.getText();
            logger.trace("Variable BBS ("+BBDBDBName.name+"//"+BBDBDBName.treeID()+"//"+BBDBDBName.nodeID()+"//"+BBDBDBName.parentID()+"//"+BBDBDBName.paramDefID()+") updated to :"+BBDBDBName.limits);
            saveNode(BBDBDBName);
        } else if (this.BBDBUsername != null && !this.BBDBDBUsernameText.getText().equals(BBDBUsername.limits)) {
            BBDBUsername.limits = BBDBDBUsernameText.getText();
            logger.trace("Variable BBS ("+BBDBUsername.name+"//"+BBDBUsername.treeID()+"//"+BBDBUsername.nodeID()+"//"+BBDBUsername.parentID()+"//"+BBDBUsername.paramDefID()+") updated to :"+BBDBUsername.limits);
            saveNode(BBDBUsername);
        } else if (this.BBDBPassword != null && !this.BBDBDBPasswordText.getText().equals(BBDBPassword.limits)) {
            BBDBPassword.limits = BBDBDBPasswordText.getText();
            logger.trace("Variable BBS ("+BBDBPassword.name+"//"+BBDBPassword.treeID()+"//"+BBDBPassword.nodeID()+"//"+BBDBPassword.parentID()+"//"+BBDBPassword.paramDefID()+") updated to :"+BBDBPassword.limits);
            saveNode(BBDBPassword);
        } else if (this.ParmDBInstrument != null && !this.ParmDBInstrumentText.getText().equals(ParmDBInstrument.limits)) {
            ParmDBInstrument.limits = ParmDBInstrumentText.getText();
            logger.trace("Variable BBS ("+ParmDBInstrument.name+"//"+ParmDBInstrument.treeID()+"//"+ParmDBInstrument.nodeID()+"//"+ParmDBInstrument.parentID()+"//"+ParmDBInstrument.paramDefID()+") updated to :"+ParmDBInstrument.limits);
            saveNode(ParmDBInstrument);
        } else if (this.ParmDBLocalSky != null && !this.ParmDBLocalSkyText.getText().equals(ParmDBLocalSky.limits)) {
            ParmDBLocalSky.limits = ParmDBLocalSkyText.getText();
            logger.trace("Variable BBS ("+ParmDBLocalSky.name+"//"+ParmDBLocalSky.treeID()+"//"+ParmDBLocalSky.nodeID()+"//"+ParmDBLocalSky.parentID()+"//"+ParmDBLocalSky.paramDefID()+") updated to :"+ParmDBLocalSky.limits);
            saveNode(ParmDBLocalSky);
        }*/
    }
    
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
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
        stationsUseAllCheckbox = new javax.swing.JCheckBox();
        addStationButton = new javax.swing.JButton();
        deleteStationButton = new javax.swing.JButton();
        stepsPanel = new javax.swing.JPanel();
        stepsScrollPane = new javax.swing.JScrollPane();
        stepsTree = new javax.swing.JTree();
        stepsModsPanel = new javax.swing.JPanel();
        addStepButton = new javax.swing.JButton();
        removeStepButton = new javax.swing.JButton();
        modifyStepButton = new javax.swing.JButton();
        loadTemplateStepButton = new javax.swing.JButton();
        stepsMoveUpDownPanel = new javax.swing.JPanel();
        moveStepUpButton = new javax.swing.JButton();
        moveStepDownButton = new javax.swing.JButton();
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
        strategyPanel.add(inputDataText, new org.netbeans.lib.awtextra.AbsoluteConstraints(160, 30, 120, -1));

        strategyRevertButton.setText("Revert");
        strategyRevertButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                strategyRevertButtonActionPerformed(evt);
            }
        });

        strategyPanel.add(strategyRevertButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 500, -1, -1));

        stationsPanel.setLayout(new java.awt.BorderLayout());

        stationsPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Station Names"));
        stationsPanel.setToolTipText("Identifiers of the participating stations.");
        stationsList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "1", "2", "3", "4", "5" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        stationsList.setToolTipText("Identifiers of the participating stations.");
        stationsScrollPane.setViewportView(stationsList);

        stationsPanel.add(stationsScrollPane, java.awt.BorderLayout.CENTER);

        stationsModPanel.setLayout(new java.awt.GridBagLayout());

        stationsUseAllCheckbox.setText("Use all stations");
        stationsUseAllCheckbox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        stationsUseAllCheckbox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        stationsUseAllCheckbox.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                stationsUseAllCheckboxStateChanged(evt);
            }
        });

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stationsModPanel.add(stationsUseAllCheckbox, gridBagConstraints);

        addStationButton.setText("A");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stationsModPanel.add(addStationButton, gridBagConstraints);

        deleteStationButton.setText("D");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stationsModPanel.add(deleteStationButton, gridBagConstraints);

        stationsPanel.add(stationsModPanel, java.awt.BorderLayout.SOUTH);

        strategyPanel.add(stationsPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 150, 220, 330));

        stepsPanel.setLayout(new java.awt.BorderLayout());

        stepsPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Steps"));
        stepsPanel.setToolTipText("The names of the steps that compose the strategy.");
        stepsPanel.setPreferredSize(new java.awt.Dimension(100, 100));
        stepsTree.setToolTipText("The names of the steps that compose the strategy.");
        stepsScrollPane.setViewportView(stepsTree);

        stepsPanel.add(stepsScrollPane, java.awt.BorderLayout.CENTER);

        stepsModsPanel.setLayout(new java.awt.GridBagLayout());

        stepsModsPanel.setMinimumSize(new java.awt.Dimension(100, 30));
        stepsModsPanel.setPreferredSize(new java.awt.Dimension(100, 30));
        addStepButton.setText("Add");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepsModsPanel.add(addStepButton, gridBagConstraints);

        removeStepButton.setText("Delete");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepsModsPanel.add(removeStepButton, gridBagConstraints);

        modifyStepButton.setText("Modify");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepsModsPanel.add(modifyStepButton, gridBagConstraints);

        loadTemplateStepButton.setText("Load from template");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 3;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepsModsPanel.add(loadTemplateStepButton, gridBagConstraints);

        stepsPanel.add(stepsModsPanel, java.awt.BorderLayout.SOUTH);

        stepsMoveUpDownPanel.setLayout(new java.awt.GridBagLayout());

        stepsMoveUpDownPanel.setMinimumSize(new java.awt.Dimension(50, 60));
        stepsMoveUpDownPanel.setPreferredSize(new java.awt.Dimension(50, 60));
        moveStepUpButton.setText("U");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepsMoveUpDownPanel.add(moveStepUpButton, gridBagConstraints);

        moveStepDownButton.setText("D");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepsMoveUpDownPanel.add(moveStepDownButton, gridBagConstraints);

        stepsPanel.add(stepsMoveUpDownPanel, java.awt.BorderLayout.EAST);

        strategyPanel.add(stepsPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(250, 150, 400, 330));

        correlationPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        correlationPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Correlation"));
        correlationSelectionLabel.setText("Selection :");
        correlationPanel.add(correlationSelectionLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 30, -1, -1));

        correlationSelectionBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "AUTO", "CROSS", "ALL" }));
        correlationSelectionBox.setToolTipText("Station correlations to use.\n\nAUTO: Use only correlations of each station with itself (i.e. no base lines).Not yet implemented.\nCROSS: Use only correlations between stations (i.e. base lines).\nALL: Use both AUTO and CROSS correlations.");
        correlationPanel.add(correlationSelectionBox, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 50, 80, -1));

        correlationTypeLabel.setText("Type :");
        correlationPanel.add(correlationTypeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 10, -1, -1));

        correlationTypeList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "XX", "XY", "YX", "YY" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        correlationTypeList.setToolTipText("Correlations of which polarizations to use, one or more of XX,XY,YX,YY. \n\nAs an example, suppose you select 'XX' here and set Selection to AUTO, then the X polarization signal of each station is correlated with itself. However if we set Selection to CROSS, then the X polarization of station A is correlated with the X polarization of station B for each base line.");
        correlationTypeScrollPane.setViewportView(correlationTypeList);

        correlationPanel.add(correlationTypeScrollPane, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 30, 50, 80));

        strategyPanel.add(correlationPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(480, 20, 170, 120));

        workDomainSizePanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        workDomainSizePanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Work Domain Size"));
        workDomainSizePanel.setToolTipText("Size of the work domain in frequency and time. A work domain represents an amount of input data that is loaded into memory and processed as a single block. A large work domain size should reduce the overhead due to disk access.");
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

        integrationIntervalPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        integrationIntervalPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Integration"));
        integrationIntervalPanel.setToolTipText("Cell size for integration. Allows the user to perform operations on a lower resolution, which should be faster in most cases");
        integrationFrequencyLabel.setText("Freq. Interval :");
        integrationIntervalPanel.add(integrationFrequencyLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, -1, -1));

        integrationIntervalPanel.add(integrationFrequencyText, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 20, 70, -1));

        integrationFrequencyUnitLabel.setText("Hz");
        integrationIntervalPanel.add(integrationFrequencyUnitLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 20, -1, -1));

        integrationTimeLabel.setText("Time Interval :");
        integrationIntervalPanel.add(integrationTimeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 50, -1, -1));

        integrationIntervalPanel.add(integrationTimeText, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 50, 70, -1));

        integrationTimeUnitLabel.setText("s");
        integrationIntervalPanel.add(integrationTimeUnitLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 50, 10, -1));

        strategyPanel.add(integrationIntervalPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(250, 60, 220, 80));

        strategyScrollPane.setViewportView(strategyPanel);

        add(strategyScrollPane, java.awt.BorderLayout.CENTER);

    }// </editor-fold>//GEN-END:initComponents

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        if(evt.getActionCommand() == "Save Settings") {
            saveInput();
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
                
    private void stationsUseAllCheckboxStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_stationsUseAllCheckboxStateChanged
        if(this.stationsUseAllCheckbox.isSelected()){
            this.stationsList.setBackground(Color.LIGHT_GRAY);
            this.stationsList.setEnabled(false);
        }else{
            this.stationsList.setBackground(Color.WHITE);
            this.stationsList.setEnabled(true);
        }
    }//GEN-LAST:event_stationsUseAllCheckboxStateChanged
                    
    private void strategyRevertButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_strategyRevertButtonActionPerformed
        this.restoreBBSStrategyPanel();
    }//GEN-LAST:event_strategyRevertButtonActionPerformed
        
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
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
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
    private javax.swing.JLabel inputDataLabel;
    private javax.swing.JTextField inputDataText;
    private javax.swing.JLabel integrationFrequencyLabel;
    private javax.swing.JTextField integrationFrequencyText;
    private javax.swing.JLabel integrationFrequencyUnitLabel;
    private javax.swing.JPanel integrationIntervalPanel;
    private javax.swing.JLabel integrationTimeLabel;
    private javax.swing.JTextField integrationTimeText;
    private javax.swing.JLabel integrationTimeUnitLabel;
    private javax.swing.JButton loadTemplateStepButton;
    private javax.swing.JButton modifyStepButton;
    private javax.swing.JButton moveStepDownButton;
    private javax.swing.JButton moveStepUpButton;
    private javax.swing.JButton removeStepButton;
    private javax.swing.JList stationsList;
    private javax.swing.JPanel stationsModPanel;
    private javax.swing.JPanel stationsPanel;
    private javax.swing.JScrollPane stationsScrollPane;
    private javax.swing.JCheckBox stationsUseAllCheckbox;
    private javax.swing.JPanel stepsModsPanel;
    private javax.swing.JPanel stepsMoveUpDownPanel;
    private javax.swing.JPanel stepsPanel;
    private javax.swing.JScrollPane stepsScrollPane;
    private javax.swing.JTree stepsTree;
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
    private javax.swing.event.EventListenerList listenerList =  null;
    
    /**
     * Registers ActionListener to receive events.
     * @param listener The listener to register.
     */
    public synchronized void addActionListener(java.awt.event.ActionListener listener) {
        
        if (listenerList == null ) {
            listenerList = new javax.swing.event.EventListenerList();
        }
        listenerList.add(java.awt.event.ActionListener.class, listener);
    }
    
    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeActionListener(java.awt.event.ActionListener listener) {
        
        listenerList.remove(java.awt.event.ActionListener.class, listener);
    }
    
    /**
     * Notifies all registered listeners about the event.
     *
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {
        
        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed(event);
            }
        }
    }
    
    
    
    
    
}
