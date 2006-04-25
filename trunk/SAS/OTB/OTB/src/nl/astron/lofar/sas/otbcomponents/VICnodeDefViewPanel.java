/*
 * NodeViewPanel.java
 *
 * Created on 26 januari 2006, 15:47
 */

package nl.astron.lofar.sas.otbcomponents;

import java.awt.event.ActionEvent;
import java.rmi.RemoteException;
import java.util.Iterator;
import java.util.TreeMap;
import javax.swing.DefaultComboBoxModel;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jVICnodeDef;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import org.apache.log4j.Logger;

/**
 *
 * @author  coolen
 */
public class VICnodeDefViewPanel extends javax.swing.JPanel {
    
    static Logger logger = Logger.getLogger(VICnodeDefViewPanel.class);    

   
    /** Creates new form BeanForm based upon aVICnodeDef
     *
     * @params  aNode   node to obtain the info from
     *
     */
    public VICnodeDefViewPanel(MainFrame aMainFrame,jVICnodeDef aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode = aNode;
        itsOtdbRmi=itsMainFrame.getSharedVars().getOTDBrmi();
        initComboLists();
        initPanel(aNode);
    }
    
    /** Creates new form BeanForm */
    public VICnodeDefViewPanel() {
        initComponents();
    }
    
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
            itsOtdbRmi=itsMainFrame.getSharedVars().getOTDBrmi();
            initComboLists();
        } else {
            logger.debug("No Mainframe supplied");
        }
    }
    
    public void setNode(jVICnodeDef aNode) {
        initPanel(aNode);
    }

    private void initComboLists() {
        DefaultComboBoxModel aClassifModel = new DefaultComboBoxModel();
        TreeMap aClassifMap = itsOtdbRmi.getClassif();
        Iterator classifIt = aClassifMap.keySet().iterator();
        while (classifIt.hasNext()) {
            aClassifModel.addElement((String)aClassifMap.get(classifIt.next()));
        }
        ClassificationText.setModel(aClassifModel);
    }
     
    private void initPanel(jVICnodeDef aNode) {
        itsNode = aNode;
         if (aNode != null) {
            setName(aNode.name);
            setVersion(String.valueOf(aNode.version));
            setClassif(String.valueOf(aNode.classif));
            setConstraints(aNode.constraints);
            setDescription(aNode.description);
        } else {
            logger.debug("ERROR:  no node given");
        }
    }
    
    /** Returns the Given Name for this Node */
    public String getNodeName() {
        return this.NameText.getText();
    }
    
    private void setNodeName(String aS) {
        this.NameText.setText(aS);
    }
    
    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableName(boolean enabled) {
        this.NameText.setEnabled(enabled);
    }
    
    /** Returns the Given Version for this Node */
    public String getVersion() {
        return this.VersionText.getText();
    }
    
    private void setVersion(String aS) {
        this.VersionText.setText(aS);
    }
    
    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableVersion(boolean enabled) {
        this.VersionText.setEnabled(enabled);
    }
    

    /** Returns the Given Classification for this Node */
    public String getClassif() {
        return (String)this.ClassificationText.getSelectedItem();
    }
    
    private void setClassif(String aS) {
        try {
            this.ClassificationText.setSelectedItem(itsOtdbRmi.getRemoteTypes().getClassif(aS));
        } catch (RemoteException e) {
            logger.debug("Error: GetParamType failed " + e);
        }
    }
    
    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableClassif(boolean enabled) {
        this.ClassificationText.setEnabled(enabled);
    }
    
    /** Returns the Given Constraints for this Node */
    public String getConstraints() {
        return this.ConstraintsText.getText();
    }
    
    private void setConstraints(String aS) {
        this.ConstraintsText.setText(aS);
    }

    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableConstraints(boolean enabled) {
        this.ConstraintsText.setEnabled(enabled);
    }    
    
    /** Returns the Given Description for this Node */
    public String getDescription() {
        return this.DescriptionText.getText();
    }
    
    private void setDescription(String aS) {
        this.DescriptionText.setText(aS);
    }
    
    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableDescription(boolean enabled) {
        this.DescriptionText.setEnabled(enabled);
        this.DescriptionText.setEditable(enabled);
    }    

    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
        this.NodeApplyButton.setEnabled(enabled);
        this.NodeCancelButton.setEnabled(enabled);
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
        this.NodeApplyButton.setVisible(visible);
        this.NodeCancelButton.setVisible(visible);
    }

    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
        enableName(enabled);
        enableVersion(enabled);
        enableClassif(enabled);
        enableConstraints(enabled);
        enableDescription(enabled);
        enableButtons(enabled);
    }
    
   
    private void saveInput() {
        // Just check all possible fields that CAN change. The enabled method will take care if they COULD be changed.
        // this way we keep this panel general for multiple use
        boolean hasChanged = false;
        if (itsNode != null) {
            try {


                if (!itsNode.constraints.equals(getConstraints())) { 
                    itsNode.constraints=getConstraints();
                    hasChanged=true;
                }

                if (!itsNode.description.equals(getDescription())) { 
                    itsNode.description=getDescription();
                    hasChanged=true;
                }

                if (hasChanged) {
                    if (!itsOtdbRmi.getRemoteMaintenance().saveComponentNode(itsNode)) {
                        logger.error("Saving node failed: "+ itsOtdbRmi.getRemoteMaintenance().errorMsg());
                    }
                    
                    ActionEvent evt = new ActionEvent(this,-1,"VICnodeChanged");
                    this.fireActionListenerActionPerformed(evt); 
                    
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
        jLabel1 = new javax.swing.JLabel();
        NameLabel = new javax.swing.JLabel();
        NameText = new javax.swing.JTextField();
        VersionLabel = new javax.swing.JLabel();
        VersionText = new javax.swing.JTextField();
        ClassificationLabel = new javax.swing.JLabel();
        ClassificationText = new javax.swing.JComboBox();
        ConstraintsLabel = new javax.swing.JLabel();
        ConstraintsText = new javax.swing.JTextField();
        DescriptionText = new javax.swing.JTextArea();
        NodeCancelButton = new javax.swing.JButton();
        NodeApplyButton = new javax.swing.JButton();

        setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("VICnodeDef View Panel");
        add(jLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(70, 10, 460, 20));

        NameLabel.setText("Name :");
        add(NameLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 50, 80, -1));

        NameText.setText("None");
        NameText.setToolTipText("Name for this Node");
        NameText.setMaximumSize(new java.awt.Dimension(440, 19));
        NameText.setMinimumSize(new java.awt.Dimension(440, 19));
        NameText.setPreferredSize(new java.awt.Dimension(440, 19));
        add(NameText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 50, 430, -1));

        VersionLabel.setText("Version :");
        add(VersionLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 80, -1, -1));

        VersionText.setText("None");
        VersionText.setToolTipText("Version number for this VICnodeDef");
        VersionText.setMaximumSize(new java.awt.Dimension(200, 19));
        VersionText.setMinimumSize(new java.awt.Dimension(200, 19));
        VersionText.setPreferredSize(new java.awt.Dimension(200, 19));
        add(VersionText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 80, 200, -1));

        ClassificationLabel.setText("Classification :");
        add(ClassificationLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 110, -1, -1));

        ClassificationText.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        add(ClassificationText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 110, 200, 20));

        ConstraintsLabel.setText("Constraints :");
        add(ConstraintsLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 140, -1, -1));

        ConstraintsText.setText("None");
        ConstraintsText.setToolTipText("Limits for this Node");
        ConstraintsText.setMaximumSize(new java.awt.Dimension(200, 19));
        ConstraintsText.setMinimumSize(new java.awt.Dimension(200, 19));
        ConstraintsText.setPreferredSize(new java.awt.Dimension(200, 19));
        add(ConstraintsText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 140, 430, -1));

        DescriptionText.setRows(4);
        DescriptionText.setBorder(javax.swing.BorderFactory.createTitledBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true), "Description"));
        DescriptionText.setEnabled(false);
        add(DescriptionText, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 220, 540, 100));

        NodeCancelButton.setText("Cancel");
        NodeCancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                NodeCancelButtonActionPerformed(evt);
            }
        });

        add(NodeCancelButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 350, -1, -1));

        NodeApplyButton.setText("Apply");
        NodeApplyButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                NodeApplyButtonActionPerformed(evt);
            }
        });

        add(NodeApplyButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 350, 70, -1));

    }// </editor-fold>//GEN-END:initComponents

    private void NodeApplyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NodeApplyButtonActionPerformed
        saveInput();
    }//GEN-LAST:event_NodeApplyButtonActionPerformed

    private void NodeCancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NodeCancelButtonActionPerformed
        initPanel(itsNode);
    }//GEN-LAST:event_NodeCancelButtonActionPerformed
    
    private jVICnodeDef itsNode = null;
    private MainFrame   itsMainFrame;
    private OtdbRmi     itsOtdbRmi;   
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel ClassificationLabel;
    private javax.swing.JComboBox ClassificationText;
    private javax.swing.JLabel ConstraintsLabel;
    private javax.swing.JTextField ConstraintsText;
    private javax.swing.JTextArea DescriptionText;
    private javax.swing.JLabel NameLabel;
    private javax.swing.JTextField NameText;
    private javax.swing.JButton NodeApplyButton;
    private javax.swing.JButton NodeCancelButton;
    private javax.swing.JLabel VersionLabel;
    private javax.swing.JTextField VersionText;
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
