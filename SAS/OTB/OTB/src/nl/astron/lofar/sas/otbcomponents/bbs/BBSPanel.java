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
                    setField(itsNode,aParam,aNode);
                    
                    //we need to get all the childs from the following nodes as well.
                }else if (LofarUtils.keyName(aNode.name).equals("ParmDB")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }else if (LofarUtils.keyName(aNode.name).equals("BBDB")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }else if (LofarUtils.keyName(aNode.name).equals("Strategy")) {
                    this.aBBSStrategyPanel.setMainFrame(this.itsMainFrame);
                    this.aBBSStrategyPanel.setContent(aNode);
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
                setField(aNode,aParam,aHWNode);
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
        this.ParmDBHistoryText.setText(ParmDBHistory.limits);
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
    private void setField(jOTDBnode parent,jOTDBparam aParam, jOTDBnode aNode) {
        // OLAP_HW settings
        if (aParam==null) {
            return;
        }
        boolean isRef = LofarUtils.isReference(aNode.limits);
        String aKeyName = LofarUtils.keyName(aNode.name);
        String parentName = String.valueOf(parent.name);
        
        if(parentName.equals("BBS")){
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
            }
        } else if(parentName.equals("BBDB")){
            
            if (aKeyName.equals("DBName")) {
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
            }
        } else if(parentName.equals("ParmDB")){
            if (aKeyName.equals("Instrument")) {
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
            }else if (aKeyName.equals("History")) {
                this.ParmDBHistoryText.setToolTipText(aParam.description);
                this.ParmDBHistory=aNode;
                if (isRef && aParam != null) {
                    ParmDBHistoryText.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    ParmDBHistoryText.setText(aNode.limits);
                }
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
        } 
        if (this.BBDBHost != null && !this.BBDBHostText.getText().equals(BBDBHost.limits)) {
            BBDBHost.limits = BBDBHostText.getText();
            logger.trace("Variable BBS ("+BBDBHost.name+"//"+BBDBHost.treeID()+"//"+BBDBHost.nodeID()+"//"+BBDBHost.parentID()+"//"+BBDBHost.paramDefID()+") updated to :"+BBDBHost.limits);
            saveNode(BBDBHost);
        } 
        if (this.BBDBPort != null && !this.BBDBPortText.getText().equals(BBDBPort.limits)) {
            BBDBPort.limits = BBDBPortText.getText();
            logger.trace("Variable BBS ("+BBDBPort.name+"//"+BBDBPort.treeID()+"//"+BBDBPort.nodeID()+"//"+BBDBPort.parentID()+"//"+BBDBPort.paramDefID()+") updated to :"+BBDBPort.limits);
            saveNode(BBDBPort);
        } 
        if (this.BBDBDBName != null && !this.BBDBDBNameText.getText().equals(BBDBDBName.limits)) {
            BBDBDBName.limits = BBDBDBNameText.getText();
            logger.trace("Variable BBS ("+BBDBDBName.name+"//"+BBDBDBName.treeID()+"//"+BBDBDBName.nodeID()+"//"+BBDBDBName.parentID()+"//"+BBDBDBName.paramDefID()+") updated to :"+BBDBDBName.limits);
            saveNode(BBDBDBName);
        } 
        if (this.BBDBUsername != null && !this.BBDBDBUsernameText.getText().equals(BBDBUsername.limits)) {
            BBDBUsername.limits = BBDBDBUsernameText.getText();
            logger.trace("Variable BBS ("+BBDBUsername.name+"//"+BBDBUsername.treeID()+"//"+BBDBUsername.nodeID()+"//"+BBDBUsername.parentID()+"//"+BBDBUsername.paramDefID()+") updated to :"+BBDBUsername.limits);
            saveNode(BBDBUsername);
        } 
        if (this.BBDBPassword != null && !this.BBDBDBPasswordText.getText().equals(BBDBPassword.limits)) {
            BBDBPassword.limits = BBDBDBPasswordText.getText();
            logger.trace("Variable BBS ("+BBDBPassword.name+"//"+BBDBPassword.treeID()+"//"+BBDBPassword.nodeID()+"//"+BBDBPassword.parentID()+"//"+BBDBPassword.paramDefID()+") updated to :"+BBDBPassword.limits);
            saveNode(BBDBPassword);
        } 
        if (this.ParmDBInstrument != null && !this.ParmDBInstrumentText.getText().equals(ParmDBInstrument.limits)) {
            ParmDBInstrument.limits = ParmDBInstrumentText.getText();
            logger.trace("Variable BBS ("+ParmDBInstrument.name+"//"+ParmDBInstrument.treeID()+"//"+ParmDBInstrument.nodeID()+"//"+ParmDBInstrument.parentID()+"//"+ParmDBInstrument.paramDefID()+") updated to :"+ParmDBInstrument.limits);
            saveNode(ParmDBInstrument);
        } 
        if (this.ParmDBLocalSky != null && !this.ParmDBLocalSkyText.getText().equals(ParmDBLocalSky.limits)) {
            ParmDBLocalSky.limits = ParmDBLocalSkyText.getText();
            logger.trace("Variable BBS ("+ParmDBLocalSky.name+"//"+ParmDBLocalSky.treeID()+"//"+ParmDBLocalSky.nodeID()+"//"+ParmDBLocalSky.parentID()+"//"+ParmDBLocalSky.paramDefID()+") updated to :"+ParmDBLocalSky.limits);
            saveNode(ParmDBLocalSky);
        }
        if (this.ParmDBHistory != null && !this.ParmDBHistoryText.getText().equals(ParmDBHistory.limits)) {
            ParmDBHistory.limits = ParmDBHistoryText.getText();
            logger.trace("Variable BBS ("+ParmDBHistory.name+"//"+ParmDBHistory.treeID()+"//"+ParmDBHistory.nodeID()+"//"+ParmDBHistory.parentID()+"//"+ParmDBHistory.paramDefID()+") updated to :"+ParmDBHistory.limits);
            saveNode(ParmDBHistory);
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
        BBSGlobalSettingsPanel = new javax.swing.JPanel();
        BBSGlobalSettingsFieldPanel = new javax.swing.JPanel();
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
        ParmDBHistoryText = new javax.swing.JTextField();
        ParmDBHistoryLabel = new javax.swing.JLabel();
        configurationRevertButton = new javax.swing.JButton();
        BBSDatasetDeRefText = new javax.swing.JTextField();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();
        aBBSStrategyPanel = new nl.astron.lofar.sas.otbcomponents.bbs.BBSStrategyPanel();

        setLayout(new java.awt.BorderLayout());

        BBSGlobalSettingsPanel.setLayout(new java.awt.BorderLayout());

        BBSGlobalSettingsFieldPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        BBSDatasetLabel.setText("Dataset :");
        BBSGlobalSettingsFieldPanel.add(BBSDatasetLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 30, -1, -1));

        BBSDatasetText.setToolTipText("Path to the measurement set");
        BBSDatasetText.setMaximumSize(new java.awt.Dimension(440, 19));
        BBSDatasetText.setMinimumSize(new java.awt.Dimension(440, 19));
        BBSDatasetText.setPreferredSize(new java.awt.Dimension(440, 19));
        BBSGlobalSettingsFieldPanel.add(BBSDatasetText, new org.netbeans.lib.awtextra.AbsoluteConstraints(130, 30, 220, -1));

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

        BBSGlobalSettingsFieldPanel.add(BBDatabasePanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 60, 350, 170));

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

        ParmDBHistoryText.setToolTipText("Path to the AIPS++ table containing the local sky model parameters");
        ParmDBHistoryText.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ParmDBHistoryTextActionPerformed(evt);
            }
        });

        ParmDBPanel.add(ParmDBHistoryText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 80, 220, 20));

        ParmDBHistoryLabel.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        ParmDBHistoryLabel.setText("History :");
        ParmDBPanel.add(ParmDBHistoryLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 80, 100, 20));

        BBSGlobalSettingsFieldPanel.add(ParmDBPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 240, 350, 110));

        configurationRevertButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Undo16.gif")));
        configurationRevertButton.setText("Revert");
        configurationRevertButton.setMaximumSize(new java.awt.Dimension(100, 25));
        configurationRevertButton.setMinimumSize(new java.awt.Dimension(100, 25));
        configurationRevertButton.setPreferredSize(new java.awt.Dimension(100, 25));
        configurationRevertButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                configurationRevertButtonActionPerformed(evt);
            }
        });

        BBSGlobalSettingsFieldPanel.add(configurationRevertButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 360, 100, -1));

        BBSDatasetDeRefText.setEditable(false);
        BBSDatasetDeRefText.setToolTipText("Dereferenced and actually used value.");
        BBSDatasetDeRefText.setMaximumSize(new java.awt.Dimension(440, 19));
        BBSDatasetDeRefText.setMinimumSize(new java.awt.Dimension(440, 19));
        BBSDatasetDeRefText.setPreferredSize(new java.awt.Dimension(440, 19));
        BBSGlobalSettingsFieldPanel.add(BBSDatasetDeRefText, new org.netbeans.lib.awtextra.AbsoluteConstraints(360, 30, 180, -1));

        BBSGlobalSettingsPanel.add(BBSGlobalSettingsFieldPanel, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });

        BBSGlobalSettingsPanel.add(buttonPanel1, java.awt.BorderLayout.SOUTH);

        jTabbedPane1.addTab("Global Settings", BBSGlobalSettingsPanel);

        jTabbedPane1.addTab("Strategy", aBBSStrategyPanel);

        add(jTabbedPane1, java.awt.BorderLayout.CENTER);

    }// </editor-fold>//GEN-END:initComponents

    private void ParmDBHistoryTextActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ParmDBHistoryTextActionPerformed
// TODO add your handling code here:
    }//GEN-LAST:event_ParmDBHistoryTextActionPerformed

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
    private jOTDBnode ParmDBHistory;
    
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
    private javax.swing.JPanel BBSGlobalSettingsFieldPanel;
    private javax.swing.JPanel BBSGlobalSettingsPanel;
    private javax.swing.JLabel ParmDBHistoryLabel;
    private javax.swing.JTextField ParmDBHistoryText;
    private javax.swing.JLabel ParmDBInstrumentLabel;
    private javax.swing.JTextField ParmDBInstrumentText;
    private javax.swing.JLabel ParmDBLocalSkyLabel;
    private javax.swing.JTextField ParmDBLocalSkyText;
    private javax.swing.JPanel ParmDBPanel;
    private nl.astron.lofar.sas.otbcomponents.bbs.BBSStrategyPanel aBBSStrategyPanel;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JButton configurationRevertButton;
    private javax.swing.JTabbedPane jTabbedPane1;
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
