/*
 * ComponentMaintenancePanel.java
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
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.treemanagers.TemplateTreeManager;
import org.apache.log4j.Logger;

/**
 * Initial class to construct templates,  is not needed anymore
 *
 * @created 24-01-2006, 14:58
 * @author  Coolen
 * @version $Id$
 * @updated
 */
public class TemplateConstructionPanel extends javax.swing.JPanel  
        implements IPluginPanel {
    
    static Logger logger = Logger.getLogger(ComponentMaintenancePanel.class);
    static String name = "Component_Maintenance";   
    
    /** Creates new form BeanForm */
    public TemplateConstructionPanel() {
        initComponents();
        initialize();
    }
 
    public void initialize() {
        treePanel.setTitle("Template List");
        buttonPanel1.addButton("Delete");
        buttonPanel1.addButton("Duplicate");
        buttonPanel1.addButton("Info");
        buttonPanel1.addButton("Cancel");
        buttonPanel1.addButton("Save");
        buttonPanel1.addButton("Exit");
    }

    public boolean initializePlugin(MainFrame mainframe) {
        itsMainFrame = mainframe;
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
            
            TemplateTreeManager treeManager = TemplateTreeManager.getInstance(itsMainFrame.getUserAccount());
            
            itsMainFrame.setHourglassCursor();
            // and create a new root
            treePanel.newRootNode(treeManager.getRootNode(itsTreeID));
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
        jLabel2 = new javax.swing.JLabel();
        jSplitPane1 = new javax.swing.JSplitPane();
        TreeBasePanel = new javax.swing.JPanel();
        treePanel = new nl.astron.lofar.sas.otbcomponents.TreePanel();
        jPanel3 = new javax.swing.JPanel();
        jLabel4 = new javax.swing.JLabel();
        jTextField1 = new javax.swing.JTextField();
        jSplitPane2 = new javax.swing.JSplitPane();
        jPanel1 = new javax.swing.JPanel();
        templateUpButton = new javax.swing.JButton();
        templateLeftButton = new javax.swing.JButton();
        templateRightButton = new javax.swing.JButton();
        templateDownButton = new javax.swing.JButton();
        jPanel2 = new javax.swing.JPanel();
        jLabel3 = new javax.swing.JLabel();
        jScrollPane1 = new javax.swing.JScrollPane();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        setLayout(new java.awt.BorderLayout());

        jLabel2.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel2.setText("Template Construction");
        add(jLabel2, java.awt.BorderLayout.NORTH);

        jSplitPane1.setDividerLocation(375);
        TreeBasePanel.setLayout(new java.awt.BorderLayout());

        TreeBasePanel.add(treePanel, java.awt.BorderLayout.CENTER);

        jPanel3.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jLabel4.setText("Parameter Name");
        jPanel3.add(jLabel4, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 10, 120, -1));

        jPanel3.add(jTextField1, new org.netbeans.lib.awtextra.AbsoluteConstraints(170, 5, 170, 20));

        TreeBasePanel.add(jPanel3, java.awt.BorderLayout.SOUTH);

        jSplitPane1.setLeftComponent(TreeBasePanel);

        jSplitPane2.setDividerLocation(125);
        jPanel1.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        templateUpButton.setText("^");
        templateUpButton.setToolTipText("Move chosen template up");
        jPanel1.add(templateUpButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 200, 50, 29));

        templateLeftButton.setText("<");
        templateLeftButton.setToolTipText("Move chosen template left");
        jPanel1.add(templateLeftButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 240, -1, 29));

        templateRightButton.setText(">");
        templateRightButton.setToolTipText("Move chosen template right");
        jPanel1.add(templateRightButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(70, 240, -1, 29));

        templateDownButton.setText("v");
        templateDownButton.setToolTipText("Move chosen template down");
        jPanel1.add(templateDownButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 280, 50, 29));

        jSplitPane2.setLeftComponent(jPanel1);

        jPanel2.setLayout(new java.awt.BorderLayout());

        jLabel3.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel3.setText("Components List");
        jPanel2.add(jLabel3, java.awt.BorderLayout.NORTH);

        jPanel2.add(jScrollPane1, java.awt.BorderLayout.CENTER);

        jSplitPane2.setRightComponent(jPanel2);

        jSplitPane1.setRightComponent(jSplitPane2);

        add(jSplitPane1, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });

        add(buttonPanel1, java.awt.BorderLayout.SOUTH);

    }// </editor-fold>//GEN-END:initComponents

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        logger.debug("actionPerformed: " + evt);
        logger.debug("Trigger: "+evt.getActionCommand());
        if (evt.getActionCommand().equals("Exit")) {
            itsMainFrame.unregisterPlugin(getFriendlyName());
            itsMainFrame.showPanel(MainPanel.getFriendlyNameStatic());
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    private MainFrame itsMainFrame;

    // keep the TreeId that belongs to this panel
    private int itsTreeID = 0;
    private boolean changed = false;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel TreeBasePanel;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JSplitPane jSplitPane1;
    private javax.swing.JSplitPane jSplitPane2;
    private javax.swing.JTextField jTextField1;
    private javax.swing.JButton templateDownButton;
    private javax.swing.JButton templateLeftButton;
    private javax.swing.JButton templateRightButton;
    private javax.swing.JButton templateUpButton;
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
