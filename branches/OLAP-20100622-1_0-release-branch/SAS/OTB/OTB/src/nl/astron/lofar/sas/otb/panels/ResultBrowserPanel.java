/*
 * ResultBrowserPanel.java
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
import java.rmi.RemoteException;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.Set;
import java.util.Vector;
import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.JTabbedPane;
import javax.swing.event.TreeModelEvent;
import javax.swing.event.TreeModelListener;
import javax.swing.tree.TreePath;
import nl.astron.lofar.lofarutils.LofarUtils;
import org.apache.log4j.Logger;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBparam;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.ResultPanelHelper;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.jParmDBnode;
import nl.astron.lofar.sas.otb.util.treemanagers.ParmDBTreeManager;
import nl.astron.lofar.sas.otb.util.treemanagers.ResultTreeManager;
import nl.astron.lofar.sas.otb.util.treenodes.TreeNode;
import nl.astron.lofar.sas.otbcomponents.VerticalButtonPanel;
import nl.astron.lofar.sas.otbcomponents.NodeViewPanel;
import nl.astron.lofar.sas.otbcomponents.ParameterViewPanel;
import nl.astron.lofar.sas.otbcomponents.TreeInfoDialog;
import nl.astron.lofar.sas.otbcomponents.TreePanel;


/**
 * This panel contains a TreePanel and some textfields that display information
 * about the selected treenode.
 * It is intended to show Observation Trees that are about to run, are running or have run, and thus
 * are NOT intended to do heavy configuration (allthough there are some possibilities left to do so for the
 * time being). Configuring jobs needs 2 be done in the Template.
 *
 * Also a log screen to be able to view logging on the
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
    private TreeModelListener parmDBTreelistener;
    
    
    /** Creates new form BeanForm */
    public ResultBrowserPanel() {
        initComponents();
    }
    public void initialize() {
        userAccount = itsMainFrame.getUserAccount();
        treePanel.setTitle("Observation Tree");


        buttonPanel1.addButton("Query Panel");
        buttonPanel1.addButton("Schedule");
        buttonPanel1.addButton("Exit");


        buttonPanel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanelActionPerformed(evt);
            }
        });


        // determine userpanels from the Panelhelper
        Set<String> allKeys = itsPanelHelper.getKeys();
        Iterator iter = allKeys.iterator();
        while ( iter.hasNext()) {
            String aKey = iter.next().toString();
            if (!aKey.equals("*")) {
                try {
                    if (OtdbRmi.getRemoteMaintenance().getItemList(itsTreeID, "%" + aKey).size() > 0) {
                        buttonPanel.addButton(aKey);
                    }
                } catch (Exception e) {
                 logger.fatal("Exception during getItemList.",e);
                }
            }
        }

        try {
            itsTreeType = OtdbRmi.getRemoteTypes().getTreeType(OtdbRmi.getRemoteOTDB().getTreeInfo(itsTreeID, false).type);
        } catch (RemoteException ex) {
            logger.debug("Error getting treetype");
        }

        if (userAccount.isObserver() && !itsTreeType.equalsIgnoreCase("hardware")) {
            return;
        }


        //Sample code to have ParmDB in the tree.
        // only for treeview panel
        
        parmDBTreelistener = new TreeModelListener(){
            public void treeStructureChanged(TreeModelEvent e){}
            public void treeNodesRemoved(TreeModelEvent e){}
            public void treeNodesChanged(TreeModelEvent e){}
            public void treeNodesInserted(TreeModelEvent e){
                TreeNode item = (TreeNode)e.getSource();
                if (LofarUtils.keyName(item.getName()).equalsIgnoreCase("ParmDB")){
                    try {
                        //remove the pointers to the parmdb tables
                        item.removeAllChildren();
                        
                        //add the parmdb nodes
                        Vector childs =
                                OtdbRmi.getRemoteMaintenance().getItemList(((jOTDBnode)item.getUserObject()).treeID(), ((jOTDBnode)item.getUserObject()).nodeID(), 1);
                        
                        Enumeration parmdbparms = childs.elements();
                        while( parmdbparms.hasMoreElements()  ) {
                            jOTDBnode parmdbparmitem = (jOTDBnode)parmdbparms.nextElement();
                            //only add values that mean something
                            if(parmdbparmitem.limits != null && !parmdbparmitem.limits.equalsIgnoreCase("")){
                                String[] args = new String[3];
                                String tableName = LofarUtils.keyName(parmdbparmitem.name);
                                args[0]= tableName;
                                args[1]="ParmDB";
                                args[2]=parmdbparmitem.limits;
                                TreeNode parmDBnode =ParmDBTreeManager.getInstance(itsMainFrame.getUserAccount()).getRootNode(args);
                                
                                TreeNode newNode = new TreeNode(ParmDBTreeManager.getInstance(itsMainFrame.getUserAccount()),parmdbparmitem,parmdbparmitem.name);
                                item.add(parmDBnode);
                            }
                        }
                    } catch (RemoteException ex) {
                        logger.error("ParmDB Plotter could not be loaded : "+ex.getMessage(),ex);
                    }
                }
            }
        };
        
    }
    
    public boolean initializePlugin(MainFrame mainframe) {
        if (mainframe == null) {
            logger.debug("ERROR, no mainframe given");
            return false;
        }
        itsMainFrame=mainframe;
        itsTreeID=itsMainFrame.getSharedVars().getTreeID();

        initialize();

        jSplitPane.setDividerLocation(150);
        treePanel.addTreeSelectionListener(new javax.swing.event.TreeSelectionListener() {
            public void valueChanged(javax.swing.event.TreeSelectionEvent evt) {
                treePanelValueChanged(evt);
            }
        });

        
        // check access
        if(userAccount.isAdministrator() || itsTreeType.equalsIgnoreCase("hardware")) {
            jSplitPane.setLeftComponent(treePanel);
        } else if(userAccount.isAstronomer()) {
            jSplitPane.setLeftComponent(treePanel);
        } else if(userAccount.isInstrumentScientist()) {
            jSplitPane.setLeftComponent(treePanel);
        } else if (userAccount.isObserver()&& !itsTreeType.equalsIgnoreCase("hardware")) {
            jSplitPane.setLeftComponent(buttonPanel);
        }

        jSplitPane.setRightComponent(jTabbedPane1);

        add(jSplitPane, java.awt.BorderLayout.CENTER);
        
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
            if (userAccount.isObserver() && !itsTreeType.equalsIgnoreCase("hardware")) {
                if (itsLastSelectedPath != null && itsLastSelectedPath.getPathCount()>0) {
                   this.setNewRootNode();
                   treePanel.setSelectionPath(itsLastSelectedPath);
                } else {
                    this.setNewRootNode();
                }
            }
            this.setChanged(false);
        }
    }
    
    
    public static String getFriendlyNameStatic() {
        return name;
    }
    
    public void setNewRootNode() {
        
        if (!userAccount.isObserver() || itsTreeType.equalsIgnoreCase("hardware")) {
        
            try {
                ResultTreeManager treeManager = ResultTreeManager.getInstance(itsMainFrame.getUserAccount());
                treeManager.addTreeModelListener(parmDBTreelistener);
                itsMainFrame.setHourglassCursor();

                // and create a new root
                treePanel.newRootNode(treeManager.getRootNode(itsTreeID));
                itsMainFrame.setNormalCursor();
            } catch (Exception e) {
                logger.debug("Exception during setNewRootNode: " + e);
            }
        }
    }


    private void treePanelValueChanged(javax.swing.event.TreeSelectionEvent evt) {
        logger.debug("treeSelectionEvent: " + evt);
        if (evt != null && evt.getNewLeadSelectionPath() != null &&
                evt.getNewLeadSelectionPath().getLastPathComponent() != null) {

            TreeNode treeNode = (TreeNode)evt.getNewLeadSelectionPath().getLastPathComponent();

            changeTreeSelection(treeNode);
        }
    }

    private void buttonPanelActionPerformed(java.awt.event.ActionEvent evt) {
        logger.debug("actionPerformed: " + evt);
        logger.debug("Trigger: "+evt.getActionCommand());
        try {
            Vector aL = OtdbRmi.getRemoteMaintenance().getItemList(itsTreeID, "%"+evt.getActionCommand());
            logger.debug("nr nodes found: " + aL.size());
            logger.debug("nodes: " + aL);
            if (aL.size()> 0) {
              changeSelection((jOTDBnode)aL.elementAt(0));
              itsSelectedButton = evt.getActionCommand();
            } else {
                logger.error("No panels for this choice");
            }
        } catch(Exception e) {
            logger.fatal("Exception during getItemList.",e);
        }
    }

    private void changeSelection(TreeNode aNode) {
        itsMainFrame.setHourglassCursor();
        jTabbedPane1.removeAll();

        // Check if the nodename uses specific panels and create them
        Vector aPanelList=null;

        if(aNode.getUserObject() instanceof jOTDBnode){
            if (itsPanelHelper.isKey(LofarUtils.keyName(aNode.getName()))) {
                aPanelList=itsPanelHelper.getPanels(LofarUtils.keyName(aNode.getName()));
            } else {
                aPanelList=itsPanelHelper.getPanels("*");
            }
        }else if(aNode.getUserObject() instanceof jParmDBnode){
            aPanelList=itsPanelHelper.getPanels("ParmDBValues");
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
                    (aPanelName.contains("ParameterViewPanel") && !aNode.isLeaf())|
                    (aPanelName.contains("ParSetViewPanel") && aNode.isLeaf())) {
                skip = true;
            }
            if (!skip) {
                logger.debug("Getting panel for: "+aPanelName);
                try {
                    p = (JPanel) Class.forName(aPanelName).newInstance();
                } catch (ClassNotFoundException ex) {
                    logger.debug("Error during getPanel: "+ ex);
                    itsMainFrame.setNormalCursor();
                    return;
                } catch (InstantiationException ex) {
                    logger.debug("Error during getPanel: "+ ex);
                    itsMainFrame.setNormalCursor();
                    return;
                } catch (IllegalAccessException ex) {
                    logger.debug("Error during getPanel: "+ ex);
                    itsMainFrame.setNormalCursor();
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
        itsMainFrame.setNormalCursor();
    }

    private void changeSelection(jOTDBnode aNode) {
        itsMainFrame.setHourglassCursor();
        jTabbedPane1.removeAll();

        // Check if the nodename uses specific panels and create them
        Vector aPanelList=null;

        if (itsPanelHelper.isKey(LofarUtils.keyName(aNode.name))) {
            aPanelList=itsPanelHelper.getPanels(LofarUtils.keyName(aNode.name));
        } else {
            aPanelList=itsPanelHelper.getPanels("*");
        }


        // Loop through all the panels and fill the tabPanel with them
        Iterator it = aPanelList.iterator();
        while (it.hasNext()) {
            boolean skip = false;
            JPanel p=null;
            String aPanelName= it.next().toString();
            // Check if the wanted panel is the Node or Parameter Panel. if so only add depending on leaf
            if ((aPanelName.contains("NodeViewPanel") && aNode.leaf) |
                    (aPanelName.contains("ParameterViewPanel") && !aNode.leaf)|
                    (aPanelName.contains("ParSetViewPanel") && aNode.leaf)) {
                skip = true;
            }
            if (!skip) {
                logger.debug("Getting panel for: "+aPanelName);
                try {
                    p = (JPanel) Class.forName(aPanelName).newInstance();
                } catch (ClassNotFoundException ex) {
                    logger.debug("Error during getPanel: "+ ex);
                    itsMainFrame.setNormalCursor();
                    return;
                } catch (InstantiationException ex) {
                    logger.debug("Error during getPanel: "+ ex);
                    itsMainFrame.setNormalCursor();
                    return;
                } catch (IllegalAccessException ex) {
                    logger.debug("Error during getPanel: "+ ex);
                    itsMainFrame.setNormalCursor();
                    return;
                }
                if (p!=null) {
                    IViewPanel viewPanel = (IViewPanel)p;
                    if(viewPanel.isSingleton()){
                        IViewPanel singletonPanel = (IViewPanel)viewPanel.getInstance();
                        p = null;
                        jTabbedPane1.addTab(singletonPanel.getShortName(),null,viewPanel.getInstance(),"");
                        singletonPanel.setMainFrame(itsMainFrame);
                        singletonPanel.setContent(aNode);
                    }else{
                        jTabbedPane1.addTab(viewPanel.getShortName(),null,p,"");
                        viewPanel.setMainFrame(itsMainFrame);
                        viewPanel.setContent(aNode);
                    }

                }
            } else {
                logger.debug("Skipping panel for: "+aPanelName);
            }
        }
        itsMainFrame.setNormalCursor();
    }

    private void changeTreeSelection(TreeNode aNode) {

        if (userAccount.isObserver() && !itsTreeType.equalsIgnoreCase("hardware")) {
            return;
        }

        // save selected panel
        int savedSelection=jTabbedPane1.getSelectedIndex();
        logger.debug("ChangeSelection for node: " + aNode.getName());
        jOTDBparam aParam = null;

        changeSelection(aNode);

        
        if (savedSelection > -1 && savedSelection < jTabbedPane1.getComponentCount()) {
            jTabbedPane1.setSelectedIndex(savedSelection);
        } else if (jTabbedPane1.getComponentCount() > 0) {
            jTabbedPane1.setSelectedIndex(0);
        }
    }
    
    private boolean viewInfo() {
        int [] id={itsTreeID};
           
        if (itsTreeID > -1) {
            // show treeInfo dialog
            treeInfoDialog = new TreeInfoDialog(true,id, itsMainFrame);
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
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();
        jSplitPane = new javax.swing.JSplitPane();

        setLayout(new java.awt.BorderLayout());

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });
        add(buttonPanel1, java.awt.BorderLayout.SOUTH);

        jSplitPane.setDividerLocation(250);
        add(jSplitPane, java.awt.BorderLayout.CENTER);
    }// </editor-fold>//GEN-END:initComponents
            
    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        logger.debug("actionPerformed: " + evt);
        
        if(evt.getActionCommand().equals("Query Panel")) {
            // ToDo
        } else if(evt.getActionCommand().equals("Schedule")) {
            if (itsTreeID > 0) {
                if (viewInfo() ) {
                    logger.debug("Tree has been changed, reloading table line");
                    // flag has to be set that ppl using this treeid should be able to see that it's info has been changed
                    itsMainFrame.setChanged(this.getFriendlyName(),true);
                }
            }
        } else if(evt.getActionCommand().equals("Exit")) {
            if (!userAccount.isObserver() || itsTreeType.equalsIgnoreCase("hardware")){
                ResultTreeManager treeManager = ResultTreeManager.getInstance(itsMainFrame.getUserAccount());
                treeManager.removeTreeModelListener(parmDBTreelistener);
            }
            itsMainFrame.unregisterPlugin(this.getFriendlyName());
            itsMainFrame.showPanel(MainPanel.getFriendlyNameStatic());
            
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    private UserAccount userAccount = null;
    private MainFrame itsMainFrame;
    private boolean changed=false;
    private TreeInfoDialog treeInfoDialog = null;
    // keep the TreeId that belongs to this panel
    private int itsTreeID = 0;
    private String itsTreeType="";
    private String itsSelectedButton="";

    TreePath itsLastSelectedPath = null;
    private JTabbedPane jTabbedPane1 = new javax.swing.JTabbedPane();
    private NodeViewPanel nodeViewPanel1 = new nl.astron.lofar.sas.otbcomponents.NodeViewPanel();
    private ParameterViewPanel parameterViewPanel1 = new nl.astron.lofar.sas.otbcomponents.ParameterViewPanel();
    private TreePanel treePanel = new nl.astron.lofar.sas.otbcomponents.TreePanel();
    private VerticalButtonPanel buttonPanel = new nl.astron.lofar.sas.otbcomponents.VerticalButtonPanel();

    
    private ResultPanelHelper itsPanelHelper=ResultPanelHelper.getResultPanelHelper();
    
    /// TOBECHANGED
    private nl.astron.lofar.sas.otbcomponents.ParmDBPlotPanel parmDBPlotPanel1;
    ///
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JSplitPane jSplitPane;
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
