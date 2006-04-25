/*
 * ComponentPanel
 *
 * Created on 06 April 2006
 */

package nl.astron.lofar.sas.otbcomponents;

import java.awt.event.ActionEvent;
import java.rmi.RemoteException;
import java.util.Iterator;
import java.util.TreeMap;
import javax.swing.DefaultComboBoxModel;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import org.apache.log4j.Logger;

/**
 *
 * @author  coolen
 */
public class ComponentPanel extends javax.swing.JPanel {
    
    static Logger logger = Logger.getLogger(ComponentPanel.class);    

   
    /** Creates new form BeanForm based upon aParameter
     *
     * @params  aParam   Param to obtain the info from
     *
     */    
    public ComponentPanel(MainFrame aMainFrame,jOTDBparam aParam) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsParam = aParam;
        itsOtdbRmi=itsMainFrame.getSharedVars().getOTDBrmi();
        initComboLists();
        initPanel(aParam);
    }
    
    /** Creates new form BeanForm */
    public ComponentPanel() {
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
    
    public void setParam(jOTDBparam aParam) {
        if (aParam != null) {
            itsParam=aParam;
            initPanel(aParam);
        } else {
            logger.debug("No param supplied");
        }
    }
    
    private void initComboLists() {
        DefaultComboBoxModel aTypeModel = new DefaultComboBoxModel();
        TreeMap aTypeMap = itsOtdbRmi.getParamType();
        Iterator typeIt = aTypeMap.keySet().iterator();
        while (typeIt.hasNext()) {
            aTypeModel.addElement((String)aTypeMap.get(typeIt.next()));
        }
        ParamTypeText.setModel(aTypeModel);
        
        DefaultComboBoxModel aUnitModel = new DefaultComboBoxModel();
        TreeMap aUnitMap=itsOtdbRmi.getUnit();
        Iterator unitIt = aUnitMap.keySet().iterator();
        while (unitIt.hasNext()) {
            aUnitModel.addElement((String)aUnitMap.get(unitIt.next()));
        }
        ParamUnitText.setModel(aUnitModel);
    }

    private void initPanel(jOTDBparam aParam) {
        if (aParam != null) {
            setParamName(aParam.name);
            setType(aParam.type);
            setUnit(aParam.unit);
            setLimits(String.valueOf(aParam.limits));
            setDescription(aParam.description);
        } else {
            logger.debug("ERROR:  no Param given");
        }
    }
    
    /** Returns the Given Name for this Param */
    public String getParamName() {
        return this.ParamNameText.getText();
    }
    
    private void setParamName(String aS) {
        this.ParamNameText.setText(aS);
    }
    
    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableParamName(boolean enabled) {
        this.ParamNameText.setEnabled(enabled);
    }

    /** Returns the Given Type for this Param */
    public String getType() {
        return (String)this.ParamTypeText.getSelectedItem();
    }
    
    private void setType(short aS) {
        try {
            this.ParamTypeText.setSelectedItem(itsOtdbRmi.getRemoteTypes().getParamType(aS));
        } catch (RemoteException e) {
            logger.debug("Error: GetParamType failed " + e);
       }
    }
    
    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableType(boolean enabled) {
        this.ParamTypeText.setEnabled(enabled);
    }
    
        /** Returns the Given Unit for this Param */
    public String getUnit() {
        return (String)this.ParamUnitText.getSelectedItem();
    }
    
    private void setUnit(short aS) {
        try {
            this.ParamUnitText.setSelectedItem(itsOtdbRmi.getRemoteTypes().getUnit(aS));
        } catch (RemoteException e) {
            logger.debug("ERROR: getUnit failed " + e);
        }
    }
    
    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableUnit(boolean enabled) {
        this.ParamUnitText.setEnabled(enabled);
    }
    
    /** Returns the Given Limits for this Param */
    public String getLimits() {
        return this.ParamLimitsText.getText();
    }
    
    private void setLimits(String aS) {
        this.ParamLimitsText.setText(aS);
    }

    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableLimits(boolean enabled) {
        this.ParamLimitsText.setEnabled(enabled);
    }

    /** Returns the Given Description for this Param */
    public String getDescription() {
        return this.ParamDescriptionText.getText();
    }
    
    private void setDescription(String aS) {
        this.ParamDescriptionText.setText(aS);
    }
    
    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableDescription(boolean enabled) {
        this.ParamDescriptionText.setEnabled(enabled);
        this.ParamDescriptionText.setEditable(enabled);
    }


    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
        this.ParamApplyButton.setEnabled(enabled);
        this.ParamCancelButton.setEnabled(enabled);
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
        this.ParamApplyButton.setVisible(visible);
        this.ParamCancelButton.setVisible(visible);
    }
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
        enableParamName(enabled);
        enableType(enabled);
        enableUnit(enabled);
        enableLimits(enabled);
        enableDescription(enabled);
        enableButtons(enabled);
    }
    

    private void saveInput() {
        // Just check all possible fields that CAN change. The enabled method will take care if they COULD be changed.
        // this way we keep this panel general for multiple use
        boolean hasChanged = false;
        if (itsParam != null) {
            try {
                if (itsParam.type != itsOtdbRmi.getRemoteTypes().getParamType(getType())) { 
                    itsParam.type=itsOtdbRmi.getRemoteTypes().getParamType(getType());
                    hasChanged=true;
                }

                if (itsParam.unit != itsOtdbRmi.getRemoteTypes().getUnit(getUnit())) { 
                    itsParam.unit=itsOtdbRmi.getRemoteTypes().getUnit(getUnit());
                    hasChanged=true;
                }

                if (!itsParam.description.equals(getDescription())) { 
                    itsParam.description=getDescription();
                    hasChanged=true;
                }

                if (!itsParam.limits.equals(getLimits())) { 
                    itsParam.limits=getLimits();
                    hasChanged=true;
                }
                
                if (hasChanged) {
                    if (!itsOtdbRmi.getRemoteMaintenance().saveParam(itsParam)) {
                        logger.error("Saving param "+itsParam.nodeID()+","+itsParam.paramID()+"failed: "+ itsOtdbRmi.getRemoteMaintenance().errorMsg());
                    }
                    ActionEvent evt = new ActionEvent(this,-1,"component Changed");
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
        ParamNameLabel = new javax.swing.JLabel();
        ParamTypeLabel = new javax.swing.JLabel();
        ParamLimitsLabel = new javax.swing.JLabel();
        ParamNameText = new javax.swing.JTextField();
        ParamCancelButton = new javax.swing.JButton();
        ParamApplyButton = new javax.swing.JButton();
        ParamDescriptionText = new javax.swing.JTextArea();
        ParamUnitLabel = new javax.swing.JLabel();
        jLabel1 = new javax.swing.JLabel();
        ParamTypeText = new javax.swing.JComboBox();
        ParamUnitText = new javax.swing.JComboBox();
        ParamLimitsText = new javax.swing.JTextField();

        ParamNameLabel.setText("Name :");

        ParamTypeLabel.setText("Type :");

        ParamLimitsLabel.setText("Limits :");

        ParamNameText.setText("None");
        ParamNameText.setToolTipText("Name for this Node");
        ParamNameText.setMaximumSize(new java.awt.Dimension(440, 19));
        ParamNameText.setMinimumSize(new java.awt.Dimension(440, 19));
        ParamNameText.setPreferredSize(new java.awt.Dimension(440, 19));

        ParamCancelButton.setText("Cancel");
        ParamCancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ParamCancelButtonActionPerformed(evt);
            }
        });

        ParamApplyButton.setText("Apply");
        ParamApplyButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ParamApplyButtonActionPerformed(evt);
            }
        });

        ParamDescriptionText.setRows(3);
        ParamDescriptionText.setBorder(javax.swing.BorderFactory.createTitledBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true), "Description"));

        ParamUnitLabel.setText("Unit :");

        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("Component View Panel");

        ParamTypeText.setEditable(true);

        ParamUnitText.setEditable(true);

        ParamLimitsText.setText("None");

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(114, 114, 114)
                .add(jLabel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 320, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
            .add(layout.createSequentialGroup()
                .add(40, 40, 40)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(ParamCancelButton)
                        .add(5, 5, 5)
                        .add(ParamApplyButton))
                    .add(ParamDescriptionText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 530, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(layout.createSequentialGroup()
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(ParamLimitsLabel)
                            .add(ParamUnitLabel)
                            .add(ParamTypeLabel)
                            .add(ParamNameLabel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 80, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                        .add(20, 20, 20)
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(ParamNameText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 430, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(ParamUnitText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 240, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(ParamTypeText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 240, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(ParamLimitsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 240, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))))
                .add(57, 57, 57))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(10, 10, 10)
                .add(jLabel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 20, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(20, 20, 20)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(ParamNameLabel)
                    .add(ParamNameText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(15, 15, 15)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(ParamLimitsLabel)
                    .add(ParamLimitsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(15, 15, 15)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(ParamUnitLabel)
                    .add(ParamUnitText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(11, 11, 11)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(ParamTypeLabel)
                    .add(ParamTypeText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(32, 32, 32)
                .add(ParamDescriptionText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 80, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(41, 41, 41)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(ParamCancelButton)
                    .add(ParamApplyButton))
                .add(30, 30, 30))
        );
    }// </editor-fold>//GEN-END:initComponents

    private void ParamApplyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ParamApplyButtonActionPerformed
        saveInput();
    }//GEN-LAST:event_ParamApplyButtonActionPerformed

    private void ParamCancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ParamCancelButtonActionPerformed
        initPanel(itsParam);
    }//GEN-LAST:event_ParamCancelButtonActionPerformed
    
    private MainFrame  itsMainFrame;
    private OtdbRmi    itsOtdbRmi;
    private jOTDBparam itsParam;

    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton ParamApplyButton;
    private javax.swing.JButton ParamCancelButton;
    private javax.swing.JTextArea ParamDescriptionText;
    private javax.swing.JLabel ParamLimitsLabel;
    private javax.swing.JTextField ParamLimitsText;
    private javax.swing.JLabel ParamNameLabel;
    private javax.swing.JTextField ParamNameText;
    private javax.swing.JLabel ParamTypeLabel;
    private javax.swing.JComboBox ParamTypeText;
    private javax.swing.JLabel ParamUnitLabel;
    private javax.swing.JComboBox ParamUnitText;
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
