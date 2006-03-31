/*
 * SamplePanel.java
 *
 * Created on January 13, 2006, 2:58 PM
 */

package nl.astron.lofar.sas.otb.panels;

import java.rmi.RemoteException;
import org.apache.log4j.Logger;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
import nl.astron.lofar.sas.otb.util.OTDBtreeNode;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otbcomponents.TreeInfoDialog;


/**
 * This panel contains a TreePanel and some textfields that display information
 * about the selected node. Also a log screen to be able to view logging on the
 * selected jobs.
 *
 * An event listener was added to catch TreeSelection events from the TreePanel
 *
 * @author Coolen
 */
public class ResultBrowserPanel extends javax.swing.JPanel 
                       implements IPluginPanel {

    static Logger logger = Logger.getLogger(ResultBrowserPanel.class);
    static String name = "ResultBrowser";
    
        
    /** Creates new form BeanForm */
    public ResultBrowserPanel() {
        initComponents();
        initialize();
    }
        
    public void initialize() {
        treePanel.setTitle("Observation Tree");
        buttonPanel1.addButton("Query Panel");
        buttonPanel1.addButton("Info");
        buttonPanel1.addButton("Exit");
        
        nodeViewPanel1.enableInstances(true);
        nodeViewPanel1.enableLimits(true);
        nodeViewPanel1.enableDescription(true);
        nodeViewPanel1.enableButtons(true);
        
        parameterViewPanel1.enableLimits(true);
        parameterViewPanel1.enableDescription(true);        
        parameterViewPanel1.enableButtons(true);
    }

    public boolean initializePlugin(MainFrame mainframe) {
        itsTreeID=itsMainFrame.getTreeID();
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
        
        setFieldValidations();

        return true;
    }
        
    public String getFriendlyName() {
        return getFriendlyNameStatic();
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

    
    public static String getFriendlyNameStatic() {
        return name;
    }
    
    public void setNewRootNode() {

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

     private void changeTreeSelection(jOTDBnode aNode) {
        logger.debug("ChangeSelection for node: " + aNode.name);
        itsSelectedNode=aNode;
        jOTDBparam aParam = null;
        if (aNode.leaf) {
            try {
                jTabbedPane1.setSelectedComponent(parameterViewPanel1);
                aParam = itsMainFrame.getOTDBrmi().getRemoteMaintenance().getParam(itsTreeID, aNode.paramDefID());
                parameterViewPanel1.setParam(aParam);
                nodeViewPanel1.setAllEnabled(false);
            } catch (RemoteException ex) {
                logger.debug("Error during getParam: "+ ex);
            }
        } else {
            // this node is a node
            jTabbedPane1.setSelectedComponent(nodeViewPanel1);
            nodeViewPanel1.setNode(aNode);
            parameterViewPanel1.setParam(aParam);
            parameterViewPanel1.setAllEnabled(false);
        }
    }
    
    private void setFieldValidations() {
        parameterViewPanel1.enableParamName(false);
        parameterViewPanel1.enableIndex(false);
        parameterViewPanel1.enableType(false);
        parameterViewPanel1.enableUnit(false);
        parameterViewPanel1.enablePruning(false);
        parameterViewPanel1.enableValMoment(false);
        parameterViewPanel1.enableRuntimeMod(false);
        parameterViewPanel1.enableLimits(true);
        parameterViewPanel1.enableDescription(true);                
        parameterViewPanel1.enableButtons(true);
        nodeViewPanel1.enableNodeName(false);
        nodeViewPanel1.enableIndex(false);
        nodeViewPanel1.enableInstances(true);
        nodeViewPanel1.enableLimits(true);
        nodeViewPanel1.enableDescription(true);
        nodeViewPanel1.enableButtons(true);            
    }     
     
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
     
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        java.awt.GridBagConstraints gridBagConstraints;

        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();
        jSplitPane = new javax.swing.JSplitPane();
        treePanel = new nl.astron.lofar.sas.otbcomponents.TreePanel();
        jTabbedPane1 = new javax.swing.JTabbedPane();
        nodeViewPanel1 = new nl.astron.lofar.sas.otbcomponents.NodeViewPanel();
        parameterViewPanel1 = new nl.astron.lofar.sas.otbcomponents.ParameterViewPanel();
        logParamPanel1 = new nl.astron.lofar.sas.otbcomponents.LogParamPanel();
        tablePanel1 = new nl.astron.lofar.sas.otbcomponents.TablePanel();

        setLayout(new java.awt.BorderLayout());

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });

        add(buttonPanel1, java.awt.BorderLayout.SOUTH);

        jSplitPane.setDividerLocation(250);
        treePanel.addTreeSelectionListener(new javax.swing.event.TreeSelectionListener() {
            public void valueChanged(javax.swing.event.TreeSelectionEvent evt) {
                treePanelValueChanged(evt);
            }
        });

        jSplitPane.setLeftComponent(treePanel);

        add(jSplitPane, java.awt.BorderLayout.WEST);

        jTabbedPane1.addTab("Node", nodeViewPanel1);

        jTabbedPane1.addTab("Parameter", parameterViewPanel1);

        jTabbedPane1.addTab("Log", logParamPanel1);

        jTabbedPane1.addTab("Query Results", tablePanel1);

        add(jTabbedPane1, java.awt.BorderLayout.CENTER);

    }// </editor-fold>//GEN-END:initComponents

    private void treePanelValueChanged(javax.swing.event.TreeSelectionEvent evt) {//GEN-FIRST:event_treePanelValueChanged
        logger.debug("treeSelectionEvent: " + evt);
        changeTreeSelection(((OTDBtreeNode)evt.getNewLeadSelectionPath().getLastPathComponent()).getOTDBnode());
    }//GEN-LAST:event_treePanelValueChanged

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        logger.debug("actionPerformed: " + evt);
        
        if(evt.getActionCommand() == "Query Panel") {
            // ToDo
        } else if(evt.getActionCommand() == "Info") {
            if (itsTreeID > 0) {
                if (viewInfo() ) {
                    logger.debug("Tree has been changed, reloading table line");
                    // flag has to be set that ppl using this treeid should be able to see that it's info has been changed
                    itsMainFrame.setChanged(this.getFriendlyName(),true);
                }
            }
        } else if(evt.getActionCommand() == "Exit") {
            itsMainFrame.unregisterPlugin(this.getFriendlyName());
            itsMainFrame.showPanel(MainPanel.getFriendlyNameStatic());
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    private MainFrame itsMainFrame;
    private boolean changed=false;
    private jOTDBnode itsSelectedNode = null;
    private TreeInfoDialog treeInfoDialog = null;  
    // keep the TreeId that belongs to this panel
    private int itsTreeID = 0;    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JSplitPane jSplitPane;
    private javax.swing.JTabbedPane jTabbedPane1;
    private nl.astron.lofar.sas.otbcomponents.LogParamPanel logParamPanel1;
    private nl.astron.lofar.sas.otbcomponents.NodeViewPanel nodeViewPanel1;
    private nl.astron.lofar.sas.otbcomponents.ParameterViewPanel parameterViewPanel1;
    private nl.astron.lofar.sas.otbcomponents.TablePanel tablePanel1;
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
