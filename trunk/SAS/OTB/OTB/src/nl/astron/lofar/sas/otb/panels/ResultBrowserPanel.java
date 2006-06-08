/*
 * SamplePanel.java
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
 *
 */

package nl.astron.lofar.sas.otb.panels;


import java.awt.event.ActionEvent;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.Vector;
import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;
import javax.swing.event.TreeModelEvent;
import javax.swing.event.TreeModelListener;
import org.apache.log4j.Logger;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.ResultPanelHelper;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.treemanagers.OTDBNodeTreeManager;
import nl.astron.lofar.sas.otb.util.treemanagers.ParmDBTreeManager;
import nl.astron.lofar.sas.otb.util.treenodes.TreeNode;
import nl.astron.lofar.sas.otbcomponents.TreeInfoDialog;


/**
 * This panel contains a TreePanel and some textfields that display information
 * about the selected treenode. Also a log screen to be able to view logging on the
 * selected jobs.
 *
 * An event listener was added to catch TreeSelection events from the TreePanel
 *
 * @created 13-01-2006, 14:58
 * @author Coolen
 * @version $Id$
 * @updated
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
        
    }
    
    public boolean initializePlugin(MainFrame mainframe) {
        if (mainframe == null) {
            logger.debug("ERROR, no mainframe given");
            return false;
        }
        itsMainFrame=mainframe;
        itsTreeID=itsMainFrame.getSharedVars().getTreeID();
        
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
        
        
        itsMainFrame.setHourglassCursor();
        try {
            OTDBNodeTreeManager treeManager = OTDBNodeTreeManager.getInstance(itsMainFrame.getUserAccount());
            treeManager.addTreeModelListener(new TreeModelListener(){
                public void treeStructureChanged(TreeModelEvent e){}
                public void treeNodesRemoved(TreeModelEvent e){}
                public void treeNodesChanged(TreeModelEvent e){}
                public void treeNodesInserted(TreeModelEvent e){
                    TreeNode item = (TreeNode)e.getSource();
                    if(item.getName().equalsIgnoreCase("Observation.AO")){
                        String[] args = new String[3];
                        args[0]="ParmDB";
                        args[1]="BBS";
                        args[2]="/home/pompert/transfer/tParmFacade.in_mep";
                        TreeNode parmDBnode =ParmDBTreeManager.getInstance(itsMainFrame.getUserAccount()).getRootNode(args);
                        boolean alreadyPresent = false;
                        Enumeration children = item.children();
                        while(children.hasMoreElements() && !alreadyPresent){
                            TreeNode child = (TreeNode)children.nextElement();
                            if (child.getName().equals(parmDBnode.getName())){
                                alreadyPresent=true;
                            }
                        }
                        if(!alreadyPresent){
                            item.add(parmDBnode);
                        }
                        args = new String[3];
                        args[0]="ParmDB-2";
                        args[1]="BBS";
                        args[2]="/home/pompert/transfer/test2ParmFacade.in_mep";
                        TreeNode parmDBnode2 =ParmDBTreeManager.getInstance(itsMainFrame.getUserAccount()).getRootNode(args);
                        boolean alreadyPresent2 = false;
                        while(children.hasMoreElements() && !alreadyPresent2){
                            TreeNode child = (TreeNode)children.nextElement();
                            if (child.getName().equals(parmDBnode2.getName())){
                                alreadyPresent2=true;
                            }
                        }
                        if(!alreadyPresent2){
                            item.add(parmDBnode2);
                        }
                        
                    }
                }
                
                
            });
            
            // and create a new root
            treePanel.newRootNode(treeManager.getRootNode(itsTreeID));
            itsMainFrame.setNormalCursor();
        } catch (Exception e) {
            logger.debug("Exception during setNewRootNode: " + e);
        }
    }
    
    private void changeTreeSelection(TreeNode aNode) {
        // save selected panel
        int savedSelection=jTabbedPane1.getSelectedIndex();
        logger.debug("ChangeSelection for node: " + aNode.getName());
        itsSelectedNode=aNode;
        jOTDBparam aParam = null;
        
        jTabbedPane1.removeAll();
        
        // Check if the nodename uses specific panels and create them
        Vector aPanelList=null;
        if (itsPanelHelper.isKey(aNode.getName())) {
            aPanelList=itsPanelHelper.getPanels(aNode.getName());
        } else {
            aPanelList=itsPanelHelper.getPanels("*");
        }
        
        if (aNode.isLeaf()) {
        } else {
        }
        
        // Loop through all the panels and fill the tabPanel with them
        Iterator it = aPanelList.iterator();
        while (it.hasNext()) {
            boolean skip = false;
            JPanel p=null;
            String aPanelName= it.next().toString();
            // Check if the wanted panel is the Node or Parameter Panel. if so only add depending on leaf
            if ((aPanelName.contains("NodeViewPanel") && aNode.isLeaf()) |
                    (aPanelName.contains("ParameterViewPanel") && !aNode.isLeaf())) {
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
                    IViewPanel viewPanel = (IViewPanel)p;
                    if(viewPanel.isSingleton()){
                        IViewPanel singletonPanel = (IViewPanel)viewPanel.getInstance();
                        p = null;
                        jTabbedPane1.addTab(singletonPanel.getShortName(),null,viewPanel.getInstance(),"");
                        singletonPanel.setMainFrame(itsMainFrame);
                        singletonPanel.setContent(aNode.getUserObject());
                    }else{
                        jTabbedPane1.addTab(viewPanel.getShortName(),null,p,"");
                        viewPanel.setMainFrame(itsMainFrame);
                        viewPanel.setContent(aNode.getUserObject());
                    }
                   
                }
            } else {
                logger.debug("Skipping panel for: "+aPanelName);
            }
        }
        if (savedSelection > -1 && savedSelection < jTabbedPane1.getComponentCount()) {
            jTabbedPane1.setSelectedIndex(savedSelection);
        } else if (jTabbedPane1.getComponentCount() > 0) {
            jTabbedPane1.setSelectedIndex(0);
        }
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
    
    private void createPopupMenu(java.awt.event.MouseEvent evt) {
        if (jTabbedPane1.getSelectedComponent() != null) {
            if (((IViewPanel)jTabbedPane1.getSelectedComponent()).hasPopupMenu()) {
                ((IViewPanel)jTabbedPane1.getSelectedComponent()).createPopupMenu((JComponent) evt.getSource(), evt.getX(), evt.getY());
            }
        }
    }
    
    private void popupMenuActionHandler(ActionEvent evt) {
        logger.debug("PopupMenu choice: "+ evt.getActionCommand());
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
        treePanel.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mousePressed(java.awt.event.MouseEvent evt) {
                treePanelMousePressed(evt);
            }
        });

        jSplitPane.setLeftComponent(treePanel);

        jTabbedPane1.setMinimumSize(new java.awt.Dimension(600, 480));
        jSplitPane.setRightComponent(jTabbedPane1);

        add(jSplitPane, java.awt.BorderLayout.CENTER);

    }// </editor-fold>//GEN-END:initComponents
    
    private void treePanelMousePressed(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_treePanelMousePressed
        logger.debug("treeMouseEvent: " + evt.getButton());
        if (evt == null) {
            return;
        }
        //check if right button was clicked
        if(SwingUtilities.isRightMouseButton(evt)) {
            logger.debug("Right Mouse Button clicked"+evt.getSource().toString());
            createPopupMenu(evt);
            
        }
    }//GEN-LAST:event_treePanelMousePressed
    
    private void treePanelValueChanged(javax.swing.event.TreeSelectionEvent evt) {//GEN-FIRST:event_treePanelValueChanged
        logger.debug("treeSelectionEvent: " + evt);
        if (evt != null && evt.getNewLeadSelectionPath() != null &&
                evt.getNewLeadSelectionPath().getLastPathComponent() != null) {
            
            TreeNode treeNode = (TreeNode)evt.getNewLeadSelectionPath().getLastPathComponent();
            
            changeTreeSelection(treeNode);
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
    private TreeNode itsSelectedNode = null;
    private TreeInfoDialog treeInfoDialog = null;
    // keep the TreeId that belongs to this panel
    private int itsTreeID = 0;
    
    private ResultPanelHelper itsPanelHelper=ResultPanelHelper.getResultPanelHelper();
    
    /// TOBECHANGED
    private nl.astron.lofar.sas.otbcomponents.ParmDBPlotPanel parmDBPlotPanel1;
    ///
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JSplitPane jSplitPane;
    private javax.swing.JTabbedPane jTabbedPane1;
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
