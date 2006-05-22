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
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.treenodes.TreeNode;
import org.apache.log4j.Logger;

/**
 *
 * @author  coolen
 */
public class ParameterViewPanel extends javax.swing.JPanel implements IViewPanel {
    
    static Logger logger = Logger.getLogger(ParameterViewPanel.class);    
    static String name = "Parameter";
   
    /** Creates new form based upon aParameter
     *
     * @params  aParam   Param to obtain the info from
     *
     */    
    public ParameterViewPanel(MainFrame aMainFrame,jOTDBparam aParam) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsParam = aParam;
        itsOtdbRmi=itsMainFrame.getSharedVars().getOTDBrmi();
        initComboLists();
        initPanel();
    }
    
    /** Creates new form based upon aNode
     *
     * @params  aNode   Node to obtain the info from
     *
     */    
    public ParameterViewPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode = aNode;
        getParam(itsNode);
        itsOtdbRmi=itsMainFrame.getSharedVars().getOTDBrmi();
        initComboLists();
        initPanel();
    }

    /** Creates new form BeanForm */
    public ParameterViewPanel() {
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
    
    public String getShortName() {
        return name;
    }
    
    public void setContent(Object anObject) {
        if (anObject != null) {
            TreeNode aNode = (TreeNode)anObject;
            itsNode = (jOTDBnode)aNode.getUserObject();
            initPanel();
            getParam(itsNode);
            initPanel();
        } else {
            logger.debug("No node supplied");
        }
    }
    
    private void getParam(jOTDBnode aNode) {
        if (aNode == null) {
            logger.debug("ERROR: Empty Node supplied for getParam");
            return;
        }
        itsNode=aNode;
        try {
            itsParam = itsMainFrame.getSharedVars().getOTDBrmi().getRemoteMaintenance().getParam(itsMainFrame.getSharedVars().getTreeID(),
                    aNode.paramDefID());
        } catch (RemoteException ex) {
            logger.debug("Error during getParam: "+ ex);
            itsParam=null;
            return;
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

    private void initPanel() {
    
        // check access
        UserAccount userAccount = itsMainFrame.getUserAccount();
        
        // For now:
        enableLimits(true);
        enableDescription(true);
        
        if(userAccount.isAdministrator()) {
            // enable/disable certain controls
        }
        if(userAccount.isAstronomer()) {
            // enable/disable certain controls
        }
        if(userAccount.isInstrumentScientist()) {
            // enable/disable certain controls
        }

        if (itsParam != null) {
            setParamName(itsParam.name);
            setIndex(String.valueOf(itsParam.index));
            setType(itsParam.type);
            setUnit(itsParam.unit);
            setPruning(String.valueOf(itsParam.pruning));
            setValMoment(String.valueOf(itsParam.valMoment));
            setRuntimeMod(itsParam.runtimeMod);
            setLimits(String.valueOf(itsParam.limits));
            setDescription(itsParam.description);
        } else {
            logger.debug("ERROR:  no Param given");
        }
    }
    
    private String getParamName() {
        return this.ParamNameText.getText();
    }
    
    private void setParamName(String aS) {
        this.ParamNameText.setText(aS);
    }
    
    private void enableParamName(boolean enabled) {
        this.ParamNameText.setEnabled(enabled);
    }

    private String getIndex() {
        return this.ParamIndexText.getText();
    }
    
    private void setIndex(String aS) {
        this.ParamIndexText.setText(aS);
    }
    
    private void enableIndex(boolean enabled) {
        this.ParamIndexText.setEnabled(enabled);
    }

    private String getType() {
        return (String)this.ParamTypeText.getSelectedItem();
    }
    
    private void setType(short aS) {
        try {
            this.ParamTypeText.setSelectedItem(itsOtdbRmi.getRemoteTypes().getParamType(aS));
        } catch (RemoteException e) {
            logger.debug("Error: GetParamType failed " + e);
       }
    }
    
    private void enableType(boolean enabled) {
        this.ParamTypeText.setEnabled(enabled);
    }
    
    private String getUnit() {
        return (String)this.ParamUnitText.getSelectedItem();
    }
    
    private void setUnit(short aS) {
        try {
            this.ParamUnitText.setSelectedItem(itsOtdbRmi.getRemoteTypes().getUnit(aS));
        } catch (RemoteException e) {
            logger.debug("ERROR: getUnit failed " + e);
        }
    }
    
    private void enableUnit(boolean enabled) {
        this.ParamUnitText.setEnabled(enabled);
    }
    
    private String getPruning() {
        return this.ParamPruningText.getText();
    }
    
    private void setPruning(String aS) {
        this.ParamPruningText.setText(aS);    
    }

    private void enablePruning(boolean enabled) {
        this.ParamPruningText.setEnabled(enabled);
    }

    private String getValMoment() {
        return this.ParamValMomentText.getText();
    }
    
    private void setValMoment(String aS) {
        this.ParamValMomentText.setText(aS);    
    }

    private void enableValMoment(boolean enabled) {
        this.ParamValMomentText.setEnabled(enabled);
    }

    private boolean getRuntimeMod() {
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

    private void enableRuntimeMod(boolean enabled) {
        this.ParamRuntimeModText.setEnabled(enabled);
    }
    
    
    private String getLimits() {
        return this.ParamLimitsText.getText();
    }
    
    private void setLimits(String aS) {
        this.ParamLimitsText.setText(aS);
    }

    private void enableLimits(boolean enabled) {
        this.ParamLimitsText.setEnabled(enabled);
    }

    private String getDescription() {
        return this.ParamDescriptionText.getText();
    }
    
    private void setDescription(String aS) {
        this.ParamDescriptionText.setText(aS);
    }
    
    private void enableDescription(boolean enabled) {
        this.ParamDescriptionText.setEnabled(enabled);
        this.ParamDescriptionText.setEditable(enabled);
    }

    /** Enables the buttons
     *
     * @param   enabled     true/false enable/disable
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

                if (!String.valueOf(itsParam.pruning).equals(getPruning())) { 
                    itsParam.pruning=Integer.valueOf(getPruning()).shortValue();
                    hasChanged=true;
                }

                if (!String.valueOf(itsParam.valMoment).equals(getValMoment())) { 
                    itsParam.valMoment=Integer.valueOf(getValMoment()).shortValue();
                    hasChanged=true;
                }

                if (itsParam.runtimeMod != getRuntimeMod()) { 
                    itsParam.runtimeMod=getRuntimeMod();
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

        ParamNameLabel.setText("Name :");

        ParamIndexLabel.setText("Index :");

        ParamTypeLabel.setText("Type :");

        ParamLimitsLabel.setText("Limits :");

        ParamIndexText.setText("None");
        ParamIndexText.setMaximumSize(new java.awt.Dimension(200, 19));
        ParamIndexText.setMinimumSize(new java.awt.Dimension(200, 19));
        ParamIndexText.setPreferredSize(new java.awt.Dimension(200, 19));

        ParamPruningText.setText("-1");
        ParamPruningText.setToolTipText("Number of Instances for this Node ");
        ParamPruningText.setMaximumSize(new java.awt.Dimension(200, 19));
        ParamPruningText.setMinimumSize(new java.awt.Dimension(200, 19));
        ParamPruningText.setPreferredSize(new java.awt.Dimension(200, 19));

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

        ParamPruningLabel.setText("Pruning :");

        ParamValMomentLabel.setText("ValMoment :");

        ParamRuntimeModLabel.setText("RuntimeMod :");

        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("Parameter View Panel");

        ParamTypeText.setEditable(true);

        ParamUnitText.setEditable(true);

        ParamValMomentText.setText("None");

        ParamLimitsText.setText("None");

        ParamRuntimeModText.setEditable(true);
        ParamRuntimeModText.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "false", "true" }));

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(114, 114, 114)
                        .add(jLabel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 320, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(layout.createSequentialGroup()
                        .add(40, 40, 40)
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(layout.createSequentialGroup()
                                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                                    .add(ParamIndexLabel)
                                    .add(ParamNameLabel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 80, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(ParamTypeLabel)
                                    .add(ParamUnitLabel)
                                    .add(ParamPruningLabel)
                                    .add(ParamValMomentLabel)
                                    .add(ParamRuntimeModLabel)
                                    .add(ParamLimitsLabel))
                                .add(20, 20, 20)
                                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                                    .add(ParamLimitsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 240, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(ParamRuntimeModText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 240, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(ParamValMomentText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 240, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(ParamPruningText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 240, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(ParamUnitText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 240, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(ParamTypeText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 240, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(ParamNameText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 430, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(ParamIndexText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 240, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                            .add(ParamDescriptionText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 530, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(layout.createSequentialGroup()
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(ParamCancelButton)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(ParamApplyButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 70, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))))
                .add(403, 403, 403))
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
                .add(11, 11, 11)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(ParamIndexLabel)
                    .add(ParamIndexText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(6, 6, 6)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(ParamTypeLabel)
                    .add(ParamTypeText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(ParamUnitLabel)
                    .add(ParamUnitText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(ParamPruningLabel)
                    .add(ParamPruningText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(ParamValMomentLabel)
                    .add(ParamValMomentText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(ParamRuntimeModLabel)
                    .add(ParamRuntimeModText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(ParamLimitsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(ParamLimitsLabel))
                .add(19, 19, 19)
                .add(ParamDescriptionText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 80, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(30, 30, 30)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(ParamApplyButton)
                    .add(ParamCancelButton))
                .addContainerGap(26, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents

    private void ParamApplyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ParamApplyButtonActionPerformed
        saveInput();
    }//GEN-LAST:event_ParamApplyButtonActionPerformed

    private void ParamCancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ParamCancelButtonActionPerformed
        initPanel();
    }//GEN-LAST:event_ParamCancelButtonActionPerformed
    
    private MainFrame  itsMainFrame;
    private OtdbRmi    itsOtdbRmi;
    private jOTDBnode  itsNode;
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
