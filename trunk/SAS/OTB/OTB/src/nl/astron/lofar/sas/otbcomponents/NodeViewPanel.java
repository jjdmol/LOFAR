/*
 * NodeViewPanel.java
 *
 * Created on 26 januari 2006, 15:47
 */

package nl.astron.lofar.sas.otbcomponents;

import java.rmi.RemoteException;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.panels.MainPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import org.apache.log4j.Logger;

/**
 *
 * @author  coolen
 */
public class NodeViewPanel extends javax.swing.JPanel {
    
    static Logger logger = Logger.getLogger(MainPanel.class);    

   
    /** Creates new form BeanForm based upon aNode
     *
     * @params  aNode   node to obtain the info from
     *
     */
    public NodeViewPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode = aNode;
        itsOtdbRmi=itsMainFrame.getOTDBrmi();
        initPanel(aNode);
    }
    
    /** Creates new form BeanForm */
    public NodeViewPanel() {
        initComponents();
    }
    
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
            itsOtdbRmi=itsMainFrame.getOTDBrmi();
        } else {
            logger.debug("No Mainframe supplied");
        }
    }
    
    public void setNode(jOTDBnode aNode) {
        initPanel(aNode);
    }

     private void initPanel(jOTDBnode aNode) {
        itsNode = aNode;
         if (aNode != null) {
            setNodeName(aNode.name);
            setIndex(String.valueOf(aNode.index));
            setInstances(String.valueOf(aNode.instances));
            setLimits(String.valueOf(aNode.limits));
            setDescription(aNode.description);
        } else {
            logger.debug("ERROR:  no node given");
        }
    }
    
    /** Returns the Given Name for this Node */
    public String getNodeName() {
        return this.NodeNameText.getText();
    }
    
    private void setNodeName(String aS) {
        this.NodeNameText.setText(aS);
    }
    
    /** Returns the Given Index for this Node */
    public String getIndex() {
        return this.NodeIndexText.getText();
    }
    
    private void setIndex(String aS) {
        this.NodeIndexText.setText(aS);
    }
    
    /** Returns the Given Instances for this Node */
    public String getInstances() {
        return this.NodeInstancesText.getText();
    }
    
    private void setInstances(String aS) {
        this.NodeInstancesText.setText(aS);
    }

    /** Returns the Given Limits for this Node */
    public String getLimits() {
        return this.NodeLimitsText.getText();
    }
    
    private void setLimits(String aS) {
        this.NodeLimitsText.setText(aS);
    }

    /** Returns the Given Description for this Node */
    public String getDescription() {
        return this.NodeDescriptionText.getText();
    }
    
    private void setDescription(String aS) {
        this.NodeDescriptionText.setText(aS);
    }
    
    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableNodeName(boolean enabled) {
        this.NodeNameText.setEnabled(enabled);
    }

    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableIndex(boolean enabled) {
        this.NodeIndexText.setEnabled(enabled);
    }

    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableInstances(boolean enabled) {
        this.NodeInstancesText.setEnabled(enabled);
    }

    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableLimits(boolean enabled) {
        this.NodeLimitsText.setEnabled(enabled);
    }

    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableDescription(boolean enabled) {
        this.NodeDescriptionText.setEnabled(enabled);
        this.NodeDescriptionText.setEditable(enabled);
    }


    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
        this.NodeApplyButton.setEnabled(enabled);
        this.NodeCancelButton.setEnabled(enabled);
    }
    
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
        enableNodeName(enabled);
        enableIndex(enabled);
        enableInstances(enabled);
        enableLimits(enabled);
        enableDescription(enabled);
        enableButtons(enabled);
    }
    
    private void saveInput() {
        // Just check all possible fields that CAN change. The enabled method will take care if they COULD be changed.
        // this way we keep this panel general for multiple use
        boolean hasChanged = false;
        if (itsNode != null) {
            try {


                if (!String.valueOf(itsNode.instances).equals(getInstances())) { 
                    itsNode.instances=Integer.valueOf(getInstances()).shortValue();
                    hasChanged=true;
                }

                if (!itsNode.limits.equals(getLimits())) { 
                    itsNode.limits=getLimits();
                    hasChanged=true;
                }

                if (!itsNode.description.equals(getDescription())) { 
                    itsNode.description=getDescription();
                    hasChanged=true;
                }

                if (hasChanged) {
                    if (!itsOtdbRmi.getRemoteMaintenance().saveNode(itsNode)) {
                        logger.error("Saving node failed: "+ itsOtdbRmi.getRemoteMaintenance().errorMsg());
                    }
                } 
               
            } catch (RemoteException ex) {
                logger.debug("error in Remote connection");
            }
        } else {
            logger.debug("ERROR:  no Param given");
        }
    }
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        NodeNameLabel = new javax.swing.JLabel();
        NodeIndexLabel = new javax.swing.JLabel();
        NodeInstancesLabel = new javax.swing.JLabel();
        NodeLimitsLabel = new javax.swing.JLabel();
        NodeIndexText = new javax.swing.JTextField();
        NodeInstancesText = new javax.swing.JTextField();
        NodeLimitsText = new javax.swing.JTextField();
        NodeNameText = new javax.swing.JTextField();
        NodeCancelButton = new javax.swing.JButton();
        NodeApplyButton = new javax.swing.JButton();
        NodeDescriptionText = new javax.swing.JTextArea();
        jLabel1 = new javax.swing.JLabel();

        setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        NodeNameLabel.setText("Name :");
        add(NodeNameLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 50, 80, -1));

        NodeIndexLabel.setText("Index :");
        add(NodeIndexLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 70, -1, -1));

        NodeInstancesLabel.setText("Instances :");
        add(NodeInstancesLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 100, -1, -1));

        NodeLimitsLabel.setText("Limits :");
        add(NodeLimitsLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 120, -1, -1));

        NodeIndexText.setText("None");
        NodeIndexText.setMaximumSize(new java.awt.Dimension(200, 19));
        NodeIndexText.setMinimumSize(new java.awt.Dimension(200, 19));
        NodeIndexText.setPreferredSize(new java.awt.Dimension(200, 19));
        add(NodeIndexText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 70, 200, -1));

        NodeInstancesText.setText("-1");
        NodeInstancesText.setToolTipText("Number of Instances for this Node ");
        NodeInstancesText.setMaximumSize(new java.awt.Dimension(200, 19));
        NodeInstancesText.setMinimumSize(new java.awt.Dimension(200, 19));
        NodeInstancesText.setPreferredSize(new java.awt.Dimension(200, 19));
        add(NodeInstancesText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 100, 200, -1));

        NodeLimitsText.setText("None");
        NodeLimitsText.setToolTipText("Limits for this Node");
        NodeLimitsText.setMaximumSize(new java.awt.Dimension(200, 19));
        NodeLimitsText.setMinimumSize(new java.awt.Dimension(200, 19));
        NodeLimitsText.setPreferredSize(new java.awt.Dimension(200, 19));
        add(NodeLimitsText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 120, 200, -1));

        NodeNameText.setText("None");
        NodeNameText.setToolTipText("Name for this Node");
        NodeNameText.setMaximumSize(new java.awt.Dimension(440, 19));
        NodeNameText.setMinimumSize(new java.awt.Dimension(440, 19));
        NodeNameText.setPreferredSize(new java.awt.Dimension(440, 19));
        add(NodeNameText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 50, 240, -1));

        NodeCancelButton.setText("Cancel");
        NodeCancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                NodeCancelButtonActionPerformed(evt);
            }
        });

        add(NodeCancelButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 280, -1, -1));

        NodeApplyButton.setText("Apply");
        NodeApplyButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                NodeApplyButtonActionPerformed(evt);
            }
        });

        add(NodeApplyButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 280, 70, -1));

        NodeDescriptionText.setRows(4);
        NodeDescriptionText.setBorder(javax.swing.BorderFactory.createTitledBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true), "Description"));
        NodeDescriptionText.setEnabled(false);
        add(NodeDescriptionText, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 150, 540, 100));

        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("Node View Panel");
        add(jLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(70, 10, 460, 20));

    }// </editor-fold>//GEN-END:initComponents

    private void NodeApplyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NodeApplyButtonActionPerformed
        saveInput();
    }//GEN-LAST:event_NodeApplyButtonActionPerformed

    private void NodeCancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NodeCancelButtonActionPerformed
        initPanel(itsNode);
    }//GEN-LAST:event_NodeCancelButtonActionPerformed
    
    jOTDBnode itsNode = null;
    private MainFrame  itsMainFrame;
    private OtdbRmi    itsOtdbRmi;   
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton NodeApplyButton;
    private javax.swing.JButton NodeCancelButton;
    private javax.swing.JTextArea NodeDescriptionText;
    private javax.swing.JLabel NodeIndexLabel;
    private javax.swing.JTextField NodeIndexText;
    private javax.swing.JLabel NodeInstancesLabel;
    private javax.swing.JTextField NodeInstancesText;
    private javax.swing.JLabel NodeLimitsLabel;
    private javax.swing.JTextField NodeLimitsText;
    private javax.swing.JLabel NodeNameLabel;
    private javax.swing.JTextField NodeNameText;
    private javax.swing.JLabel jLabel1;
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
