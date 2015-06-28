/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * TBBControlPanel.java
 *
 * Created on 2-aug-2010, 14:41:58
 */

package nl.astron.lofar.sas.otbcomponents;

import java.awt.Component;
import java.rmi.RemoteException;
import java.util.ArrayList;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.ListSelectionModel;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBtree;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.tablemodels.ParamExtensionTableModel;
import org.apache.log4j.Logger;

/**
 *
 * @author coolen
 */
public class TBBControlPanel extends javax.swing.JPanel implements IViewPanel{

    static Logger logger = Logger.getLogger(TBBControlPanel.class);
    static String name = "TBBControlPanel";

    private jOTDBnode            itsNode = null;
    private MainFrame            itsMainFrame;
    private String               itsTreeType="";
    private boolean              editParamExtension=false;
    private int                  itsSelectedRow = -1;
    private ParamExtensionDialog paramExtensionDialog = null;

    // TBBControl parameters
    private jOTDBnode itsNoCoincChann;
    private jOTDBnode itsCoincidenceTime;
    private jOTDBnode itsDoDirectionFit;
    private jOTDBnode itsMinElevation;
    private jOTDBnode itsMaxFitVariance;
    private jOTDBnode itsParamExtension;

    private ParamExtensionTableModel itsParamExtensionTableModel = null;

    /** Creates new form TBBControlPanel */
    public TBBControlPanel() {
        initComponents();
    }

    /**
     * Creates new AntennaConfigPanel instance using a given MainFrame instance and
     * the OTDBnode needed to fill the panel with correct data.
     *
     * @param  aMainFrame   the OTB instance
     * @param  aNode        the node to obtain the BBS Strategy information from
     * (should be the BBS.Strategy node in the component tree)
     */
    /** Creates new form AntennaConfigPanel */
    public TBBControlPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode=aNode;
        initPanel();
    }

    /**
     * Sets the OTB MainFrame instance in this panel.
     * @param aMainFrame the MainFrame instance to associate with
     */
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
        } else {
            logger.debug("No Mainframe supplied");
        }
    }
    /**
     * Returns the friendly name of this panel
     * @return String the friendly name of this panel
     */
    public String getShortName() {
        return name;
    }

     /**
     * This method will attempt to fill this panel with a given jOTDBnode object.
     * <br><br>
     * <b>Important</b>: The jOTDBnode to be passed on should always be the 'BBS.Strategy' node.
     * @param anObject the BBS Strategy jOTDBnode to be displayed in the GUI.
     */
    public void setContent(Object anObject) {

        itsNode=(jOTDBnode)anObject;
        //it is assumed itsNode is the Observation node.
        jOTDBparam aParam=null;
        try {
            //we need to get all the childs from this node.
            // So we get the node itself and look for its childs
            ArrayList<jOTDBnode> TBBnode = new ArrayList(OtdbRmi.getRemoteMaintenance().getItemList(itsNode.treeID(), "%TBBControl",false));

            if (TBBnode.isEmpty() ) {
                logger.error("TBBControl not found, no content");
                return;
            }


            itsNode=TBBnode.get(0);
            ArrayList <jOTDBnode> childs = new ArrayList(OtdbRmi.getRemoteMaintenance().getItemList(itsNode.treeID(), itsNode.nodeID(),1));
            // get all the params per child
            for (jOTDBnode aNode:childs) {
                aParam=null;

                // We need to keep all the nodes needed by this panel
                // if the node is a leaf we need to get the pointed to value via Param.
                if (aNode.leaf) {
                    aParam = OtdbRmi.getRemoteMaintenance().getParam(aNode);
                    setField(itsNode,aParam,aNode);
                }
           }
        } catch (RemoteException ex) {
            String aS="RemoteError during getComponentParam: "+ ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            return;
        }

        initPanel();
        //reset all values
        restore();

    }

    /** saves the given node back to the database
     */
    private void saveNode(jOTDBnode aNode) {
        if (aNode == null) {
            return;
        }
        try {
            OtdbRmi.getRemoteMaintenance().saveNode(aNode);
        } catch (RemoteException ex) {
            String aS="Error: saveNode failed : " + ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
        }
    }

     public boolean saveInput() {
         
        // NoCoincChann
        if (itsNoCoincChann != null && !noCoincChann.getText().equals(itsNoCoincChann.limits)) {
            itsNoCoincChann.limits = noCoincChann.getText();
            saveNode(itsNoCoincChann);
        }

        //CoincidenceTime
        if (itsCoincidenceTime != null && !coincidenceTime.getText().equals(itsCoincidenceTime.limits)) {
            itsCoincidenceTime.limits = coincidenceTime.getText();
            saveNode(itsCoincidenceTime);
        }

        // DoDirectFit
        if (itsDoDirectionFit != null && !doDirectionFit.getSelectedItem().toString().equals(itsDoDirectionFit.limits)) {
            itsDoDirectionFit.limits = doDirectionFit.getSelectedItem().toString();
            saveNode(itsDoDirectionFit);
        }

        // minElevation
        if (itsMinElevation != null && !minElevation.getText().equals(itsMinElevation.limits)) {
            itsMinElevation.limits = minElevation.getText();
            saveNode(itsMinElevation);
        }

        // maxFitVariance
        if (itsMaxFitVariance != null && !maxFitVariance.getText().equals(itsMaxFitVariance.limits)) {
            itsMaxFitVariance.limits = maxFitVariance.getText();
            saveNode(itsMaxFitVariance);
        }

        String keyvals = itsParamExtensionTableModel.getTable();
        if (itsParamExtension != null && !keyvals.equals(itsParamExtension.limits)) {
            itsParamExtension.limits = keyvals;
            saveNode(itsParamExtension);
        }

        // reset all buttons, flags and tables to initial start position. So the panel now reflects the new, saved situation
        initPanel();

        return true;
    }

    /** Restore original Values in  panel
     */
    public void restore() {

      if (itsNoCoincChann != null) {
          noCoincChann.setText(itsNoCoincChann.limits);
      }

      if (itsCoincidenceTime != null) {
          coincidenceTime.setText(itsCoincidenceTime.limits);
      }

      if (itsDoDirectionFit != null) {
          doDirectionFit.setSelectedItem(itsDoDirectionFit.limits);
      }

      if (itsMinElevation != null) {
          minElevation.setText(itsMinElevation.limits);
      }
      
      if (itsMaxFitVariance != null) {
          maxFitVariance.setText(itsMaxFitVariance.limits);
      }

      if (itsParamExtension != null ) {
          itsParamExtensionTableModel.fillTable(itsParamExtension.limits,false);
      }
    }

    private void initialize() {

        itsParamExtensionTableModel = new ParamExtensionTableModel();
        paramExtensionTable.setTableModel(itsParamExtensionTableModel);
        paramExtensionTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        paramExtensionTable.setColumnSize("Key",40);
        paramExtensionTable.setColumnSize("Value",40);

        paramExtensionTable.repaint();
    }

    private void initPanel() {

        itsMainFrame.setHourglassCursor();
        initialize();

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
            try {
                //figure out the caller
                jOTDBtree aTree = OtdbRmi.getRemoteOTDB().getTreeInfo(itsNode.treeID(),false);
                itsTreeType=OtdbRmi.getTreeType().get(aTree.type);
            } catch (RemoteException ex) {
                String aS="TBBControlPanel: Error getting treeInfo/treetype" + ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                itsTreeType="";
            }
         } else {
            logger.debug("no node given");
        }

        // set defaults/initial settings
        restore();



        if (itsTreeType.equals("VHtree")) {
            this.setButtonsVisible(false);
            this.setAllEnabled(false);
        }


        itsMainFrame.setNormalCursor();

    }


    /**
     * This method allows the OTB to know if the BBSStrategyPanel should
     * only have one instance per OTB session. For now this method returns <i>false</i>
     * @return <i>true</i> - if the panel should be loaded only once, <i>false</i> - panel can
     * be instantiated more than once.
     */
    public boolean isSingleton() {
        return false;
    }
    /**
     * This method returns a BBSStrategyPanel instance
     * @return a new BBSStrategyPanel instance
     */
    public JPanel getInstance() {
        return new TBBControlPanel();
    }
    /**
     * This method tells the OTB if the AntennaConfigPanel
     * can show a popup menu object in the OTB.
     * Returns <i>false</i> for now.
     *
     * @return <i>true</i> - if the panel has a popup menu available,
     * <i>false</i> - if the panel does not have a popup menu available.
     */
    public boolean hasPopupMenu() {
        return false;
    }
    /**
     * Creates a popup menu for this panel in the OTB. Not used.
     */
    public void createPopupMenu(Component aComponent,int x, int y) {
    }
    /**
     * Handles the choice from the popupmenu in the OTB. Not used.
     */
    public void popupMenuHandler(java.awt.event.ActionEvent evt) {
    }

    /** Enables/disables the buttons (unused)
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
    }

    /** Sets the buttons visible/invisible (unused)
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
    }
    /** Enables/disables the complete form (unused)
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
    }

   /**
     * Sets the different fields in the GUI, using the names of the nodes provided
     * @param parent the parent node of the node to be displayed
     * @param aParam the parameter of the node to be displayed if applicable
     * @param aNode  the node to be displayed
     */
    private void setField(jOTDBnode parent,jOTDBparam aParam, jOTDBnode aNode) {

        // TBBControl
        if (aParam==null) {
            return;
        }
        boolean isRef = LofarUtils.isReference(aNode.limits);
        String aKeyName = LofarUtils.keyName(aNode.name);
        String parentName = LofarUtils.keyName(String.valueOf(parent.name));
        /* Set's the different fields in the GUI */

        logger.debug("setField for: "+ aNode.name);
        try {
            if (OtdbRmi.getRemoteTypes().getParamType(aParam.type).substring(0,1).equals("p")) {
               // Have to get new param because we need the unresolved limits field.
               aParam = OtdbRmi.getRemoteMaintenance().getParam(aNode.treeID(),aNode.paramDefID());
            }
        } catch (RemoteException ex) {
            String aS="Error during getParam: "+ ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            return;
        }

        if(parentName.equals("TBBControl")){
            // TBBControl parameters
            switch (aKeyName) {
                case "NoCoincChann":
                    noCoincChann.setToolTipText(aParam.description);
                    itsNoCoincChann=aNode;
                    break;
                case "CoincidenceTime":
                    coincidenceTime.setToolTipText(aParam.description);
                    itsCoincidenceTime=aNode;
                    break;
                case "DoDirectionFit":
                    doDirectionFit.setToolTipText(aParam.description);
                    LofarUtils.setPopupComboChoices(doDirectionFit,aParam.limits);
                    itsDoDirectionFit=aNode;
                    if (!aNode.limits.equals("")) {
                        doDirectionFit.setSelectedItem(aNode.limits);
                    }
                    break;
                case "MinElevation":
                    minElevation.setToolTipText(aParam.description);
                    itsMinElevation=aNode;
                    break;
                case "MaxFitVariance":
                    maxFitVariance.setToolTipText(aParam.description);
                    itsMaxFitVariance=aNode;
                    break;
                case "ParamExtension":
                    paramExtensionTable.setToolTipText(aParam.description);
                    itsParamExtension=aNode;
                    break;
            }
        }
    }

    private void addParamExtension() {

        itsSelectedRow=-1;
        // set selection to defaults.
        String [] selection = {"",""};
        if (editParamExtension) {
            itsSelectedRow = paramExtensionTable.getSelectedRow();
            selection = itsParamExtensionTableModel.getSelection(itsSelectedRow);

            // if no row is selected, nothing to be done
            if (selection == null || selection[0].equals("")) {
                return;
            }
        }
        paramExtensionDialog = new ParamExtensionDialog(itsMainFrame,true,selection);
        paramExtensionDialog.setLocationRelativeTo(this);
        if (editParamExtension) {
            paramExtensionDialog.setBorderTitle("edit Parameter Extension");
        } else {
            paramExtensionDialog.setBorderTitle("add new Parameter Extension");
        }
        paramExtensionDialog.setVisible(true);

        // check if something has changed
        if (paramExtensionDialog.hasChanged()) {
            String[] newRow = paramExtensionDialog.getKeyVal();
            // check if we are editting an entry or adding a new entry
            if (editParamExtension) {
                itsParamExtensionTableModel.updateRow(newRow,itsSelectedRow);
                // set editting = false
                editParamExtension=false;
            } else {
                itsParamExtensionTableModel.addRow(newRow[0],newRow[1]);
            }
        }

        this.editKeyValButton.setEnabled(false);
        this.deleteKeyValButton.setEnabled(false);
        if (paramExtensionTable.getTableModel().getRowCount() == 8 ) {
            this.addKeyValButton.setEnabled(false);
        } else {
            this.addKeyValButton.setEnabled(true);
        }

    }

    private void deleteParamExtension() {
        int row = paramExtensionTable.getSelectedRow();

        if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this ParameterExtension ?","Delete Parameter Extension",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
            if (row > -1) {
                itsParamExtensionTableModel.removeRow(row);
                // No selection anymore after delete, so buttons disabled again
                this.editKeyValButton.setEnabled(false);
                this.deleteKeyValButton.setEnabled(false);


            }
        }

      if (paramExtensionTable.getTableModel().getRowCount() == 8) {
        this.editKeyValButton.setEnabled(false);
      } else {
        this.editKeyValButton.setEnabled(true);
      }
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        noCoincChann = new javax.swing.JTextField();
        jLabel1 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        coincidenceTime = new javax.swing.JTextField();
        jLabel3 = new javax.swing.JLabel();
        doDirectionFit = new javax.swing.JComboBox();
        jLabel4 = new javax.swing.JLabel();
        minElevation = new javax.swing.JTextField();
        jLabel5 = new javax.swing.JLabel();
        maxFitVariance = new javax.swing.JTextField();
        paramExtensionTable = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        addKeyValButton = new javax.swing.JButton();
        editKeyValButton = new javax.swing.JButton();
        deleteKeyValButton = new javax.swing.JButton();
        jLabel6 = new javax.swing.JLabel();

        jLabel1.setText("Nr of  Coincidence channels: ");

        jLabel2.setText("Coincidence Time:");

        jLabel3.setText("Do Direction Fit:");

        doDirectionFit.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));

        jLabel4.setText("Minimal Elevation:");

        jLabel5.setText("Maximum fit Variance");

        paramExtensionTable.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                paramExtensionTableMouseClicked(evt);
            }
        });

        addKeyValButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_add.gif"))); // NOI18N
        addKeyValButton.setText("Add KeyVal");
        addKeyValButton.setToolTipText("Add new Key value pair");
        addKeyValButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addKeyValButtonActionPerformed(evt);
            }
        });

        editKeyValButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_edit.gif"))); // NOI18N
        editKeyValButton.setText("Edit KeyVal");
        editKeyValButton.setToolTipText("edit the selected key value pair");
        editKeyValButton.setEnabled(false);
        editKeyValButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                editKeyValButtonActionPerformed(evt);
            }
        });

        deleteKeyValButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_delete.png"))); // NOI18N
        deleteKeyValButton.setText("Delete Keyval");
        deleteKeyValButton.setToolTipText("delete the selected key value pair");
        deleteKeyValButton.setEnabled(false);
        deleteKeyValButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                deleteKeyValButtonActionPerformed(evt);
            }
        });

        jLabel6.setText("Param Extension");

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(layout.createSequentialGroup()
                        .addComponent(paramExtensionTable, javax.swing.GroupLayout.PREFERRED_SIZE, 616, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addContainerGap())
                    .addGroup(layout.createSequentialGroup()
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                                .addComponent(jLabel2, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addComponent(jLabel1, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addComponent(jLabel3, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                            .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                                .addComponent(jLabel5, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addComponent(jLabel4, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 128, Short.MAX_VALUE)))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 31, Short.MAX_VALUE)
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(maxFitVariance, javax.swing.GroupLayout.PREFERRED_SIZE, 150, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(doDirectionFit, javax.swing.GroupLayout.PREFERRED_SIZE, 150, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(coincidenceTime, javax.swing.GroupLayout.PREFERRED_SIZE, 150, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(noCoincChann, javax.swing.GroupLayout.PREFERRED_SIZE, 149, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(minElevation, javax.swing.GroupLayout.PREFERRED_SIZE, 150, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGap(793, 793, 793))
                    .addGroup(layout.createSequentialGroup()
                        .addComponent(addKeyValButton)
                        .addGap(30, 30, 30)
                        .addComponent(editKeyValButton)
                        .addGap(18, 18, 18)
                        .addComponent(deleteKeyValButton)
                        .addContainerGap(734, Short.MAX_VALUE))
                    .addGroup(layout.createSequentialGroup()
                        .addComponent(jLabel6)
                        .addContainerGap(1033, Short.MAX_VALUE))))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addGap(27, 27, 27)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel1)
                    .addComponent(noCoincChann, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(18, 18, 18)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel2)
                    .addComponent(coincidenceTime, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(18, 18, 18)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel3)
                    .addComponent(doDirectionFit, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(18, 18, 18)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel4)
                    .addComponent(minElevation, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(18, 18, 18)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel5)
                    .addComponent(maxFitVariance, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(21, 21, 21)
                .addComponent(jLabel6)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(paramExtensionTable, javax.swing.GroupLayout.PREFERRED_SIZE, 334, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(18, 18, 18)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(addKeyValButton)
                    .addComponent(editKeyValButton)
                    .addComponent(deleteKeyValButton))
                .addGap(135, 135, 135))
        );
    }// </editor-fold>//GEN-END:initComponents

    private void addKeyValButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addKeyValButtonActionPerformed
        editParamExtension=false;
        addParamExtension();
    }//GEN-LAST:event_addKeyValButtonActionPerformed

    private void deleteKeyValButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteKeyValButtonActionPerformed
        deleteParamExtension();
    }//GEN-LAST:event_deleteKeyValButtonActionPerformed

    private void editKeyValButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_editKeyValButtonActionPerformed
        editParamExtension=true;
        addParamExtension(); 
    }//GEN-LAST:event_editKeyValButtonActionPerformed

    private void paramExtensionTableMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_paramExtensionTableMouseClicked
        editKeyValButton.setEnabled(true);
        deleteKeyValButton.setEnabled(true);
    }//GEN-LAST:event_paramExtensionTableMouseClicked


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton addKeyValButton;
    private javax.swing.JTextField coincidenceTime;
    private javax.swing.JButton deleteKeyValButton;
    private javax.swing.JComboBox doDirectionFit;
    private javax.swing.JButton editKeyValButton;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JTextField maxFitVariance;
    private javax.swing.JTextField minElevation;
    private javax.swing.JTextField noCoincChann;
    private nl.astron.lofar.sas.otbcomponents.TablePanel paramExtensionTable;
    // End of variables declaration//GEN-END:variables

    /**
     * Utility field used by event firing mechanism.
     */
    private javax.swing.event.EventListenerList myListenerList =  null;

    /**
     * Registers ActionListener to receive events.
     * @param listener The listener to register.
     */
    public synchronized void addActionListener(java.awt.event.ActionListener listener) {

        if (myListenerList == null ) {
            myListenerList = new javax.swing.event.EventListenerList();
        }
        myListenerList.add(java.awt.event.ActionListener.class, listener);
    }

    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeActionListener(java.awt.event.ActionListener listener) {

        myListenerList.remove(java.awt.event.ActionListener.class, listener);
    }

    /**
     * Notifies all registered listeners about the event.
     *
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {

        if (myListenerList == null) return;
        Object[] listeners = myListenerList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed(event);
            }
        }
    }
}
