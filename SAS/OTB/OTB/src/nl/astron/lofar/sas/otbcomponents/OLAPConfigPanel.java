/*
 * OLAPConficPanel.java
 *
 * Created on 17 mey 2006, 13:47
 *
 * Panel for OLAP specific Configuration
 */

package nl.astron.lofar.sas.otbcomponents;

import java.util.Iterator;
import java.util.Vector;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.UserAccount;
import org.apache.log4j.Logger;

/**
 *
 * @author  coolen
 */
public class OLAPConfigPanel extends javax.swing.JPanel implements IViewPanel{
    
    static Logger logger = Logger.getLogger(OLAPConfigPanel.class);    
    static String name = "OLAPConfig";

   
    /** Creates new form BeanForm based upon aNode
     *
     * @params  aNode   node to obtain the info from
     *
     */
    public OLAPConfigPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode=aNode;
        initPanel();
    }
    
    /** Creates new form BeanForm */
    public OLAPConfigPanel() {
        initComponents();
    }
    
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
        } else {
            logger.debug("No Mainframe supplied");
        }
    }
    
    public String getShortName() {
        return name;
    }
    
    public void setContent(Object anObject) {
        itsNode=(jOTDBnode)anObject;
        initPanel();
    }

     private void initPanel() {
        // check access
        UserAccount userAccount = itsMainFrame.getUserAccount();

        // for now:
        setAllEnabled(true);
        
        if(userAccount.isAdministrator()) {
            // enable/disable certain controls
        }
        if(userAccount.isAstronomer()) {
            // enable/disable certain controls
        }
        if(userAccount.isInstrumentScientist()) {
            // enable/disable certain controls
        }
        
        
        
         if (itsNode != null) {
            // [TODO]
            // Fill from existing cfg needed ????
         } else {
            logger.debug("ERROR:  no node given");
        }
    }
    
    private int getNrSamples() {
        return Integer.valueOf(this.NrSamplesText.getText()).intValue();
    }
    
    private void setNrSamples(int anI) {
        this.NrSamplesText.setText(String.valueOf(anI));
    }
    
    private double getNrBufSec() {
        return Double.valueOf(this.NrBufSecText.getText()).doubleValue();
    }
    
    private void setNrBufSec(double aD) {
        this.NrBufSecText.setText(String.valueOf(aD));
    }
    
    private boolean getUseAMC() {
        if (this.UseAMCCheckBox.isSelected()) {
            return true;
        } else {
            return false;
        }
    }
    
    private void setUseAMC(boolean aB) {
        this.UseAMCCheckBox.setSelected(aB);
    }

    private int getNrNodesCell() {
        return Integer.valueOf(this.NrNodesCellText.getText()).intValue();
    }
    
    private void setNrNodesCell(int anI) {
        this.NrNodesCellText.setText(String.valueOf(anI));
    }

    private int getNrSubbandsCell() {
        return Integer.valueOf(this.NrSubbandsCellText.getText()).intValue();
    }
    
    private void setNrSubbandsCell(int anI) {
        this.NrSubbandsCellText.setText(String.valueOf(anI));
    }
    
    private int getNrFilterTabs() {
        return Integer.valueOf(this.NrFilterTabsText.getText()).intValue();
    }
    
    private void setNrFilterTabs(int anI) {
        this.NrFilterTabsText.setText(String.valueOf(anI));
    }

    private void enableNrSamples(boolean enabled) {
        this.NrSamplesText.setEnabled(enabled);
    }

    private void enableNrBufSec(boolean enabled) {
        this.NrBufSecText.setEnabled(enabled);
    }

    private void enableUseAMC(boolean enabled) {
        this.UseAMCCheckBox.setEnabled(enabled);
    }

    private void enableNrNodesCell(boolean enabled) {
        this.NrNodesCellText.setEnabled(enabled);
    }

    private void enableNrSubbandsCell(boolean enabled) {
        this.NrSubbandsCellText.setEditable(enabled);
    }

    private void enableNrFilterTabs(boolean enabled) {
        this.NrFilterTabsText.setEditable(enabled);
    }


    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
        this.enableSpecificButtons(enabled);
        this.enableHardwareButtons(enabled);
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
        this.setSpecificButtonsVisible(visible);
        this.setHardwareButtonsVisible(visible);
    }

    
    private String getAMCServerHost() {
        return this.AMCServerHostText.getText();
    }
    
    private void setAMCServerHost(String aS) {
        this.AMCServerHostText.setText(aS);
    }
    
    private String getDelayCompensationHost() {
    return this.DelayCompensationHostText.getText();
    }
    
    private void setDelayCompensationHost(String aS) {
        this.DelayCompensationHostText.setText(aS);
    }
    
    private String getInputSectionClusterHost() {
        return this.InputSectionClusterHostText.getText();
    }
    
    private void setInputSectionClusterHost(String aS) {
        this.InputSectionClusterHostText.setText(aS);
    }
    
    private String getClusterNodeNames() {
        return this.InputNodeNamesText.getText();
    }
    
    private void setClusterNodeNames(String aS) {
        this.InputNodeNamesText.setText(aS);
    }

    private String getStorageClusterHost() {
        return this.StorageClusterHostText.getText();
    }
    
    private void setStorageClusterHost(String aS) {
        this.StorageClusterHostText.setText(aS);
    }

    private String getStorageNodeNames() {
        return this.StorageNodeNamesText.getText();
    }
    
    private void setStorageNodeNames(String aS) {
        this.StorageNodeNamesText.setText(aS);
    }

    private String getBGLPartition() {
        return this.BGLPartitionText.getText();
    }
    
    private void setBGLPartition(String aS) {
        this.BGLPartitionText.setText(aS);
    }

    private int getAMCServerPort() {
        return Integer.valueOf(this.AMCServerPortText.getText()).intValue();
    }
    
    private void setAMCServerHost(int anI) {
        this.AMCServerPortText.setText(String.valueOf(anI));
    }

    private Vector<Integer> getDelayInputPorts() {
        String aS[]=this.DelayInputPortsText.getText().split(",");
        Vector<Integer> aV = new Vector<Integer>(aS.length);
        for (int i = 0 ; i< aS.length;i++) {
            aV.add(Integer.valueOf(aS[i]));
        }
        return aV;
    }
    
    private void setDelayInputPorts(Vector aV) {
        String aS="";
        Iterator it = aV.iterator();
        if (it.hasNext()) {
            aS+=(String)it.next();
        }
        while (it.hasNext()) {
            aS+=","+(String)it.next();
        }
        
        this.DelayInputPortsText.setText(aS);
    }

    private void enableSpecificButtons(boolean enabled) {
        this.SpecificApplyButton.setEnabled(enabled);
        this.SpecificCancelButton.setEnabled(enabled);
    }
    
    private void setSpecificButtonsVisible(boolean visible) {
        this.SpecificApplyButton.setVisible(visible);
        this.SpecificCancelButton.setVisible(visible);
    }

    private void enableHardwareButtons(boolean enabled) {
        this.HardwareApplyButton.setEnabled(enabled);
        this.HardwareCancelButton.setEnabled(enabled);
    }
    
    private void setHardwareButtonsVisible(boolean visible) {
        this.HardwareApplyButton.setVisible(visible);
        this.HardwareCancelButton.setVisible(visible);
    }
    
    private void enableAMCServerHost(boolean enabled) {
        this.AMCServerHostText.setEditable(enabled);  
    }
    
    private void enableDelayCompensationHost(boolean enabled) {
        this.DelayCompensationHostText.setEditable(enabled);  
    }
    
    private void enableInputSectionHost(boolean enabled) {
        this.InputSectionClusterHostText.setEditable(enabled);  
    }

    private void enableInputSectionMachines(boolean enabled) {
        this.InputNodeNamesText.setEditable(enabled);  
    }

    private void enableStorageHost(boolean enabled) {
        this.StorageClusterHostText.setEditable(enabled);  
    }

    private void enableStorageMachines(boolean enabled) {
        this.StorageNodeNamesText.setEditable(enabled);  
    }

    private void enableBGLProcPartition(boolean enabled) {
        this.BGLPartitionText.setEditable(enabled);  
    }
    
    private void enableAMCServerPort(boolean enabled) {
        this.AMCServerPortText.setEditable(enabled);  
    }

    private void enableDelayInputPorts(boolean enabled) {
        this.DelayInputPortsText.setEditable(enabled);  
    }

    private void enableInputBGLPorts(boolean enabled) {
        this.InputBGLProcPortsText.setEditable(enabled);  
    }

    private void enableBGLProcStoragePorts(boolean enabled) {
        this.BGLProcStoragePortsText.setEditable(enabled);  
    }

    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
        // Specific
        enableNrSamples(enabled);
        enableNrBufSec(enabled);
        enableUseAMC(enabled);
        enableNrNodesCell(enabled);
        enableNrSubbandsCell(enabled);
        enableNrFilterTabs(enabled);

        // Hardware
        enableAMCServerHost(enabled);
        enableDelayCompensationHost(enabled);
        enableInputSectionHost(enabled);
        enableInputSectionMachines(enabled);
        enableStorageHost(enabled);
        enableStorageMachines(enabled);
        enableBGLProcPartition(enabled);
        enableAMCServerPort(enabled);
        enableDelayInputPorts(enabled);
        enableInputBGLPorts(enabled);
        enableBGLProcStoragePorts(enabled);

        enableSpecificButtons(enabled);
        enableHardwareButtons(enabled);
    }
    
    private void saveInput() {
        // Just check all possible fields that CAN change. The enabled method will take care if they COULD be changed.
        // this way we keep this panel general for multiple use
 
        boolean hasChanged = false;
        
    //    [TODO]
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        jTabbedPane1 = new javax.swing.JTabbedPane();
        OLAPSpecificPanel = new javax.swing.JPanel();
        NrSamplesLabel = new javax.swing.JLabel();
        NrSamplesText = new javax.swing.JTextField();
        NrBufSecLabel = new javax.swing.JLabel();
        NrBufSecText = new javax.swing.JTextField();
        UseAMCLabel = new javax.swing.JLabel();
        UseAMCCheckBox = new javax.swing.JCheckBox();
        NrNodesCellLabel = new javax.swing.JLabel();
        NrNodesCellText = new javax.swing.JTextField();
        NrSubbandsCellLabel = new javax.swing.JLabel();
        NrSubbandsCellText = new javax.swing.JTextField();
        NrFilterTabsLabel = new javax.swing.JLabel();
        NrFilterTabsText = new javax.swing.JTextField();
        SpecificCancelButton = new javax.swing.JButton();
        SpecificApplyButton = new javax.swing.JButton();
        OLAPHardwarePanel = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        AMCServerHostText = new javax.swing.JTextField();
        jLabel2 = new javax.swing.JLabel();
        DelayCompensationHostText = new javax.swing.JTextField();
        jLabel3 = new javax.swing.JLabel();
        InputSectionClusterHostText = new javax.swing.JTextField();
        jLabel4 = new javax.swing.JLabel();
        InputNodeNamesText = new javax.swing.JTextField();
        jLabel5 = new javax.swing.JLabel();
        StorageClusterHostText = new javax.swing.JTextField();
        jLabel11 = new javax.swing.JLabel();
        StorageNodeNamesText = new javax.swing.JTextField();
        jLabel6 = new javax.swing.JLabel();
        BGLPartitionText = new javax.swing.JTextField();
        jLabel7 = new javax.swing.JLabel();
        AMCServerPortText = new javax.swing.JTextField();
        jLabel8 = new javax.swing.JLabel();
        DelayInputPortsText = new javax.swing.JTextField();
        jLabel9 = new javax.swing.JLabel();
        InputBGLProcPortsText = new javax.swing.JTextField();
        jLabel10 = new javax.swing.JLabel();
        BGLProcStoragePortsText = new javax.swing.JTextField();
        HardwareApplyButton = new javax.swing.JButton();
        HardwareCancelButton = new javax.swing.JButton();

        NrSamplesLabel.setText("#Samples :");

        NrSamplesText.setToolTipText("Nr of Samples to integrate");
        NrSamplesText.setMaximumSize(new java.awt.Dimension(440, 19));
        NrSamplesText.setMinimumSize(new java.awt.Dimension(440, 19));
        NrSamplesText.setPreferredSize(new java.awt.Dimension(440, 19));

        NrBufSecLabel.setText("#Seconds to buffer :");

        NrBufSecText.setToolTipText("Number of seconds that need 2 be buffered");
        NrBufSecText.setMaximumSize(new java.awt.Dimension(200, 19));
        NrBufSecText.setMinimumSize(new java.awt.Dimension(200, 19));
        NrBufSecText.setPreferredSize(new java.awt.Dimension(200, 19));

        UseAMCLabel.setText("Use AMC server :");

        UseAMCCheckBox.setToolTipText("Do you want to use an AMC server?");
        UseAMCCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        UseAMCCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));

        NrNodesCellLabel.setText("#Nodes/cell :");

        NrNodesCellText.setToolTipText("Number of Nodes per Cell");
        NrNodesCellText.setMaximumSize(new java.awt.Dimension(200, 19));
        NrNodesCellText.setMinimumSize(new java.awt.Dimension(200, 19));
        NrNodesCellText.setPreferredSize(new java.awt.Dimension(200, 19));

        NrSubbandsCellLabel.setText("#Subbands/cell :");

        NrSubbandsCellText.setToolTipText("Number of Subbands per cell");
        NrSubbandsCellText.setMaximumSize(new java.awt.Dimension(200, 19));
        NrSubbandsCellText.setMinimumSize(new java.awt.Dimension(200, 19));
        NrSubbandsCellText.setPreferredSize(new java.awt.Dimension(200, 19));

        NrFilterTabsLabel.setText("#Filter tabs :");

        NrFilterTabsText.setToolTipText("Number of Filter Tabs for PPFl");

        SpecificCancelButton.setText("Cancel");
        SpecificCancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SpecificCancelButtonActionPerformed(evt);
            }
        });

        SpecificApplyButton.setText("Apply");
        SpecificApplyButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SpecificApplyButtonActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout OLAPSpecificPanelLayout = new org.jdesktop.layout.GroupLayout(OLAPSpecificPanel);
        OLAPSpecificPanel.setLayout(OLAPSpecificPanelLayout);
        OLAPSpecificPanelLayout.setHorizontalGroup(
            OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(OLAPSpecificPanelLayout.createSequentialGroup()
                .addContainerGap()
                .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(OLAPSpecificPanelLayout.createSequentialGroup()
                        .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(UseAMCLabel)
                            .add(NrNodesCellLabel)
                            .add(NrSubbandsCellLabel)
                            .add(NrFilterTabsLabel)
                            .add(NrBufSecLabel)
                            .add(NrSamplesLabel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 80, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                        .add(17, 17, 17)
                        .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(UseAMCCheckBox)
                            .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                                .add(org.jdesktop.layout.GroupLayout.LEADING, NrBufSecText, 0, 0, Short.MAX_VALUE)
                                .add(org.jdesktop.layout.GroupLayout.LEADING, NrSamplesText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 173, Short.MAX_VALUE))
                            .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                                .add(org.jdesktop.layout.GroupLayout.LEADING, NrNodesCellText, 0, 0, Short.MAX_VALUE)
                                .add(org.jdesktop.layout.GroupLayout.LEADING, NrSubbandsCellText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 174, Short.MAX_VALUE)
                                .add(NrFilterTabsText))))
                    .add(OLAPSpecificPanelLayout.createSequentialGroup()
                        .add(SpecificApplyButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 70, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(SpecificCancelButton)))
                .addContainerGap(536, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
        );
        OLAPSpecificPanelLayout.setVerticalGroup(
            OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, OLAPSpecificPanelLayout.createSequentialGroup()
                .addContainerGap()
                .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(NrSamplesLabel)
                    .add(NrSamplesText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 19, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(NrBufSecLabel)
                    .add(NrBufSecText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(UseAMCLabel)
                    .add(UseAMCCheckBox))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(NrNodesCellLabel)
                    .add(NrNodesCellText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(NrSubbandsCellLabel)
                    .add(NrSubbandsCellText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(NrFilterTabsLabel)
                    .add(NrFilterTabsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(46, 46, 46)
                .add(OLAPSpecificPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(SpecificApplyButton)
                    .add(SpecificCancelButton))
                .add(305, 305, 305))
        );
        jTabbedPane1.addTab("OLAP Specific", OLAPSpecificPanel);

        jLabel1.setText("AMCServer Host:");

        AMCServerHostText.setToolTipText("Give Machine where AMC server runs (hostname or IP address)");

        jLabel2.setText("DelayCompensation Host:");

        DelayCompensationHostText.setToolTipText("Machine where DelayCompensation runs (Hostname or IP address)");

        jLabel3.setText("InputSection Cluster Host:");

        InputSectionClusterHostText.setToolTipText("Cluster for InputSection(Hostname or IP address)");

        jLabel4.setText("Input Section Machines:");

        InputNodeNamesText.setToolTipText("comma seperated list with all machinenames for InputSection");

        jLabel5.setText("Storage Cluster Host:");

        StorageClusterHostText.setToolTipText("Cluster for Storage (Hostname or IP address)");

        jLabel11.setText("Storage Machines:");
        jLabel11.setToolTipText("null");

        StorageNodeNamesText.setToolTipText("comma seperated list with all machinenames for Storage");

        jLabel6.setText("Partition for BGL_Processing:");

        BGLPartitionText.setToolTipText("Partition on which BGL_Processing runs.");

        jLabel7.setText("AMC Server Port:");

        AMCServerPortText.setToolTipText("Port to reach AMC Server");

        jLabel8.setText("Delay -> Input Ports:");

        DelayInputPortsText.setToolTipText("Ports to use for Delay -> Input (Range like:  8000-8100 or 8000-  or 8000,8001,8003,8005) ");

        jLabel9.setText("Input -> BGL_Proc Ports:");

        InputBGLProcPortsText.setToolTipText("Ports to use for Input -> BGL_Proc (Range like:  8000-8100 or 8000-  or 8000,8001,8003,8005)");

        jLabel10.setText("BGL_Proc -> Storage Ports:");

        BGLProcStoragePortsText.setToolTipText("Ports to use for BGL_Proc -> Storage (Range like:  8000-8100 or 8000-  or 8000,8001,8003,8005)");

        HardwareApplyButton.setText("Apply");

        HardwareCancelButton.setText("Cancel");

        org.jdesktop.layout.GroupLayout OLAPHardwarePanelLayout = new org.jdesktop.layout.GroupLayout(OLAPHardwarePanel);
        OLAPHardwarePanel.setLayout(OLAPHardwarePanelLayout);
        OLAPHardwarePanelLayout.setHorizontalGroup(
            OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(OLAPHardwarePanelLayout.createSequentialGroup()
                .addContainerGap()
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(OLAPHardwarePanelLayout.createSequentialGroup()
                        .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(jLabel1)
                            .add(jLabel2)
                            .add(jLabel3)
                            .add(jLabel4)
                            .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                                .add(org.jdesktop.layout.GroupLayout.LEADING, jLabel11, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .add(org.jdesktop.layout.GroupLayout.LEADING, jLabel5, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                            .add(jLabel6)
                            .add(jLabel7)
                            .add(jLabel8)
                            .add(jLabel9)
                            .add(jLabel10))
                        .add(13, 13, 13)
                        .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                                .add(StorageNodeNamesText)
                                .add(InputNodeNamesText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 582, Short.MAX_VALUE)
                                .add(StorageClusterHostText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 177, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .add(DelayCompensationHostText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 177, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .add(AMCServerHostText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 177, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .add(InputSectionClusterHostText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 177, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                            .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                                .add(org.jdesktop.layout.GroupLayout.LEADING, BGLProcStoragePortsText)
                                .add(org.jdesktop.layout.GroupLayout.LEADING, InputBGLProcPortsText)
                                .add(org.jdesktop.layout.GroupLayout.LEADING, DelayInputPortsText)
                                .add(org.jdesktop.layout.GroupLayout.LEADING, AMCServerPortText)
                                .add(org.jdesktop.layout.GroupLayout.LEADING, BGLPartitionText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 176, Short.MAX_VALUE))))
                    .add(OLAPHardwarePanelLayout.createSequentialGroup()
                        .add(HardwareApplyButton)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(HardwareCancelButton)))
                .addContainerGap(94, Short.MAX_VALUE))
        );
        OLAPHardwarePanelLayout.setVerticalGroup(
            OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(OLAPHardwarePanelLayout.createSequentialGroup()
                .add(42, 42, 42)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel1)
                    .add(AMCServerHostText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel2)
                    .add(DelayCompensationHostText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel3)
                    .add(InputSectionClusterHostText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel4)
                    .add(InputNodeNamesText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel5)
                    .add(StorageClusterHostText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(8, 8, 8)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel11, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 11, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(StorageNodeNamesText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel6)
                    .add(BGLPartitionText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel7)
                    .add(AMCServerPortText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel8)
                    .add(DelayInputPortsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel9)
                    .add(InputBGLProcPortsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel10)
                    .add(BGLProcStoragePortsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(30, 30, 30)
                .add(OLAPHardwarePanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(HardwareApplyButton)
                    .add(HardwareCancelButton))
                .addContainerGap(86, Short.MAX_VALUE))
        );
        jTabbedPane1.addTab("OLAP Hardware", OLAPHardwarePanel);

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jTabbedPane1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 843, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jTabbedPane1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 488, Short.MAX_VALUE)
        );
    }// </editor-fold>//GEN-END:initComponents

    private void SpecificApplyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SpecificApplyButtonActionPerformed
        saveInput();
    }//GEN-LAST:event_SpecificApplyButtonActionPerformed

    private void SpecificCancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SpecificCancelButtonActionPerformed
        initPanel();
    }//GEN-LAST:event_SpecificCancelButtonActionPerformed
    
    private jOTDBnode itsNode = null;
    private MainFrame  itsMainFrame;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JTextField AMCServerHostText;
    private javax.swing.JTextField AMCServerPortText;
    private javax.swing.JTextField BGLPartitionText;
    private javax.swing.JTextField BGLProcStoragePortsText;
    private javax.swing.JTextField DelayCompensationHostText;
    private javax.swing.JTextField DelayInputPortsText;
    private javax.swing.JButton HardwareApplyButton;
    private javax.swing.JButton HardwareCancelButton;
    private javax.swing.JTextField InputBGLProcPortsText;
    private javax.swing.JTextField InputNodeNamesText;
    private javax.swing.JTextField InputSectionClusterHostText;
    private javax.swing.JLabel NrBufSecLabel;
    private javax.swing.JTextField NrBufSecText;
    private javax.swing.JLabel NrFilterTabsLabel;
    private javax.swing.JTextField NrFilterTabsText;
    private javax.swing.JLabel NrNodesCellLabel;
    private javax.swing.JTextField NrNodesCellText;
    private javax.swing.JLabel NrSamplesLabel;
    private javax.swing.JTextField NrSamplesText;
    private javax.swing.JLabel NrSubbandsCellLabel;
    private javax.swing.JTextField NrSubbandsCellText;
    private javax.swing.JPanel OLAPHardwarePanel;
    private javax.swing.JPanel OLAPSpecificPanel;
    private javax.swing.JButton SpecificApplyButton;
    private javax.swing.JButton SpecificCancelButton;
    private javax.swing.JTextField StorageClusterHostText;
    private javax.swing.JTextField StorageNodeNamesText;
    private javax.swing.JCheckBox UseAMCCheckBox;
    private javax.swing.JLabel UseAMCLabel;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel10;
    private javax.swing.JLabel jLabel11;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JLabel jLabel8;
    private javax.swing.JLabel jLabel9;
    private javax.swing.JTabbedPane jTabbedPane1;
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
