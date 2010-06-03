/*
 *  ParmDBConfigPanel.java
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
import nl.astron.lofar.sas.otb.jotdb3.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBparam;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import org.apache.log4j.Logger;

/**
 * Panel for ParmDBConfigPanel specific configuration
 * 
 * @author pompert
 * @version $Id$
 * @created 11-07-2006, 13:37
 */
public class ParmDBConfigPanel extends javax.swing.JPanel implements IViewPanel{
    
    static Logger logger = Logger.getLogger(ParmDBConfigPanel.class);
    static String name = "ParmDB Configuration";
    
    
    /** Creates new form BeanForm based upon aNode
     *
     * @params  aNode   node to obtain the info from
     *
     */
    public ParmDBConfigPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode=aNode;
        initialize();
        initPanel();
    }
    
    /** Creates new form BeanForm */
    public ParmDBConfigPanel() {
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
        return new ParmDBConfigPanel();
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
    
    /** Restore original Values in Global Settings panel
     */
    private void restoreBBSGlobalSettingsPanel() {
        
        this.ParmDBInstrumentText.setText(ParmDBInstrument.limits);
        this.ParmDBLocalSkyText.setText(ParmDBLocalSky.limits);
        this.ParmDBHistoryText.setText(ParmDBHistory.limits);
    }
    
    private void initialize() {
        buttonPanel1.addButton("Apply Settings");
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
        String parentName = LofarUtils.keyName(parent.name);
        
        if(parentName.equals("ParmDB")){
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
           OtdbRmi.getRemoteMaintenance().saveNode(aNode);
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
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        ParmDBPanel = new javax.swing.JPanel();
        ParmDBInstrumentLabel = new javax.swing.JLabel();
        ParmDBInstrumentText = new javax.swing.JTextField();
        ParmDBLocalSkyLabel = new javax.swing.JLabel();
        ParmDBLocalSkyText = new javax.swing.JTextField();
        ParmDBHistoryText = new javax.swing.JTextField();
        ParmDBHistoryLabel = new javax.swing.JLabel();
        configurationRevertButton = new javax.swing.JButton();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        setLayout(new java.awt.BorderLayout());

        ParmDBPanel.setToolTipText("Information about the parameter databases (e.g. instrument parameters, local sky model).");
        ParmDBPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        ParmDBInstrumentLabel.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        ParmDBInstrumentLabel.setText("Instrument :");
        ParmDBPanel.add(ParmDBInstrumentLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 100, 20));

        ParmDBInstrumentText.setToolTipText("Path to the AIPS++ table containing the instrument parameters");
        ParmDBPanel.add(ParmDBInstrumentText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 20, 420, 20));

        ParmDBLocalSkyLabel.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        ParmDBLocalSkyLabel.setText("Local Sky :");
        ParmDBPanel.add(ParmDBLocalSkyLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 50, 100, 20));

        ParmDBLocalSkyText.setToolTipText("Path to the AIPS++ table containing the local sky model parameters");
        ParmDBPanel.add(ParmDBLocalSkyText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 50, 420, 20));

        ParmDBHistoryText.setToolTipText("Path to the AIPS++ table containing the local sky model parameters");
        ParmDBPanel.add(ParmDBHistoryText, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 80, 420, 20));

        ParmDBHistoryLabel.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        ParmDBHistoryLabel.setText("History :");
        ParmDBPanel.add(ParmDBHistoryLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 80, 100, 20));

        configurationRevertButton.setText("Revert");
        configurationRevertButton.setMaximumSize(new java.awt.Dimension(100, 25));
        configurationRevertButton.setMinimumSize(new java.awt.Dimension(100, 25));
        configurationRevertButton.setPreferredSize(new java.awt.Dimension(100, 25));
        configurationRevertButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                configurationRevertButtonActionPerformed(evt);
            }
        });
        ParmDBPanel.add(configurationRevertButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 120, 80, -1));

        add(ParmDBPanel, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });
        add(buttonPanel1, java.awt.BorderLayout.SOUTH);
    }// </editor-fold>//GEN-END:initComponents

    private void configurationRevertButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_configurationRevertButtonActionPerformed
        this.restoreBBSGlobalSettingsPanel();
    }//GEN-LAST:event_configurationRevertButtonActionPerformed

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        if(evt.getActionCommand().equals("Apply Settings")) {
            saveInput();
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
                
    private jOTDBnode itsNode = null;
    private MainFrame  itsMainFrame;
    private Vector<jOTDBparam> itsParamList;
    
    private jOTDBnode ParmDBInstrument;
    private jOTDBnode ParmDBLocalSky;
    private jOTDBnode ParmDBHistory;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel ParmDBHistoryLabel;
    private javax.swing.JTextField ParmDBHistoryText;
    private javax.swing.JLabel ParmDBInstrumentLabel;
    private javax.swing.JTextField ParmDBInstrumentText;
    private javax.swing.JLabel ParmDBLocalSkyLabel;
    private javax.swing.JTextField ParmDBLocalSkyText;
    private javax.swing.JPanel ParmDBPanel;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JButton configurationRevertButton;
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
