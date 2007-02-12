/*
 * ObservationPanel.java
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

package nl.astron.lofar.sas.otbcomponents.userpanels;

import java.awt.Component;
import java.rmi.RemoteException;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.DefaultListModel;
import javax.swing.JList;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.border.TitledBorder;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otbcomponents.OLAPConfigPanel;
import org.apache.log4j.Logger;

/**
 * Panel for Observation specific configuration
 *
 * @author  Coolen
 *
 * @created 17-02-2007, 15:07
 *
 * @version $Id$
 */
public class ObservationPanel extends javax.swing.JPanel implements IViewPanel{
    
    static Logger logger = Logger.getLogger(OLAPConfigPanel.class);    
    static String name = "OLAPConfig";
    
   
     /** Creates new form BeanForm based upon aNode
     *
     * @params  aNode   node to obtain the info from
     *
     */
    public ObservationPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsOtdbRmi=itsMainFrame.getSharedVars().getOTDBrmi();
        itsNode=aNode;
        initialize();
        initPanel();
    }
    
    /** Creates new form BeanForm */
    public ObservationPanel() {
        initComponents();
        initialize();
    }
    
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
            itsOtdbRmi=itsMainFrame.getSharedVars().getOTDBrmi();
        } else {
            logger.debug("No Mainframe supplied");
        }
    }
    
    public String getShortName() {
        return name;
    }
    
    public void setContent(Object anObject) {   
        itsNode=(jOTDBnode)anObject;
        jOTDBparam aParam=null;
        try {
            
            //we need to get all the childs from this node.    
            Vector childs = itsOtdbRmi.getRemoteMaintenance().getItemList(itsNode.treeID(), itsNode.nodeID(), 1);
            
            // get all the params per child
            Enumeration e = childs.elements();
            while( e.hasMoreElements()  ) {
                aParam=null;
            
                jOTDBnode aNode = (jOTDBnode)e.nextElement();
                        
                // We need to keep all the nodes needed by this panel
                // if the node is a leaf we need to get the pointed to value via Param.
                if (aNode.leaf) {
                    aParam = itsOtdbRmi.getRemoteMaintenance().getParam(aNode);
                    setField(aParam,aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("Beam")) {
                    //we need to get all the childs from this node also.    
                    Vector beamChilds = itsOtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1);
            
                    // get all the params per child
                    Enumeration e1 = beamChilds.elements();
                    while( e1.hasMoreElements()  ) {
                       
                        jOTDBnode aBeamNode = (jOTDBnode)e1.nextElement();
                        aParam=null;
                        // We need to keep all the params needed by this panel
                        if (aBeamNode.leaf) {
                            aParam = itsOtdbRmi.getRemoteMaintenance().getParam(aBeamNode);
                        }
                        setField(aParam,aBeamNode);
                    }
                } else if (LofarUtils.keyName(aNode.name).equals("VirtualInstrument")) {
                    //we need to get all the childs from this node also.    
                    Vector VIChilds = itsOtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1);
            
                    // get all the params per child
                    Enumeration e2 = VIChilds.elements();
                    while( e2.hasMoreElements()  ) {
                       
                        jOTDBnode aVINode = (jOTDBnode)e2.nextElement();
                        aParam=null;
                        // We need to keep all the params needed by this panel
                        if (aVINode.leaf) {
                            aParam = itsOtdbRmi.getRemoteMaintenance().getParam(aVINode);
                        }
                        setField(aParam,aVINode);
                    }
                }
            }
        } catch (RemoteException ex) {
            logger.debug("Error during getComponentParam: "+ ex);
            itsParamList=null;
            return;
        }
        
        initPanel();
    }
    public boolean isSingleton() {
        return false;
    }
    
    public JPanel getInstance() {
        return new ObservationPanel();
    }
    public boolean hasPopupMenu() {
        return false;
    }
    
    
    /** create popup menu for this panel
     *
     *  // build up the menu
     *  aPopupMenu= new JPopupMenu();
     *  aMenuItem=new JMenuItem("Choice 1");        
     *  aMenuItem.addActionListener(new java.awt.event.ActionListener() {
     *      public void actionPerformed(java.awt.event.ActionEvent evt) {
     *          popupMenuHandler(evt);
     *      }
     *  });
     *  aMenuItem.setActionCommand("Choice 1");
     *  aPopupMenu.add(aMenuItem);
     *  aPopupMenu.setOpaque(true);
     *
     *
     *  aPopupMenu.show(aComponent, x, y );        
     */
    public void createPopupMenu(Component aComponent,int x, int y) {
        JPopupMenu aPopupMenu=null;
        JMenuItem  aMenuItem=null;
        
    }
    
    /** handles the choice from the popupmenu 
     *
     * depending on the choices that are possible for this panel perform the action for it
     *
     *      if (evt.getActionCommand().equals("Choice 1")) {
     *          perform action
     *      }  
     */
    public void popupMenuHandler(java.awt.event.ActionEvent evt) {
    }
    
    /* Set's the different fields in the GUI */
    private void setField(jOTDBparam aParam, jOTDBnode aNode) {
        // Generic Observation
        if (aNode==null) {
            return;
        }
        logger.debug("setField for: "+ aNode.name);
        boolean isRef = LofarUtils.isReference(aNode.limits);
        String aKeyName = LofarUtils.keyName(aNode.name);   
        try {
            if (itsOtdbRmi.getRemoteTypes().getParamType(aParam.type).substring(0,1).equals("p")) {
               // Have to get new param because we need the unresolved limits field.
               aParam = itsOtdbRmi.getRemoteMaintenance().getParam(aNode.treeID(),aNode.paramDefID());                
            }
        } catch (RemoteException ex) {
            logger.debug("Error during getParam: "+ ex);
        }
        
        // Observation Specific parameters
        if (aKeyName.equals("MSNameMask")) {
            inputMSNameMask.setToolTipText(aParam.description);
            itsMSNameMask=aNode;
            if (isRef && aParam != null) {
                inputMSNameMask.setText(aNode.limits + " : " + aParam.limits);
            } else {
                inputMSNameMask.setText(aNode.limits);
            }
        } else if (aKeyName.equals("antennaArray")) {        
            inputAntennaArray.setToolTipText(aParam.description);
            LofarUtils.setPopupComboChoices(inputAntennaArray,aParam.limits);
            if (!aNode.limits.equals("")) {
                inputAntennaArray.setSelectedItem(aNode.limits);            
            }
            itsAntennaArray=aNode;
        } else if (aKeyName.equals("bandFilter")) {        
            inputBandFilter.setToolTipText(aParam.description);
            LofarUtils.setPopupComboChoices(inputBandFilter,aParam.limits);
            if (!aNode.limits.equals("")) {
              inputBandFilter.setSelectedItem(aNode.limits);
            }
            itsBandFilter=aNode;
        } else if (aKeyName.equals("clockMode")) {        
            inputClockMode.setToolTipText(aParam.description);
            LofarUtils.setPopupComboChoices(inputClockMode,aParam.limits);
            if (!aNode.limits.equals("")) {
              inputClockMode.setSelectedItem(aNode.limits);
            }
            itsClockMode=aNode;
        } else if (aKeyName.equals("nyquistZone")) {        
            inputNyquistZone.setToolTipText(aParam.description);
            LofarUtils.setPopupComboChoices(inputNyquistZone,aParam.limits);
            if (!aNode.limits.equals("")) {
              inputNyquistZone.setSelectedItem(aNode.limits);
            }
            itsNyquistZone=aNode;
        } else if (aKeyName.equals("beamletList")) {        
            inputBeamletList.setToolTipText(aParam.description);
            itsBeamletList=aNode;
            if (isRef && aParam != null) {
                inputBeamletList.setText(aNode.limits + " : " + aParam.limits);
            } else {
                inputBeamletList.setText(aNode.limits);
            }
        } else if (aKeyName.equals("receiverList")) {        
            inputReceiverList.setToolTipText(aParam.description);
            itsReceiverList=aNode;
            if (isRef && aParam != null) {
                inputReceiverList.setText(aNode.limits + " : " + aParam.limits);
            } else {
                inputReceiverList.setText(aNode.limits);
            }
        } else if (aKeyName.equals("subbandList")) {        
            inputSubbandList.setToolTipText(aParam.description);
            itsSubbandList=aNode;
            if (isRef && aParam != null) {
                inputSubbandList.setText(aNode.limits + " : " + aParam.limits);
            } else {
                inputSubbandList.setText(aNode.limits);
            }

            // Observation Beam parameters

        
        } else if (aKeyName.equals("angle1")) {        
            inputAngle1.setToolTipText(aParam.description);
            itsAngle1=aNode;
            if (isRef && aParam != null) {
                inputAngle1.setText(aNode.limits + " : " + aParam.limits);
            } else {
                inputAngle1.setText(aNode.limits);
            }
        } else if (aKeyName.equals("angle2")) {        
            inputAngle2.setToolTipText(aParam.description);
            itsAngle2=aNode;
            if (isRef && aParam != null) {
                inputAngle2.setText(aNode.limits + " : " + aParam.limits);
            } else {
                inputAngle2.setText(aNode.limits);
            }
        } else if (aKeyName.equals("angleTimes")) {        
            inputAngleTimes.setToolTipText(aParam.description);
            itsAngleTimes=aNode;
            if (isRef && aParam != null) {
                inputAngleTimes.setText(aNode.limits + " : " + aParam.limits);
            } else {
                inputAngleTimes.setText(aNode.limits);
            }
        } else if (aKeyName.equals("directionTypes")) {        
            inputDirectionTypes.setToolTipText(aParam.description);
            LofarUtils.setPopupComboChoices(inputDirectionTypes,aParam.limits);
            if (!aNode.limits.equals("")) {
              inputDirectionTypes.setSelectedItem(aNode.limits);
            }
            itsDirectionTypes=aNode;

            // Observation VirtualObservation parameters

        } else if (aKeyName.equals("stationList")) {        
            this.stationsList.setToolTipText(aParam.description);
            this.itsStationList = aNode;
                
            //set the checkbox correctly when no stations are provided in the data
            if(itsStationList.limits == null || itsStationList.limits.equals("[]")){
                stationsList.setModel(new DefaultListModel());
            }else{
                TitledBorder aBorder = (TitledBorder)this.stationsPanel.getBorder();
                if (isRef && aParam != null) {
                    aBorder.setTitle("Station Names (Referenced)");
                    LofarUtils.fillList(stationsList,aParam.limits,false);
                } else {
                    aBorder.setTitle("Station Names");
                    LofarUtils.fillList(stationsList,aNode.limits,false);
                }
            }
        }
    }

    
    /** Restore original Values in  panel
     */
    private void restore() {
      // Observation Specific parameters
      inputMSNameMask.setText(itsMSNameMask.limits);
      inputAntennaArray.setSelectedItem(itsAntennaArray.limits);
      inputBandFilter.setSelectedItem(itsBandFilter.limits);
      inputClockMode.setSelectedItem(itsClockMode.limits);
      inputNyquistZone.setSelectedItem(itsNyquistZone.limits);
      inputBeamletList.setText(itsBeamletList.limits);
      inputReceiverList.setText(itsReceiverList.limits);
      inputSubbandList.setText(itsSubbandList.limits);
      inputDescription.setText("");
    
      // Observation Beam parameters
      inputAngle1.setText(itsAngle1.limits);
      inputAngle2.setText(itsAngle2.limits);
      inputAngleTimes.setText(itsAngleTimes.limits);
      inputDirectionTypes.setSelectedItem(itsDirectionTypes.limits);
    
  
      // Observation VirtualObservation parameters
      //set the checkbox correctly when no stations are provided in the data
      if(itsStationList.limits == null || itsStationList.limits.equals("[]")){
        stationsList.setModel(new DefaultListModel());
      }else{
        TitledBorder aBorder = (TitledBorder)this.stationsPanel.getBorder();
        aBorder.setTitle("Station Names");
        LofarUtils.fillList(stationsList,itsStationList.limits,false);
      }
    }
    
    
    private void initialize() {
        buttonPanel1.addButton("Restore");
        buttonPanel1.addButton("Save");
        this.stationsList.setModel(new DefaultListModel());
        
        // hardcoded for now. In a later stage they MUST be taken out of
        // the param, or from a special class that contains all possible stations
        this.modifyStationsCombobox.removeAllItems();
        this.modifyStationsCombobox.addItem("CS001");
        this.modifyStationsCombobox.addItem("CS008");
        this.modifyStationsCombobox.addItem("CS010");
        this.modifyStationsCombobox.addItem("CS016");
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
        
    /** saves the given node back to the database
     */
    private void saveNode(jOTDBnode aNode) {
        if (aNode == null) {
            return;
        }
        try {
            itsOtdbRmi.getRemoteMaintenance().saveNode(aNode); 
        } catch (RemoteException ex) {
            logger.debug("Error: saveNode failed : " + ex);
        } 
    }
    
    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
    }        
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
    }
    
    private void saveInput() {
        boolean hasChanged = false;

        // Beam        
        if (itsAngleTimes != null && !inputAngleTimes.equals(itsAngleTimes.limits)) {  
            itsAngleTimes.limits = inputAngleTimes.getText();
            saveNode(itsAngleTimes);
        }
        if (itsAngle1 != null && !inputAngle1.equals(itsAngle1.limits)) {  
            itsAngle1.limits = inputAngle1.getText();
            saveNode(itsAngle1);
        }
        if (itsAngle2 != null && !inputAngle2.equals(itsAngle2.limits)) {  
            itsAngle2.limits = inputAngle2.getText();
            saveNode(itsAngle2);
        }
        if (itsDirectionTypes != null && !inputDirectionTypes.getSelectedItem().toString().equals(itsDirectionTypes.limits)) {  
            itsDirectionTypes.limits = inputDirectionTypes.getSelectedItem().toString();
            saveNode(itsDirectionTypes);
        }
        
        //VirtualInstrument
        if (this.itsStationList != null && !LofarUtils.createList(stationsList,false).equals(itsStationList.limits)) {
            itsStationList.limits = LofarUtils.createList(stationsList,false);
            logger.trace("Variable VirtualInstrumenst ("+itsStationList.name+"//"+itsStationList.treeID()+"//"+itsStationList.nodeID()+"//"+itsStationList.parentID()+"//"+itsStationList.paramDefID()+") updating to :"+itsStationList.limits);
            saveNode(itsStationList);
        }
        
        // Generic Observation
        if (itsMSNameMask != null && !inputMSNameMask.equals(itsMSNameMask.limits)) {  
            itsMSNameMask.limits = inputMSNameMask.getText();
            saveNode(itsMSNameMask);
        }
        if (itsBeamletList != null && !inputBeamletList.equals(itsBeamletList.limits)) {  
            itsBeamletList.limits = inputBeamletList.getText();
            saveNode(itsBeamletList);
        }
        if (itsReceiverList != null && !inputReceiverList.equals(itsReceiverList.limits)) {  
            itsReceiverList.limits = inputReceiverList.getText();
            saveNode(itsReceiverList);
        }
        if (itsSubbandList != null && !inputSubbandList.equals(itsSubbandList.limits)) {  
            itsSubbandList.limits = inputSubbandList.getText();
            saveNode(itsSubbandList);
        }
        if (itsAntennaArray != null && !inputAntennaArray.getSelectedItem().toString().equals(itsAntennaArray.limits)) {
            itsAntennaArray.limits = inputAntennaArray.getSelectedItem().toString();
            saveNode(itsAntennaArray);
        }
        if (itsBandFilter != null && !inputBandFilter.getSelectedItem().toString().equals(itsBandFilter.limits)) {
            itsBandFilter.limits = inputBandFilter.getSelectedItem().toString();
            saveNode(itsBandFilter);
        }
        if (itsClockMode != null && !inputClockMode.getSelectedItem().toString().equals(itsClockMode.limits)) {
            itsClockMode.limits = inputClockMode.getSelectedItem().toString();
            saveNode(itsClockMode);
        }
        if (itsNyquistZone != null && !inputNyquistZone.getSelectedItem().toString().equals(itsNyquistZone.limits)) {
            itsNyquistZone.limits = inputNyquistZone.getSelectedItem().toString();
            saveNode(itsNyquistZone);
        }

        
    }
    
    private void changeDescription(jOTDBnode aNode) {
        if (aNode == null) {
            return;
        }
        try {
            jOTDBparam aParam = itsOtdbRmi.getRemoteMaintenance().getParam(aNode);
            // check if the node changed, and if the description was changed, if so ask if the new description
            // should be saved.
            if (itsOldDescriptionParam == null | itsOldDescriptionParam !=aParam) {
                if (itsOldDescriptionParam != null) {
                    if (!inputDescription.getText().equals(itsOldDescriptionParam.description) && !inputDescription.getText().equals("")) {
                        int answer=JOptionPane.showConfirmDialog(this,"The old description was altered, do you want to save the old one ?","alert",JOptionPane.YES_NO_OPTION);
                        if (answer == JOptionPane.YES_OPTION) {
                            if (!itsOtdbRmi.getRemoteMaintenance().saveParam(itsOldDescriptionParam)) {
                                logger.error("Saving param "+itsOldDescriptionParam.nodeID()+","+itsOldDescriptionParam.paramID()+"failed: "+ itsOtdbRmi.getRemoteMaintenance().errorMsg());
                            }                          
                        }
                    }
                }
            }
            itsOldDescriptionParam=aParam;
            inputDescription.setText(aParam.description);
        } catch (RemoteException ex) {
            logger.debug("Error during getParam: "+ ex);
            return;
        }
    }
    

    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        java.awt.GridBagConstraints gridBagConstraints;

        jPanel1 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        jPanel2 = new javax.swing.JPanel();
        jPanel7 = new javax.swing.JPanel();
        jPanel3 = new javax.swing.JPanel();
        labelAngle1 = new javax.swing.JLabel();
        inputAngle1 = new javax.swing.JTextField();
        labelAngle2 = new javax.swing.JLabel();
        inputAngle2 = new javax.swing.JTextField();
        labelAngleTimes = new javax.swing.JLabel();
        inputAngleTimes = new javax.swing.JTextField();
        labelDirectionTypes = new javax.swing.JLabel();
        inputDirectionTypes = new javax.swing.JComboBox();
        jPanel5 = new javax.swing.JPanel();
        stationsPanel = new javax.swing.JPanel();
        stationsScrollPane = new javax.swing.JScrollPane();
        stationsList = new javax.swing.JList();
        stationsModPanel = new javax.swing.JPanel();
        stationsButtonPanel = new javax.swing.JPanel();
        deleteStationButton = new javax.swing.JButton();
        addStationButton = new javax.swing.JButton();
        modifyStationsCombobox = new javax.swing.JComboBox();
        jPanel10 = new javax.swing.JPanel();
        labelMSNameMask = new javax.swing.JLabel();
        inputMSNameMask = new javax.swing.JTextField();
        labelAntennaArray = new javax.swing.JLabel();
        inputAntennaArray = new javax.swing.JComboBox();
        labelBandFilter = new javax.swing.JLabel();
        inputBandFilter = new javax.swing.JComboBox();
        inputClockMode = new javax.swing.JComboBox();
        labelNyquistZone = new javax.swing.JLabel();
        inputNyquistZone = new javax.swing.JComboBox();
        labelClockMode = new javax.swing.JLabel();
        jPanel4 = new javax.swing.JPanel();
        labelStationList = new javax.swing.JPanel();
        labelBeamletList = new javax.swing.JLabel();
        inputBeamletList = new javax.swing.JTextField();
        labelReceiverList = new javax.swing.JLabel();
        inputReceiverList = new javax.swing.JTextField();
        labelSubbandList = new javax.swing.JLabel();
        inputSubbandList = new javax.swing.JTextField();
        descriptionScrollPane = new javax.swing.JScrollPane();
        inputDescription = new javax.swing.JTextArea();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        setLayout(new java.awt.BorderLayout());

        jPanel1.setLayout(new java.awt.BorderLayout());

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder(""));
        jPanel1.setPreferredSize(new java.awt.Dimension(100, 25));
        jLabel1.setFont(new java.awt.Font("Tahoma", 1, 11));
        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("Observation Details");
        jPanel1.add(jLabel1, java.awt.BorderLayout.CENTER);

        add(jPanel1, java.awt.BorderLayout.NORTH);

        jPanel2.setLayout(new java.awt.BorderLayout());

        jPanel2.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));
        jPanel3.setBorder(javax.swing.BorderFactory.createTitledBorder("Beam Input"));
        jPanel3.setPreferredSize(new java.awt.Dimension(200, 125));
        jPanel3.setRequestFocusEnabled(false);
        jPanel3.setVerifyInputWhenFocusTarget(false);
        labelAngle1.setText("Angle 1:");

        inputAngle1.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                inputAngle1FocusGained(evt);
            }
        });

        labelAngle2.setText("Angle 2 :");

        inputAngle2.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                inputAngle2FocusGained(evt);
            }
        });

        labelAngleTimes.setText("AngleTimes :");

        inputAngleTimes.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                inputAngleTimesFocusGained(evt);
            }
        });

        labelDirectionTypes.setText("DirectionTypes:");

        inputDirectionTypes.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1" }));
        inputDirectionTypes.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                inputDirectionTypesFocusGained(evt);
            }
        });

        org.jdesktop.layout.GroupLayout jPanel3Layout = new org.jdesktop.layout.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel3Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(labelDirectionTypes)
                    .add(labelAngleTimes))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(inputDirectionTypes, 0, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(inputAngleTimes, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 131, Short.MAX_VALUE))
                .add(28, 28, 28)
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(labelAngle2, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(labelAngle1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 66, Short.MAX_VALUE))
                .add(13, 13, 13)
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(inputAngle1)
                    .add(inputAngle2, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 190, Short.MAX_VALUE))
                .addContainerGap(22, Short.MAX_VALUE))
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel3Layout.createSequentialGroup()
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelAngleTimes)
                    .add(inputAngleTimes, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(labelAngle1)
                    .add(inputAngle1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(15, 15, 15)
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelDirectionTypes)
                    .add(inputDirectionTypes, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(labelAngle2)
                    .add(inputAngle2, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(46, 46, 46))
        );

        jPanel5.setBorder(javax.swing.BorderFactory.createTitledBorder("Virtual Instrument Input"));
        stationsPanel.setLayout(new java.awt.BorderLayout());

        stationsPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Station Names"));
        stationsPanel.setToolTipText("Identifiers of the participating stations.");
        stationsList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "1", "2", "3", "4", "5" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        stationsList.setToolTipText("Names of the participating stations.");
        stationsList.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                stationsListFocusGained(evt);
            }
        });
        stationsList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent evt) {
                stationsListValueChanged(evt);
            }
        });

        stationsScrollPane.setViewportView(stationsList);

        stationsPanel.add(stationsScrollPane, java.awt.BorderLayout.CENTER);

        stationsModPanel.setLayout(new java.awt.BorderLayout());

        stationsButtonPanel.setLayout(new java.awt.GridBagLayout());

        deleteStationButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Delete16.gif")));
        deleteStationButton.setToolTipText("Remove the selected Station from the list");
        deleteStationButton.setEnabled(false);
        deleteStationButton.setMaximumSize(new java.awt.Dimension(30, 25));
        deleteStationButton.setMinimumSize(new java.awt.Dimension(30, 25));
        deleteStationButton.setPreferredSize(new java.awt.Dimension(30, 25));
        deleteStationButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                deleteStationButtonActionPerformed(evt);
            }
        });

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 1;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stationsButtonPanel.add(deleteStationButton, gridBagConstraints);

        addStationButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otbcomponents/bbs/icons/general/Add16.gif")));
        addStationButton.setToolTipText("Add the station entered to the list");
        addStationButton.setMaximumSize(new java.awt.Dimension(30, 25));
        addStationButton.setMinimumSize(new java.awt.Dimension(30, 25));
        addStationButton.setPreferredSize(new java.awt.Dimension(30, 25));
        addStationButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addStationButtonActionPerformed(evt);
            }
        });

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        stationsButtonPanel.add(addStationButton, gridBagConstraints);

        stationsModPanel.add(stationsButtonPanel, java.awt.BorderLayout.SOUTH);

        modifyStationsCombobox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        stationsModPanel.add(modifyStationsCombobox, java.awt.BorderLayout.CENTER);

        stationsPanel.add(stationsModPanel, java.awt.BorderLayout.SOUTH);

        org.jdesktop.layout.GroupLayout jPanel5Layout = new org.jdesktop.layout.GroupLayout(jPanel5);
        jPanel5.setLayout(jPanel5Layout);
        jPanel5Layout.setHorizontalGroup(
            jPanel5Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel5Layout.createSequentialGroup()
                .addContainerGap()
                .add(stationsPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 200, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(43, Short.MAX_VALUE))
        );
        jPanel5Layout.setVerticalGroup(
            jPanel5Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel5Layout.createSequentialGroup()
                .add(stationsPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 278, Short.MAX_VALUE)
                .addContainerGap())
        );

        jPanel10.setBorder(javax.swing.BorderFactory.createTitledBorder("Generic Observation Input"));
        labelMSNameMask.setText("MSNameMask:");

        inputMSNameMask.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                inputMSNameMaskFocusGained(evt);
            }
        });

        labelAntennaArray.setText("Antenna Array: ");

        inputAntennaArray.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        inputAntennaArray.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                inputAntennaArrayFocusGained(evt);
            }
        });

        labelBandFilter.setText("BandFilter:");

        inputBandFilter.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        inputBandFilter.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                inputBandFilterFocusGained(evt);
            }
        });

        inputClockMode.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        inputClockMode.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                inputClockModeFocusGained(evt);
            }
        });

        labelNyquistZone.setText("Nyquist Zone:");

        inputNyquistZone.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        inputNyquistZone.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                inputNyquistZoneFocusGained(evt);
            }
        });

        labelClockMode.setText("Clock Mode:");

        org.jdesktop.layout.GroupLayout jPanel10Layout = new org.jdesktop.layout.GroupLayout(jPanel10);
        jPanel10.setLayout(jPanel10Layout);
        jPanel10Layout.setHorizontalGroup(
            jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel10Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel10Layout.createSequentialGroup()
                        .add(jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                            .add(labelAntennaArray, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .add(labelBandFilter, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                            .add(inputAntennaArray, 0, 143, Short.MAX_VALUE)
                            .add(inputBandFilter, 0, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                        .add(12, 12, 12)
                        .add(jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                            .add(labelClockMode, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .add(labelNyquistZone, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                        .add(18, 18, 18)
                        .add(jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                            .add(inputClockMode, 0, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .add(inputNyquistZone, 0, 178, Short.MAX_VALUE)))
                    .add(jPanel10Layout.createSequentialGroup()
                        .add(labelMSNameMask, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 80, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(inputMSNameMask, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 331, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                .add(29, 29, 29))
        );
        jPanel10Layout.setVerticalGroup(
            jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel10Layout.createSequentialGroup()
                .add(jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelMSNameMask)
                    .add(inputMSNameMask, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, 34, Short.MAX_VALUE)
                .add(jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelAntennaArray)
                    .add(inputAntennaArray, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(labelClockMode)
                    .add(inputClockMode, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(13, 13, 13)
                .add(jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelBandFilter)
                    .add(inputBandFilter, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(labelNyquistZone)
                    .add(inputNyquistZone, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(47, 47, 47))
        );

        org.jdesktop.layout.GroupLayout jPanel7Layout = new org.jdesktop.layout.GroupLayout(jPanel7);
        jPanel7.setLayout(jPanel7Layout);
        jPanel7Layout.setHorizontalGroup(
            jPanel7Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel7Layout.createSequentialGroup()
                .add(jPanel7Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(jPanel10, 0, 551, Short.MAX_VALUE)
                    .add(jPanel3, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 551, Short.MAX_VALUE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel5, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanel7Layout.setVerticalGroup(
            jPanel7Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, jPanel7Layout.createSequentialGroup()
                .add(jPanel7Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(org.jdesktop.layout.GroupLayout.LEADING, jPanel5, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(jPanel7Layout.createSequentialGroup()
                        .add(jPanel3, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 125, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(jPanel10, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                .addContainerGap())
        );
        jPanel2.add(jPanel7, java.awt.BorderLayout.NORTH);

        labelStationList.setBorder(javax.swing.BorderFactory.createTitledBorder("Generic Observation Lists"));
        labelBeamletList.setText("Beamlets :");

        inputBeamletList.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                inputBeamletListFocusGained(evt);
            }
        });

        labelReceiverList.setText("Receivers :");

        inputReceiverList.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                inputReceiverListFocusGained(evt);
            }
        });

        labelSubbandList.setText("Subbands :");

        inputSubbandList.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                inputSubbandListFocusGained(evt);
            }
        });

        descriptionScrollPane.setBorder(javax.swing.BorderFactory.createTitledBorder("Description"));
        inputDescription.setColumns(20);
        inputDescription.setRows(5);
        descriptionScrollPane.setViewportView(inputDescription);

        org.jdesktop.layout.GroupLayout labelStationListLayout = new org.jdesktop.layout.GroupLayout(labelStationList);
        labelStationList.setLayout(labelStationListLayout);
        labelStationListLayout.setHorizontalGroup(
            labelStationListLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(labelStationListLayout.createSequentialGroup()
                .addContainerGap()
                .add(labelStationListLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(labelStationListLayout.createSequentialGroup()
                        .add(descriptionScrollPane, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 790, Short.MAX_VALUE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED))
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, labelStationListLayout.createSequentialGroup()
                        .add(labelStationListLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(labelStationListLayout.createSequentialGroup()
                                .add(labelSubbandList, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 67, Short.MAX_VALUE)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED))
                            .add(labelStationListLayout.createSequentialGroup()
                                .add(labelReceiverList, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 64, Short.MAX_VALUE)
                                .add(7, 7, 7))
                            .add(labelStationListLayout.createSequentialGroup()
                                .add(labelBeamletList, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 62, Short.MAX_VALUE)
                                .add(9, 9, 9)))
                        .add(labelStationListLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(org.jdesktop.layout.GroupLayout.TRAILING, inputBeamletList, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 719, Short.MAX_VALUE)
                            .add(org.jdesktop.layout.GroupLayout.TRAILING, inputReceiverList, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 719, Short.MAX_VALUE)
                            .add(org.jdesktop.layout.GroupLayout.TRAILING, inputSubbandList, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 719, Short.MAX_VALUE))))
                .addContainerGap())
        );
        labelStationListLayout.setVerticalGroup(
            labelStationListLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(labelStationListLayout.createSequentialGroup()
                .addContainerGap()
                .add(labelStationListLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelReceiverList)
                    .add(inputReceiverList, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(14, 14, 14)
                .add(labelStationListLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelSubbandList)
                    .add(inputSubbandList, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(15, 15, 15)
                .add(labelStationListLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelBeamletList)
                    .add(inputBeamletList, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(54, 54, 54)
                .add(descriptionScrollPane, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(167, Short.MAX_VALUE))
        );

        org.jdesktop.layout.GroupLayout jPanel4Layout = new org.jdesktop.layout.GroupLayout(jPanel4);
        jPanel4.setLayout(jPanel4Layout);
        jPanel4Layout.setHorizontalGroup(
            jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(labelStationList, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );
        jPanel4Layout.setVerticalGroup(
            jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel4Layout.createSequentialGroup()
                .add(labelStationList, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );
        jPanel2.add(jPanel4, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });

        jPanel2.add(buttonPanel1, java.awt.BorderLayout.SOUTH);

        add(jPanel2, java.awt.BorderLayout.CENTER);

    }// </editor-fold>//GEN-END:initComponents

    private void inputSubbandListFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_inputSubbandListFocusGained
        changeDescription(itsSubbandList);
    }//GEN-LAST:event_inputSubbandListFocusGained

    private void inputReceiverListFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_inputReceiverListFocusGained
        changeDescription(itsReceiverList);
    }//GEN-LAST:event_inputReceiverListFocusGained

    private void inputBeamletListFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_inputBeamletListFocusGained
        changeDescription(itsBeamletList);
    }//GEN-LAST:event_inputBeamletListFocusGained

    private void inputNyquistZoneFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_inputNyquistZoneFocusGained
        changeDescription(itsNyquistZone);
    }//GEN-LAST:event_inputNyquistZoneFocusGained

    private void inputClockModeFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_inputClockModeFocusGained
        changeDescription(itsClockMode);
    }//GEN-LAST:event_inputClockModeFocusGained

    private void inputBandFilterFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_inputBandFilterFocusGained
        changeDescription(itsBandFilter);
    }//GEN-LAST:event_inputBandFilterFocusGained

    private void inputAntennaArrayFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_inputAntennaArrayFocusGained
        changeDescription(itsAntennaArray);
    }//GEN-LAST:event_inputAntennaArrayFocusGained

    private void inputMSNameMaskFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_inputMSNameMaskFocusGained
        changeDescription(itsMSNameMask);
    }//GEN-LAST:event_inputMSNameMaskFocusGained

    private void stationsListFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_stationsListFocusGained
        changeDescription(itsStationList);
    }//GEN-LAST:event_stationsListFocusGained

    private void inputAngle2FocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_inputAngle2FocusGained
        changeDescription(itsAngle2);
    }//GEN-LAST:event_inputAngle2FocusGained

    private void inputDirectionTypesFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_inputDirectionTypesFocusGained
        changeDescription(itsDirectionTypes);
    }//GEN-LAST:event_inputDirectionTypesFocusGained

    private void inputAngleTimesFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_inputAngleTimesFocusGained
        changeDescription(itsAngleTimes);
    }//GEN-LAST:event_inputAngleTimesFocusGained

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        if(evt.getActionCommand() == "Save") {
            saveInput();
        } else if(evt.getActionCommand() == "Restore") {
            restore();
        }

    }//GEN-LAST:event_buttonPanel1ActionPerformed

    private void addStationButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addStationButtonActionPerformed
        String toBeAddedStation = (String)this.modifyStationsCombobox.getSelectedItem();
        DefaultListModel theStationModel = (DefaultListModel)stationsList.getModel();
        if(!theStationModel.contains(toBeAddedStation)){
            theStationModel.addElement(toBeAddedStation);
        }
    }//GEN-LAST:event_addStationButtonActionPerformed

    private void deleteStationButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteStationButtonActionPerformed
        DefaultListModel theStationModel = (DefaultListModel)stationsList.getModel();
        int[] selectedIndices = stationsList.getSelectedIndices();
        while(selectedIndices.length>0){
            theStationModel.remove(selectedIndices[0]);
            selectedIndices = stationsList.getSelectedIndices();
        }
        if(theStationModel.size()==0){
            this.deleteStationButton.setEnabled(false);
        }
    }//GEN-LAST:event_deleteStationButtonActionPerformed

    private void stationsListValueChanged(javax.swing.event.ListSelectionEvent evt) {//GEN-FIRST:event_stationsListValueChanged
        int[] selectedIndices = ((JList)evt.getSource()).getSelectedIndices();
        if(selectedIndices.length>0){
            this.deleteStationButton.setEnabled(true);
        }else{
            this.deleteStationButton.setEnabled(false);
        }
    }//GEN-LAST:event_stationsListValueChanged

    private void inputAngle1FocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_inputAngle1FocusGained
        changeDescription(itsAngle1);
    }//GEN-LAST:event_inputAngle1FocusGained
    
    private jOTDBnode  itsNode = null;
    private MainFrame  itsMainFrame;
    private OtdbRmi    itsOtdbRmi;
    private Vector<jOTDBparam> itsParamList;
    private jOTDBparam itsOldDescriptionParam;
    
    // Observation Specific parameters
    private jOTDBnode itsMSNameMask;
    private jOTDBnode itsAntennaArray;
    private jOTDBnode itsBandFilter;
    private jOTDBnode itsClockMode;
    private jOTDBnode itsNyquistZone;
    private jOTDBnode itsBeamletList;
    private jOTDBnode itsReceiverList;
    private jOTDBnode itsSubbandList;
  
    
    // Observation Beam parameters
    private jOTDBnode itsAngle1;
    private jOTDBnode itsAngle2;
    private jOTDBnode itsAngleTimes;
    private jOTDBnode itsDirectionTypes;
    
    // Observation VirtualObservation parameters
    private jOTDBnode itsStationList;


    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton addStationButton;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JButton deleteStationButton;
    private javax.swing.JScrollPane descriptionScrollPane;
    private javax.swing.JTextField inputAngle1;
    private javax.swing.JTextField inputAngle2;
    private javax.swing.JTextField inputAngleTimes;
    private javax.swing.JComboBox inputAntennaArray;
    private javax.swing.JComboBox inputBandFilter;
    private javax.swing.JTextField inputBeamletList;
    private javax.swing.JComboBox inputClockMode;
    private javax.swing.JTextArea inputDescription;
    private javax.swing.JComboBox inputDirectionTypes;
    private javax.swing.JTextField inputMSNameMask;
    private javax.swing.JComboBox inputNyquistZone;
    private javax.swing.JTextField inputReceiverList;
    private javax.swing.JTextField inputSubbandList;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel10;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanel5;
    private javax.swing.JPanel jPanel7;
    private javax.swing.JLabel labelAngle1;
    private javax.swing.JLabel labelAngle2;
    private javax.swing.JLabel labelAngleTimes;
    private javax.swing.JLabel labelAntennaArray;
    private javax.swing.JLabel labelBandFilter;
    private javax.swing.JLabel labelBeamletList;
    private javax.swing.JLabel labelClockMode;
    private javax.swing.JLabel labelDirectionTypes;
    private javax.swing.JLabel labelMSNameMask;
    private javax.swing.JLabel labelNyquistZone;
    private javax.swing.JLabel labelReceiverList;
    private javax.swing.JPanel labelStationList;
    private javax.swing.JLabel labelSubbandList;
    private javax.swing.JComboBox modifyStationsCombobox;
    private javax.swing.JPanel stationsButtonPanel;
    private javax.swing.JList stationsList;
    private javax.swing.JPanel stationsModPanel;
    private javax.swing.JPanel stationsPanel;
    private javax.swing.JScrollPane stationsScrollPane;
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
