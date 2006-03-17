/*
TreeInfoDialog.java
 *
 * Created on 26 januari 2006, 11:33
 */

package nl.astron.lofar.sas.otbcomponents;
import java.util.Iterator;
import java.util.TreeMap;
import javax.swing.DefaultComboBoxModel;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import org.apache.log4j.Logger;


/**
 *
 * @author  coolen
 */
public class TreeInfoDialog extends javax.swing.JDialog {
    static Logger logger = Logger.getLogger(TreeInfoDialog.class);
    static String name = "TreeInfoDialog";
    
    /** Creates new form BeanForm
     * 
     * @param   parent      Frame where this dialog belongs
     * @param   modal       Should the Dialog be modal or not
     * @param   aFocus      PIC/VIC/Templates/Components
     */
    public TreeInfoDialog(java.awt.Frame parent, boolean modal, jOTDBtree aTree, MainFrame aMainFrame ) {
        super(parent, modal);
        initComponents();
        itsMainFrame = aMainFrame;
        itsTree = aTree;
        itsOtdbRmi=itsMainFrame.getOTDBrmi();
        isAdministrator=itsMainFrame.getUserAccount().isAdministrator();
        initComboLists();
        initView();
        initFocus();
        getRootPane().setDefaultButton(saveButton);
        
    }

     /**
     * Getter for property itsTree.
     * @return Value of property itsTree.
     */
    public jOTDBtree getTree() {
        return this.itsTree;
    }

    /**
     * Getter for property hasChanged.
     * @return Value of hasChanged.
     */
    public boolean isChanged() {
        return hasChanged;
    }
    
    private void initFocus() {
        
        String aTreeType=itsOtdbRmi.getTreeType().get(itsTree.type);
            
        // PIC
        if (aTreeType.equals("hardware")) {
            momIDLabel.setVisible(false);
            momIDInput.setVisible(false);
            originalTreeIDLabel.setVisible(false);
            originalTreeIDInput.setVisible(false);
            campaignLabel.setVisible(false);
            campaignInput.setVisible(false);
            startTimeLabel.setVisible(false);
            startTimeInput.setVisible(false);
            stopTimeLabel.setVisible(false);
            stopTimeInput.setVisible(false);
            // VICtemplate    
        } else if (aTreeType.equals("VItemplate")) {
            startTimeLabel.setVisible(false);
            startTimeInput.setVisible(false);
            stopTimeLabel.setVisible(false);
            stopTimeInput.setVisible(false);
            
        // VIC
        } else if (aTreeType.equals("VHtree")) {
            
        }
        if (isAdministrator) {
            classificationInput.setEnabled(true);
        }
    }
    
    private void initComboLists() {
        DefaultComboBoxModel aClassifModel = new DefaultComboBoxModel();
        TreeMap aClassifMap=itsOtdbRmi.getClassif();
        Iterator classifIt = aClassifMap.keySet().iterator();
        while (classifIt.hasNext()) {
            aClassifModel.addElement((String)aClassifMap.get(classifIt.next()));
        }
        classificationInput.setModel(aClassifModel);
        
        DefaultComboBoxModel aStateModel = new DefaultComboBoxModel();
        TreeMap aStateMap=itsOtdbRmi.getTreeState();
        Iterator stateIt = aStateMap.keySet().iterator();
        while (stateIt.hasNext()) {
            aStateModel.addElement((String)aStateMap.get(stateIt.next()));
        }
        stateInput.setModel(aStateModel);
    }
    
    /* Fill the view */
    private void initView() {
        // prepare a active/obsolete only combobox for special case state changes
        DefaultComboBoxModel aModel = (DefaultComboBoxModel)stateInput.getModel();
        aModel.removeAllElements();
        aModel.addElement("active");
        aModel.addElement("obsolete");
        
        // keep the fields that can be changed
        itsTreeState=itsOtdbRmi.getTreeState().get(itsTree.state);
        itsClassification = itsOtdbRmi.getClassif().get(itsTree.classification);
        itsDescription = itsTree.description;     
        if (!itsTreeState.equals("active") && !itsTreeState.equals("obsolete")) {
            aModel.addElement(itsTreeState);
        }
        
        String aTreeType=itsOtdbRmi.getTreeType().get(itsTree.type);        if (aTreeType.equals("hardware") || aTreeType.equals("VItemplate")) {
            stateInput.setModel(aModel);
        }

        treeIDInput.setText(String.valueOf(itsTree.treeID()));
        momIDInput.setText(String.valueOf(itsTree.momID()));
        classificationInput.setSelectedItem(itsTreeState);
        creatorInput.setText(itsTree.creator);
        creationDateInput.setText(itsTree.creationDate);
        typeInput.setText((String)itsOtdbRmi.getTreeType().get(itsTree.type));
        stateInput.setSelectedItem(itsTreeState);
        originalTreeIDInput.setText(String.valueOf(itsTree.originalTree));
        campaignInput.setText(itsTree.campaign);
        startTimeInput.setText(itsTree.starttime);
        stopTimeInput.setText(itsTree.stoptime);
        descriptionInput.setText(itsDescription);
    }
    
    private void setNewTree() {
        try {
            if ( !itsClassification.equals(classificationInput.getSelectedItem().toString())) {
                hasChanged=true;
                itsTree.classification=itsOtdbRmi.getRemoteTypes().getClassif(classificationInput.getSelectedItem().toString());
            }
            if (!itsTreeState.equals(stateInput.getSelectedItem().toString())) {
                hasChanged=true;
                itsTree.state=itsOtdbRmi.getRemoteTypes().getTreeState(stateInput.getSelectedItem().toString());
            }
            if (!itsDescription.equals(descriptionInput.getText())) {
                hasChanged=true;
                itsTree.description = descriptionInput.getText();
            }
        } catch (Exception e) {
          logger.debug("Getting ClassificationType via RMI and JNI failed");  
        }
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        jLabel2 = new javax.swing.JLabel();
        stateInput = new javax.swing.JComboBox();
        jLabel3 = new javax.swing.JLabel();
        descriptionInput = new javax.swing.JTextArea();
        cancelButton = new javax.swing.JButton();
        saveButton = new javax.swing.JButton();
        topLabel = new javax.swing.JLabel();
        jLabel1 = new javax.swing.JLabel();
        treeIDInput = new javax.swing.JTextField();
        jLabel4 = new javax.swing.JLabel();
        classificationInput = new javax.swing.JComboBox();
        jLabel6 = new javax.swing.JLabel();
        creatorInput = new javax.swing.JTextField();
        startTimeLabel = new javax.swing.JLabel();
        stopTimeLabel = new javax.swing.JLabel();
        startTimeInput = new javax.swing.JTextField();
        stopTimeInput = new javax.swing.JTextField();
        momIDLabel = new javax.swing.JLabel();
        momIDInput = new javax.swing.JTextField();
        jLabel10 = new javax.swing.JLabel();
        creationDateInput = new javax.swing.JTextField();
        jLabel11 = new javax.swing.JLabel();
        typeInput = new javax.swing.JTextField();
        originalTreeIDLabel = new javax.swing.JLabel();
        originalTreeIDInput = new javax.swing.JTextField();
        campaignLabel = new javax.swing.JLabel();
        campaignInput = new javax.swing.JTextField();

        getContentPane().setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("LOFAR View TreeInfo");
        setAlwaysOnTop(true);
        setModal(true);
        setName("loadFileDialog");
        setResizable(false);
        jLabel2.setText("State:");
        getContentPane().add(jLabel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 230, -1, 20));

        stateInput.setToolTipText("State Selection");
        getContentPane().add(stateInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 230, 170, -1));

        jLabel3.setText("Description :");
        getContentPane().add(jLabel3, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 390, 130, 20));

        descriptionInput.setRows(3);
        descriptionInput.setToolTipText("Set Description for this tree");
        getContentPane().add(descriptionInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 410, 420, 60));

        cancelButton.setText("Cancel");
        cancelButton.setToolTipText("Cancel filesearch");
        cancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                cancelButtonActionPerformed(evt);
            }
        });

        getContentPane().add(cancelButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 490, -1, -1));

        saveButton.setText("Save");
        saveButton.setToolTipText("Save changes");
        saveButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                saveButtonActionPerformed(evt);
            }
        });

        getContentPane().add(saveButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(90, 490, -1, -1));

        topLabel.setFont(new java.awt.Font("Tahoma", 1, 11));
        topLabel.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        topLabel.setText("tree information");
        getContentPane().add(topLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 570, -1));

        jLabel1.setText("ID:");
        getContentPane().add(jLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 50, -1, 20));

        treeIDInput.setToolTipText("Tree ID in database");
        treeIDInput.setEnabled(false);
        getContentPane().add(treeIDInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 50, 90, 20));

        jLabel4.setText("Classification:");
        getContentPane().add(jLabel4, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 110, -1, 20));

        classificationInput.setToolTipText("Select Classification");
        classificationInput.setEnabled(false);
        getContentPane().add(classificationInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 110, 170, -1));

        jLabel6.setText("Creator:");
        getContentPane().add(jLabel6, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 140, -1, 20));

        creatorInput.setToolTipText("Owner for this TreeNode");
        creatorInput.setEnabled(false);
        getContentPane().add(creatorInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 140, 330, -1));

        startTimeLabel.setText("StartTime:");
        getContentPane().add(startTimeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 320, -1, 20));

        stopTimeLabel.setText("StopTime:");
        getContentPane().add(stopTimeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 350, -1, 20));

        startTimeInput.setToolTipText("Start Time");
        getContentPane().add(startTimeInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 320, 330, -1));

        stopTimeInput.setToolTipText("Stop Time");
        getContentPane().add(stopTimeInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 350, 330, -1));

        momIDLabel.setText("MoMID:");
        getContentPane().add(momIDLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 80, -1, 20));

        momIDInput.setToolTipText("MoMID");
        momIDInput.setEnabled(false);
        getContentPane().add(momIDInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 80, 90, -1));

        jLabel10.setText("CreationDate:");
        getContentPane().add(jLabel10, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 170, -1, 20));

        creationDateInput.setToolTipText("Date this entry was created");
        creationDateInput.setEnabled(false);
        getContentPane().add(creationDateInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 170, 330, -1));

        jLabel11.setText("Type:");
        getContentPane().add(jLabel11, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 200, -1, 20));

        typeInput.setEnabled(false);
        getContentPane().add(typeInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 200, 330, -1));

        originalTreeIDLabel.setText("OriginalTree:");
        getContentPane().add(originalTreeIDLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 260, -1, 20));

        originalTreeIDInput.setToolTipText("Original Tree ID");
        originalTreeIDInput.setEnabled(false);
        getContentPane().add(originalTreeIDInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 260, 90, -1));

        campaignLabel.setText("Campaign:");
        getContentPane().add(campaignLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 290, -1, 20));

        campaignInput.setToolTipText("Campaign name");
        getContentPane().add(campaignInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 290, 330, -1));

        pack();
    }
    // </editor-fold>//GEN-END:initComponents

    private void saveButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_saveButtonActionPerformed
        setNewTree();
        setVisible(false);
        dispose();
    }//GEN-LAST:event_saveButtonActionPerformed

    private void cancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_cancelButtonActionPerformed
        setVisible(false);
        dispose();
    }//GEN-LAST:event_cancelButtonActionPerformed


    private MainFrame itsMainFrame;
    private OtdbRmi   itsOtdbRmi;
    private jOTDBtree itsTree;
    private boolean   isAdministrator;
    private boolean   hasChanged=false;
    private String    itsClassification;
    private String    itsTreeState;
    private String    itsDescription;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JTextField campaignInput;
    private javax.swing.JLabel campaignLabel;
    private javax.swing.JButton cancelButton;
    private javax.swing.JComboBox classificationInput;
    private javax.swing.JTextField creationDateInput;
    private javax.swing.JTextField creatorInput;
    private javax.swing.JTextArea descriptionInput;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel10;
    private javax.swing.JLabel jLabel11;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JTextField momIDInput;
    private javax.swing.JLabel momIDLabel;
    private javax.swing.JTextField originalTreeIDInput;
    private javax.swing.JLabel originalTreeIDLabel;
    private javax.swing.JButton saveButton;
    private javax.swing.JTextField startTimeInput;
    private javax.swing.JLabel startTimeLabel;
    private javax.swing.JComboBox stateInput;
    private javax.swing.JTextField stopTimeInput;
    private javax.swing.JLabel stopTimeLabel;
    private javax.swing.JLabel topLabel;
    private javax.swing.JTextField treeIDInput;
    private javax.swing.JTextField typeInput;
    // End of variables declaration//GEN-END:variables

}
