/*
 *  BBSPanel.java
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
    
    static Logger logger = Logger.getLogger(BBSPanel.class);
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
    
    
    /** Restore original Values in Global Settings panel
     */
    private void restoreBBSGlobalSettingsPanel() {
        
        // Global Settings parameters
        this.BBSDatasetText.setText(dataSet.limits);
        this.BBDBHostText.setText(BBDBHost.limits);
        this.BBDBPortText.setText(BBDBPort.limits);
        this.BBDBDBNameText.setText(BBDBDBName.limits);
        this.BBDBDBUsernameText.setText(BBDBUsername.limits);
        this.BBDBDBPasswordText.setText(BBDBPassword.limits);
        
        this.ParmDBInstrumentText.setText(ParmDBInstrument.limits);
        this.ParmDBLocalSkyText.setText(ParmDBLocalSky.limits);
        
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
        String aKeyName = LofarUtils.keyName(aNode.name);
        
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
        }
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
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
        this.setOverviewButtonsVisible(visible);
    }
    private void enableOverviewButtons(boolean enabled) {
        this.configurationRevertButton.setEnabled(enabled);
    }
    
    private void setOverviewButtonsVisible(boolean visible) {
        this.configurationRevertButton.setVisible(visible);
    }
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
        enableOverviewButtons(enabled);
    }
    
    private void saveInput() {
       
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
        }
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
        BBSDatasetDeRefText = new javax.swing.JTextField();
        BBSStrategyPanel = new nl.astron.lofar.sas.otbcomponents.bbs.BBSStrategyPanel();
        BBSStepExplorerPanel = new javax.swing.JPanel();
        stepExplorerScrollPanel = new javax.swing.JScrollPane();
        stepExplorerPanel = new javax.swing.JPanel();
        explorePreviousStepButton = new javax.swing.JButton();
        exploreParentStepButton = new javax.swing.JButton();
        exploreFirstChildStepButton = new javax.swing.JButton();
        exploreNextStepButton = new javax.swing.JButton();
        stepExplorerRevertButton = new javax.swing.JButton();
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
        seOperationAttributeUnitLabel1 = new javax.swing.JLabel();
        seOperationAttributeUnitLabel2 = new javax.swing.JLabel();
        seOperationAttributeGroup4 = new javax.swing.JPanel();
        seOperationAttributeGroup3 = new javax.swing.JPanel();
        stepExplorerSourcesModsPanel2 = new javax.swing.JPanel();
        addSolvableParmButton1 = new javax.swing.JButton();
        modifySolvableParmButton1 = new javax.swing.JButton();
        deleteSolvableParmButton1 = new javax.swing.JButton();
        seSolvableParmScrollPane1 = new javax.swing.JScrollPane();
        seSolvableParmList1 = new javax.swing.JList();
        seOperationAttributeGroup2 = new javax.swing.JPanel();
        seOperationAttributeSolvableParmPanel = new javax.swing.JPanel();
        addSolvableParmButton = new javax.swing.JButton();
        modifySolvableParmButton = new javax.swing.JButton();
        deleteSolvableParmButton = new javax.swing.JButton();
        seSolvableParmScrollPane = new javax.swing.JScrollPane();
        seSolvableParmList = new javax.swing.JList();
        stepExplorerOutputDataPanel = new javax.swing.JPanel();
        stepExplorerOutputDataText = new javax.swing.JTextField();
        stepExplorerSaveButton = new javax.swing.JButton();
        baselineGlobalPanel = new javax.swing.JPanel();
        baseLineCorrelationPanel = new javax.swing.JPanel();
        blCorrelationSelectionLabel = new javax.swing.JLabel();
        blCorrelationSelectionBox = new javax.swing.JComboBox();
        blCorrelationTypeLabel = new javax.swing.JLabel();
        blCorrelationTypeScrollPane = new javax.swing.JScrollPane();
        blCorrelationTypeList = new javax.swing.JList();
        BaselineSelectionPanel = new javax.swing.JPanel();
        baselineStationsScrollPane = new javax.swing.JScrollPane();
        baselineStationsTable = new javax.swing.JTable();
        baselineModsPanel = new javax.swing.JPanel();
        addBaseLineButton = new javax.swing.JButton();
        modifyBaseLineButton = new javax.swing.JButton();
        deleteBaseLineButton = new javax.swing.JButton();
        baselineUseAllCheckbox = new javax.swing.JCheckBox();
        stepExplorerGlobalSources = new javax.swing.JPanel();
        stepExplorerSourcesPanel1 = new javax.swing.JPanel();
        stepExplorerSourcesModsPanel1 = new javax.swing.JPanel();
        addSourceButton1 = new javax.swing.JButton();
        modifySourceButton1 = new javax.swing.JButton();
        deleteSourceButton2 = new javax.swing.JButton();
        stepExplorerSourcesScrollPane1 = new javax.swing.JScrollPane();
        stepExplorerSourcesList1 = new javax.swing.JList();
        stepExplorerSourcesPanel = new javax.swing.JPanel();
        stepExplorerSourcesModsPanel = new javax.swing.JPanel();
        addSourceButton = new javax.swing.JButton();
        modifySourceButton = new javax.swing.JButton();
        deleteSourceButton1 = new javax.swing.JButton();
        stepExplorerSourcesScrollPane = new javax.swing.JScrollPane();
        stepExplorerSourcesList = new javax.swing.JList();
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

        ParmDBPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("ParmDB Name"));
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

        BBSDatasetDeRefText.setEditable(false);
        BBSDatasetDeRefText.setToolTipText("Dereferenced and actually used value.");
        BBSDatasetDeRefText.setMaximumSize(new java.awt.Dimension(440, 19));
        BBSDatasetDeRefText.setMinimumSize(new java.awt.Dimension(440, 19));
        BBSDatasetDeRefText.setPreferredSize(new java.awt.Dimension(440, 19));
        BBSGlobalSettingsPanel.add(BBSDatasetDeRefText, new org.netbeans.lib.awtextra.AbsoluteConstraints(360, 30, 180, -1));

        jTabbedPane1.addTab("Global Settings", BBSGlobalSettingsPanel);

        jTabbedPane1.addTab("Strategy", BBSStrategyPanel);

        BBSStepExplorerPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        explorePreviousStepButton.setText("View previous step");
        explorePreviousStepButton.setEnabled(false);
        stepExplorerPanel.add(explorePreviousStepButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 10, -1, -1));

        exploreParentStepButton.setText("View parent multistep");
        stepExplorerPanel.add(exploreParentStepButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 10, -1, -1));

        exploreFirstChildStepButton.setText("View first child step");
        exploreFirstChildStepButton.setEnabled(false);
        stepExplorerPanel.add(exploreFirstChildStepButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(370, 10, -1, -1));

        exploreNextStepButton.setText("View next step");
        stepExplorerPanel.add(exploreNextStepButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(540, 10, -1, -1));

        stepExplorerRevertButton.setText("Revert");
        stepExplorerRevertButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                stepExplorerRevertButtonActionPerformed(evt);
            }
        });

        stepExplorerPanel.add(stepExplorerRevertButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 620, 80, -1));

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

        stepExplorerPanel.add(stepExplorerInstrumentModelPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 230, 160, 120));

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

        seOperationAttributeUnitLabel1.setText("Hz");
        seOperationAttributeGroup1.add(seOperationAttributeUnitLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 20, -1, 20));

        seOperationAttributeUnitLabel2.setText("s");
        seOperationAttributeGroup1.add(seOperationAttributeUnitLabel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 40, 20, 20));

        seOperationAttributesInputPanel.add(seOperationAttributeGroup1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 110, 180, 70));

        seOperationAttributeGroup4.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        seOperationAttributeGroup4.setBorder(javax.swing.BorderFactory.createTitledBorder("Parameters"));
        seOperationAttributeGroup3.setLayout(new java.awt.BorderLayout());

        seOperationAttributeGroup3.setBorder(javax.swing.BorderFactory.createTitledBorder("Excluded"));
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

        seOperationAttributeGroup4.add(seOperationAttributeGroup3, new org.netbeans.lib.awtextra.AbsoluteConstraints(180, 20, 160, 140));

        seOperationAttributeGroup2.setLayout(new java.awt.BorderLayout());

        seOperationAttributeGroup2.setBorder(javax.swing.BorderFactory.createTitledBorder("Solvable"));
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

        seOperationAttributeGroup4.add(seOperationAttributeGroup2, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 160, 140));

        seOperationAttributesInputPanel.add(seOperationAttributeGroup4, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 10, 350, 170));

        seOperationAttributesScrollPane.setViewportView(seOperationAttributesInputPanel);

        stepExplorerOperationAttributesPanel.add(seOperationAttributesScrollPane, java.awt.BorderLayout.CENTER);

        stepExplorerOperationPanel.add(stepExplorerOperationAttributesPanel, java.awt.BorderLayout.CENTER);

        stepExplorerPanel.add(stepExplorerOperationPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 360, 670, 250));

        stepExplorerOutputDataPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerOutputDataPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Output Data Column"));
        stepExplorerOutputDataText.setText("OUTDATA2");
        stepExplorerOutputDataPanel.add(stepExplorerOutputDataText, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 30, 140, -1));

        stepExplorerPanel.add(stepExplorerOutputDataPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(530, 230, 160, 120));

        stepExplorerSaveButton.setText("Save");
        stepExplorerSaveButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                stepExplorerSaveButtonActionPerformed(evt);
            }
        });

        stepExplorerPanel.add(stepExplorerSaveButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(610, 620, 70, -1));

        baselineGlobalPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        baselineGlobalPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Baseline"));
        baseLineCorrelationPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        baseLineCorrelationPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Correlation"));
        blCorrelationSelectionLabel.setText("Selection :");
        baseLineCorrelationPanel.add(blCorrelationSelectionLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 30, -1, -1));

        blCorrelationSelectionBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "AUTO", "CROSS", "ALL" }));
        blCorrelationSelectionBox.setToolTipText("Station correlations to use.\n\nAUTO: Use only correlations of each station with itself (i.e. no base lines).Not yet implemented.\nCROSS: Use only correlations between stations (i.e. base lines).\nALL: Use both AUTO and CROSS correlations.");
        baseLineCorrelationPanel.add(blCorrelationSelectionBox, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 50, 80, -1));

        blCorrelationTypeLabel.setText("Type :");
        baseLineCorrelationPanel.add(blCorrelationTypeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 10, -1, -1));

        blCorrelationTypeList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "XX", "XY", "YX", "YY" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        blCorrelationTypeList.setToolTipText("Correlations of which polarizations to use, one or more of XX,XY,YX,YY. \n\nAs an example, suppose you select 'XX' here and set Selection to AUTO, then the X polarization signal of each station is correlated with itself. However if we set Selection to CROSS, then the X polarization of station A is correlated with the X polarization of station B for each base line.");
        blCorrelationTypeScrollPane.setViewportView(blCorrelationTypeList);

        baseLineCorrelationPanel.add(blCorrelationTypeScrollPane, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 30, 50, 80));

        baselineGlobalPanel.add(baseLineCorrelationPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 180, 170, 120));

        BaselineSelectionPanel.setLayout(new java.awt.BorderLayout());

        BaselineSelectionPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Selection"));
        baselineStationsTable.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {
                {"0", "1"},
                {"0", "2"},
                {"0", "3"},
                {"1", "0"},
                {"1", "2"},
                {"1,2,3", "1,2,3"}
            },
            new String [] {
                "Station 1", "Station 2"
            }
        ) {
            Class[] types = new Class [] {
                java.lang.String.class, java.lang.String.class
            };
            boolean[] canEdit = new boolean [] {
                false, false
            };

            public Class getColumnClass(int columnIndex) {
                return types [columnIndex];
            }

            public boolean isCellEditable(int rowIndex, int columnIndex) {
                return canEdit [columnIndex];
            }
        });
        baselineStationsTable.setToolTipText("The baselines used");
        baselineStationsScrollPane.setViewportView(baselineStationsTable);

        BaselineSelectionPanel.add(baselineStationsScrollPane, java.awt.BorderLayout.CENTER);

        baselineModsPanel.setLayout(new java.awt.GridBagLayout());

        addBaseLineButton.setText("A");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.ipadx = 7;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        baselineModsPanel.add(addBaseLineButton, gridBagConstraints);

        modifyBaseLineButton.setText("M");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        baselineModsPanel.add(modifyBaseLineButton, gridBagConstraints);

        deleteBaseLineButton.setText("D");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 3;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.ipadx = 6;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        baselineModsPanel.add(deleteBaseLineButton, gridBagConstraints);

        baselineUseAllCheckbox.setText("Use all baselines");
        baselineUseAllCheckbox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        baselineUseAllCheckbox.setHorizontalAlignment(javax.swing.SwingConstants.RIGHT);
        baselineUseAllCheckbox.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                baselineUseAllCheckboxStateChanged(evt);
            }
        });

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        baselineModsPanel.add(baselineUseAllCheckbox, gridBagConstraints);

        BaselineSelectionPanel.add(baselineModsPanel, java.awt.BorderLayout.SOUTH);

        baselineGlobalPanel.add(BaselineSelectionPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 300, 160));

        stepExplorerPanel.add(baselineGlobalPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 40, 320, 310));

        stepExplorerGlobalSources.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        stepExplorerGlobalSources.setBorder(javax.swing.BorderFactory.createTitledBorder("Sources"));
        stepExplorerSourcesPanel1.setLayout(new java.awt.BorderLayout());

        stepExplorerSourcesPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder("Predict"));
        stepExplorerSourcesModsPanel1.setLayout(new java.awt.GridBagLayout());

        addSourceButton1.setText("A");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel1.add(addSourceButton1, gridBagConstraints);

        modifySourceButton1.setText("M");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel1.add(modifySourceButton1, gridBagConstraints);

        deleteSourceButton2.setText("D");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stepExplorerSourcesModsPanel1.add(deleteSourceButton2, gridBagConstraints);

        stepExplorerSourcesPanel1.add(stepExplorerSourcesModsPanel1, java.awt.BorderLayout.SOUTH);

        stepExplorerSourcesList1.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "3C347", "3C348", "3C349", "3C350" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        stepExplorerSourcesScrollPane1.setViewportView(stepExplorerSourcesList1);

        stepExplorerSourcesPanel1.add(stepExplorerSourcesScrollPane1, java.awt.BorderLayout.CENTER);

        stepExplorerGlobalSources.add(stepExplorerSourcesPanel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, -1, 150));

        stepExplorerSourcesPanel.setLayout(new java.awt.BorderLayout());

        stepExplorerSourcesPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Peel"));
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

        stepExplorerGlobalSources.add(stepExplorerSourcesPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(180, 20, 150, 150));

        stepExplorerPanel.add(stepExplorerGlobalSources, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 40, 340, 180));

        stepExplorerScrollPanel.setViewportView(stepExplorerPanel);

        BBSStepExplorerPanel.add(stepExplorerScrollPanel, java.awt.BorderLayout.CENTER);

        jTabbedPane1.addTab("Step Explorer", BBSStepExplorerPanel);

        add(jTabbedPane1, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });

        add(buttonPanel1, java.awt.BorderLayout.SOUTH);

    }// </editor-fold>//GEN-END:initComponents
            
    private void stepExplorerSaveButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_stepExplorerSaveButtonActionPerformed
// TODO add your handling code here:
    }//GEN-LAST:event_stepExplorerSaveButtonActionPerformed
        
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
        this.restoreBBSGlobalSettingsPanel();
    }//GEN-LAST:event_configurationRevertButtonActionPerformed
        
    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        if(evt.getActionCommand() == "Save Settings") {
            saveInput();
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    private jOTDBnode itsNode = null;
    private MainFrame  itsMainFrame;
    private Vector<jOTDBparam> itsParamList;
    
    // Global Settings parameters
    private jOTDBnode dataSet;
    
    private jOTDBnode BBDBHost;
    private jOTDBnode BBDBPort;
    private jOTDBnode BBDBDBName;
    private jOTDBnode BBDBUsername;
    private jOTDBnode BBDBPassword;
    
    private jOTDBnode ParmDBInstrument;
    private jOTDBnode ParmDBLocalSky;
    
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
    private javax.swing.JTextField BBSDatasetDeRefText;
    private javax.swing.JLabel BBSDatasetLabel;
    private javax.swing.JTextField BBSDatasetText;
    private javax.swing.JPanel BBSGlobalSettingsPanel;
    private javax.swing.JPanel BBSStepExplorerPanel;
    private nl.astron.lofar.sas.otbcomponents.bbs.BBSStrategyPanel BBSStrategyPanel;
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
    private javax.swing.JButton addSourceButton1;
    private javax.swing.JPanel baseLineCorrelationPanel;
    private javax.swing.JPanel baselineGlobalPanel;
    private javax.swing.JPanel baselineModsPanel;
    private javax.swing.JScrollPane baselineStationsScrollPane;
    private javax.swing.JTable baselineStationsTable;
    private javax.swing.JCheckBox baselineUseAllCheckbox;
    private javax.swing.JComboBox blCorrelationSelectionBox;
    private javax.swing.JLabel blCorrelationSelectionLabel;
    private javax.swing.JLabel blCorrelationTypeLabel;
    private javax.swing.JList blCorrelationTypeList;
    private javax.swing.JScrollPane blCorrelationTypeScrollPane;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JButton configurationRevertButton;
    private javax.swing.JButton deleteBaseLineButton;
    private javax.swing.JButton deleteInstrumentModelButton;
    private javax.swing.JButton deleteSolvableParmButton;
    private javax.swing.JButton deleteSolvableParmButton1;
    private javax.swing.JButton deleteSourceButton1;
    private javax.swing.JButton deleteSourceButton2;
    private javax.swing.JButton exploreFirstChildStepButton;
    private javax.swing.JButton exploreNextStepButton;
    private javax.swing.JButton exploreParentStepButton;
    private javax.swing.JButton explorePreviousStepButton;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.JButton modifyBaseLineButton;
    private javax.swing.JButton modifyInstrumentModelButton;
    private javax.swing.JButton modifySolvableParmButton;
    private javax.swing.JButton modifySolvableParmButton1;
    private javax.swing.JButton modifySourceButton;
    private javax.swing.JButton modifySourceButton1;
    private javax.swing.JPanel seOperationAttributeGroup1;
    private javax.swing.JPanel seOperationAttributeGroup2;
    private javax.swing.JPanel seOperationAttributeGroup3;
    private javax.swing.JPanel seOperationAttributeGroup4;
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
    private javax.swing.JLabel seOperationAttributeUnitLabel1;
    private javax.swing.JLabel seOperationAttributeUnitLabel2;
    private javax.swing.JPanel seOperationAttributesInputPanel;
    private javax.swing.JScrollPane seOperationAttributesScrollPane;
    private javax.swing.JList seSolvableParmList;
    private javax.swing.JList seSolvableParmList1;
    private javax.swing.JScrollPane seSolvableParmScrollPane;
    private javax.swing.JScrollPane seSolvableParmScrollPane1;
    private javax.swing.JPanel stepExplorerGlobalSources;
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
    private javax.swing.JPanel stepExplorerPanel;
    private javax.swing.JButton stepExplorerRevertButton;
    private javax.swing.JButton stepExplorerSaveButton;
    private javax.swing.JScrollPane stepExplorerScrollPanel;
    private javax.swing.JList stepExplorerSourcesList;
    private javax.swing.JList stepExplorerSourcesList1;
    private javax.swing.JPanel stepExplorerSourcesModsPanel;
    private javax.swing.JPanel stepExplorerSourcesModsPanel1;
    private javax.swing.JPanel stepExplorerSourcesModsPanel2;
    private javax.swing.JPanel stepExplorerSourcesPanel;
    private javax.swing.JPanel stepExplorerSourcesPanel1;
    private javax.swing.JScrollPane stepExplorerSourcesScrollPane;
    private javax.swing.JScrollPane stepExplorerSourcesScrollPane1;
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
