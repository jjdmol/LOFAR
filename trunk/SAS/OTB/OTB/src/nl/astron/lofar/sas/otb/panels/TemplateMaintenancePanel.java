/*
 * TemplateMaintenancePanel.java
 *
 * Created on March 17, 2006, 1:53 PM
 */

package nl.astron.lofar.sas.otb.panels;

import java.rmi.RemoteException;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
import nl.astron.lofar.sas.otb.util.OTDBtreeNode;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otbcomponents.TreeInfoDialog;
import org.apache.log4j.Logger;

/**
 * The template maintenance screen is used for modifying repository templates 
 * (status usable, being specified and example).
 * or specifying an instrument configuration for an observation request.
 *
 * @author  coolen
 */
public class TemplateMaintenancePanel extends javax.swing.JPanel
        implements IPluginPanel {
    
    static Logger logger = Logger.getLogger(TemplateMaintenancePanel.class);
    static String name = "Template_Maintenance";
    
    
    /** Creates new form TemplateMaintenancePanel */
    public TemplateMaintenancePanel() {
        initComponents();
        initialize();
    }
    
    public boolean initializePlugin(MainFrame mainframe) {
        itsMainFrame = mainframe;
        itsTreeID=itsMainFrame.getTreeID();
        parameterViewPanel1.setMainFrame(itsMainFrame);
        
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
 
        // initialize the tree
        setNewRootNode();

        return true;
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
    
    public void setNewRootNode(){
        try {
            jOTDBnode otdbNode=null;
            if (itsTreeID == 0 ) {
                // create a sample root node.
                otdbNode = new jOTDBnode(0,0,0,0);
                otdbNode.name = "No TreeSelection";
            } else {
                otdbNode = itsMainFrame.getOTDBrmi().getRemoteMaintenance().getTopNode(itsTreeID);
            }
        
            // put the OTDBnode in a wrapper for the tree
            OTDBtreeNode otdbTreeNode = new OTDBtreeNode(otdbNode, itsMainFrame.getOTDBrmi());

            // and create a new root
            treePanel.newRootNode(otdbTreeNode);        
        } catch (Exception e) {
            logger.debug("Exception during setNewRootNode: " + e);
        }
    }
    
    public String getFriendlyName() {
        return getFriendlyNameStatic()+"("+itsTreeID+")";
    }

    public static String getFriendlyNameStatic() {
        return name;
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
        jTabbedPane1 = new javax.swing.JTabbedPane();
        nodeViewPanel1 = new nl.astron.lofar.sas.otbcomponents.NodeViewPanel();
        parameterViewPanel1 = new nl.astron.lofar.sas.otbcomponents.ParameterViewPanel();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        setLayout(new java.awt.BorderLayout());

        jSplitPane1.setDividerLocation(350);
        treePanel.addTreeSelectionListener(new javax.swing.event.TreeSelectionListener() {
            public void valueChanged(javax.swing.event.TreeSelectionEvent evt) {
                treePanelValueChanged(evt);
            }
        });

        jSplitPane1.setLeftComponent(treePanel);

        nodeViewPanel1.setEnabled(false);
        jTabbedPane1.addTab("Node", nodeViewPanel1);

        parameterViewPanel1.setEnabled(false);
        jTabbedPane1.addTab("Param", parameterViewPanel1);

        jSplitPane1.setRightComponent(jTabbedPane1);

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
        changeTreeSelection(((OTDBtreeNode)evt.getNewLeadSelectionPath().getLastPathComponent()).getOTDBnode());
    }//GEN-LAST:event_treePanelValueChanged

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        logger.debug("actionPerformed: " + evt);
        logger.debug("Trigger: "+evt.getActionCommand());
        if (evt.getActionCommand().equals("Delete")) {
            //Check  if the selected node isn't a leaf
            if (itsSelectedNode != null && !itsSelectedNode.leaf) {
                try {
                    if (itsMainFrame.getOTDBrmi().getRemoteMaintenance().deleteNode(itsSelectedNode)) {
                        logger.debug("Node + children deleted");
                        setNewRootNode();
                    }
                } catch (RemoteException ex) {
                    logger.debug("Error during deletion of Node: "+ex);
                }
            }
        } else if (evt.getActionCommand().equals("Duplicate")) {
            
        } else if (evt.getActionCommand().equals("Info")) {
            if (itsTreeID > 0) {
                if (viewInfo() ) {
                    logger.debug("Tree has been changed, reloading table line");
                    // flag has to be set that ppl using this treeid should be able to see that it's info has been changed
                    itsMainFrame.setChanged(this.getFriendlyName(),true);
                }
            }

        } else if (evt.getActionCommand().equals("Cancel")) {
            itsMainFrame.unregisterPlugin(this.getFriendlyName());
            itsMainFrame.showPanel(MainPanel.getFriendlyNameStatic());

        } else if (evt.getActionCommand().equals("Save")) {

        } else if (evt.getActionCommand().equals("Save & Exit")) {

        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    /** Launch TreeInfoDialog,
     *
     * @param  aTreeID  The ID of the chosen tree.
     */
    private boolean viewInfo() {
        //get the selected tree from the database
        
        try {
            jOTDBtree aSelectedTree=itsMainFrame.getOTDBrmi().getRemoteOTDB().getTreeInfo(itsTreeID, false);
            
            if (aSelectedTree != null) {
                // show treeInfo dialog
                treeInfoDialog = new TreeInfoDialog(itsMainFrame,true,aSelectedTree, itsMainFrame);
                treeInfoDialog.setLocationRelativeTo(this);
                treeInfoDialog.setVisible(true);

                if (treeInfoDialog.isChanged()) {
                    logger.debug("tree has been changed and saved");
                } else {
                    logger.debug("tree has not been changed");
                }
               
            } else {
                logger.debug("no tree selected");
            }
        } catch (Exception e) {
            logger.debug("Error in viewInfo: " + e);
        }
        return treeInfoDialog.isChanged();
    }

    private void changeTreeSelection(jOTDBnode aNode) {
        logger.debug("ChangeSelection for node: " + aNode.name);
        itsSelectedNode=aNode;
        if (aNode.leaf) {
            jOTDBparam aParam = null;
            try {
                jTabbedPane1.setSelectedComponent(parameterViewPanel1);
                aParam = itsMainFrame.getOTDBrmi().getRemoteMaintenance().getParam(itsTreeID, aNode.paramDefID());
                nodeViewPanel1.setEnabled(false);
                parameterViewPanel1.setEnabled(true);
                parameterViewPanel1.setParam(aParam); 
            } catch (RemoteException ex) {
                logger.debug("Error during getParam: "+ ex);
            }
        } else {
            // this node is a node
            jTabbedPane1.setSelectedComponent(nodeViewPanel1);
            parameterViewPanel1.setEnabled(false);
            nodeViewPanel1.setEnabled(true);
            nodeViewPanel1.setNode(aNode);
            
        }
    }
    
    private void initialize() {
        treePanel.setTitle("Template List");
        buttonPanel1.addButton("Delete");
        buttonPanel1.addButton("Duplicate");
        buttonPanel1.addButton("Info");
        buttonPanel1.addButton("Cancel");
        buttonPanel1.addButton("Save");
        buttonPanel1.addButton("Save & Exit");
        
        nodeViewPanel1.enableInstances(true);
        nodeViewPanel1.enableLimits(true);
        nodeViewPanel1.enableDescription(true);
        
        parameterViewPanel1.enableLimits(true);
        parameterViewPanel1.enableDescription(true);
    }
    
    private MainFrame itsMainFrame;
    private jOTDBnode itsSelectedNode = null;
    private TreeInfoDialog treeInfoDialog = null;  
    // keep the TreeId that belongs to this panel
    private int itsTreeID = 0;
    private boolean changed = false;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JSplitPane jSplitPane1;
    private javax.swing.JTabbedPane jTabbedPane1;
    private nl.astron.lofar.sas.otbcomponents.NodeViewPanel nodeViewPanel1;
    private nl.astron.lofar.sas.otbcomponents.ParameterViewPanel parameterViewPanel1;
    private nl.astron.lofar.sas.otbcomponents.TreePanel treePanel;
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
        listenerList.add (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeActionListener(java.awt.event.ActionListener listener) {

        listenerList.remove (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Notifies all registered listeners about the event.
     * 
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {

        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList ();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed (event);
            }
        }
    }
    
}
