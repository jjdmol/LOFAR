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
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.rmi.RemoteException;
import java.util.Arrays;
import java.util.BitSet;
import java.util.Collection;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.DefaultListModel;
import javax.swing.JFileChooser;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.ListSelectionModel;
import javax.swing.border.TitledBorder;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.tablemodels.BeamConfigurationTableModel;
import nl.astron.lofar.sas.otb.util.tablemodels.BeamformerConfigurationTableModel;
import nl.astron.lofar.sas.otbcomponents.BeamDialog;
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
    
    static Logger logger = Logger.getLogger(ObservationPanel.class);    
    static String name = "ObservationPanel";
    
   
     /** Creates new form BeanForm based upon aNode
     *
     * @params  aNode   node to obtain the info from
     *
     */
    public ObservationPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
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
        } else {
            logger.debug("No Mainframe supplied");
        }
    }
    
    public String getShortName() {
        return name;
    }
    
    public void setContent(Object anObject) {   
        itsNode=(jOTDBnode)anObject;
        
        //fire up filling for AntennaConfigPanel
        this.antennaConfigPanel.setMainFrame(this.itsMainFrame);
        this.antennaConfigPanel.setContent(this.itsNode);

        jOTDBparam aParam=null;
        try {
            // get old tree description
            itsOldTreeDescription = OtdbRmi.getRemoteOTDB().getTreeInfo(itsNode.treeID(),false).description;
            inputTreeDescription.setText(itsOldTreeDescription);   
            
            //we need to get all the childs from this node.    
            Vector childs = OtdbRmi.getRemoteMaintenance().getItemList(itsNode.treeID(), itsNode.nodeID(), 1);
            
            // get all the params per child
            Enumeration e = childs.elements();
            while( e.hasMoreElements()  ) {
                aParam=null;
            
                jOTDBnode aNode = (jOTDBnode)e.nextElement();
                        
                // We need to keep all the nodes needed by this panel
                // if the node is a leaf we need to get the pointed to value via Param.
                if (aNode.leaf) {
                    aParam = OtdbRmi.getRemoteMaintenance().getParam(aNode);
                    setField(itsNode,aParam,aNode);
                //we need to get all the childs from the following nodes as well.
                }else if (LofarUtils.keyName(aNode.name).contains("Beam") && !LofarUtils.keyName(aNode.name).contains("Beamformer")) {
                    itsBeams.addElement(aNode);
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }else if (LofarUtils.keyName(aNode.name).contains("Beamformer")) {
                    itsBeamformers.addElement(aNode);
                    this.retrieveAndDisplayChildDataForNode(aNode);
                } else if (LofarUtils.keyName(aNode.name).equals("VirtualInstrument")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }
            }
        } catch (RemoteException ex) {
            logger.debug("Error during getComponentParam: "+ ex);
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
        return true;
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
        
        aPopupMenu= new JPopupMenu();
        // For VIC trees
        if (itsTreeType.equals("VHtree")) {
            //  Fill in menu as in the example above
            aMenuItem=new JMenuItem("Create ParSet File");        
            aMenuItem.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    popupMenuHandler(evt);
                }
            });
            aMenuItem.setActionCommand("Create ParSet File");
            aPopupMenu.add(aMenuItem);
            
        // For template trees
        } else if (itsTreeType.equals("VItemplate")) {
                
        }
        
        aPopupMenu.setOpaque(true);
        aPopupMenu.show(aComponent, x, y );       
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
         if (evt.getActionCommand().equals("Create ParSet File")) {
            logger.debug("Create ParSet File");
            int aTreeID=itsMainFrame.getSharedVars().getTreeID();
            if (fc == null) {
                fc = new JFileChooser();
                fc.setApproveButtonText("Save");
            }
            // try to get a new filename to write the parsetfile to
            if (fc.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
                try {
                    File aFile = fc.getSelectedFile();
                    
                    // create filename that can be used at the remote site    
                    String aRemoteFileName="/tmp/"+aTreeID+"-"+itsNode.name+"_"+itsMainFrame.getUserAccount().getUserName()+".ParSet";
                    
                    // write the parset
                    OtdbRmi.getRemoteMaintenance().exportTree(aTreeID,itsNode.nodeID(),aRemoteFileName,2,false); 
                    
                    //obtain the remote file
                    byte[] dldata = OtdbRmi.getRemoteFileTrans().downloadFile(aRemoteFileName);

                    BufferedOutputStream output = new BufferedOutputStream(new FileOutputStream(aFile));
                    output.write(dldata,0,dldata.length);
                    output.flush();
                    output.close();
                    logger.debug("File written to: " + aFile.getPath());
                } catch (RemoteException ex) {
                    logger.debug("exportTree failed : " + ex);
                } catch (FileNotFoundException ex) {
                    logger.debug("Error during newPICTree creation: "+ ex);
                } catch (IOException ex) {
                    logger.debug("Error during newPICTree creation: "+ ex);
                }
            }
        }       
    }
    
     /** 
     * Helper method that retrieves the child nodes for a given jOTDBnode, 
     * and triggers setField() accordingly.
     * @param aNode the node to retrieve and display child data of.
     */
    private void retrieveAndDisplayChildDataForNode(jOTDBnode aNode){
        jOTDBparam aParam=null;
        try {
            Vector HWchilds = OtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1);
            // get all the params per child
            Enumeration e1 = HWchilds.elements();
            while( e1.hasMoreElements()  ) {
                
                jOTDBnode aHWNode = (jOTDBnode)e1.nextElement();
                aParam=null;
                // We need to keep all the params needed by this panel
                if (aHWNode.leaf) {
                    aParam = OtdbRmi.getRemoteMaintenance().getParam(aHWNode);
                }
                setField(aNode,aParam,aHWNode);
            }
        } catch (RemoteException ex) {
            logger.debug("Error during retrieveAndDisplayChildDataForNode: "+ ex);
            return;
        }
    }
    
    /**
     * Sets the different fields in the GUI, using the names of the nodes provided
     * @param parent the parent node of the node to be displayed
     * @param aParam the parameter of the node to be displayed if applicable
     * @param aNode  the node to be displayed
     */
    private void setField(jOTDBnode parent,jOTDBparam aParam, jOTDBnode aNode) {

        // Generic Observation
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
            logger.debug("Error during getParam: "+ ex);
        }
        
        if(parentName.equals("Observation")){        
        // Observation Specific parameters
            if (aKeyName.equals("MSNameMask")) {
                inputMSNameMask.setToolTipText(aParam.description);
               itsMSNameMask=aNode;
                if (isRef && aParam != null) {
                    inputMSNameMask.setText(aNode.limits + " : " + aParam.limits);
                } else {
                    inputMSNameMask.setText(aNode.limits);
                }
            } else if (aKeyName.equals("receiverList")) {
                inputReceiverList.setToolTipText(aParam.description);
                itsReceiverList=aNode;
                if (isRef && aParam != null) {
                    inputReceiverList.setText(aNode.limits + " : " + aParam.limits);
               } else {
                    inputReceiverList.setText(aNode.limits);
               }
            } else if (aKeyName.equals("nrSlotsInFrame")) {        
                inputNrSlotsInFrame.setToolTipText(aParam.description);
                itsNrSlotsInFrame=aNode;
                if (isRef && aParam != null) {
                    inputNrSlotsInFrame.setText(aNode.limits + " : " + aParam.limits);
               } else {
                    inputNrSlotsInFrame.setText(aNode.limits);
               }
            } else if (aKeyName.equals("nrBeams")) {
                itsNrBeams=aNode;
            } else if (aKeyName.equals("nrBeamformers")) {
                itsNrBeamformers=aNode;
            }
        } else if(parentName.contains("Beam") && !parentName.contains("Beamformer")){        
            // Observation Beam parameters
            if (aKeyName.equals("angle1")) {        
                if (isRef && aParam != null) {
                    itsAngle1.add(aNode.limits + " : " + aParam.limits);
                } else {
                    itsAngle1.add(aNode.limits);
                }
            } else if (aKeyName.equals("angle2")) {        
                if (isRef && aParam != null) {
                    itsAngle2.add(aNode.limits + " : " + aParam.limits);
                } else {
                    itsAngle2.add(aNode.limits);
                }
            } else if (aKeyName.equals("directionTypes")) { 
                itsDirectionTypeChoices=aParam.limits;
                itsDirectionTypes.add(aNode.limits);
            } else if (aKeyName.equals("beamletList")) {        
                if (isRef && aParam != null) {
                    itsBeamletList.add(aNode.limits + " : " + aParam.limits);
               } else {
                    itsBeamletList.add(aNode.limits);
               }
            } else if (aKeyName.equals("subbandList")) {        
                if (isRef && aParam != null) {
                    itsSubbandList.add(aNode.limits + " : " + aParam.limits);
                } else {
                    itsSubbandList.add(aNode.limits);
                }                
            }
        } else if(parentName.contains("Beamformer")){        
            // Observation Beamformer parameters
            if (aKeyName.equals("stationList")) {        
                if (isRef && aParam != null) {
                    itsStations.add(aNode.limits + " : " + aParam.limits);
                } else {
                    itsStations.add(aNode.limits);
                }
                
                // add found stations to usedStationList
                String[] aList = aNode.limits.split("[,]");
                for (int i=0; i < aList.length;i++ ) {
                  itsUsedBeamformStations.add(aList[i]);
                }
            }
        } else if(parentName.equals("VirtualInstrument")){        
            // Observation VirtualInstrument parameters

            if (aKeyName.equals("stationList")) {        
                this.stationList.setToolTipText(aParam.description);
                this.itsStationList = aNode;

                //set the checkbox correctly when no stations are provided in the data
                if(itsStationList.limits == null || itsStationList.limits.equals("[]")){
                    stationList.setModel(new DefaultListModel());
                }else{
                    TitledBorder aBorder = (TitledBorder)this.stationsPanel.getBorder();
                    if (isRef && aParam != null) {
                        aBorder.setTitle("Station Names (Referenced)");
                        LofarUtils.fillList(stationList,aParam.limits,false);
                    } else {
                        aBorder.setTitle("Station Names");
                        LofarUtils.fillList(stationList,aNode.limits,false);
                    }
                }
            }
        }
    }

    /** Fill the Selectable stations for the Beamformer input
     * 
     * Every time the stationList changes we need to fill this list again.
     * while filling we have to check if the removed station are not filled
     * in a beamformer allready. If so show warning and remove the station from
     * that list.
     * Further we need to fill this list only with the station in the station list 
     * as far as they are not allready used in the beamformerlists 
     */
    private void fillBeamformerStationList() {
        
        itsAvailableBeamformStations.clear();
        int size = stationList.getModel().getSize();
        String[] tempList=new String[size];
        DefaultListModel itsModel = new DefaultListModel();
        beamformerStationList.setModel(itsModel);
        // get the  available virtual instrument station list
        for (int i=0; i< size; i++) {
            String station = stationList.getModel().getElementAt(i).toString();
            
            // Check if station is not allready used 
            if (itsUsedBeamformStations.indexOf(station) < 0) {
                tempList[i] = station; 
                itsModel.addElement(station);
                // check if station is not allready there
                if (itsAvailableBeamformStations.indexOf(station)<0){
                    itsAvailableBeamformStations.addElement(station);
                }
            }
        }
        if(itsAvailableBeamformStations.isEmpty()) {
            addBeamformerButton.setEnabled(false);
        } else {
            addBeamformerButton.setEnabled(true);
        }
    }
    
    /* add a station to the active beamformer
     */
    void addBeamformerStation(String aStation) {
        // check if a selection in the beamformer table is active.
        if (aStation == null) {
            return;
        }
        int row=beamformerConfigurationPanel.getSelectedRow();
        if (row < 0  || aStation.equals("")) {
            return;
        }
        
        String selection = itsBeamformerConfigurationTableModel.getSelection(row);
        if (selection != null) {
            if (!selection.equals("")){
                selection = selection.concat(",");            
            }
            selection = selection.concat(aStation);
            itsUsedBeamformStations.add(aStation);
            itsAvailableBeamformStations.remove(aStation);
            itsBeamformerConfigurationTableModel.updateRow(selection,row);
            beamformerConfigurationPanel.setSelectedRow(row, row);
            fillBeamformerStationList();
        }
    }

    /** Restore original Values in  panel
     */
    private void restore() {

      // Observation Specific parameters
      inputMSNameMask.setText(itsMSNameMask.limits);
      inputReceiverList.setText(itsReceiverList.limits);
      inputNrSlotsInFrame.setText(itsNrSlotsInFrame.limits);
      inputDescription.setText("");
      inputTreeDescription.setText(itsOldTreeDescription);
    
      if (!itsTreeType.equals("VHtree")) {
         // Observation Beam parameters
         // create original Beamlet Bitset
         fillBeamletBitset();
      }
      // set table back to initial values
      itsBeamConfigurationTableModel.fillTable(itsTreeType,itsDirectionTypes,itsAngle1,itsAngle2,itsSubbandList,itsBeamletList);
      
      itsBeamformerConfigurationTableModel.fillTable(itsTreeType, itsStations);
      
      for (int i=0; i<itsStations.size();i++){
        String[] involvedStations = itsStations.elementAt(i).split("[,]");
        for (int j = 0; j < involvedStations.length;j++) {
            if (!itsUsedBeamformStations.contains(involvedStations[j])) {
              itsUsedBeamformStations.add(involvedStations[j]);
            }
        }
      }
      
      
      // Observation VirtualInstrument parameters
      //set the checkbox correctly when no stations are provided in the data
      if(itsStationList.limits == null || itsStationList.limits.equals("[]")){
        stationList.setModel(new DefaultListModel());
      }else{
        TitledBorder aBorder = (TitledBorder)this.stationsPanel.getBorder();
        aBorder.setTitle("Station Names");
        LofarUtils.fillList(stationList,itsStationList.limits,false);
      }
      
      fillBeamformerStationList();
      
      if (beamConfigurationPanel.getTableModel().getRowCount() == 8) {
        this.addBeamButton.setEnabled(false);
      } else {
        this.addBeamButton.setEnabled(true);
      }
     // also restore antennaConfigPanel
      antennaConfigPanel.restore();
    }

    
    /** fill the Beamlet bitset to see what Beamlets have been set. To be able to determine later if a given Beamlet is indeed free.
     */
    private void fillBeamletBitset() {
        itsUsedBeamlets.clear();
        for (int i=1;i<itsBeamletList.size();i++) {
            BitSet aNewBitSet=LofarUtils.beamletToBitSet(LofarUtils.expandedArrayString(itsBeamletList.elementAt(i)));
            
            // check if no duplication between the two bitsets
            if (itsUsedBeamlets.intersects(aNewBitSet)) {
                String errorMsg = "ERROR:  This BeamletList has beamlets defined that are allready used in a prior BeamConfiguration!!!!!  BeamNr: "+i;
                JOptionPane.showMessageDialog(this,errorMsg,"BeamletError",JOptionPane.ERROR_MESSAGE);
                logger.error(errorMsg );
                return;
            }
            
            // No intersection, both bitsets can be or
            itsUsedBeamlets.or(aNewBitSet);
        }
    }
    
     
    private void initialize() {
        buttonPanel1.addButton("Restore");
        buttonPanel1.addButton("Apply");
        this.stationList.setModel(new DefaultListModel());
        this.beamformerStationList.setModel(new DefaultListModel());

        
      
        itsBeamConfigurationTableModel = new BeamConfigurationTableModel();
        beamConfigurationPanel.setTableModel(itsBeamConfigurationTableModel);
        beamConfigurationPanel.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        beamConfigurationPanel.setColumnSize("dirtype",20);
        beamConfigurationPanel.setColumnSize("angle 1",20);
        beamConfigurationPanel.setColumnSize("angle 2",20);
        beamConfigurationPanel.setColumnSize("subbands",200);
        beamConfigurationPanel.setColumnSize("beamlets",200);
        beamConfigurationPanel.repaint();
        
        itsBeamformerConfigurationTableModel = new BeamformerConfigurationTableModel();
        beamformerConfigurationPanel.setTableModel(itsBeamformerConfigurationTableModel);
        beamformerConfigurationPanel.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        beamformerConfigurationPanel.setColumnSize("Beamformer",20);
        beamformerConfigurationPanel.setColumnSize("Stations",200);
        beamformerConfigurationPanel.repaint();

    }
    
    private void initPanel() {

        itsMainFrame.setHourglassCursor();

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
                logger.debug("ObservationPanel: Error getting treeInfo/treetype" + ex);
                itsTreeType="";
            }         } else {
            logger.debug("ERROR:  no node given");
        }
        
        // set defaults
        // create initial beamletBitset
        // create initial table
        restore();
        
        for (int i=0; i < stationList.getModel().getSize();i++) {
          if (! itsUsedBeamformStations.contains(stationList.getModel().getElementAt(i)) &&
                  itsAvailableBeamformStations.indexOf(stationList.getModel().getElementAt(i).toString())< 0) {
              itsAvailableBeamformStations.add(stationList.getModel().getElementAt(i).toString());
          }
        }

        
        if (itsTreeType.equals("VHtree")) {
            this.setButtonsVisible(false);
            this.setAllEnabled(false);
        }

        itsMainFrame.setNormalCursor();

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
            logger.debug("Error: saveNode failed : " + ex);
        } 
    }
    
    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
        addBeamButton.setEnabled(enabled);
        editBeamButton.setEnabled(enabled);
        deleteBeamButton.setEnabled(enabled);
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
        addBeamButton.setVisible(visible);
        editBeamButton.setVisible(visible);
        deleteBeamButton.setVisible(visible);
    }        
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
        this.inputDescription.setEnabled(enabled);
        this.inputMSNameMask.setEnabled(enabled);
        this.inputReceiverList.setEnabled(enabled);
        this.inputTreeDescription.setEnabled(enabled);
    }
    
    private boolean saveInput() {
        // Beam        
        int i=0;
        //delete all Beams from the table (excluding the Default one); 
        
        // Keep the 1st one, it's the default Beam
        try {
            for (i=1; i< itsBeams.size(); i++) {
                OtdbRmi.getRemoteMaintenance().deleteNode(itsBeams.elementAt(i));  
            }        
        } catch (RemoteException ex) {
            logger.error("Error during deletion of defaultNode: "+ex);
            return false;
        }  
        
        // now that all Nodes are deleted we should collect the tables input and create new Beams to save to the database.
        itsBeamConfigurationTableModel.getTable(itsDirectionTypes,itsAngle1,itsAngle2,itsSubbandList,itsBeamletList);
        // keep defaultTBBsetting save
        jOTDBnode aDefaultNode= itsBeams.elementAt(0);
        itsBeams.clear();        
        try {
            // for all elements
            for (i=1; i < itsDirectionTypes.size();i++) {
        
                // make a dupnode from the default node, give it the next number in the count,get the elements and fill all values from the elements
                // with the values from the set fields and save the elements again
                //
                // Duplicates the given node (and its parameters and children)
                int aN = OtdbRmi.getRemoteMaintenance().dupNode(itsNode.treeID(),aDefaultNode.nodeID(),(short)(i-1));
                if (aN <= 0) {
                    logger.error("Something went wrong with duplicating tree no ("+i+") will try to save remainder");
                } else {
                    // we got a new duplicate whos children need to be filled with the settings from the panel.
                    jOTDBnode aNode = OtdbRmi.getRemoteMaintenance().getNode(itsNode.treeID(),aN);
                    // store new duplicate in itsBeams.
                    itsBeams.add(aNode);
                
                    Vector HWchilds = OtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1);
                    // get all the params per child
                    Enumeration e1 = HWchilds.elements();
                    while( e1.hasMoreElements()  ) {
                        jOTDBnode aHWNode = (jOTDBnode)e1.nextElement();
                        String aKeyName = LofarUtils.keyName(aHWNode.name);
                        if (aKeyName.equals("directionTypes")) {
                            aHWNode.limits=itsDirectionTypes.elementAt(i);
                        } else if (aKeyName.equals("angle1")) {
                            aHWNode.limits=itsAngle1.elementAt(i);
                        } else if (aKeyName.equals("angle2")) {
                            aHWNode.limits=itsAngle2.elementAt(i);
                        } else if (aKeyName.equals("subbandList")) {
                            aHWNode.limits=itsSubbandList.elementAt(i);
                        } else if (aKeyName.equals("beamletList")) {
                            aHWNode.limits=itsBeamletList.elementAt(i);
                        }
                        saveNode(aHWNode);
                    }
                }
            }
            
            // store new number of instances in baseSetting
            aDefaultNode.instances=(short)(itsDirectionTypes.size()-1); // - default at -1 
            saveNode(aDefaultNode);

        } catch (RemoteException ex) {
            logger.error("Error during duplication and save : " + ex);
            return false;
        }

        // same for beamformer
        i=0;
        //delete all Beamformers from the table (excluding the Default one); 
        
        // Keep the 1st one, it's the default Beam
        try {
            for (i=1; i< itsBeamformers.size(); i++) {
                OtdbRmi.getRemoteMaintenance().deleteNode(itsBeamformers.elementAt(i));  
            }        
        } catch (RemoteException ex) {
            logger.error("Error during deletion of defaultNode: "+ex);
            return false;
        }  

        itsBeamformerConfigurationTableModel.getTable(itsStations);
        // keep default save
        jOTDBnode aDefaultBFNode= itsBeamformers.elementAt(0);
        itsBeamformers.clear(); 
        // validate table
        for (int j=0; j< itsStations.size(); j++) {
            if (itsStations.get(j).equals("")) {
                itsStations.remove(j);
            }
        }
        try {
            // for all elements
            for (i=1; i < itsStations.size();i++) {
        
                // make a dupnode from the default node, give it the next number in the count,get the elements and fill all values from the elements
                // with the values from the set fields and save the elements again
                //
                // Duplicates the given node (and its parameters and children)
                int aN = OtdbRmi.getRemoteMaintenance().dupNode(itsNode.treeID(),aDefaultBFNode.nodeID(),(short)(i-1));
                if (aN <= 0) {
                    logger.error("Something went wrong with duplicating tree no ("+i+") will try to save remainder");
                } else {
                    // we got a new duplicate whos children need to be filled with the settings from the panel.
                    jOTDBnode aNode = OtdbRmi.getRemoteMaintenance().getNode(itsNode.treeID(),aN);
                    // store new duplicate in itsBeamformers.
                    itsBeamformers.add(aNode);
                
                    Vector HWchilds = OtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1);
                    // get all the params per child
                    Enumeration e1 = HWchilds.elements();
                    while( e1.hasMoreElements()  ) {
                        jOTDBnode aHWNode = (jOTDBnode)e1.nextElement();
                        String aKeyName = LofarUtils.keyName(aHWNode.name);
                        if (aKeyName.equals("stationList")) {
                            aHWNode.limits=itsStations.elementAt(i);
                        }
                        saveNode(aHWNode);
                    }
                }
            }
            
            // store new number of instances in baseSetting
            aDefaultBFNode.instances=(short)(itsStations.size()-1); // - default at -1
            saveNode(aDefaultBFNode);

        } catch (RemoteException ex) {
            logger.error("Error during duplication and save : " + ex);
            return false;
        }
        
        

        
        // Generic Observation
        if (itsMSNameMask != null && !this.inputMSNameMask.getText().equals(itsMSNameMask.limits)) {
            itsMSNameMask.limits = inputMSNameMask.getText();
            saveNode(itsMSNameMask);
        }
        if (itsReceiverList != null && !inputReceiverList.getText().equals(itsReceiverList.limits)) {
            itsReceiverList.limits = inputReceiverList.getText();
            saveNode(itsReceiverList);
        }
        if (itsNrSlotsInFrame != null && !inputNrSlotsInFrame.getText().equals(itsNrSlotsInFrame.limits)) {
            itsNrSlotsInFrame.limits = inputNrSlotsInFrame.getText();
            saveNode(itsNrSlotsInFrame);
        }
        if (itsNrBeams != null && !Integer.toString(beamConfigurationPanel.getTableModel().getRowCount()).equals(itsNrBeams.limits)) {
            itsNrBeams.limits = Integer.toString(beamConfigurationPanel.getTableModel().getRowCount());
            saveNode(itsNrBeams);
        }
        
        if (itsNrBeamformers != null && !Integer.toString(beamformerConfigurationPanel.getTableModel().getRowCount()).equals(itsNrBeamformers.limits)) {
            itsNrBeamformers.limits = Integer.toString(beamformerConfigurationPanel.getTableModel().getRowCount());
            saveNode(itsNrBeamformers);
        }
        // treeDescription
        if (itsOldTreeDescription != null && !inputTreeDescription.getText().equals(itsOldTreeDescription)) {
            try {
                if (!OtdbRmi.getRemoteMaintenance().setDescription(itsNode.treeID(), inputTreeDescription.getText())) {
                    logger.debug("Error during setDescription: "+OtdbRmi.getRemoteMaintenance().errorMsg());                        
                } 
            } catch (RemoteException ex) {
                logger.debug("Error: saveNode failed : " + ex);
            } 
            itsOldTreeDescription = inputTreeDescription.getText();
            itsMainFrame.setChanged("Home",true);
            itsMainFrame.setChanged("Template_Maintenance("+itsNode.treeID()+")" ,true);
            itsMainFrame.checkChanged("Template_Maintenance("+itsNode.treeID()+")");
        }

        return true;   
    }
    
    private void changeDescription(jOTDBnode aNode) {
        if (aNode == null) {
            return;
        }
        try {
            jOTDBparam aParam = OtdbRmi.getRemoteMaintenance().getParam(aNode);
            // check if the node changed, and if the description was changed, if so ask if the new description
            // should be saved.
            if (itsOldDescriptionParam == null | itsOldDescriptionParam !=aParam) {
                if (itsOldDescriptionParam != null) {
                    if (!inputDescription.getText().equals(itsOldDescriptionParam.description) && !inputDescription.getText().equals("")) {
                        int answer=JOptionPane.showConfirmDialog(this,"The old description was altered, do you want to save the old one ?","alert",JOptionPane.YES_NO_OPTION);
                        if (answer == JOptionPane.YES_OPTION) {
                            if (!OtdbRmi.getRemoteMaintenance().saveParam(itsOldDescriptionParam)) {
                                logger.error("Saving param "+itsOldDescriptionParam.nodeID()+","+itsOldDescriptionParam.paramID()+"failed: "+ OtdbRmi.getRemoteMaintenance().errorMsg());
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

    private void deleteBeamformer() {
        String selection = itsBeamformerConfigurationTableModel.getSelection(beamformerConfigurationPanel.getSelectedRow());
        if (selection== null) {
            return;
        }
        String[] involvedStations = selection.split("[,]");
        for (int j = 0; j < involvedStations.length;j++) {
            itsUsedBeamformStations.remove(involvedStations[j]);
        }
        itsBeamformerConfigurationTableModel.removeRow(beamformerConfigurationPanel.getSelectedRow());
        Vector<String> sl=new Vector<String>();
        itsBeamformerConfigurationTableModel.getTable(sl);
        itsBeamformerConfigurationTableModel.fillTable(itsTreeType, sl);
        fillBeamformerStationList();
    }
    
    private void deleteBeam() {
        int row = beamConfigurationPanel.getSelectedRow();
        // if removed then the old Beamlets's should be removed form the checklist also
        String oldBeamlets = itsBeamConfigurationTableModel.getSelection(row)[4];
        BitSet beamletSet = LofarUtils.beamletToBitSet(LofarUtils.expandedArrayString(oldBeamlets));
        
        if (JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this Beam ?","Delete Beam",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION ) {
            if (row > -1) {
                itsBeamConfigurationTableModel.removeRow(row);
                itsUsedBeamlets.xor(beamletSet);
                // No selection anymore after delete, so buttons disabled again
                this.editBeamButton.setEnabled(false);
                this.deleteBeamButton.setEnabled(false);


            }
        } 
        
      if (beamConfigurationPanel.getTableModel().getRowCount() == 8) {
        this.addBeamButton.setEnabled(false);
      } else {
        this.addBeamButton.setEnabled(true);
      }
    }
    
    private void addBeam() {
     
        BitSet aBS=itsUsedBeamlets;
        itsSelectedRow=-1;
        // set selection to defaults.
        String [] selection = {itsDirectionTypes.elementAt(0),itsAngle1.elementAt(0),
                               itsAngle2.elementAt(0),itsSubbandList.elementAt(0),itsBeamletList.elementAt(0)};
        if (editting) {
            itsSelectedRow = beamConfigurationPanel.getSelectedRow();
            selection = itsBeamConfigurationTableModel.getSelection(itsSelectedRow);
                       
            BitSet oldBeamlets = LofarUtils.beamletToBitSet(LofarUtils.expandedArrayString(selection[4]));
            aBS.xor(oldBeamlets);
            // if no row is selected, nothing to be done
            if (selection == null || selection[0].equals("")) {
                return;
            }
        }
        beamDialog = new BeamDialog(itsMainFrame,true,aBS,selection,itsDirectionTypeChoices,editting);
        beamDialog.setLocationRelativeTo(this);
        if (editting) {
            beamDialog.setBorderTitle("edit Beam");
        } else {
            beamDialog.setBorderTitle("add new Beam");            
        }
        beamDialog.setVisible(true);
        
        // check if something has changed 
        if (beamDialog.hasChanged()) {
            String[] newRow = beamDialog.getBeam();
            itsUsedBeamlets=beamDialog.getBeamletList();
            // check if we are editting an entry or adding a new entry
            if (editting) {
                itsBeamConfigurationTableModel.updateRow(newRow,itsSelectedRow);
                // set editting = false
                editting=false;
            } else {            
                itsBeamConfigurationTableModel.addRow(newRow);
            }
        }
        
        this.editBeamButton.setEnabled(false);
        this.deleteBeamButton.setEnabled(false);
        if (beamConfigurationPanel.getTableModel().getRowCount() == 8 ) {
            this.addBeamButton.setEnabled(false);
        } else {
            this.addBeamButton.setEnabled(true);
        }
        
    }
    
    private void checkBeamformers(Object[] stations ){

        Vector<Integer> delrows=new Vector<Integer>();
        Vector<String> aVS= new Vector<String>();
        for(int k=0; k< stations.length;k++) {
            aVS.add((String)stations[k]);
        }
        this.itsBeamformerConfigurationTableModel.getTable(itsStations);
        
        for (int i = 1; i< itsStations.size();i++) {
            String sl = itsStations.get(i);
            Collection s = Arrays.asList(sl.split("[,]"));
            if (aVS.containsAll(s)) {
                break;
            }
            if (delrows != null && !delrows.contains(i-1)) {
                delrows.add(i-1);
            }                
        }
        // check if we found matches with stations that no longer exist, if so the beamformer involved
        // will be deleted.
        
        for (int i=0; i < delrows.size(); i++) {
            beamformerConfigurationPanel.setSelectedRow(delrows.get(i),delrows.get(i));
            deleteBeamformer();
        }        
    }

    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jToolBar1 = new javax.swing.JToolBar();
        jPanel1 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        jTabbedPane1 = new javax.swing.JTabbedPane();
        antennaConfigPanel = new nl.astron.lofar.sas.otbcomponents.AntennaConfigPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        jPanel2 = new javax.swing.JPanel();
        treeDescriptionScrollPane = new javax.swing.JScrollPane();
        inputTreeDescription = new javax.swing.JTextArea();
        descriptionScrollPane = new javax.swing.JScrollPane();
        inputDescription = new javax.swing.JTextArea();
        jPanel10 = new javax.swing.JPanel();
        labelMSNameMask = new javax.swing.JLabel();
        inputMSNameMask = new javax.swing.JTextField();
        labelNrSlotsInFrame = new javax.swing.JLabel();
        inputNrSlotsInFrame = new javax.swing.JTextField();
        inputReceiverList = new javax.swing.JTextField();
        labelReceiverList = new javax.swing.JLabel();
        jPanel3 = new javax.swing.JPanel();
        beamConfigurationPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        addBeamButton = new javax.swing.JButton();
        editBeamButton = new javax.swing.JButton();
        deleteBeamButton = new javax.swing.JButton();
        jPanel5 = new javax.swing.JPanel();
        stationsPanel = new javax.swing.JPanel();
        stationsScrollPane = new javax.swing.JScrollPane();
        stationList = new javax.swing.JList();
        stationsModPanel = new javax.swing.JPanel();
        stationsButtonPanel = new javax.swing.JPanel();
        jPanel4 = new javax.swing.JPanel();
        beamformerConfigurationPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        jLabel2 = new javax.swing.JLabel();
        addBeamformerButton = new javax.swing.JButton();
        deleteBeamformerButton = new javax.swing.JButton();
        jLabel3 = new javax.swing.JLabel();
        jLabel4 = new javax.swing.JLabel();
        jScrollPane2 = new javax.swing.JScrollPane();
        beamformerStationsScrollPane = new javax.swing.JScrollPane();
        beamformerStationList = new javax.swing.JList();
        buttonPanel1 = new nl.astron.lofar.sas.otbcomponents.ButtonPanel();

        jToolBar1.setRollover(true);

        setLayout(new java.awt.BorderLayout());

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder(""));
        jPanel1.setPreferredSize(new java.awt.Dimension(100, 25));
        jPanel1.setLayout(new java.awt.BorderLayout());

        jLabel1.setFont(new java.awt.Font("Tahoma", 1, 11));
        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("Observation Details");
        jPanel1.add(jLabel1, java.awt.BorderLayout.CENTER);

        add(jPanel1, java.awt.BorderLayout.NORTH);

        jTabbedPane1.addChangeListener(new javax.swing.event.ChangeListener() {
            public void stateChanged(javax.swing.event.ChangeEvent evt) {
                jTabbedPane1StateChanged(evt);
            }
        });

        antennaConfigPanel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                antennaConfigPanelActionPerformed(evt);
            }
        });
        jTabbedPane1.addTab("Station", antennaConfigPanel);

        jPanel2.setBorder(javax.swing.BorderFactory.createTitledBorder(""));

        treeDescriptionScrollPane.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Observation Tree Description", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N

        inputTreeDescription.setColumns(20);
        inputTreeDescription.setRows(5);
        inputTreeDescription.setToolTipText("The description set here will go to the Tree Description");
        treeDescriptionScrollPane.setViewportView(inputTreeDescription);

        descriptionScrollPane.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Field Descriptions.", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N

        inputDescription.setColumns(20);
        inputDescription.setEditable(false);
        inputDescription.setRows(5);
        descriptionScrollPane.setViewportView(inputDescription);

        jPanel10.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Generic Observation Input", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N

        labelMSNameMask.setText("MSNameMask:");

        inputMSNameMask.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                inputMSNameMaskFocusGained(evt);
            }
        });

        labelNrSlotsInFrame.setText("# Slots In Frame");

        inputNrSlotsInFrame.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                inputNrSlotsInFrameFocusGained(evt);
            }
        });

        inputReceiverList.setToolTipText("This field will be removes as soon as the input in the AntennaConfig tab wil be used for receiver selection.");
        inputReceiverList.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                inputReceiverListFocusGained(evt);
            }
        });

        labelReceiverList.setText("Used Receivers");

        org.jdesktop.layout.GroupLayout jPanel10Layout = new org.jdesktop.layout.GroupLayout(jPanel10);
        jPanel10.setLayout(jPanel10Layout);
        jPanel10Layout.setHorizontalGroup(
            jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel10Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(labelMSNameMask, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 103, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(labelReceiverList, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 89, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(jPanel10Layout.createSequentialGroup()
                        .add(68, 68, 68)
                        .add(labelNrSlotsInFrame)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(inputNrSlotsInFrame))
                    .add(inputMSNameMask, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 743, Short.MAX_VALUE)
                    .add(inputReceiverList)))
        );
        jPanel10Layout.setVerticalGroup(
            jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel10Layout.createSequentialGroup()
                .add(jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelMSNameMask)
                    .add(inputMSNameMask, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                .add(jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(labelNrSlotsInFrame)
                    .add(inputNrSlotsInFrame, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(18, 18, 18)
                .add(jPanel10Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(inputReceiverList, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(labelReceiverList))
                .addContainerGap(26, Short.MAX_VALUE))
        );

        jPanel3.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Beam Configuration", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N
        jPanel3.setPreferredSize(new java.awt.Dimension(200, 125));
        jPanel3.setRequestFocusEnabled(false);
        jPanel3.setVerifyInputWhenFocusTarget(false);

        beamConfigurationPanel.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                beamConfigurationPanelMouseClicked(evt);
            }
        });

        addBeamButton.setText("add beam");
        addBeamButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addBeamButtonActionPerformed(evt);
            }
        });

        editBeamButton.setText("edit beam");
        editBeamButton.setEnabled(false);
        editBeamButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                editBeamButtonActionPerformed(evt);
            }
        });

        deleteBeamButton.setText("delete beam");
        deleteBeamButton.setEnabled(false);
        deleteBeamButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                deleteBeamButtonActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout jPanel3Layout = new org.jdesktop.layout.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel3Layout.createSequentialGroup()
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel3Layout.createSequentialGroup()
                        .add(addBeamButton)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(editBeamButton)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(deleteBeamButton))
                    .add(beamConfigurationPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 858, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel3Layout.createSequentialGroup()
                .add(beamConfigurationPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 154, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel3Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(editBeamButton)
                    .add(addBeamButton)
                    .add(deleteBeamButton)))
        );

        jPanel5.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Virtual Instrument", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N

        stationsPanel.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Station Names", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0))); // NOI18N
        stationsPanel.setToolTipText("Identifiers of the participating stations.");
        stationsPanel.setLayout(new java.awt.BorderLayout());

        stationList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "1", "2", "3", "4", "5" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        stationList.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        stationList.setToolTipText("Names of the participating stations.");
        stationList.setEnabled(false);
        stationsScrollPane.setViewportView(stationList);

        stationsPanel.add(stationsScrollPane, java.awt.BorderLayout.CENTER);

        stationsModPanel.setLayout(new java.awt.BorderLayout());

        stationsButtonPanel.setLayout(new java.awt.GridBagLayout());
        stationsModPanel.add(stationsButtonPanel, java.awt.BorderLayout.SOUTH);

        stationsPanel.add(stationsModPanel, java.awt.BorderLayout.SOUTH);

        org.jdesktop.layout.GroupLayout jPanel5Layout = new org.jdesktop.layout.GroupLayout(jPanel5);
        jPanel5.setLayout(jPanel5Layout);
        jPanel5Layout.setHorizontalGroup(
            jPanel5Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel5Layout.createSequentialGroup()
                .addContainerGap()
                .add(stationsPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 217, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanel5Layout.setVerticalGroup(
            jPanel5Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(stationsPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 176, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
        );

        jPanel4.setBorder(javax.swing.BorderFactory.createTitledBorder(null, "Beamformer Input", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11), new java.awt.Color(0, 0, 0))); // NOI18N

        beamformerConfigurationPanel.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                beamformerConfigurationPanelMouseClicked(evt);
            }
        });

        jLabel2.setFont(new java.awt.Font("Tahoma", 1, 12));
        jLabel2.setText("Stations");

        addBeamformerButton.setText("add beamformer");
        addBeamformerButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                addBeamformerButtonActionPerformed(evt);
            }
        });

        deleteBeamformerButton.setText("delete beamformer");
        deleteBeamformerButton.setEnabled(false);
        deleteBeamformerButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                deleteBeamformerButtonActionPerformed(evt);
            }
        });

        jLabel3.setText("doubleclick on a station to ");

        jLabel4.setText("add it to the highlighted beamformer");

        beamformerStationList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "1", "2", "3", "4", "5" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        beamformerStationList.setToolTipText("Stations that can be used in the selected beamformer");
        beamformerStationList.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                beamformerStationListMouseClicked(evt);
            }
        });
        beamformerStationsScrollPane.setViewportView(beamformerStationList);

        jScrollPane2.setViewportView(beamformerStationsScrollPane);

        org.jdesktop.layout.GroupLayout jPanel4Layout = new org.jdesktop.layout.GroupLayout(jPanel4);
        jPanel4.setLayout(jPanel4Layout);
        jPanel4Layout.setHorizontalGroup(
            jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel4Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(beamformerConfigurationPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 397, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(jPanel4Layout.createSequentialGroup()
                        .add(addBeamformerButton)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(deleteBeamformerButton)))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jLabel2)
                    .add(jPanel4Layout.createSequentialGroup()
                        .add(24, 24, 24)
                        .add(jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(jLabel3)
                            .add(jScrollPane2, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 151, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(jLabel4))))
                .add(279, 279, 279))
        );
        jPanel4Layout.setVerticalGroup(
            jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel4Layout.createSequentialGroup()
                .add(jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(jPanel4Layout.createSequentialGroup()
                        .add(jLabel2)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .add(jScrollPane2, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 107, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(beamformerConfigurationPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 128, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel4Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                        .add(addBeamformerButton)
                        .add(deleteBeamformerButton))
                    .add(jPanel4Layout.createSequentialGroup()
                        .add(jLabel3)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(jLabel4)))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        org.jdesktop.layout.GroupLayout jPanel2Layout = new org.jdesktop.layout.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel2Layout.createSequentialGroup()
                .add(jPanel2Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel10, 0, 884, Short.MAX_VALUE)
                    .add(jPanel2Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                        .add(org.jdesktop.layout.GroupLayout.LEADING, jPanel2Layout.createSequentialGroup()
                            .add(jPanel4, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 632, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                            .add(jPanel5, 0, 242, Short.MAX_VALUE))
                        .add(org.jdesktop.layout.GroupLayout.LEADING, jPanel3, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 884, Short.MAX_VALUE))
                    .add(descriptionScrollPane, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 884, Short.MAX_VALUE)
                    .add(treeDescriptionScrollPane, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 884, Short.MAX_VALUE))
                .add(811, 811, 811))
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel2Layout.createSequentialGroup()
                .add(jPanel3, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 213, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel2Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(jPanel4, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(jPanel5, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 209, Short.MAX_VALUE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel10, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(descriptionScrollPane, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 54, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(treeDescriptionScrollPane, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 60, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(31, 31, 31))
        );

        jScrollPane1.setViewportView(jPanel2);

        jTabbedPane1.addTab("Generic", jScrollPane1);

        add(jTabbedPane1, java.awt.BorderLayout.CENTER);

        buttonPanel1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                buttonPanel1ActionPerformed(evt);
            }
        });
        add(buttonPanel1, java.awt.BorderLayout.SOUTH);
    }// </editor-fold>//GEN-END:initComponents

    private void deleteBeamButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteBeamButtonActionPerformed
        deleteBeam();
    }//GEN-LAST:event_deleteBeamButtonActionPerformed

    private void editBeamButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_editBeamButtonActionPerformed
        editting=true;
        addBeam();
    }//GEN-LAST:event_editBeamButtonActionPerformed

    private void addBeamButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addBeamButtonActionPerformed
        addBeam();
    }//GEN-LAST:event_addBeamButtonActionPerformed

    private void beamConfigurationPanelMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_beamConfigurationPanelMouseClicked
        editBeamButton.setEnabled(true);
        deleteBeamButton.setEnabled(true);
    }//GEN-LAST:event_beamConfigurationPanelMouseClicked

    private void inputMSNameMaskFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_inputMSNameMaskFocusGained
        changeDescription(itsMSNameMask);
    }//GEN-LAST:event_inputMSNameMaskFocusGained

    private void buttonPanel1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_buttonPanel1ActionPerformed
        if(evt.getActionCommand().equals("Apply")) {
            itsMainFrame.setHourglassCursor();
            saveInput();
            // also save the input from the AntennaConfig Panel
            this.antennaConfigPanel.saveInput();
            // reset all buttons, flags and tables to initial start position. So the panel now reflects the new, saved situation
            initPanel();
            itsMainFrame.setNormalCursor();
        } else if(evt.getActionCommand().equals("Restore")) {
            itsMainFrame.setHourglassCursor();
            restore();
            itsMainFrame.setNormalCursor();
        }

    }//GEN-LAST:event_buttonPanel1ActionPerformed

    private void inputNrSlotsInFrameFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_inputNrSlotsInFrameFocusGained
        changeDescription(itsNrSlotsInFrame);
    }//GEN-LAST:event_inputNrSlotsInFrameFocusGained

    private void beamformerConfigurationPanelMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_beamformerConfigurationPanelMouseClicked
        deleteBeamformerButton.setEnabled(true);
    }//GEN-LAST:event_beamformerConfigurationPanelMouseClicked

    private void deleteBeamformerButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_deleteBeamformerButtonActionPerformed
        // delete beamformer, ask confirmation
        int answer=JOptionPane.showConfirmDialog(this,"Are you sure you want to delete this beamformer?","alert",JOptionPane.YES_NO_OPTION);
        if (answer == JOptionPane.NO_OPTION) {
            return;
        }
        // get stations involved and add them back to the available station list
        
        deleteBeamformer();
    }//GEN-LAST:event_deleteBeamformerButtonActionPerformed

    private void addBeamformerButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_addBeamformerButtonActionPerformed
        String selection="";
        if (this.itsAvailableBeamformStations.size()>0){  
          itsBeamformerConfigurationTableModel.addRow(selection);
          beamformerConfigurationPanel.setSelectedRow((itsBeamformerConfigurationTableModel.getRowCount()-1),(itsBeamformerConfigurationTableModel.getRowCount()-1));
          JOptionPane.showMessageDialog(this,"New Beamformer created, add stations via doubleclick in stationslist","New Beamformer",JOptionPane.YES_OPTION);
        }
    }//GEN-LAST:event_addBeamformerButtonActionPerformed

    private void beamformerStationListMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_beamformerStationListMouseClicked
        if (evt.getClickCount() > 1) {
            //doubleClick event
            addBeamformerStation((String) beamformerStationList.getSelectedValue());
        }
    }//GEN-LAST:event_beamformerStationListMouseClicked

    private void jTabbedPane1StateChanged(javax.swing.event.ChangeEvent evt) {//GEN-FIRST:event_jTabbedPane1StateChanged
        if (jTabbedPane1.getTitleAt(jTabbedPane1.getSelectedIndex()).equals("Station")) {
        } else if (jTabbedPane1.getTitleAt(jTabbedPane1.getSelectedIndex()).equals("Generic")) {
            LofarUtils.fillList(stationList, antennaConfigPanel.getUsedStations(),false);
            DefaultListModel aM = (DefaultListModel)stationList.getModel();
            checkBeamformers(aM.toArray());
            fillBeamformerStationList();
        }
    }//GEN-LAST:event_jTabbedPane1StateChanged

    private void inputReceiverListFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event_inputReceiverListFocusGained
        changeDescription(itsReceiverList);
    }//GEN-LAST:event_inputReceiverListFocusGained

    private void antennaConfigPanelActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_antennaConfigPanelActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_antennaConfigPanelActionPerformed
    
    private jOTDBnode                         itsNode = null;
    private MainFrame                         itsMainFrame;
    private jOTDBparam                        itsOldDescriptionParam;
    private String                            itsDirectionTypeChoices;
    private String                            itsOldTreeDescription;
    private String                            itsTreeType="";
    private BeamConfigurationTableModel       itsBeamConfigurationTableModel = null;
    private BeamformerConfigurationTableModel itsBeamformerConfigurationTableModel = null;
    private JFileChooser                      fc = null;
    private BeamDialog                        beamDialog = null;
    
    // Observation Specific parameters
    private jOTDBnode itsMSNameMask;
    private jOTDBnode itsReceiverList;
    private jOTDBnode itsNrSlotsInFrame;
    private jOTDBnode itsNrBeams;
    private jOTDBnode itsNrBeamformers;
  
    
    // Beams
    private Vector<jOTDBnode> itsBeams          = new Vector<jOTDBnode>();
    // Observation Beam parameters
    private Vector<String>    itsAngle1         = new Vector<String>();
    private Vector<String>    itsAngle2         = new Vector<String>();
    private Vector<String>    itsDirectionTypes = new Vector<String>();
    private Vector<String>    itsSubbandList    = new Vector<String>();
    private Vector<String>    itsBeamletList    = new Vector<String>();
    
    // Beamformers
    private Vector<jOTDBnode> itsBeamformers    = new Vector<jOTDBnode>();
    private Vector<String>    itsStations       = new Vector<String>();
   
    // Observation Virtual Instrument parameters
    private jOTDBnode itsStationList;

    // keeps lists of available (unused)  and all used stations for Beamformer creation
    private Vector<String>    itsAvailableBeamformStations       = new Vector<String>();
    private Vector<String>    itsUsedBeamformStations            = new Vector<String>();
    // each beamlet has its bit in the bitset
    private BitSet   itsUsedBeamlets = new BitSet(216);
    private boolean  editting = false;
    private int      itsSelectedRow = -1;


    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton addBeamButton;
    private javax.swing.JButton addBeamformerButton;
    private nl.astron.lofar.sas.otbcomponents.AntennaConfigPanel antennaConfigPanel;
    private nl.astron.lofar.sas.otbcomponents.TablePanel beamConfigurationPanel;
    private nl.astron.lofar.sas.otbcomponents.TablePanel beamformerConfigurationPanel;
    private javax.swing.JList beamformerStationList;
    private javax.swing.JScrollPane beamformerStationsScrollPane;
    private nl.astron.lofar.sas.otbcomponents.ButtonPanel buttonPanel1;
    private javax.swing.JButton deleteBeamButton;
    private javax.swing.JButton deleteBeamformerButton;
    private javax.swing.JScrollPane descriptionScrollPane;
    private javax.swing.JButton editBeamButton;
    private javax.swing.JTextArea inputDescription;
    private javax.swing.JTextField inputMSNameMask;
    private javax.swing.JTextField inputNrSlotsInFrame;
    private javax.swing.JTextField inputReceiverList;
    private javax.swing.JTextArea inputTreeDescription;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel10;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanel5;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.JToolBar jToolBar1;
    private javax.swing.JLabel labelMSNameMask;
    private javax.swing.JLabel labelNrSlotsInFrame;
    private javax.swing.JLabel labelReceiverList;
    private javax.swing.JList stationList;
    private javax.swing.JPanel stationsButtonPanel;
    private javax.swing.JPanel stationsModPanel;
    private javax.swing.JPanel stationsPanel;
    private javax.swing.JScrollPane stationsScrollPane;
    private javax.swing.JScrollPane treeDescriptionScrollPane;
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
        myListenerList.add (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeActionListener(java.awt.event.ActionListener listener) {

        myListenerList.remove (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Notifies all registered listeners about the event.
     * 
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {

        if (myListenerList == null) return;
        Object[] listeners = myListenerList.getListenerList ();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed (event);
            }
        }
    }    
}
