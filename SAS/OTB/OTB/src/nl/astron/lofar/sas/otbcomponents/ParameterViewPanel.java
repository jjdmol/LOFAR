/*
 * ParamViewPanel.java
 *
 * Created on 26 januari 2006, 15:47
 */

package nl.astron.lofar.sas.otbcomponents;

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
public class ParameterViewPanel extends javax.swing.JPanel {
    
    static Logger logger = Logger.getLogger(ParameterViewPanel.class);    

   
    /** Creates new form BeanForm based upon aParameter
     *
     * @params  aParam   Param to obtain the info from
     *
     */    
    public ParameterViewPanel(MainFrame aMainFrame,jOTDBparam aParam) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsParam = aParam;
        itsOtdbRmi=itsMainFrame.getOTDBrmi();
        initComboLists();
        initPanel(aParam);
    }
    
    /** Creates new form BeanForm */
    public ParameterViewPanel() {
        initComponents();
    }
    
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
            itsOtdbRmi=itsMainFrame.getOTDBrmi();
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
            setIndex(String.valueOf(aParam.index));
            setType(aParam.type);
            setUnit(aParam.unit);
            setPruning(String.valueOf(aParam.pruning));
            setValMoment(String.valueOf(aParam.valMoment));
            setRuntimeMod(aParam.runtimeMod);
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

    /** Returns the Given Index for this Param */
    public String getIndex() {
        return this.ParamIndexText.getText();
    }
    
    private void setIndex(String aS) {
        this.ParamIndexText.setText(aS);
    }
    
    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableIndex(boolean enabled) {
        this.ParamIndexText.setEnabled(enabled);
    }

    /** Returns the Given Type for this Param */
    public String getType() {
        return (String)this.ParamTypeText.getSelectedItem();
    }
    
    private void setType(short aS) {
        try {
            this.ParamTypeText.setSelectedItem(itsOtdbRmi.getRemoteTypes().getParamType(itsParam.type));
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
            this.ParamUnitText.setSelectedItem(itsOtdbRmi.getRemoteTypes().getUnit(itsParam.unit));
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
    
    /** Returns the Given pruning for this Param */
    public String getPruning() {
        return this.ParamPruningText.getText();
    }
    
    private void setPruning(String aS) {
        this.ParamPruningText.setText(aS);    
    }

    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enablePruning(boolean enabled) {
        this.ParamPruningText.setEnabled(enabled);
    }

    /** Returns the Given valMoment for this Param */
    public String getValMoment() {
        return this.ParamValMomentText.getText();
    }
    
    private void setValMoment(String aS) {
        this.ParamValMomentText.setText(aS);    
    }

    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableValMoment(boolean enabled) {
        this.ParamValMomentText.setEnabled(enabled);
    }

    /** Returns the Given runtimeMod for this Param */
    public boolean getRuntimeMod() {
        if (this.ParamRuntimeModText.getSelectedItem().equals("true")) {
            return true;
        } else {
            return false;
        }
    }
    
    private void setRuntimeMod(boolean aB) {
        if (aB) {
            this.ParamRuntimeModText.setSelectedItem("true");
        } else {
            this.ParamRuntimeModText.setSelectedItem("false");            
        }
    }

    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableRuntimeMod(boolean enabled) {
        this.ParamRuntimeModText.setEnabled(enabled);
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
    
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
        enableParamName(enabled);
        enableIndex(enabled);
        enableType(enabled);
        enableUnit(enabled);
        enablePruning(enabled);
        enableValMoment(enabled);
        enableRuntimeMod(enabled);
        enableLimits(enabled);
        enableDescription(enabled);
        enableButtons(enabled);
    }
    


    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        ParamNameLabel = new javax.swing.JLabel();
        ParamIndexLabel = new javax.swing.JLabel();
        ParamTypeLabel = new javax.swing.JLabel();
        ParamLimitsLabel = new javax.swing.JLabel();
        ParamIndexText = new javax.swing.JTextField();
        ParamPruningText = new javax.swing.JTextField();
        ParamNameText = new javax.swing.JTextField();
        ParamCancelButton = new javax.swing.JButton();
        ParamApplyButton = new javax.swing.JButton();
        ParamDescriptionText = new javax.swing.JTextArea();
        ParamUnitLabel = new javax.swing.JLabel();
        ParamPruningLabel = new javax.swing.JLabel();
        ParamValMomentLabel = new javax.swing.JLabel();
        ParamRuntimeModLabel = new javax.swing.JLabel();
        jLabel1 = new javax.swing.JLabel();
        ParamTypeText = new javax.swing.JComboBox();
        ParamUnitText = new javax.swing.JComboBox();
        ParamValMomentText = new javax.swing.JTextField();
        ParamLimitsText = new javax.swing.JTextField();
        ParamRuntimeModText = new javax.swing.JComboBox();

        setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        ParamNameLabel.setText("Name :");
        add(ParamNameLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 50, 80, -1));

        ParamIndexLabel.setText("Index :");
        add(ParamIndexLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 75, -1, -1));

        ParamTypeLabel.setText("Type :");
        add(ParamTypeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 100, -1, -1));

        ParamLimitsLabel.setText("Limits :");
        add(ParamLimitsLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 225, -1, -1));

        ParamIndexText.setEditable(false);
        ParamIndexText.setText("None");
        ParamIndexText.setEnabled(false);
        ParamIndexText.setMaximumSize(new java.awt.Dimension(200, 19));
        ParamIndexText.setMinimumSize(new java.awt.Dimension(200, 19));
        ParamIndexText.setPreferredSize(new java.awt.Dimension(200, 19));
        add(ParamIndexText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 75, 240, -1));

        ParamPruningText.setEditable(false);
        ParamPruningText.setText("-1");
        ParamPruningText.setToolTipText("Number of Instances for this Node ");
        ParamPruningText.setEnabled(false);
        ParamPruningText.setMaximumSize(new java.awt.Dimension(200, 19));
        ParamPruningText.setMinimumSize(new java.awt.Dimension(200, 19));
        ParamPruningText.setPreferredSize(new java.awt.Dimension(200, 19));
        add(ParamPruningText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 150, 240, -1));

        ParamNameText.setEditable(false);
        ParamNameText.setText("None");
        ParamNameText.setToolTipText("Name for this Node");
        ParamNameText.setEnabled(false);
        ParamNameText.setMaximumSize(new java.awt.Dimension(440, 19));
        ParamNameText.setMinimumSize(new java.awt.Dimension(440, 19));
        ParamNameText.setPreferredSize(new java.awt.Dimension(440, 19));
        add(ParamNameText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 50, 240, -1));

        ParamCancelButton.setText("Cancel");
        ParamCancelButton.setEnabled(false);
        add(ParamCancelButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 400, -1, -1));

        ParamApplyButton.setText("Apply");
        ParamApplyButton.setEnabled(false);
        add(ParamApplyButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 400, 70, -1));

        ParamDescriptionText.setEditable(false);
        ParamDescriptionText.setRows(3);
        ParamDescriptionText.setBorder(javax.swing.BorderFactory.createTitledBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true), "Description"));
        ParamDescriptionText.setEnabled(false);
        add(ParamDescriptionText, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 250, 530, 80));

        ParamUnitLabel.setText("Unit :");
        add(ParamUnitLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 125, -1, -1));

        ParamPruningLabel.setText("Pruning :");
        add(ParamPruningLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 150, -1, -1));

        ParamValMomentLabel.setText("ValMoment :");
        add(ParamValMomentLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 175, -1, -1));

        ParamRuntimeModLabel.setText("RuntimeMod :");
        add(ParamRuntimeModLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 200, -1, -1));

        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("Parameter View Panel");
        add(jLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(114, 10, 320, 20));

        ParamTypeText.setEnabled(false);
        add(ParamTypeText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 100, 240, -1));

        ParamUnitText.setEnabled(false);
        add(ParamUnitText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 125, 240, -1));

        ParamValMomentText.setEditable(false);
        ParamValMomentText.setText("None");
        ParamValMomentText.setEnabled(false);
        add(ParamValMomentText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 175, 240, -1));

        ParamLimitsText.setEditable(false);
        ParamLimitsText.setText("None");
        ParamLimitsText.setEnabled(false);
        add(ParamLimitsText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 225, 240, -1));

        ParamRuntimeModText.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "false", "true" }));
        ParamRuntimeModText.setEnabled(false);
        add(ParamRuntimeModText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 200, 240, -1));

    }// </editor-fold>//GEN-END:initComponents
    
    private MainFrame  itsMainFrame;
    private OtdbRmi    itsOtdbRmi;
    private jOTDBparam itsParam;

    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton ParamApplyButton;
    private javax.swing.JButton ParamCancelButton;
    private javax.swing.JTextArea ParamDescriptionText;
    private javax.swing.JLabel ParamIndexLabel;
    private javax.swing.JTextField ParamIndexText;
    private javax.swing.JLabel ParamLimitsLabel;
    private javax.swing.JTextField ParamLimitsText;
    private javax.swing.JLabel ParamNameLabel;
    private javax.swing.JTextField ParamNameText;
    private javax.swing.JLabel ParamPruningLabel;
    private javax.swing.JTextField ParamPruningText;
    private javax.swing.JLabel ParamRuntimeModLabel;
    private javax.swing.JComboBox ParamRuntimeModText;
    private javax.swing.JLabel ParamTypeLabel;
    private javax.swing.JComboBox ParamTypeText;
    private javax.swing.JLabel ParamUnitLabel;
    private javax.swing.JComboBox ParamUnitText;
    private javax.swing.JLabel ParamValMomentLabel;
    private javax.swing.JTextField ParamValMomentText;
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
