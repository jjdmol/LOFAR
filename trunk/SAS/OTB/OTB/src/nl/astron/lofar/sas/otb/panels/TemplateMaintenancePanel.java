/*
 * TemplateMaintenancePanel.java
 *
 * Created on March 17, 2006, 1:53 PM
 */

package nl.astron.lofar.sas.otb.panels;

import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.util.OTDBtreeNode;
import nl.astron.lofar.sas.otb.util.UserAccount;
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
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        setLayout(new java.awt.BorderLayout());

        jSplitPane1.setDividerLocation(350);
        jSplitPane1.setLeftComponent(treePanel);

        jSplitPane1.setRightComponent(jTabbedPane1);

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
        if (evt.getActionCommand().equals("Cancel")) {
            itsMainFrame.unregisterPlugin(this.getFriendlyName());
            itsMainFrame.showPanel(MainPanel.getFriendlyNameStatic());
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    private void initialize() {
        treePanel.setTitle("Template List");
        buttonPanel1.addButton("Delete");
        buttonPanel1.addButton("Duplicate");
        buttonPanel1.addButton("Info");
        buttonPanel1.addButton("Cancel");
        buttonPanel1.addButton("Save");
        buttonPanel1.addButton("Save & Exit");
    }
    
    private MainFrame itsMainFrame;

    // keep the TreeId that belongs to this panel
    private int itsTreeID = 0;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JSplitPane jSplitPane1;
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
