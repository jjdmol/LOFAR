/*
 * TemplateMaintenancePanel.java
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

package nl.astron.lofar.sas.otb.panels;

import java.rmi.RemoteException;
import java.util.Iterator;
import java.util.Vector;
import javax.swing.JComponent;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;
import javax.swing.tree.TreePath;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.util.ConfigPanelHelper;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.treemanagers.TemplateTreeManager;
import nl.astron.lofar.sas.otb.util.treenodes.TreeNode;
import nl.astron.lofar.sas.otbcomponents.TreeInfoDialog;
import org.apache.log4j.Logger;

/**
 * The template maintenance screen is used for modifying repository templates 
 * (status usable, being specified and example).
 * or specifying an instrument configuration for an observation request.
 *
 * @created 17-03-2006, 13:53
 * @author  Coolen
 * @version $Id$
 * @updated
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
            if (itsLastSelectedPath != null && itsLastSelectedPath.getPathCount()>0) {
               this.setNewRootNode();
               treePanel.setSelectionPath(itsLastSelectedPath);
            } else {
                this.setNewRootNode();
            }
            this.setChanged(false);
        }
    }
    
    public void setNewRootNode(){
        try {
            TemplateTreeManager treeManager = TemplateTreeManager.getInstance(itsMainFrame.getUserAccount());
            
            itsMainFrame.setHourglassCursor();
            // and create a new root
            TreePath aP=itsLastSelectedPath;
            treePanel.newRootNode(treeManager.getRootNode(itsTreeID));
            itsLastSelectedPath=aP;
            itsMainFrame.setNormalCursor();
        } catch (Exception e) {
            logger.debug("Exception during setNewRootNode: " );
            e.printStackTrace();
        }
    }
    
    public String getFriendlyName() {
        return getFriendlyNameStatic()+"("+itsTreeID+")";
    }

    public static String getFriendlyNameStatic() { 
        return name;
    }
  
    private void createPopupMenu(java.awt.event.MouseEvent evt) {
        if (jTabbedPane1.getSelectedComponent() != null) {
            if (((IViewPanel)jTabbedPane1.getSelectedComponent()).hasPopupMenu()) {
                ((IViewPanel)jTabbedPane1.getSelectedComponent()).createPopupMenu((JComponent) evt.getSource(), evt.getX(), evt.getY());
            }
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
        treePanel.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mousePressed(java.awt.event.MouseEvent evt) {
                treePanelMousePressed(evt);
            }
        });

        jSplitPane1.setLeftComponent(treePanel);

        jTabbedPane1.addTab("Node", nodeViewPanel1);

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
            if  (treePanel.getSelectionPath() != null){
                itsLastSelectedPath = treePanel.getSelectionPath();
            }
            TreeNode treeNode = (TreeNode)evt.getNewLeadSelectionPath().getLastPathComponent();
            
            if(treeNode.getUserObject() instanceof jOTDBnode){
                changeTreeSelection((jOTDBnode)treeNode.getUserObject());    
            }
        }
    }//GEN-LAST:event_treePanelValueChanged

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        logger.debug("actionPerformed: " + evt);
        logger.debug("Trigger: "+evt.getActionCommand());
        if (evt.getActionCommand().equals("Delete")) {
            //Check  if the selected node isn't a leaf
            if (itsSelectedNode != null && !itsSelectedNode.leaf) {
                if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this node ?","Delete Tree",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
                    try {
                        if (OtdbRmi.getRemoteMaintenance().deleteNode(itsSelectedNode)) {
                            logger.debug("Node + children deleted");
                            setNewRootNode();
                        }
                    } catch (RemoteException ex) {
                        logger.debug("Error during deletion of Node: "+ex);
                    }
                }
            }
        } else if (evt.getActionCommand().equals("Duplicate")) {
            
            //!!!!!  if duplicating a node the default node.instances need to be set to +1 instance!!!!!!
            itsMainFrame.ToDo();
            
            
            //Check  if the selected node isn't a leaf
            if (itsSelectedNode != null && !itsSelectedNode.leaf) {
                String answer=JOptionPane.showInputDialog(this,"What is the index for the new subtree?","Enter indexNumber",JOptionPane.QUESTION_MESSAGE);
                if (answer!=null || !answer.equals("")) {
                    short idx=Integer.valueOf(answer).shortValue();
                    if (idx < 0) {
                        logger.debug("Index value smaller then 1 not allowed");
                        return;
                    }
                    try {
                        int aN=OtdbRmi.getRemoteMaintenance().dupNode(itsTreeID,itsSelectedNode.nodeID(),idx);
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
            }
        } else if (evt.getActionCommand().equals("Change Status")) {
            int answer=JOptionPane.showConfirmDialog(this,"Altering the info wil automaticly close this Maintainance window. Do you want to continue ?","alert",JOptionPane.YES_NO_OPTION);
            if (itsTreeID > 0 && JOptionPane.YES_OPTION == answer ) {
                if (viewInfo()) {
                    logger.debug("Tree has been changed, reloading table line");
                    // flag has to be set that ppl using this treeid should be able to see that it's info has been changed
                    itsMainFrame.setChanged(this.getFriendlyName(),true);
                    // if the treeinfo has been changed then it could be into a state where the loaded TemplateMaintenance panel
                    // is invalid (can not be changed anymore )  So it's better to close down this panel.
                    itsMainFrame.unregisterPlugin(this.getFriendlyName());
                    itsMainFrame.showPanel(MainPanel.getFriendlyNameStatic());
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
        
        int [] id=new int[1];
        id[0]=itsTreeID;

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

    private void changeTreeSelection(jOTDBnode aNode) {
        // save selected panel
        int savedSelection=jTabbedPane1.getSelectedIndex();
        logger.debug("ChangeSelection for node: " + aNode.name);
        itsSelectedNode=aNode;
        jOTDBparam aParam = null;
        
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
        if (treePanel == null) {
            buttonPanel1.setButtonEnabled("Duplicate",false);
            buttonPanel1.setButtonEnabled("Delete",false);

        } else {
            int[] selectedRows =treePanel.getSelectedRows();
            if (selectedRows == null || selectedRows.length <=0 ||selectedRows[0] ==  0 || aNode.leaf ) {
                buttonPanel1.setButtonEnabled("Duplicate",false);
                buttonPanel1.setButtonEnabled("Delete",false);
            } else {
//                buttonPanel1.setButtonEnabled("Duplicate",true);
                buttonPanel1.setButtonEnabled("Delete",true);
            }
        }
        if (savedSelection > -1 && savedSelection < jTabbedPane1.getComponentCount()) {
            jTabbedPane1.setSelectedIndex(savedSelection);
        } else if (jTabbedPane1.getComponentCount() > 0) {
            jTabbedPane1.setSelectedIndex(0);
        }
    }
    

   
    private void initialize() {
        treePanel.setTitle("Template List");
        buttonPanel1.addButton("Delete");
        buttonPanel1.addButton("Duplicate");
        buttonPanel1.addButton("Change Status");
        buttonPanel1.addButton("Exit");
        
        buttonPanel1.setButtonEnabled("Duplicate",false);
        
        nodeViewPanel1.enableButtons(true);
       
        parameterViewPanel1.enableButtons(true);
    }
    
    private MainFrame itsMainFrame;
    private jOTDBnode itsSelectedNode = null;
    private TreeInfoDialog treeInfoDialog = null;
    TreePath itsLastSelectedPath = null;

    // keep the TreeId that belongs to this panel
    private int itsTreeID = 0;
    private boolean changed = false;
    
    private ConfigPanelHelper itsPanelHelper=ConfigPanelHelper.getConfigPanelHelper();

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
    private javax.swing.event.EventListenerList myListenerList =  null;

    /**
     * Registers ActionListener to receive events.
     * @param listener The listener to register.
     */
    public synchronized void addActionListener(java.awt.event.ActionListener listener) {

        if (myListenerList == null ) {
            myListenerList = new javax.swing.event.EventListenerList();
        }
        myListenerList.add (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeActionListener(java.awt.event.ActionListener listener) {

        myListenerList.remove (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Notifies all registered listeners about the event.
     * 
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {

        if (myListenerList == null) return;
        Object[] listeners = myListenerList.getListenerList ();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed (event);
            }
        }
    }
    
}
