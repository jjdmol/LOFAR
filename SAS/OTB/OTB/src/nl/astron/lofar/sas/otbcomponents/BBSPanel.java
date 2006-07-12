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
        this.detailRevertButton.setEnabled(enabled);
    }
    
    private void setDetailsButtonsVisible(boolean visible) {
        this.detailRevertButton.setVisible(visible);
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
        jTabbedPane1 = new javax.swing.JTabbedPane();
        BBSConfigurationPanel = new javax.swing.JPanel();
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
        BBSStrategyPanel = new javax.swing.JPanel();
        detailParm1Label = new javax.swing.JLabel();
        detailParm1Text = new javax.swing.JTextField();
        detailRevertButton = new javax.swing.JButton();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        setLayout(new java.awt.BorderLayout());

        BBSConfigurationPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        BBSDatasetLabel.setText("Dataset :");
        BBSConfigurationPanel.add(BBSDatasetLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 30, -1, -1));

        BBSDatasetText.setToolTipText("Name of the Measurement Set");
        BBSDatasetText.setMaximumSize(new java.awt.Dimension(440, 19));
        BBSDatasetText.setMinimumSize(new java.awt.Dimension(440, 19));
        BBSDatasetText.setPreferredSize(new java.awt.Dimension(440, 19));
        BBSConfigurationPanel.add(BBSDatasetText, new org.netbeans.lib.awtextra.AbsoluteConstraints(130, 30, 220, -1));

        BBDatabasePanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        BBDatabasePanel.setBorder(javax.swing.BorderFactory.createTitledBorder("BB Database"));
        BBDBDBPasswordLabel1.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        BBDBDBPasswordLabel1.setText("DB Password :");
        BBDatabasePanel.add(BBDBDBPasswordLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 140, 100, 20));

        BBDBDBPasswordText.setToolTipText("Database password used by Blackboard DBMS");
        BBDatabasePanel.add(BBDBDBPasswordText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 140, 220, 20));

        BBDBHostLabel.setText("Host : ");
        BBDatabasePanel.add(BBDBHostLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, -1, -1));

        BBDBHostText.setToolTipText("Hostname/IP Address of BlackBoard DBMS");
        BBDBHostText.setMaximumSize(new java.awt.Dimension(200, 19));
        BBDBHostText.setMinimumSize(new java.awt.Dimension(200, 19));
        BBDBHostText.setPreferredSize(new java.awt.Dimension(200, 19));
        BBDatabasePanel.add(BBDBHostText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 20, 220, -1));

        BBDBPortText.setToolTipText("Port used by Blackboard DBMS");
        BBDatabasePanel.add(BBDBPortText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 50, 60, 20));

        BBDBPortLabel.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        BBDBPortLabel.setText("Port :");
        BBDatabasePanel.add(BBDBPortLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 50, 81, -1));

        BBDBDBNameLabel.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        BBDBDBNameLabel.setText("DB Name :");
        BBDatabasePanel.add(BBDBDBNameLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 80, 81, -1));

        BBDBDBNameText.setToolTipText("Database name used by Blackboard DBMS");
        BBDatabasePanel.add(BBDBDBNameText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 80, 220, -1));

        BBDBDBUsernameText.setToolTipText("Database username used by Blackboard DBMS");
        BBDatabasePanel.add(BBDBDBUsernameText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 110, 220, 20));

        BBDBDBUsernameLabel.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        BBDBDBUsernameLabel.setText("DB Username :");
        BBDatabasePanel.add(BBDBDBUsernameLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 110, 100, 20));

        BBSConfigurationPanel.add(BBDatabasePanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 60, 350, 170));

        ParmDBPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        ParmDBPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("ParmDB"));
        ParmDBInstrumentLabel.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        ParmDBInstrumentLabel.setText("Instrument :");
        ParmDBPanel.add(ParmDBInstrumentLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 100, 20));

        ParmDBInstrumentText.setToolTipText("Instrument parameters (MS table)");
        ParmDBPanel.add(ParmDBInstrumentText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 20, 220, 20));

        ParmDBLocalSkyLabel.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        ParmDBLocalSkyLabel.setText("Local Sky :");
        ParmDBPanel.add(ParmDBLocalSkyLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 50, 100, 20));

        ParmDBLocalSkyText.setToolTipText("Local sky parameters (MS table)");
        ParmDBLocalSkyText.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ParmDBLocalSkyTextActionPerformed(evt);
            }
        });

        ParmDBPanel.add(ParmDBLocalSkyText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 50, 220, 20));

        BBSConfigurationPanel.add(ParmDBPanel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 240, 350, 80));

        configurationRevertButton.setText("Revert");
        configurationRevertButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                configurationRevertButtonActionPerformed(evt);
            }
        });

        BBSConfigurationPanel.add(configurationRevertButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(270, 330, 81, -1));

        jTabbedPane1.addTab("Configuration", BBSConfigurationPanel);

        BBSStrategyPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        detailParm1Label.setText("Parameter 1 :");
        BBSStrategyPanel.add(detailParm1Label, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 10, -1, -1));

        detailParm1Text.setToolTipText("Give Machine where AMC server runs (hostname or IP address)");
        BBSStrategyPanel.add(detailParm1Text, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 10, 440, -1));

        detailRevertButton.setText("Revert");
        detailRevertButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                detailRevertButtonActionPerformed(evt);
            }
        });

        BBSStrategyPanel.add(detailRevertButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 40, -1, -1));

        jTabbedPane1.addTab("Strategy", BBSStrategyPanel);

        add(jTabbedPane1, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });

        add(buttonPanel1, java.awt.BorderLayout.SOUTH);

    }// </editor-fold>//GEN-END:initComponents

    private void ParmDBLocalSkyTextActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ParmDBLocalSkyTextActionPerformed
// TODO add your handling code here:
    }//GEN-LAST:event_ParmDBLocalSkyTextActionPerformed
    
    private void configurationRevertButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_configurationRevertButtonActionPerformed
        this.restoreBBSOverviewPanel();
    }//GEN-LAST:event_configurationRevertButtonActionPerformed
    
    private void detailRevertButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_detailRevertButtonActionPerformed
        this.restoreBBSDetailsPanel();
    }//GEN-LAST:event_detailRevertButtonActionPerformed
    
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
    private javax.swing.JPanel BBSConfigurationPanel;
    private javax.swing.JLabel BBSDatasetLabel;
    private javax.swing.JTextField BBSDatasetText;
    private javax.swing.JPanel BBSStrategyPanel;
    private javax.swing.JLabel ParmDBInstrumentLabel;
    private javax.swing.JTextField ParmDBInstrumentText;
    private javax.swing.JLabel ParmDBLocalSkyLabel;
    private javax.swing.JTextField ParmDBLocalSkyText;
    private javax.swing.JPanel ParmDBPanel;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JButton configurationRevertButton;
    private javax.swing.JLabel detailParm1Label;
    private javax.swing.JTextField detailParm1Text;
    private javax.swing.JButton detailRevertButton;
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
