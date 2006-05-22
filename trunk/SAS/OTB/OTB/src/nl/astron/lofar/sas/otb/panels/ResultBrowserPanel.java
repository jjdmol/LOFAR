/*
 * SamplePanel.java
 *
 * Created on January 13, 2006, 2:58 PM
 */

package nl.astron.lofar.sas.otb.panels;


import java.util.Iterator;
import java.util.Vector;
import javax.swing.JPanel;
import javax.swing.event.TreeModelEvent;
import javax.swing.event.TreeModelListener;
import org.apache.log4j.Logger;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.ResultPanelHelper;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.jParmDBnode;
import nl.astron.lofar.sas.otb.util.treemanagers.OTDBNodeTreeManager;
import nl.astron.lofar.sas.otb.util.treemanagers.ParmDBTreeManager;
import nl.astron.lofar.sas.otb.util.treenodes.TreeNode;
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

        itsPanelHelper=new ResultPanelHelper();
        
        /// TOBECHANGED
        parmDBPlotPanel1 = new nl.astron.lofar.sas.otbcomponents.ParmDBPlotPanel();
        ///
        
        treePanel.setTitle("Observation Tree");
        buttonPanel1.addButton("Query Panel");
        buttonPanel1.addButton("Info");
        buttonPanel1.addButton("Exit");

    }
    
    public boolean initializePlugin(MainFrame mainframe) {
        if (mainframe == null) {
            logger.debug("ERROR, no mainframe given");
            return false;
        }
        itsMainFrame=mainframe;
        itsTreeID=itsMainFrame.getSharedVars().getTreeID();

        /// TOBECHANGED
        parmDBPlotPanel1.setMainFrame(itsMainFrame);
        ///
        
        
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
    
    public String getFriendlyName() {
        return getFriendlyNameStatic()+"("+itsTreeID+")";
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
            OTDBNodeTreeManager treeManager = OTDBNodeTreeManager.getInstance(itsMainFrame.getUserAccount());
            treeManager.addTreeModelListener(new TreeModelListener(){
                public void treeStructureChanged(TreeModelEvent e){}
                public void treeNodesRemoved(TreeModelEvent e){}
                public void treeNodesChanged(TreeModelEvent e){}
                public void treeNodesInserted(TreeModelEvent e){
                    TreeNode item = (TreeNode)e.getSource();
                    if(item.getName().equalsIgnoreCase("Observation.AO")){
                        String[] args = new String[2];
                        args[0]="ParmDB";
                        args[1]="BBS";
                        TreeNode parmDBnode =ParmDBTreeManager.getInstance(itsMainFrame.getUserAccount()).getRootNode(args);
                        
                        item.add(parmDBnode);
                    }
                }
                
                
            });
            
            
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
    
    private void changeTreeSelection(jOTDBnode aNode) {
        // save selected panel
        int savedSelection=jTabbedPane1.getSelectedIndex();
        logger.debug("ChangeSelection for node: " + aNode.name);
        itsSelectedNode=aNode;
        jOTDBparam aParam = null;
        
        jTabbedPane1.removeAll();

        // Check if the nodename uses specific panels and create them
        Vector aPanelList=null;
        if (itsPanelHelper.isKey(aNode.name)) {
            aPanelList=itsPanelHelper.getPanels(aNode.name);
        } else {
            aPanelList=itsPanelHelper.getPanels("*");            
        }

        if (aNode.leaf) {
        } else {
        }
        
        // Loop through all the panels and fill the tabPanel with them
        Iterator it = aPanelList.iterator();
        while (it.hasNext()) {
            boolean skip = false;
            JPanel p=null;
            String aPanelName= it.next().toString();
            // Check if the wanted panel is the Node or Parameter Panel. if so only add depending on leaf 
            if ((aPanelName.contains("NodeViewPanel") && aNode.leaf) |
                    (aPanelName.contains("ParameterViewPanel") && !aNode.leaf)) {
                skip = true;
            }
            if (!skip) {
                logger.debug("Getting panel for: "+aPanelName);
                try {
                    p = (JPanel) Class.forName(aPanelName).newInstance();
                } catch (ClassNotFoundException ex) {
                    logger.debug("Error during getPanel: "+ ex);
                    return;
                } catch (InstantiationException ex) {
                    logger.debug("Error during getPanel: "+ ex);
                    return;
                } catch (IllegalAccessException ex) {
                    logger.debug("Error during getPanel: "+ ex);
                    return;
                }
                if (p!=null) {
                    jTabbedPane1.addTab(((IViewPanel)p).getShortName(),null,p,"");
                    ((IViewPanel)p).setMainFrame(itsMainFrame);
                    ((IViewPanel)p).setContent(aNode);
                }
            } else {
                logger.debug("Skipping panel for: "+aPanelName);
            }
        }
        jTabbedPane1.setSelectedIndex(savedSelection);
    }
    
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
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();
        jSplitPane = new javax.swing.JSplitPane();
        treePanel = new nl.astron.lofar.sas.otbcomponents.TreePanel();
        jTabbedPane1 = new javax.swing.JTabbedPane();
        logParamPanel1 = new nl.astron.lofar.sas.otbcomponents.LogParamPanel();
        jPanel1 = new javax.swing.JPanel();

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

        jTabbedPane1.addTab("Log", logParamPanel1);

        jTabbedPane1.addTab("Query Results", jPanel1);

        jSplitPane.setRightComponent(jTabbedPane1);

        add(jSplitPane, java.awt.BorderLayout.WEST);

    }// </editor-fold>//GEN-END:initComponents
    
    private void treePanelValueChanged(javax.swing.event.TreeSelectionEvent evt) {//GEN-FIRST:event_treePanelValueChanged
        logger.debug("treeSelectionEvent: " + evt);
        if (evt != null && evt.getNewLeadSelectionPath() != null &&
                evt.getNewLeadSelectionPath().getLastPathComponent() != null) {
            
            TreeNode treeNode = (TreeNode)evt.getNewLeadSelectionPath().getLastPathComponent();
            
            if(treeNode.getUserObject() instanceof jOTDBnode){
                changeTreeSelection((jOTDBnode)treeNode.getUserObject());
                
            } else if(treeNode.getUserObject() instanceof jParmDBnode){
                jParmDBnode selectedNode = (jParmDBnode)treeNode.getUserObject();
                logger.debug("selected ParmDB node: "+selectedNode.name);
                
                int savedSelection=jTabbedPane1.getSelectedIndex();
                logger.debug("ChangeSelection for node: " + selectedNode.name);
                //itsSelectedNode=selectedNode;
                
                jTabbedPane1.removeTabAt(0);
                jTabbedPane1.insertTab("ParmDBPlotter",null,parmDBPlotPanel1,"",0);
                parmDBPlotPanel1.setParam(selectedNode.nodeID());
                
                
                //logParamPanel1.setNode(aNode);
                jTabbedPane1.setSelectedIndex(0);
            }
            
        }
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
    
    private ResultPanelHelper itsPanelHelper=null;

    /// TOBECHANGED
    private nl.astron.lofar.sas.otbcomponents.ParmDBPlotPanel parmDBPlotPanel1;
    ///
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JSplitPane jSplitPane;
    private javax.swing.JTabbedPane jTabbedPane1;
    private nl.astron.lofar.sas.otbcomponents.LogParamPanel logParamPanel1;
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
