/*
 * ComponentMaintenancePanel.java
 *
 * Created on 24 januari 2006, 12:47
 */

package nl.astron.lofar.sas.otb.panels;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.util.OTDBtreeNode;
import nl.astron.lofar.sas.otb.util.UserAccount;
import org.apache.log4j.Logger;

/**
 *
 * @author  Coolen
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
 
    public void initialize() {
        buttonPanel1.addButton("Delete");
        buttonPanel1.addButton("Load");
        buttonPanel1.addButton("Exit");
    }

    public void initializePlugin(MainFrame mainframe) {
        itsMainFrame = mainframe;
        
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
        // create a sample root node. This should be retrieved from the OTDB of course.
        jOTDBnode otdbNode = new jOTDBnode(0,0,0,0);
        otdbNode.name = "Node_0";

        // put the OTDBnode in a wrapper for the tree
        OTDBtreeNode otdbTreeNode = new OTDBtreeNode(otdbNode, itsMainFrame.getOTDBrmi());

        // and create a new root
        treePanel.newRootNode(null);
    }
    
    public String getFriendlyName() {
        return getFriendlyNameStatic();
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
        componentPanel1 = new nl.astron.lofar.sas.otbcomponents.ComponentPanel();
        TreeBasePanel = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        treePanel = new nl.astron.lofar.sas.otbcomponents.TreePanel();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        setLayout(new java.awt.BorderLayout());

        jSplitPane1.setDividerLocation(450);
        jSplitPane1.setRightComponent(componentPanel1);

        TreeBasePanel.setLayout(new java.awt.BorderLayout());

        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("Component list");
        TreeBasePanel.add(jLabel1, java.awt.BorderLayout.NORTH);

        TreeBasePanel.add(treePanel, java.awt.BorderLayout.CENTER);

        jSplitPane1.setLeftComponent(TreeBasePanel);

        add(jSplitPane1, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });

        add(buttonPanel1, java.awt.BorderLayout.SOUTH);

    }
    // </editor-fold>//GEN-END:initComponents

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        logger.debug("actionPerformed: " + evt);
        logger.debug("Trigger: "+evt.getActionCommand());
        if (evt.getActionCommand().equals("Exit")) {
            itsMainFrame.unregisterPlugin("Component_Maintenance");
            itsMainFrame.showPanel(MainPanel.getFriendlyNameStatic());
        }
    }//GEN-LAST:event_buttonPanel1ActionPerformed
    
    private MainFrame itsMainFrame;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel TreeBasePanel;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private nl.astron.lofar.sas.otbcomponents.ComponentPanel componentPanel1;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JSplitPane jSplitPane1;
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
