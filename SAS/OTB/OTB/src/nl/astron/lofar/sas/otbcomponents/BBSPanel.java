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
        this.overviewRevertButton.setEnabled(enabled);
    }
    
    private void setOverviewButtonsVisible(boolean visible) {
        this.overviewRevertButton.setVisible(visible);    }
    
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
        BBSOverviewPanel = new javax.swing.JPanel();
        parm1Label = new javax.swing.JLabel();
        Parm1Text = new javax.swing.JTextField();
        parm2Label = new javax.swing.JLabel();
        parm2Text = new javax.swing.JTextField();
        checkBox1Label = new javax.swing.JLabel();
        checkBox1 = new javax.swing.JCheckBox();
        overviewRevertButton = new javax.swing.JButton();
        BBSDetailPanel = new javax.swing.JPanel();
        detailParm1Label = new javax.swing.JLabel();
        detailParm1Text = new javax.swing.JTextField();
        detailRevertButton = new javax.swing.JButton();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        setLayout(new java.awt.BorderLayout());

        BBSOverviewPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        parm1Label.setText("Parameter 1 :");
        BBSOverviewPanel.add(parm1Label, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 10, -1, -1));

        Parm1Text.setToolTipText("Nr of Samples to integrate");
        Parm1Text.setMaximumSize(new java.awt.Dimension(440, 19));
        Parm1Text.setMinimumSize(new java.awt.Dimension(440, 19));
        Parm1Text.setPreferredSize(new java.awt.Dimension(440, 19));
        BBSOverviewPanel.add(Parm1Text, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 10, -1, -1));

        parm2Label.setText("Parameter 2 :");
        BBSOverviewPanel.add(parm2Label, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 40, -1, -1));

        parm2Text.setToolTipText("Number of seconds that need 2 be buffered");
        parm2Text.setMaximumSize(new java.awt.Dimension(200, 19));
        parm2Text.setMinimumSize(new java.awt.Dimension(200, 19));
        parm2Text.setPreferredSize(new java.awt.Dimension(200, 19));
        BBSOverviewPanel.add(parm2Text, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 40, 440, -1));

        checkBox1Label.setText("Checkbox");
        BBSOverviewPanel.add(checkBox1Label, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 70, 81, -1));

        checkBox1.setToolTipText("Do you want to use an AMC server?");
        checkBox1.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        checkBox1.setMargin(new java.awt.Insets(0, 0, 0, 0));
        BBSOverviewPanel.add(checkBox1, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 70, -1, -1));

        overviewRevertButton.setText("Revert");
        overviewRevertButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                overviewRevertButtonActionPerformed(evt);
            }
        });

        BBSOverviewPanel.add(overviewRevertButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 100, 81, -1));

        jTabbedPane1.addTab("BBS Overview", BBSOverviewPanel);

        BBSDetailPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        detailParm1Label.setText("Parameter 1 :");
        BBSDetailPanel.add(detailParm1Label, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 10, -1, -1));

        detailParm1Text.setToolTipText("Give Machine where AMC server runs (hostname or IP address)");
        BBSDetailPanel.add(detailParm1Text, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 10, 440, -1));

        detailRevertButton.setText("Revert");
        detailRevertButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                detailRevertButtonActionPerformed(evt);
            }
        });

        BBSDetailPanel.add(detailRevertButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 40, -1, -1));

        jTabbedPane1.addTab("BBS Detail", BBSDetailPanel);

        add(jTabbedPane1, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });

        add(buttonPanel1, java.awt.BorderLayout.SOUTH);

    }// </editor-fold>//GEN-END:initComponents
    
    private void overviewRevertButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_overviewRevertButtonActionPerformed
        this.restoreBBSOverviewPanel();
    }//GEN-LAST:event_overviewRevertButtonActionPerformed
    
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
    private javax.swing.JPanel BBSDetailPanel;
    private javax.swing.JPanel BBSOverviewPanel;
    private javax.swing.JTextField Parm1Text;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JCheckBox checkBox1;
    private javax.swing.JLabel checkBox1Label;
    private javax.swing.JLabel detailParm1Label;
    private javax.swing.JTextField detailParm1Text;
    private javax.swing.JButton detailRevertButton;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.JButton overviewRevertButton;
    private javax.swing.JLabel parm1Label;
    private javax.swing.JLabel parm2Label;
    private javax.swing.JTextField parm2Text;
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
