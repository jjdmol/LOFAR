/*
 * NodeViewPanel.java
 *
 * Created on 26 januari 2006, 15:47
 */

package nl.astron.lofar.sas.otbcomponents;

/**
 *
 * @author  coolen
 */
public class NodeViewPanel extends javax.swing.JPanel {
    
    /** Creates new form BeanForm */
    public NodeViewPanel() {
        initComponents();
    }

    /** Returns the Given Name for this Node */
    public String getNodeName() {
        return this.NodeNameText.getText();
    }
    
    private void setNodeName(String aS) {
        this.NodeNameText.setText(aS);
    }
    
    /** Returns the Given Index for this Node */
    public String getNodeIndex() {
        return this.NodeIndexText.getText();
    }
    
    private void setNodeIndex(String aS) {
        this.NodeIndexText.setText(aS);
    }
    
    /** Returns the Given Instances for this Node */
    public String getNodeInstances() {
        return this.NodeInstancesText.getText();
    }
    
    private void setNodeInstances(String aS) {
        this.NodeInstancesText.setText(aS);
    }

    /** Returns the Given Limits for this Node */
    public String getNodeLimits() {
        return this.NodeLimitsText.getText();
    }
    
    private void setNodeLimits(String aS) {
        this.NodeLimitsText.setText(aS);
    }

    /** Returns the Given Description for this Node */
    public String getNodeDescription() {
        return this.NodeDescriptionText.getText();
    }
    
    private void setNodeDescription(String aS) {
        this.NodeDescriptionText.setText(aS);
    }
    
    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableNodeName(boolean enabled) {
        NodeNameText.setEnabled(enabled);
    }

    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableNodeIndex(boolean enabled) {
        NodeIndexText.setEnabled(enabled);
    }

    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableNodeInstances(boolean enabled) {
        NodeInstancesText.setEnabled(enabled);
    }

    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableNodeLimits(boolean enabled) {
        NodeLimitsText.setEnabled(enabled);
    }

    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableNodeDescription(boolean enabled) {
        NodeDescriptionText.setEnabled(enabled);
        NodeDescriptionText.setEditable(enabled);
    }

    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
        NodeApplyButton.setEnabled(enabled);
        NodeCancelButton.setEnabled(enabled);
    }
    
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setEnabled(boolean enabled) {
        enableNodeName(enabled);
        enableNodeIndex(enabled);
        enableNodeInstances(enabled);
        enableNodeLimits(enabled);
        enableNodeDescription(enabled);
        enableButtons(enabled);
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
        NodeDescriptionLabel = new javax.swing.JLabel();
        NodeIndexText = new javax.swing.JTextField();
        NodeInstancesText = new javax.swing.JTextField();
        NodeLimitsText = new javax.swing.JTextField();
        NodeNameText = new javax.swing.JTextField();
        NodeCancelButton = new javax.swing.JButton();
        NodeApplyButton = new javax.swing.JButton();
        NodeDescriptionText = new javax.swing.JTextArea();

        setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        NodeNameLabel.setText("Name :");
        add(NodeNameLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 10, 80, -1));

        NodeIndexLabel.setText("Index :");
        add(NodeIndexLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 35, -1, -1));

        NodeInstancesLabel.setText("Instances :");
        add(NodeInstancesLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 60, -1, -1));

        NodeLimitsLabel.setText("Limits :");
        add(NodeLimitsLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 85, -1, -1));

        NodeDescriptionLabel.setText("Description :");
        add(NodeDescriptionLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 110, -1, -1));

        NodeIndexText.setText("None");
        NodeIndexText.setEnabled(false);
        NodeIndexText.setMaximumSize(new java.awt.Dimension(200, 19));
        NodeIndexText.setMinimumSize(new java.awt.Dimension(200, 19));
        NodeIndexText.setPreferredSize(new java.awt.Dimension(200, 19));
        add(NodeIndexText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 35, 200, -1));

        NodeInstancesText.setText("-1");
        NodeInstancesText.setToolTipText("Number of Instances for this Node ");
        NodeInstancesText.setEnabled(false);
        NodeInstancesText.setMaximumSize(new java.awt.Dimension(200, 19));
        NodeInstancesText.setMinimumSize(new java.awt.Dimension(200, 19));
        NodeInstancesText.setPreferredSize(new java.awt.Dimension(200, 19));
        add(NodeInstancesText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 60, 200, -1));

        NodeLimitsText.setText("None");
        NodeLimitsText.setToolTipText("Limits for this Node");
        NodeLimitsText.setEnabled(false);
        NodeLimitsText.setMaximumSize(new java.awt.Dimension(200, 19));
        NodeLimitsText.setMinimumSize(new java.awt.Dimension(200, 19));
        NodeLimitsText.setPreferredSize(new java.awt.Dimension(200, 19));
        add(NodeLimitsText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 85, 200, -1));

        NodeNameText.setText("None");
        NodeNameText.setToolTipText("Name for this Node");
        NodeNameText.setEnabled(false);
        NodeNameText.setMaximumSize(new java.awt.Dimension(440, 19));
        NodeNameText.setMinimumSize(new java.awt.Dimension(440, 19));
        NodeNameText.setPreferredSize(new java.awt.Dimension(440, 19));
        add(NodeNameText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 10, 240, -1));

        NodeCancelButton.setText("Cancel");
        NodeCancelButton.setEnabled(false);
        add(NodeCancelButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 240, -1, -1));

        NodeApplyButton.setText("Apply");
        NodeApplyButton.setEnabled(false);
        add(NodeApplyButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 240, 70, -1));

        NodeDescriptionText.setRows(3);
        NodeDescriptionText.setEnabled(false);
        add(NodeDescriptionText, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 130, 540, 80));

    }
    // </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton NodeApplyButton;
    private javax.swing.JButton NodeCancelButton;
    private javax.swing.JLabel NodeDescriptionLabel;
    private javax.swing.JTextArea NodeDescriptionText;
    private javax.swing.JLabel NodeIndexLabel;
    private javax.swing.JTextField NodeIndexText;
    private javax.swing.JLabel NodeInstancesLabel;
    private javax.swing.JTextField NodeInstancesText;
    private javax.swing.JLabel NodeLimitsLabel;
    private javax.swing.JTextField NodeLimitsText;
    private javax.swing.JLabel NodeNameLabel;
    private javax.swing.JTextField NodeNameText;
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
