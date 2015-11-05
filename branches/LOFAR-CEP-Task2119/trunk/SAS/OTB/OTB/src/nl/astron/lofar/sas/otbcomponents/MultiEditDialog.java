/* TreeInfoDialog.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

package nl.astron.lofar.sas.otbcomponents;
import java.rmi.RemoteException;
import java.util.ArrayList;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBtree;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import org.apache.log4j.Logger;


/**
 *
 * @created 24-03-2006, 12:34
 *
 * @author  coolen
 *
 * @version $Id: TreeInfoDialog.java 16420 2010-09-28 15:05:01Z coolen $
 */
public class MultiEditDialog extends javax.swing.JDialog {
    static Logger logger = Logger.getLogger(MultiEditDialog.class);
    static String name = "TreeInfoDialog";
    
    /** Creates new form BeanForm
     * 
     * @param   parent      Frame where this dialog belongs
     * @param   modal       Should the Dialog be modal or not
     * @param   aTree       the Tree we work with
     * @param   aMainFrame  the Mainframe we are part off
     */
    public MultiEditDialog(boolean modal, int[] treeIDs , MainFrame aMainFrame) {
        super(aMainFrame, modal);
        initComponents();
        itsMainFrame = aMainFrame;
        setTree(treeIDs);
    }
    
    public final void setTree(int [] treeIDs) {
        itsTreeIDs = treeIDs;
        topLabel.setText("-- MULTIPLE TREE SELECTION -- Only first Tree's info is shown \n" +
                         "Changes will be applied to all involved trees");

        try {
            // set selected Tree to first in the list
            itsTree=OtdbRmi.getRemoteOTDB().getTreeInfo(itsTreeIDs[0], false);
        } catch (RemoteException e) {
            String aS="Error getting the Treeinfo " + e;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            itsTree=null;
        }
        init();                
    }


    /*
     * Collect all changeable information based on the first treeID
     */
    private void init() {

        isInitialized = false;

        coreStationSelectionPanel.setTitle("Core");
        coreStationSelectionPanel.init();
        coreStationSelectionPanel.setEnabled(false);
        remoteStationSelectionPanel.setTitle("Remote");
        remoteStationSelectionPanel.init();
        remoteStationSelectionPanel.setEnabled(false);
        europeStationSelectionPanel.setTitle("Europe");
        europeStationSelectionPanel.init();
        europeStationSelectionPanel.setEnabled(false);
        statusLabel.setText("");

        applyStationList.setSelected(false);

        isAdministrator = itsMainFrame.getUserAccount().isAdministrator();
        jOTDBparam aParam=null;
        try {
            if (itsTree != null) {

                //We are looking for:
                // VirtualInstrument.stationList
                ArrayList<jOTDBnode> node = new ArrayList(OtdbRmi.getRemoteMaintenance().getItemList(itsTreeIDs[0], "VirtualInstrument"));
                if (node.size() > 0) {
                    
                    jOTDBnode aNode = node.get(0);
                    ArrayList<jOTDBnode> childs = new ArrayList(OtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1));
                    // get all the params per child
                    for (jOTDBnode anotherNode: childs) {
                        aParam = null;
                        // We need to keep all the params needed by this panel
                        if (anotherNode.leaf) {
                            aParam = OtdbRmi.getRemoteMaintenance().getParam(anotherNode);
                        }
                        setField("VirtualInstrument", aParam, anotherNode);
                    }
                }

                restore();

                isInitialized = true;
                getRootPane().setDefaultButton(saveButton);
            } else {
                logger.debug("No tree found to work with");
            }
        } catch (RemoteException ex) {
            String aS = "Remote Exception during init: " + ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this, aS, new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            return;
        }

    }

    /**
     * Sets the different fields in the GUI, using the names of the nodes provided
     * @param parent the parent node of the node to be displayed
     * @param aParam the parameter of the node to be displayed if applicable
     * @param aNode  the node to be displayed
     */
    private void setField(String parentName,jOTDBparam aParam, jOTDBnode aNode) {
        // Generic Observation
        if (aParam==null) {
            return;
        }
        boolean isRef = LofarUtils.isReference(aNode.limits);
        String aKeyName = LofarUtils.keyName(aNode.name);

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
        }

        if(parentName.equals("VirtualInstrument")){
            // Observation VirtualInstrument parameters

            if (aKeyName.equals("stationList")) {
                this.coreStationSelectionPanel.setToolTipText(aParam.description);
                this.remoteStationSelectionPanel.setToolTipText(aParam.description);
                this.europeStationSelectionPanel.setToolTipText(aParam.description);
                this.itsStationList = aNode;
                setStationLists(aNode.limits);
            }
        }
    }

    /**
     * gets the fields that possibly need to be changed with new values
     * @param parent the parent node of the node to be displayed
     * @param aParam the parameter of the node to be displayed if applicable
     * @param aNode  the node to be displayed
     */
    private void getAndSaveField(String parentName,jOTDBparam aParam, jOTDBnode aNode) {
        // Generic Observation
        if (aParam==null) {
            return;
        }
        boolean isRef = LofarUtils.isReference(aNode.limits);
        String aKeyName = LofarUtils.keyName(aNode.name);

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
        }

        if(parentName.equals("VirtualInstrument")){
            // Observation VirtualInstrument parameters

            if (aKeyName.equals("stationList") && setStationList) {
                aNode.limits=getUsedStations();
                saveNode(aNode);
            }
        }
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
    
    /** returns a [a,b,c] string that contain all used stations
     *
     * @return  a List with all used stations
     */
    public String getUsedStations() {
        this.itsUsedCoreStations = this.coreStationSelectionPanel.getUsedStationList();
        this.itsUsedRemoteStations = this.remoteStationSelectionPanel.getUsedStationList();
        this.itsUsedEuropeStations = this.europeStationSelectionPanel.getUsedStationList();
        String aS= "[";
        boolean first=true;
        for (int i=0; i< itsUsedCoreStations.size();i++) {
            if (first) {
                first=false;
                aS+=itsUsedCoreStations.get(i);
            } else {
                aS+=","+itsUsedCoreStations.get(i);
            }
        }
        for (int i=0; i< itsUsedRemoteStations.size();i++) {
            if (first) {
                first=false;
                aS+=itsUsedRemoteStations.get(i);
            } else {
                aS+=","+itsUsedRemoteStations.get(i);
            }
        }
        for (int i=0; i< itsUsedEuropeStations.size();i++) {
            if (first) {
                first=false;
                aS+=itsUsedEuropeStations.get(i);
            } else {
                aS+=","+itsUsedEuropeStations.get(i);
            }
        }

        aS+="]";
        return aS;
    }

    private void setStationLists(String stations) {
        itsUsedCoreStations.clear();
        itsUsedRemoteStations.clear();
        itsUsedEuropeStations.clear();

        if (stations.startsWith("[")) {
           stations = stations.substring(1, stations.length());
        }
        if (stations.endsWith("]")) {
            stations = stations.substring(0, stations.length() - 1);
        }
        if (!stations.equals("")) {
            String[] aS = stations.split("\\,");
            for (int i = 0; i < aS.length; i++) {
                switch (aS[i].substring(0,2)) {
                    case "CS":
                    itsUsedCoreStations.add(aS[i]);
                        break;
                    case "RS":
                    itsUsedRemoteStations.add(aS[i]);
                        break;
                    default:
                    itsUsedEuropeStations.add(aS[i]);
                        break;
                }
            }
            this.coreStationSelectionPanel.setUsedStationList(itsUsedCoreStations);
            this.remoteStationSelectionPanel.setUsedStationList(itsUsedRemoteStations);
            this.europeStationSelectionPanel.setUsedStationList(itsUsedEuropeStations);
        }
    }
   

    /** saves the given node back to the database
     */
    private void saveNode(jOTDBnode aNode) {
        if (aNode == null) {
            return;
        }
        try {
            statusLabel.setText("Saving tree("+aNode.treeID()+")."+aNode.name);
            statusLabel.update(statusLabel.getGraphics());
            OtdbRmi.getRemoteMaintenance().saveNode(aNode);
        } catch (RemoteException ex) {
            String aS="Error: saveNode failed : " + ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
        }
    }

    /*
     * Save all changed inputfields in the different trees
     */
    private boolean saveNewTree() {

        // make sure that if a VICtree is selected we check the start-end time first. If they are not correct, pop up a dialog.
        
        // determine what fields need to be gathered and rewritten.
        // only the ones that have actually been changed should be written


        // Virtual Instrument StationList
        if (this.itsStationList != null && setStationList) {
            itsStationList.limits = getUsedStations();
            saveNode(itsStationList);
        }






        try {
            //  changes should be set to all the selected trees minus the first one, that one has allready been done above
            // the nodes only need to be collected if the data actually was changed.
            for (int i = 1; i < itsTreeIDs.length; i++) {

                if (setStationList ) {
                    collectAndSaveVirtualInstrumentNodes(itsTreeIDs[i]);
                }

            }
            return true;
        } catch (Exception e) {
            String aS = "Exception changing metainfo via RMI and JNI failed";
            logger.error(aS);
            LofarUtils.showErrorPanel(this, aS, new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            return false;
        }
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        cancelButton = new javax.swing.JButton();
        saveButton = new javax.swing.JButton();
        jScrollPane1 = new javax.swing.JScrollPane();
        topLabel = new javax.swing.JTextArea();
        coreStationSelectionPanel = new nl.astron.lofar.sas.otbcomponents.StationSelectionPanel();
        remoteStationSelectionPanel = new nl.astron.lofar.sas.otbcomponents.StationSelectionPanel();
        europeStationSelectionPanel = new nl.astron.lofar.sas.otbcomponents.StationSelectionPanel();
        restoreButton = new javax.swing.JButton();
        statusLabel = new javax.swing.JLabel();
        applyStationList = new javax.swing.JCheckBox();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("LOFAR View TreeInfo");
        setAlwaysOnTop(true);
        setModal(true);
        setName("loadFileDialog"); // NOI18N
        setResizable(false);

        cancelButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_cancel.png"))); // NOI18N
        cancelButton.setText("Cancel");
        cancelButton.setToolTipText("Cancel filesearch");
        cancelButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        cancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                cancelButtonActionPerformed(evt);
            }
        });

        saveButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_apply.png"))); // NOI18N
        saveButton.setText("Apply");
        saveButton.setToolTipText("Apply changes");
        saveButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        saveButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                saveButtonActionPerformed(evt);
            }
        });

        jScrollPane1.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        jScrollPane1.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_NEVER);

        topLabel.setColumns(20);
        topLabel.setEditable(false);
        topLabel.setFont(new java.awt.Font("Tahoma", 1, 11));
        topLabel.setRows(2);
        topLabel.setText("The info below is from the first of the chosen trees. \nIf changed however, it will be applied to all selected trees.");
        topLabel.setOpaque(false);
        jScrollPane1.setViewportView(topLabel);

        coreStationSelectionPanel.setEnabled(false);

        remoteStationSelectionPanel.setEnabled(false);

        europeStationSelectionPanel.setEnabled(false);

        restoreButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_undo.png"))); // NOI18N
        restoreButton.setText("Restore");
        restoreButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                restoreButtonMouseClicked(evt);
            }
        });

        statusLabel.setText("Ready");
        statusLabel.setOpaque(true);

        applyStationList.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                applyStationListActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(235, 235, 235)
                        .add(jScrollPane1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 343, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(layout.createSequentialGroup()
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                            .add(layout.createSequentialGroup()
                                .add(14, 14, 14)
                                .add(restoreButton, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 161, Short.MAX_VALUE)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(cancelButton)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(saveButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 90, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(statusLabel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 575, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                            .add(org.jdesktop.layout.GroupLayout.LEADING, layout.createSequentialGroup()
                                .addContainerGap()
                                .add(applyStationList)
                                .add(18, 18, 18)
                                .add(coreStationSelectionPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(remoteStationSelectionPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(europeStationSelectionPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                        .add(53, 53, 53)))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(jScrollPane1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 40, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                            .add(coreStationSelectionPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(remoteStationSelectionPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(europeStationSelectionPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                    .add(layout.createSequentialGroup()
                        .add(131, 131, 131)
                        .add(applyStationList)))
                .add(27, 27, 27)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                    .add(restoreButton, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(cancelButton, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                        .add(saveButton, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .add(statusLabel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                .addContainerGap(23, Short.MAX_VALUE))
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    /*
     * Collect the  wanted VirtualInstrumentNodes for a specific TreeID, change the value and save again
     *
     */

    private void collectAndSaveVirtualInstrumentNodes(int aTreeID) {

        jOTDBparam aParam = null;

        try {
            ArrayList <jOTDBnode> node = new ArrayList(OtdbRmi.getRemoteMaintenance().getItemList(aTreeID, "VirtualInstrument"));
            if (node.size() > 0) {
                jOTDBnode aNode = node.get(0);
                ArrayList<jOTDBnode> childs = new ArrayList(OtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1));
                // get all the params per child
                
                for (jOTDBnode anotherNode:childs) {
                    aParam = null;
                    // We need to keep all the params needed by this panel
                    if (anotherNode.leaf) {
                        aParam = OtdbRmi.getRemoteMaintenance().getParam(anotherNode);
                    }
                    getAndSaveField("VirtualInstrument", aParam, anotherNode);
                }
            }
        } catch (RemoteException ex) {
            String aS = "Remote Exception during init: " + ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this, aS, new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            return;
        }

    }

    /*
     * Collect the  wanted VirtualInstrumentNodes for a specific TreeID, change the value and save again
     *
     */

    private void collectAndSaveObservationNodes(int aTreeID) {

        jOTDBparam aParam = null;

        try {
            ArrayList<jOTDBnode> node = new ArrayList(OtdbRmi.getRemoteMaintenance().getItemList(aTreeID, "Observation"));
            // get all the params per child
            if (node.size() > 0) {
                jOTDBnode aNode = node.get(0);
                ArrayList<jOTDBnode> childs = new ArrayList(OtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1));

                for (jOTDBnode anotherNode: childs ) {
                    aParam = null;
                    // We need to keep all the params needed by this panel
                    if (anotherNode.leaf) {
                        aParam = OtdbRmi.getRemoteMaintenance().getParam(anotherNode);
                    }
                    getAndSaveField("Observation", aParam, anotherNode);
                }
            }
        } catch (RemoteException ex) {
            String aS = "Remote Exception during init: " + ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this, aS, new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            return;
        }


    }

    /** Restore original Values in  panel
     */
    private void restore() {

      //Virtual Instrument stationList
      setStationLists(itsStationList.limits);
    }
    
    private void saveButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_saveButtonActionPerformed
        if (saveNewTree()) {
            statusLabel.setText("Ready");
            setVisible(false);
            dispose();
        }
    
    }//GEN-LAST:event_saveButtonActionPerformed

    private void cancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_cancelButtonActionPerformed
        setVisible(false);
        dispose();
    }//GEN-LAST:event_cancelButtonActionPerformed

    private void restoreButtonMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_restoreButtonMouseClicked
        restore();
    }//GEN-LAST:event_restoreButtonMouseClicked

    private void applyStationListActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_applyStationListActionPerformed
        this.coreStationSelectionPanel.setEnabled(applyStationList.isSelected());
        this.remoteStationSelectionPanel.setEnabled(applyStationList.isSelected());
        this.europeStationSelectionPanel.setEnabled(applyStationList.isSelected());
        this.setStationList = applyStationList.isSelected();
    }//GEN-LAST:event_applyStationListActionPerformed


    private MainFrame itsMainFrame = null;
    private jOTDBtree itsTree = null;
    private int []    itsTreeIDs=null;
    private String    itsName=null;
    private boolean   isAdministrator;
    private boolean   hasChanged=false;


    // Changeable values
    private ArrayList<String>    itsUsedCoreStations      = new ArrayList<>();
    private ArrayList<String>    itsUsedRemoteStations    = new ArrayList<>();
    private ArrayList<String>    itsUsedEuropeStations    = new ArrayList<>();

    // Observation Virtual Instrument parameters
    private jOTDBnode itsStationList=null;


    private boolean   isInitialized=false;
    private boolean   setStationList=false;
     
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JCheckBox applyStationList;
    private javax.swing.JButton cancelButton;
    private nl.astron.lofar.sas.otbcomponents.StationSelectionPanel coreStationSelectionPanel;
    private nl.astron.lofar.sas.otbcomponents.StationSelectionPanel europeStationSelectionPanel;
    private javax.swing.JScrollPane jScrollPane1;
    private nl.astron.lofar.sas.otbcomponents.StationSelectionPanel remoteStationSelectionPanel;
    private javax.swing.JButton restoreButton;
    private javax.swing.JButton saveButton;
    private javax.swing.JLabel statusLabel;
    private javax.swing.JTextArea topLabel;
    // End of variables declaration//GEN-END:variables

}
