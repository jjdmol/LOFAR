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
 * Panel for BBS specific configuration
 *
 * @created 11-07-2006, 13:37
 *
 * @author  pompert
 *
 * @version $Id$
 */
public class BBSPanel extends javax.swing.JPanel implements IViewPanel{
    
    static Logger logger = Logger.getLogger(OLAPConfigPanel.class);
    static String name = "BBS";
    
    
    /** Creates new form BeanForm based upon aNode
     *
     * @params  aNode   node to obtain the info from
     *
     */
    public BBSPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode=aNode;
        initialize();
        initPanel();
    }
    
    /** Creates new form BeanForm */
    public BBSPanel() {
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
                }
                /*
                 else if (LofarUtils.keyName(aNode.name).equals("OLAP_HW")) {
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
                }*/
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
        return new BBSPanel();
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
    private void restoreBBSDetailsPanel() {
        /*AMCServerHostText.setText(itsAMCServerHost.limits);
         */
        
    }
    
    /** Restore original Values in Details panel
     */
    private void restoreBBSOverviewPanel() {
       /*AMCServerHostText.setText(itsAMCServerHost.limits);
        */
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
        /*
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
        }*/
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
        this.configurationRevertButton.setEnabled(enabled);
    }
    
    private void setOverviewButtonsVisible(boolean visible) {
        this.configurationRevertButton.setVisible(visible);    
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
        boolean hasChanged = false;
        
        // BBS Parameters
        /*
        if (itsAMCServerHost != null && !AMCServerHostText.equals(itsAMCServerHost.limits)) {
            itsAMCServerHost.limits = AMCServerHostText.getText();
            saveParam(itsAMCServerHost);
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

        jTabbedPane1 = new javax.swing.JTabbedPane();
        BBSGlobalSettingsPanel = new javax.swing.JPanel();
        BBSDatasetLabel = new javax.swing.JLabel();
        BBSDatasetText = new javax.swing.JTextField();
        BBDatabasePanel = new javax.swing.JPanel();
        BBDBDBPasswordLabel1 = new javax.swing.JLabel();
        BBDBDBPasswordText = new javax.swing.JTextField();
        BBDBHostLabel = new javax.swing.JLabel();
        BBDBHostText = new javax.swing.JTextField();
        BBDBPortText = new javax.swing.JTextField();
        BBDBPortLabel = new javax.swing.JLabel();
        BBDBDBNameLabel = new javax.swing.JLabel();
        BBDBDBNameText = new javax.swing.JTextField();
        BBDBDBUsernameText = new javax.swing.JTextField();
        BBDBDBUsernameLabel = new javax.swing.JLabel();
        ParmDBPanel = new javax.swing.JPanel();
        ParmDBInstrumentLabel = new javax.swing.JLabel();
        ParmDBInstrumentText = new javax.swing.JTextField();
        ParmDBLocalSkyLabel = new javax.swing.JLabel();
        ParmDBLocalSkyText = new javax.swing.JTextField();
        configurationRevertButton = new javax.swing.JButton();
        configurationSaveButton = new javax.swing.JButton();
        BBSStrategyPanel = new javax.swing.JPanel();
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
        strategySaveButton = new javax.swing.JButton();
        BBSStepExplorerPanel = new javax.swing.JPanel();
        BaselineSelectionPanel = new javax.swing.JPanel();
        baselineStationsScrollPane = new javax.swing.JScrollPane();
        baselineStationsTable = new javax.swing.JTable();
        baselineModsPanel = new javax.swing.JPanel();
        baselineUseAllCheckbox = new javax.swing.JCheckBox();
        addBaseLineButton = new javax.swing.JButton();
        modifyBaseLineButton = new javax.swing.JButton();
        deleteBaseLineButton = new javax.swing.JButton();
        explorePreviousStepButton = new javax.swing.JButton();
        exploreParentStepButton = new javax.swing.JButton();
        exploreFirstChildStepButton = new javax.swing.JButton();
        exploreNextStepButton = new javax.swing.JButton();
        stepExplorerRevertButton = new javax.swing.JButton();
        stepExplorerSourcesPanel = new javax.swing.JPanel();
        stepExplorerSourcesModsPanel = new javax.swing.JPanel();
        addSourceButton = new javax.swing.JButton();
        modifySourceButton = new javax.swing.JButton();
        deleteSourceButton1 = new javax.swing.JButton();
        stepExplorerSourcesScrollPane = new javax.swing.JScrollPane();
        stepExplorerSourcesList = new javax.swing.JList();
        stepExplorerInstrumentModelPanel = new javax.swing.JPanel();
        stepExplorerInstrumentModelScrollPane = new javax.swing.JScrollPane();
        stepExplorerInstrumentModelList = new javax.swing.JList();
        stepExplorerInstrumentModelModsPanel = new javax.swing.JPanel();
        addInstrumentModelButton = new javax.swing.JButton();
        modifyInstrumentModelButton = new javax.swing.JButton();
        deleteInstrumentModelButton = new javax.swing.JButton();
        stepExplorerOperationPanel = new javax.swing.JPanel();
        stepExplorerOperationTypeHeaderPanel = new javax.swing.JPanel();
        stepExplorerOperationTypeLabel = new javax.swing.JLabel();
        stepExplorerOperationComboBox = new javax.swing.JComboBox();
        stepExplorerOperationAttributesPanel = new javax.swing.JPanel();
        seOperationAttributesScrollPane = new javax.swing.JScrollPane();
        seOperationAttributesInputPanel = new javax.swing.JPanel();
        seOperationAttributeLabel1 = new javax.swing.JLabel();
        seOperationAttributeText1 = new javax.swing.JTextField();
        seOperationAttributeText2 = new javax.swing.JTextField();
        seOperationAttributeLabel2 = new javax.swing.JLabel();
        seOperationAttributeText3 = new javax.swing.JTextField();
        seOperationAttributeLabel3 = new javax.swing.JLabel();
        seOperationAttributeGroup1 = new javax.swing.JPanel();
        seOperationAttributeLabel4 = new javax.swing.JLabel();
        seOperationAttributeLabel5 = new javax.swing.JLabel();
        seOperationAttributeText4 = new javax.swing.JTextField();
        seOperationAttributeText5 = new javax.swing.JTextField();
        seOperationAttributeGroup2 = new javax.swing.JPanel();
        seOperationAttributeSolvableParmPanel = new javax.swing.JPanel();
        addSolvableParmButton = new javax.swing.JButton();
        modifySolvableParmButton = new javax.swing.JButton();
        deleteSolvableParmButton = new javax.swing.JButton();
        seSolvableParmScrollPane = new javax.swing.JScrollPane();
        seSolvableParmList = new javax.swing.JList();
        seOperationAttributeGroup3 = new javax.swing.JPanel();
        stepExplorerSourcesModsPanel2 = new javax.swing.JPanel();
        addSolvableParmButton1 = new javax.swing.JButton();
        modifySolvableParmButton1 = new javax.swing.JButton();
        deleteSolvableParmButton1 = new javax.swing.JButton();
        seSolvableParmScrollPane1 = new javax.swing.JScrollPane();
        seSolvableParmList1 = new javax.swing.JList();
        stepExplorerOutputDataPanel = new javax.swing.JPanel();
        stepExplorerOutputDataText = new javax.swing.JTextField();
        stepExplorerSaveButton = new javax.swing.JButton();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        setLayout(new java.awt.BorderLayout());

        BBSGlobalSettingsPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        BBSDatasetLabel.setText("Dataset :");
        BBSGlobalSettingsPanel.add(BBSDatasetLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 30, -1, -1));

        BBSDatasetText.setToolTipText("Path to the measurement set");
        BBSDatasetText.setMaximumSize(new java.awt.Dimension(440, 19));
        BBSDatasetText.setMinimumSize(new java.awt.Dimension(440, 19));
        BBSDatasetText.setPreferredSize(new java.awt.Dimension(440, 19));
        BBSGlobalSettingsPanel.add(BBSDatasetText, new org.netbeans.lib.awtextra.AbsoluteConstraints(130, 30, 220, -1));

        BBDatabasePanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        BBDatabasePanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Black Board Database"));
        BBDatabasePanel.setToolTipText("Information about the black board database");
        BBDBDBPasswordLabel1.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        BBDBDBPasswordLabel1.setText("DB Password :");
        BBDatabasePanel.add(BBDBDBPasswordLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 140, 100, 20));

        BBDBDBPasswordText.setToolTipText("Password to access the black board database");
        BBDatabasePanel.add(BBDBDBPasswordText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 140, 220, 20));

        BBDBHostLabel.setText("Host : ");
        BBDatabasePanel.add(BBDBHostLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, -1, -1));

        BBDBHostText.setToolTipText("Hostname or IP Address of the host on which the black board database and the parameter databases reside");
        BBDBHostText.setMaximumSize(new java.awt.Dimension(200, 19));
        BBDBHostText.setMinimumSize(new java.awt.Dimension(200, 19));
        BBDBHostText.setPreferredSize(new java.awt.Dimension(200, 19));
        BBDatabasePanel.add(BBDBHostText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 20, 220, -1));

        BBDBPortText.setToolTipText("Port number on which the black board database server is listening");
        BBDatabasePanel.add(BBDBPortText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 50, 60, 20));

        BBDBPortLabel.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        BBDBPortLabel.setText("Port :");
        BBDatabasePanel.add(BBDBPortLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 50, 81, -1));

        BBDBDBNameLabel.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        BBDBDBNameLabel.setText("DB Name :");
        BBDatabasePanel.add(BBDBDBNameLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 80, 81, -1));

        BBDBDBNameText.setToolTipText("Name of the black board database");
        BBDatabasePanel.add(BBDBDBNameText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 80, 220, -1));

        BBDBDBUsernameText.setToolTipText("Username to access the black board database");
        BBDatabasePanel.add(BBDBDBUsernameText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 110, 220, 20));

        BBDBDBUsernameLabel.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        BBDBDBUsernameLabel.setText("DB Username :");
        BBDatabasePanel.add(BBDBDBUsernameLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 110, 100, 20));

        BBSGlobalSettingsPanel.add(BBDatabasePanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 60, 350, 170));

        ParmDBPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        ParmDBPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("ParmDB"));
        ParmDBPanel.setToolTipText("Information about the parameter databases (e.g. instrument parameters, local sky model).");
        ParmDBInstrumentLabel.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        ParmDBInstrumentLabel.setText("Instrument :");
        ParmDBPanel.add(ParmDBInstrumentLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 100, 20));

        ParmDBInstrumentText.setToolTipText("Path to the AIPS++ table containing the instrument parameters");
        ParmDBPanel.add(ParmDBInstrumentText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 20, 220, 20));

        ParmDBLocalSkyLabel.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        ParmDBLocalSkyLabel.setText("Local Sky :");
        ParmDBPanel.add(ParmDBLocalSkyLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 50, 100, 20));

        ParmDBLocalSkyText.setToolTipText("Path to the AIPS++ table containing the local sky model parameters");
        ParmDBLocalSkyText.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ParmDBLocalSkyTextActionPerformed(evt);
            }
        });

        ParmDBPanel.add(ParmDBLocalSkyText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 50, 220, 20));

        BBSGlobalSettingsPanel.add(ParmDBPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 240, 350, 80));

        configurationRevertButton.setText("Revert");
        configurationRevertButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                configurationRevertButtonActionPerformed(evt);
            }
        });

        BBSGlobalSettingsPanel.add(configurationRevertButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 330, 81, -1));

        configurationSaveButton.setText("Save");
        configurationSaveButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                configurationSaveButtonActionPerformed(evt);
            }
        });

        BBSGlobalSettingsPanel.add(configurationSaveButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(290, 330, -1, -1));

        jTabbedPane1.addTab("Global Settings", BBSGlobalSettingsPanel);

        BBSStrategyPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        inputDataLabel.setText("Input Data :");
        BBSStrategyPanel.add(inputDataLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 30, -1, -1));

        inputDataText.setToolTipText("Name of the column in the measurement set that contains the input data");
        BBSStrategyPanel.add(inputDataText, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 30, 120, -1));

        strategyRevertButton.setText("Revert");
        strategyRevertButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                strategyRevertButtonActionPerformed(evt);
            }
        });

        BBSStrategyPanel.add(strategyRevertButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 500, -1, -1));

        stationsPanel.setLayout(new java.awt.BorderLayout());

        stationsPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Stations"));
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

        BBSStrategyPanel.add(stationsPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 150, 220, 330));

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

        BBSStrategyPanel.add(stepsPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(250, 150, 400, 330));

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

        BBSStrategyPanel.add(correlationPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(480, 20, 170, 120));

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

        BBSStrategyPanel.add(workDomainSizePanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 60, 220, 80));

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

        BBSStrategyPanel.add(integrationIntervalPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(250, 60, 220, 80));

        strategySaveButton.setText("Save");
        strategySaveButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                strategySaveButtonActionPerformed(evt);
            }
        });

        BBSStrategyPanel.add(strategySaveButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(580, 500, -1, -1));

        jTabbedPane1.addTab("Strategy", BBSStrategyPanel);

        BBSStepExplorerPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        BaselineSelectionPanel.setLayout(new java.awt.BorderLayout());

        BaselineSelectionPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Baseline Selection"));
        baselineStationsTable.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {
                {new Integer(0), new Integer(1)},
                {new Integer(0), new Integer(2)},
                {new Integer(0), new Integer(3)},
                {new Integer(1), new Integer(2)},
                {new Integer(1), new Integer(3)},
                {new Integer(2), new Integer(3)}
            },
            new String [] {
                "Station 1", "Station 2"
            }
        ) {
            Class[] types = new Class [] {
                java.lang.Integer.class, java.lang.Integer.class
            };

            public Class getColumnClass(int columnIndex) {
                return types [columnIndex];
            }
        });
        baselineStationsTable.setToolTipText("The baselines used");
        baselineStationsScrollPane.setViewportView(baselineStationsTable);

        BaselineSelectionPanel.add(baselineStationsScrollPane, java.awt.BorderLayout.CENTER);

        baselineModsPanel.setLayout(new java.awt.GridBagLayout());

        baselineUseAllCheckbox.setText("Use all baselines");
        baselineUseAllCheckbox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        baselineUseAllCheckbox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        baselineUseAllCheckbox.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                baselineUseAllCheckboxStateChanged(evt);
            }
        });

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        baselineModsPanel.add(baselineUseAllCheckbox, gridBagConstraints);

        addBaseLineButton.setText("A");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        baselineModsPanel.add(addBaseLineButton, gridBagConstraints);

        modifyBaseLineButton.setText("M");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        baselineModsPanel.add(modifyBaseLineButton, gridBagConstraints);

        deleteBaseLineButton.setText("D");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        baselineModsPanel.add(deleteBaseLineButton, gridBagConstraints);

        BaselineSelectionPanel.add(baselineModsPanel, java.awt.BorderLayout.SOUTH);

        BBSStepExplorerPanel.add(BaselineSelectionPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 50, 310, 230));

        explorePreviousStepButton.setText("View previous step");
        explorePreviousStepButton.setEnabled(false);
        BBSStepExplorerPanel.add(explorePreviousStepButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 10, -1, -1));

        exploreParentStepButton.setText("View parent multistep");
        BBSStepExplorerPanel.add(exploreParentStepButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 10, -1, -1));

        exploreFirstChildStepButton.setText("View first child step");
        exploreFirstChildStepButton.setEnabled(false);
        BBSStepExplorerPanel.add(exploreFirstChildStepButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(370, 10, -1, -1));

        exploreNextStepButton.setText("View next step");
        BBSStepExplorerPanel.add(exploreNextStepButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(540, 10, -1, -1));

        stepExplorerRevertButton.setText("Revert");
        stepExplorerRevertButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                stepExplorerRevertButtonActionPerformed(evt);
            }
        });

        BBSStepExplorerPanel.add(stepExplorerRevertButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 490, -1, -1));

        stepExplorerSourcesPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerSourcesPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Sources"));
        stepExplorerSourcesModsPanel.setLayout(new java.awt.GridBagLayout());

        addSourceButton.setText("A");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel.add(addSourceButton, gridBagConstraints);

        modifySourceButton.setText("M");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel.add(modifySourceButton, gridBagConstraints);

        deleteSourceButton1.setText("D");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel.add(deleteSourceButton1, gridBagConstraints);

        stepExplorerSourcesPanel.add(stepExplorerSourcesModsPanel, java.awt.BorderLayout.SOUTH);

        stepExplorerSourcesList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "3C343", "3C344", "3C345", "3C346" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        stepExplorerSourcesScrollPane.setViewportView(stepExplorerSourcesList);

        stepExplorerSourcesPanel.add(stepExplorerSourcesScrollPane, java.awt.BorderLayout.CENTER);

        BBSStepExplorerPanel.add(stepExplorerSourcesPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(340, 50, 160, 230));

        stepExplorerInstrumentModelPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerInstrumentModelPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Instrument Model"));
        stepExplorerInstrumentModelList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "Bandpass", "DirGain", "Phase" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        stepExplorerInstrumentModelScrollPane.setViewportView(stepExplorerInstrumentModelList);

        stepExplorerInstrumentModelPanel.add(stepExplorerInstrumentModelScrollPane, java.awt.BorderLayout.CENTER);

        stepExplorerInstrumentModelModsPanel.setLayout(new java.awt.GridBagLayout());

        addInstrumentModelButton.setText("A");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerInstrumentModelModsPanel.add(addInstrumentModelButton, gridBagConstraints);

        modifyInstrumentModelButton.setText("M");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerInstrumentModelModsPanel.add(modifyInstrumentModelButton, gridBagConstraints);

        deleteInstrumentModelButton.setText("D");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerInstrumentModelModsPanel.add(deleteInstrumentModelButton, gridBagConstraints);

        stepExplorerInstrumentModelPanel.add(stepExplorerInstrumentModelModsPanel, java.awt.BorderLayout.SOUTH);

        BBSStepExplorerPanel.add(stepExplorerInstrumentModelPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(510, 50, 160, 230));

        stepExplorerOperationPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerOperationPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Operation"));
        stepExplorerOperationTypeHeaderPanel.setLayout(new java.awt.GridBagLayout());

        stepExplorerOperationTypeHeaderPanel.setBackground(new java.awt.Color(204, 204, 204));
        stepExplorerOperationTypeLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        stepExplorerOperationTypeLabel.setText("Type :");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerOperationTypeHeaderPanel.add(stepExplorerOperationTypeLabel, gridBagConstraints);

        stepExplorerOperationComboBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "SOLVE", "SUBTRACT", "CORRECT", "PREDICT", "SHIFT", "REFIT" }));
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerOperationTypeHeaderPanel.add(stepExplorerOperationComboBox, gridBagConstraints);

        stepExplorerOperationPanel.add(stepExplorerOperationTypeHeaderPanel, java.awt.BorderLayout.NORTH);

        stepExplorerOperationAttributesPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerOperationAttributesPanel.setBorder(javax.swing.BorderFactory.createEtchedBorder(new java.awt.Color(204, 204, 204), null));
        seOperationAttributesInputPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        seOperationAttributeLabel1.setText("Max. iterations :");
        seOperationAttributesInputPanel.add(seOperationAttributeLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, -1, -1));
        seOperationAttributeLabel1.getAccessibleContext().setAccessibleName("MaxIter");

        seOperationAttributeText1.setText("5");
        seOperationAttributesInputPanel.add(seOperationAttributeText1, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 20, 60, -1));
        seOperationAttributeText1.getAccessibleContext().setAccessibleName("MaxIterValue");

        seOperationAttributeText2.setText("1e-6");
        seOperationAttributesInputPanel.add(seOperationAttributeText2, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 50, 60, -1));
        seOperationAttributeText2.getAccessibleContext().setAccessibleName("EpsilonValue");

        seOperationAttributeLabel2.setText("Epsilon :");
        seOperationAttributesInputPanel.add(seOperationAttributeLabel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 50, -1, -1));
        seOperationAttributeLabel2.getAccessibleContext().setAccessibleName("Epsilon");

        seOperationAttributeText3.setText("0.95");
        seOperationAttributesInputPanel.add(seOperationAttributeText3, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 80, 60, -1));

        seOperationAttributeLabel3.setText("Min. converged :");
        seOperationAttributesInputPanel.add(seOperationAttributeLabel3, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 80, -1, -1));
        seOperationAttributeLabel3.getAccessibleContext().setAccessibleName("Maxiterations :");

        seOperationAttributeGroup1.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        seOperationAttributeGroup1.setBorder(javax.swing.BorderFactory.createTitledBorder("Domain Size"));
        seOperationAttributeLabel4.setText("Frequency :");
        seOperationAttributeGroup1.add(seOperationAttributeLabel4, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, -1, -1));

        seOperationAttributeLabel5.setText("Time :");
        seOperationAttributeGroup1.add(seOperationAttributeLabel5, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 40, -1, -1));

        seOperationAttributeText4.setText("500");
        seOperationAttributeGroup1.add(seOperationAttributeText4, new org.netbeans.lib.awtextra.AbsoluteConstraints(90, 20, 60, -1));

        seOperationAttributeText5.setText("2");
        seOperationAttributeGroup1.add(seOperationAttributeText5, new org.netbeans.lib.awtextra.AbsoluteConstraints(90, 40, 60, -1));

        seOperationAttributesInputPanel.add(seOperationAttributeGroup1, new org.netbeans.lib.awtextra.AbsoluteConstraints(260, 10, 160, 70));

        seOperationAttributeGroup2.setLayout(new java.awt.BorderLayout());

        seOperationAttributeGroup2.setBorder(javax.swing.BorderFactory.createTitledBorder("Solvable Parameters"));
        seOperationAttributeSolvableParmPanel.setLayout(new java.awt.GridBagLayout());

        addSolvableParmButton.setText("A");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        seOperationAttributeSolvableParmPanel.add(addSolvableParmButton, gridBagConstraints);

        modifySolvableParmButton.setText("M");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        seOperationAttributeSolvableParmPanel.add(modifySolvableParmButton, gridBagConstraints);

        deleteSolvableParmButton.setText("D");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        seOperationAttributeSolvableParmPanel.add(deleteSolvableParmButton, gridBagConstraints);

        seOperationAttributeGroup2.add(seOperationAttributeSolvableParmPanel, java.awt.BorderLayout.SOUTH);

        seSolvableParmList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "GAIN:*", "PHASE:*" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        seSolvableParmScrollPane.setViewportView(seSolvableParmList);

        seOperationAttributeGroup2.add(seSolvableParmScrollPane, java.awt.BorderLayout.CENTER);

        seOperationAttributesInputPanel.add(seOperationAttributeGroup2, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 110, 160, 230));

        seOperationAttributeGroup3.setLayout(new java.awt.BorderLayout());

        seOperationAttributeGroup3.setBorder(javax.swing.BorderFactory.createTitledBorder("Excluded Parameters"));
        stepExplorerSourcesModsPanel2.setLayout(new java.awt.GridBagLayout());

        addSolvableParmButton1.setText("A");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel2.add(addSolvableParmButton1, gridBagConstraints);

        modifySolvableParmButton1.setText("M");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel2.add(modifySolvableParmButton1, gridBagConstraints);

        deleteSolvableParmButton1.setText("D");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel2.add(deleteSolvableParmButton1, gridBagConstraints);

        seOperationAttributeGroup3.add(stepExplorerSourcesModsPanel2, java.awt.BorderLayout.SOUTH);

        seSolvableParmScrollPane1.setViewportView(seSolvableParmList1);

        seOperationAttributeGroup3.add(seSolvableParmScrollPane1, java.awt.BorderLayout.CENTER);

        seOperationAttributesInputPanel.add(seOperationAttributeGroup3, new org.netbeans.lib.awtextra.AbsoluteConstraints(260, 110, 160, 230));

        seOperationAttributesScrollPane.setViewportView(seOperationAttributesInputPanel);

        stepExplorerOperationAttributesPanel.add(seOperationAttributesScrollPane, java.awt.BorderLayout.CENTER);

        stepExplorerOperationPanel.add(stepExplorerOperationAttributesPanel, java.awt.BorderLayout.CENTER);

        BBSStepExplorerPanel.add(stepExplorerOperationPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 290, 480, 190));

        stepExplorerOutputDataPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerOutputDataPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Output Data"));
        stepExplorerOutputDataText.setText("OUTDATA2");
        stepExplorerOutputDataPanel.add(stepExplorerOutputDataText, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 140, -1));

        BBSStepExplorerPanel.add(stepExplorerOutputDataPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(510, 290, 160, 50));

        stepExplorerSaveButton.setText("Save");
        stepExplorerSaveButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                stepExplorerSaveButtonActionPerformed(evt);
            }
        });

        BBSStepExplorerPanel.add(stepExplorerSaveButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(590, 490, -1, -1));

        jTabbedPane1.addTab("Step Explorer", BBSStepExplorerPanel);

        add(jTabbedPane1, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });

        add(buttonPanel1, java.awt.BorderLayout.SOUTH);

    }// </editor-fold>//GEN-END:initComponents

    private void configurationSaveButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_configurationSaveButtonActionPerformed
// TODO add your handling code here:
    }//GEN-LAST:event_configurationSaveButtonActionPerformed

    private void strategySaveButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_strategySaveButtonActionPerformed
// TODO add your handling code here:
    }//GEN-LAST:event_strategySaveButtonActionPerformed

    private void stepExplorerSaveButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_stepExplorerSaveButtonActionPerformed
// TODO add your handling code here:
    }//GEN-LAST:event_stepExplorerSaveButtonActionPerformed

    private void stationsUseAllCheckboxStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_stationsUseAllCheckboxStateChanged
       if(this.stationsUseAllCheckbox.isSelected()){
           this.stationsList.setBackground(Color.LIGHT_GRAY);
           this.stationsList.setEnabled(false);
       }else{
           this.stationsList.setBackground(Color.WHITE);
           this.stationsList.setEnabled(true);
       }
    }//GEN-LAST:event_stationsUseAllCheckboxStateChanged

    private void stepExplorerRevertButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_stepExplorerRevertButtonActionPerformed
// TODO add your handling code here:
    }//GEN-LAST:event_stepExplorerRevertButtonActionPerformed

    private void baselineUseAllCheckboxStateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_baselineUseAllCheckboxStateChanged
       if(baselineUseAllCheckbox.isSelected()){
           this.baselineStationsTable.setBackground(Color.LIGHT_GRAY);
           this.baselineStationsTable.setEnabled(false);
       }else{
           this.baselineStationsTable.setBackground(Color.WHITE);
           this.baselineStationsTable.setEnabled(true);
       }
      
    }//GEN-LAST:event_baselineUseAllCheckboxStateChanged

    private void ParmDBLocalSkyTextActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ParmDBLocalSkyTextActionPerformed
// TODO add your handling code here:
    }//GEN-LAST:event_ParmDBLocalSkyTextActionPerformed
    
    private void configurationRevertButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_configurationRevertButtonActionPerformed
        this.restoreBBSOverviewPanel();
    }//GEN-LAST:event_configurationRevertButtonActionPerformed
    
    private void strategyRevertButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_strategyRevertButtonActionPerformed
        this.restoreBBSDetailsPanel();
    }//GEN-LAST:event_strategyRevertButtonActionPerformed
    
    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        if(evt.getActionCommand() == "Save Settings") {
            saveInput();
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    private jOTDBnode itsNode = null;
    private MainFrame  itsMainFrame;
    private Vector<jOTDBparam> itsParamList;
    /*
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
     */
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel BBDBDBNameLabel;
    private javax.swing.JTextField BBDBDBNameText;
    private javax.swing.JLabel BBDBDBPasswordLabel1;
    private javax.swing.JTextField BBDBDBPasswordText;
    private javax.swing.JLabel BBDBDBUsernameLabel;
    private javax.swing.JTextField BBDBDBUsernameText;
    private javax.swing.JLabel BBDBHostLabel;
    private javax.swing.JTextField BBDBHostText;
    private javax.swing.JLabel BBDBPortLabel;
    private javax.swing.JTextField BBDBPortText;
    private javax.swing.JPanel BBDatabasePanel;
    private javax.swing.JLabel BBSDatasetLabel;
    private javax.swing.JTextField BBSDatasetText;
    private javax.swing.JPanel BBSGlobalSettingsPanel;
    private javax.swing.JPanel BBSStepExplorerPanel;
    private javax.swing.JPanel BBSStrategyPanel;
    private javax.swing.JPanel BaselineSelectionPanel;
    private javax.swing.JLabel ParmDBInstrumentLabel;
    private javax.swing.JTextField ParmDBInstrumentText;
    private javax.swing.JLabel ParmDBLocalSkyLabel;
    private javax.swing.JTextField ParmDBLocalSkyText;
    private javax.swing.JPanel ParmDBPanel;
    private javax.swing.JButton addBaseLineButton;
    private javax.swing.JButton addInstrumentModelButton;
    private javax.swing.JButton addSolvableParmButton;
    private javax.swing.JButton addSolvableParmButton1;
    private javax.swing.JButton addSourceButton;
    private javax.swing.JButton addStationButton;
    private javax.swing.JButton addStepButton;
    private javax.swing.JPanel baselineModsPanel;
    private javax.swing.JScrollPane baselineStationsScrollPane;
    private javax.swing.JTable baselineStationsTable;
    private javax.swing.JCheckBox baselineUseAllCheckbox;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JButton configurationRevertButton;
    private javax.swing.JButton configurationSaveButton;
    private javax.swing.JPanel correlationPanel;
    private javax.swing.JComboBox correlationSelectionBox;
    private javax.swing.JLabel correlationSelectionLabel;
    private javax.swing.JLabel correlationTypeLabel;
    private javax.swing.JList correlationTypeList;
    private javax.swing.JScrollPane correlationTypeScrollPane;
    private javax.swing.JButton deleteBaseLineButton;
    private javax.swing.JButton deleteInstrumentModelButton;
    private javax.swing.JButton deleteSolvableParmButton;
    private javax.swing.JButton deleteSolvableParmButton1;
    private javax.swing.JButton deleteSourceButton1;
    private javax.swing.JButton deleteStationButton;
    private javax.swing.JButton exploreFirstChildStepButton;
    private javax.swing.JButton exploreNextStepButton;
    private javax.swing.JButton exploreParentStepButton;
    private javax.swing.JButton explorePreviousStepButton;
    private javax.swing.JLabel inputDataLabel;
    private javax.swing.JTextField inputDataText;
    private javax.swing.JLabel integrationFrequencyLabel;
    private javax.swing.JTextField integrationFrequencyText;
    private javax.swing.JLabel integrationFrequencyUnitLabel;
    private javax.swing.JPanel integrationIntervalPanel;
    private javax.swing.JLabel integrationTimeLabel;
    private javax.swing.JTextField integrationTimeText;
    private javax.swing.JLabel integrationTimeUnitLabel;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.JButton loadTemplateStepButton;
    private javax.swing.JButton modifyBaseLineButton;
    private javax.swing.JButton modifyInstrumentModelButton;
    private javax.swing.JButton modifySolvableParmButton;
    private javax.swing.JButton modifySolvableParmButton1;
    private javax.swing.JButton modifySourceButton;
    private javax.swing.JButton modifyStepButton;
    private javax.swing.JButton moveStepDownButton;
    private javax.swing.JButton moveStepUpButton;
    private javax.swing.JButton removeStepButton;
    private javax.swing.JPanel seOperationAttributeGroup1;
    private javax.swing.JPanel seOperationAttributeGroup2;
    private javax.swing.JPanel seOperationAttributeGroup3;
    private javax.swing.JLabel seOperationAttributeLabel1;
    private javax.swing.JLabel seOperationAttributeLabel2;
    private javax.swing.JLabel seOperationAttributeLabel3;
    private javax.swing.JLabel seOperationAttributeLabel4;
    private javax.swing.JLabel seOperationAttributeLabel5;
    private javax.swing.JPanel seOperationAttributeSolvableParmPanel;
    private javax.swing.JTextField seOperationAttributeText1;
    private javax.swing.JTextField seOperationAttributeText2;
    private javax.swing.JTextField seOperationAttributeText3;
    private javax.swing.JTextField seOperationAttributeText4;
    private javax.swing.JTextField seOperationAttributeText5;
    private javax.swing.JPanel seOperationAttributesInputPanel;
    private javax.swing.JScrollPane seOperationAttributesScrollPane;
    private javax.swing.JList seSolvableParmList;
    private javax.swing.JList seSolvableParmList1;
    private javax.swing.JScrollPane seSolvableParmScrollPane;
    private javax.swing.JScrollPane seSolvableParmScrollPane1;
    private javax.swing.JList stationsList;
    private javax.swing.JPanel stationsModPanel;
    private javax.swing.JPanel stationsPanel;
    private javax.swing.JScrollPane stationsScrollPane;
    private javax.swing.JCheckBox stationsUseAllCheckbox;
    private javax.swing.JList stepExplorerInstrumentModelList;
    private javax.swing.JPanel stepExplorerInstrumentModelModsPanel;
    private javax.swing.JPanel stepExplorerInstrumentModelPanel;
    private javax.swing.JScrollPane stepExplorerInstrumentModelScrollPane;
    private javax.swing.JPanel stepExplorerOperationAttributesPanel;
    private javax.swing.JComboBox stepExplorerOperationComboBox;
    private javax.swing.JPanel stepExplorerOperationPanel;
    private javax.swing.JPanel stepExplorerOperationTypeHeaderPanel;
    private javax.swing.JLabel stepExplorerOperationTypeLabel;
    private javax.swing.JPanel stepExplorerOutputDataPanel;
    private javax.swing.JTextField stepExplorerOutputDataText;
    private javax.swing.JButton stepExplorerRevertButton;
    private javax.swing.JButton stepExplorerSaveButton;
    private javax.swing.JList stepExplorerSourcesList;
    private javax.swing.JPanel stepExplorerSourcesModsPanel;
    private javax.swing.JPanel stepExplorerSourcesModsPanel2;
    private javax.swing.JPanel stepExplorerSourcesPanel;
    private javax.swing.JScrollPane stepExplorerSourcesScrollPane;
    private javax.swing.JPanel stepsModsPanel;
    private javax.swing.JPanel stepsMoveUpDownPanel;
    private javax.swing.JPanel stepsPanel;
    private javax.swing.JScrollPane stepsScrollPane;
    private javax.swing.JTree stepsTree;
    private javax.swing.JButton strategyRevertButton;
    private javax.swing.JButton strategySaveButton;
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
