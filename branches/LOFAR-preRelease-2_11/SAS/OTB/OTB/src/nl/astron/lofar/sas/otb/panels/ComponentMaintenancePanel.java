/*
 * ComponentMaintenancePanel.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
 *
 */

package nl.astron.lofar.sas.otb.panels;
import java.rmi.RemoteException;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.lofarutils.inputfieldbuilder.inputFieldBuilder;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb3.jVICnodeDef;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.treemanagers.OTDBParamTreeManager;
import nl.astron.lofar.sas.otb.util.treenodes.TreeNode;
import nl.astron.lofar.sas.otbcomponents.ComponentPanel;
import nl.astron.lofar.sas.otbcomponents.VICnodeDefViewPanel;
import org.apache.log4j.Logger;

/**
  Class that will show the components to the user, all component actions can be
 * handled from within this Gui panel.
 *
 * @created 24-01-2006
 * @author  Coolen
 * @version $Id$
 * @updated
 */
public class ComponentMaintenancePanel extends javax.swing.JPanel
        implements IPluginPanel {
    
    static Logger logger = Logger.getLogger(ComponentMaintenancePanel.class);
    static String name = "Component_Maintenance";
    
    /** Creates new form BeanForm */
    public ComponentMaintenancePanel() {
        initComponents();
        initialize();
    }
    
    public boolean hasChanged() {
        return changed;
    }
    
    public void setChanged(boolean flag) {
        changed = flag;
    }
    
    public void checkChanged() {
        if (this.hasChanged()) {
            this.setNewRootNode();
            this.setChanged(false);
        }
    }
    
    
    public boolean initializePlugin(MainFrame mainframe) {
        itsMainFrame = mainframe;
        itsComponentID=itsMainFrame.getSharedVars().getComponentID();
        componentPanel1.setMainFrame(itsMainFrame);
        VICnodeDefViewPanel1.setMainFrame(itsMainFrame);
        
        // check access
        UserAccount userAccount = itsMainFrame.getUserAccount();
        if(userAccount.isAdministrator()) {
            // enable/disable certain controls
        }
        if(userAccount.isAstronomer()) {
            // enable/disable certain controls
        }
        if(userAccount.isInstrumentScientist()) {
            // enable/disable certain controls
        }
        
        setNewRootNode();
        return true;
    }
    
    public void setNewRootNode(){
        logger.debug("SetNewRootNode for component: "+itsComponentID);
        try {
            OTDBParamTreeManager treeManager = OTDBParamTreeManager.getInstance(itsMainFrame.getUserAccount());
            
            itsMainFrame.setHourglassCursor();
            // and create a new root
            treePanel.newRootNode(treeManager.getRootNode(itsComponentID));
            itsMainFrame.setNormalCursor();
        } catch (Exception e) {
            String aS="Exception during setNewRootNode: " + e;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
        }
    }
    
    public String getFriendlyName() {
        return getFriendlyNameStatic()+"("+itsComponentID+")";
    }
    
    public static String getFriendlyNameStatic() {
        return name;
    }
    
    private void changeTreeSelection(jOTDBparam aParam) {
        logger.debug("ChangeSelection for param: " + aParam.name);
        if (inputFieldBuilder.currentInputField != null) {
            inputFieldBuilder.currentInputField.checkPopup();
        }

        itsSelectedParam=aParam;
        if (treePanel.getSelectedRows()[0] == 0) {
            try {
                jVICnodeDef aVICnodeDef = OtdbRmi.getRemoteMaintenance().getComponentNode(itsComponentID);
                VICnodeDefViewPanel1.setContent(aVICnodeDef);
                jSplitPane1.remove(componentPanel1);
                jSplitPane1.setRightComponent(VICnodeDefViewPanel1);
            } catch (RemoteException ex) {
                logger.error("Error getting VICnodeDef");
            }
        } else {
            jSplitPane1.remove(VICnodeDefViewPanel1);
            jSplitPane1.setRightComponent(componentPanel1);
            componentPanel1.setContent(aParam);
        }
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        jSplitPane1 = new javax.swing.JSplitPane();
        treePanel = new nl.astron.lofar.sas.otbcomponents.TreePanel();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        setLayout(new java.awt.BorderLayout());

        jSplitPane1.setDividerLocation(450);
        treePanel.addTreeSelectionListener(new javax.swing.event.TreeSelectionListener() {
            public void valueChanged(javax.swing.event.TreeSelectionEvent evt) {
                treePanelValueChanged(evt);
            }
        });

        jSplitPane1.setLeftComponent(treePanel);

        add(jSplitPane1, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });

        add(buttonPanel1, java.awt.BorderLayout.SOUTH);

    }// </editor-fold>//GEN-END:initComponents
    
    private void treePanelValueChanged(javax.swing.event.TreeSelectionEvent evt) {//GEN-FIRST:event_treePanelValueChanged
        logger.debug("treeSelectionEvent: " + evt);
        if (evt != null && evt.getNewLeadSelectionPath() != null &&
                evt.getNewLeadSelectionPath().getLastPathComponent() != null) {
            TreeNode componentNode= (TreeNode)evt.getNewLeadSelectionPath().getLastPathComponent();
            
            changeTreeSelection((jOTDBparam)componentNode.getUserObject());
            
        }
    }//GEN-LAST:event_treePanelValueChanged
    
    private void initialize() {
        treePanel.setTitle("Component List");
        buttonPanel1.addButton("Exit");
        
        componentPanel1= new ComponentPanel();
        VICnodeDefViewPanel1 = new VICnodeDefViewPanel();
        jSplitPane1.setRightComponent(componentPanel1);
        jSplitPane1.updateUI();
        
        //Add Actionlisteners for Component Panel and VICnodeDefPanel
        componentPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                componentPanel1ActionPerformed(evt);
            }
        });
        VICnodeDefViewPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                VICnodeDefViewPanel1ActionPerformed(evt);
            }
        });
        
    }
    
    
    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        logger.trace("actionPerformed: " + evt);
        logger.trace("Trigger: "+evt.getActionCommand());
        if (evt.getActionCommand().equals("Exit")) {
            itsMainFrame.unregisterPlugin(this.getFriendlyName());
            itsMainFrame.showPanel(MainPanel.getFriendlyNameStatic());
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    private void VICnodeDefViewPanel1ActionPerformed(java.awt.event.ActionEvent evt) {
        logger.debug("VICnodeDefActionPerformed: " + evt);
        // flag has to be set that ppl using this treeid should be able to see that it's info has been changed
        itsMainFrame.setChanged("Home",true);
    }
    
    private void componentPanel1ActionPerformed(java.awt.event.ActionEvent evt) {
        logger.debug("componentActionPerformed: " + evt);
        // flag has to be set that ppl using this treeid should be able to see that it's info has been changed
        itsMainFrame.setChanged("Home",true);
    }
    
    private MainFrame itsMainFrame;
    private jOTDBparam itsSelectedParam;
    
    // keep the ComponentId that belongs to this panel
    private int itsComponentID = 0;
    private boolean changed=false;
    
    private nl.astron.lofar.sas.otbcomponents.ComponentPanel componentPanel1;
    private nl.astron.lofar.sas.otbcomponents.VICnodeDefViewPanel VICnodeDefViewPanel1;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JSplitPane jSplitPane1;
    private nl.astron.lofar.sas.otbcomponents.TreePanel treePanel;
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
