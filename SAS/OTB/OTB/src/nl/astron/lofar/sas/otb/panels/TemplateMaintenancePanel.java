/*
 * TemplateMaintenancePanel.java
 *
 * Created on March 17, 2006, 1:53 PM
 */

package nl.astron.lofar.sas.otb.panels;

import java.rmi.RemoteException;
import javax.swing.JOptionPane;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.treemanagers.OTDBNodeTreeManager;
import nl.astron.lofar.sas.otb.util.treenodes.TreeNode;
import nl.astron.lofar.sas.otbcomponents.TreeInfoDialog;
import org.apache.log4j.Logger;

/**
 * The template maintenance screen is used for modifying repository templates 
 * (status usable, being specified and example).
 * or specifying an instrument configuration for an observation request.
 *
 * @author  Coolen
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
        itsTreeID=itsMainFrame.getSharedVars().getTreeID();
        parameterViewPanel1.setMainFrame(itsMainFrame);
        nodeViewPanel1.setMainFrame(itsMainFrame);
        
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
            OTDBNodeTreeManager treeManager = OTDBNodeTreeManager.getInstance(itsMainFrame.getUserAccount());
            
            itsMainFrame.setHourglassCursor();
            // and create a new root
            String[] args = new String[1];
            args[0]= ""+ itsTreeID;
            treePanel.newRootNode(treeManager.getRootNode(args));
            itsMainFrame.setNormalCursor();
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

        jTabbedPane1.setEnabled(false);
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
        TreeNode otdbNode= (TreeNode)evt.getNewLeadSelectionPath().getLastPathComponent();
        changeTreeSelection((jOTDBnode)otdbNode.getUserObject());
    }//GEN-LAST:event_treePanelValueChanged

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        logger.debug("actionPerformed: " + evt);
        logger.debug("Trigger: "+evt.getActionCommand());
        if (evt.getActionCommand().equals("Delete")) {
            //Check  if the selected node isn't a leaf
            if (itsSelectedNode != null && !itsSelectedNode.leaf) {
                try {
                    if (itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().deleteNode(itsSelectedNode)) {
                        logger.debug("Node + children deleted");
                        setNewRootNode();
                    }
                } catch (RemoteException ex) {
                    logger.debug("Error during deletion of Node: "+ex);
                }
            }
        } else if (evt.getActionCommand().equals("Duplicate")) {
            String answer=JOptionPane.showInputDialog(this,"What is the index for the new subtree?","Enter indexNumber",JOptionPane.QUESTION_MESSAGE);
            if (answer!=null || !answer.equals("")) {
                short idx=Integer.valueOf(answer).shortValue();
                if (idx < 0) {
                    logger.debug("Index value smaller then 1 not allowed");
                    return;
                }
                try {
                    int aN=itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().dupNode(itsTreeID,itsSelectedNode.nodeID(),idx);
                    if (aN>0) {
                        logger.debug("Node duplicated");
                        setNewRootNode();
                    } else {
                        logger.debug("Node duplication failed");
                    }
              } catch (RemoteException ex) {
                    logger.debug("Error during duplication of Node: "+ex);
              }
            }
        } else if (evt.getActionCommand().equals("Info")) {
            if (itsTreeID > 0) {
                if (viewInfo() ) {
                    logger.debug("Tree has been changed, reloading table line");
                    // flag has to be set that ppl using this treeid should be able to see that it's info has been changed
                    itsMainFrame.setChanged(this.getFriendlyName(),true);
                }
            }

        } else if (evt.getActionCommand().equals("Exit")) {
            itsMainFrame.unregisterPlugin(this.getFriendlyName());
            itsMainFrame.showPanel(MainPanel.getFriendlyNameStatic());
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    
   
    /** Launch TreeInfoDialog,
     *
     * @param  aTreeID  The ID of the chosen tree.
     */
    private boolean viewInfo() {
        //get the selected tree from the database
        
        try {
            jOTDBtree aSelectedTree=itsMainFrame.getSharedVars().getOTDBrmi().getRemoteOTDB().getTreeInfo(itsTreeID, false);
            
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
            buttonPanel1.setButtonEnabled("Duplicate",false);
            parameterViewPanel1.setContent(aNode);
        } else {
            // this node is a node
            jTabbedPane1.setSelectedComponent(nodeViewPanel1);
            nodeViewPanel1.setContent(aNode);
            if (treePanel.getSelectedRows()[0] ==  0) {
                buttonPanel1.setButtonEnabled("Duplicate",false);
            } else {
                buttonPanel1.setButtonEnabled("Duplicate",true);
            }
        }
    }
    
    private void initialize() {
        treePanel.setTitle("Template List");
        buttonPanel1.addButton("Delete");
        buttonPanel1.addButton("Duplicate");
        buttonPanel1.addButton("Info");
        buttonPanel1.addButton("Exit");
        
        nodeViewPanel1.enableButtons(true);
       
        parameterViewPanel1.enableButtons(true);
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
