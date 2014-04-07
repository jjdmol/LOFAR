/*
 * AntennaConfigPanel.java
 *
 * Created on 18 maart 2009, 7:38
 */

package nl.astron.lofar.sas.otbcomponents;

import java.awt.Component;
import java.awt.event.ActionEvent;
import java.rmi.RemoteException;
import java.util.ArrayList;
import javax.swing.JPanel;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBtree;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import org.apache.log4j.Logger;

/**
 *
 * @author  Coolen
 */
public class AntennaConfigPanel extends javax.swing.JPanel implements IViewPanel{
    static Logger logger = Logger.getLogger(AntennaConfigPanel.class);
    static String name = "AntennaConfigDialog";

    private jOTDBnode          itsNode = null;
    private MainFrame          itsMainFrame;
    private String             itsTreeType="";

    // AntennaConfig parameters
    private jOTDBnode itsAntennaArray;
    private jOTDBnode itsAntennaSet;
    private jOTDBnode itsBandFilter;
    private jOTDBnode itsStationList;


    // Clockmode
    private jOTDBnode itsClockMode;


    private ArrayList<String>    itsUsedCoreStations      = new ArrayList<>();
    private ArrayList<String>    itsUsedRemoteStations    = new ArrayList<>();
    private ArrayList<String>    itsUsedEuropeStations    = new ArrayList<>();


    /**
     * Creates new AntennaConfigPanel instance
     */
    public AntennaConfigPanel() {
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
    public AntennaConfigPanel(MainFrame aMainFrame,jOTDBnode aNode) {
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
     * @param anObject jOTDBnode to be displayed in the GUI.
     */
    public void setContent(Object anObject) {

        // trigger stationsettings also
        coreStationSelectionPanel.setTitle("Core");
        coreStationSelectionPanel.init();
        remoteStationSelectionPanel.setTitle("Remote");
        remoteStationSelectionPanel.init();
        europeStationSelectionPanel.setTitle("Europe");
        europeStationSelectionPanel.init();

        itsNode=(jOTDBnode)anObject;
        //it is assumed itsNode is the Observation node.
        jOTDBparam aParam=null;
        try {
            //we need to get all the childs from this node.
            ArrayList<jOTDBnode> childs = new ArrayList(OtdbRmi.getRemoteMaintenance().getItemList(itsNode.treeID(), itsNode.nodeID(), 1));

            // get all the params per child
            for (jOTDBnode aNode: childs) {
                aParam=null;

                // We need to keep all the nodes needed by this panel
                // if the node is a leaf we need to get the pointed to value via Param.
                if (aNode.leaf) {
                    aParam = OtdbRmi.getRemoteMaintenance().getParam(aNode);
                    setField(itsNode,aParam,aNode);

                    //we need to get all the childs from the following nodes as well.
                }else if (LofarUtils.keyName(aNode.name).equals("VirtualInstrument")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
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

    public jOTDBnode getStationList() {
        return itsStationList;
    }

    public boolean isHBAselected() {
        return false;
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

        //StationList
        if (this.itsStationList != null && !getUsedStations().equals(itsStationList.limits)) {
            itsStationList.limits = getUsedStations();
            logger.trace("Variable VirtualInstrumenst (" + itsStationList.name + "//" + itsStationList.treeID() + "//" + itsStationList.nodeID() + "//" + itsStationList.parentID() + "//" + itsStationList.paramDefID() + ") updating to :" + itsStationList.limits);
            saveNode(itsStationList);
        }

        // AntennaArray
        String antennaArray = "LBA";
        if (this.inputHBAAntennas.isSelected()) {
            antennaArray = "HBA";
        }
        if (itsAntennaArray != null && !antennaArray.equals(itsAntennaArray.limits)) {
            itsAntennaArray.limits = antennaArray;
            saveNode(itsAntennaArray);
        }

        //AntennaSet
        String antennaSet = "LBA_INNER";
        if (this.inputOuterCircle.isSelected()) {
          antennaSet = "LBA_OUTER";
        } else if (this.inputSparseEven.isSelected()) {
          antennaSet = "LBA_SPARSE_EVEN";
        } else if (this.inputSparseOdd.isSelected()) {
          antennaSet = "LBA_SPARSE_ODD";
        } else if (this.inputXPolesOnly.isSelected()) {
          antennaSet = "LBA_X";
        } else if (this.inputYPolesOnly.isSelected()) {
          antennaSet = "LBA_Y";
        } else if (this.inputHBAZero.isSelected()) {
          antennaSet = "HBA_ZERO";
        } else if (this.inputHBAOne.isSelected()) {
          antennaSet = "HBA_ONE";
        } else if (this.inputHBADual.isSelected()) {
          antennaSet = "HBA_DUAL";
        } else if (this.inputHBAJoined.isSelected()) {
          antennaSet = "HBA_JOINED";
        } else if (this.inputHBAZeroInner.isSelected()) {
            antennaSet = "HBA_ZERO_INNER";
        } else if (this.inputHBAOneInner.isSelected()) {
            antennaSet = "HBA_ONE_INNER";
        } else if (this.inputHBADualInner.isSelected()) {
            antennaSet = "HBA_DUAL_INNER";
        } else if (this.inputHBAJoinedInner.isSelected()) {
            antennaSet = "HBA_JOINED_INNER";
        }
        if (itsAntennaSet != null && !antennaSet.equals(itsAntennaSet.limits)) {
            itsAntennaSet.limits = antennaSet;
            saveNode(itsAntennaSet);
        }

        //BandFilter
        String bandFilter = "LBA_30_90";
        if (this.input1090.isSelected()) {
            bandFilter = "LBA_10_90";
        } else if (this.input1070.isSelected()) {
            bandFilter = "LBA_10_70";
        } else if (this.input3070.isSelected()) {
            bandFilter = "LBA_30_70";
        } else if (this.input110190.isSelected()) {
            bandFilter = "HBA_110_190";
        } else if (this.input170230.isSelected()) {
            bandFilter = "HBA_170_230";
        } else if (this.input210250.isSelected()) {
            bandFilter = "HBA_210_250";
        }
        if (itsBandFilter != null && !bandFilter.equals(itsBandFilter.limits)) {
            itsBandFilter.limits = bandFilter;
            saveNode(itsBandFilter);
        }

        //clock
        if (itsClockMode != null && !inputClockMode.getSelectedItem().toString().equals(itsClockMode.limits)) {
            itsClockMode.limits = inputClockMode.getSelectedItem().toString();
            saveNode(itsClockMode);
        }


        // reset all buttons, flags and tables to initial start position. So the panel now reflects the new, saved situation
        initPanel();

        return true;
    }

    /** Restore original Values in  panel
     */
    public void restore() {

      // Observation Specific parameters
      if (itsAntennaArray.limits.equals("LBA")) {
          this.inputLBAAntennas.setSelected(true);
      } else {
          this.inputHBAAntennas.setSelected(true);
      }
        switch (itsAntennaSet.limits) {
            case "LBA_OUTER":
        inputOuterCircle.setSelected(true);
                break;
            case "LBA_SPARSE_EVEN":
        inputSparseEven.setSelected(true);
                break;
            case "LBA_SPARSE_ODD":
        inputSparseOdd.setSelected(true);
                break;
            case "LBA_X":
        inputXPolesOnly.setSelected(true);
                break;
            case "LBA_Y":
        inputYPolesOnly.setSelected(true);
                break;
            case "HBA_ZERO":
        inputHBAZero.setSelected(true);
                break;
            case "HBA_ONE":
        inputHBAOne.setSelected(true);
                break;
            case "HBA_DUAL":
        inputHBADual.setSelected(true);
                break;
            case "HBA_JOINED":
        inputHBAJoined.setSelected(true);
                break;
            case "HBA_ZERO_INNER":
                inputHBAZeroInner.setSelected(true);
                break;
            case "HBA_ONE_INNER":
                inputHBAOneInner.setSelected(true);
                break;
            case "HBA_DUAL_INNER":
                inputHBADualInner.setSelected(true);
                break;
            case "HBA_JOINED_INNER":
                inputHBAJoinedInner.setSelected(true);
                break;
            default:
        inputInnerCircle.setSelected(true);
                break;
      }
        switch (itsBandFilter.limits) {
            case "LBA_10_90":
        input1090.setSelected(true);
                break;
            case "LBA_30_90":
        input3090.setSelected(true);
                break;
            case "LBA_10_70":
        input1070.setSelected(true);
                break;
            case "HBA_110_190":
        input110190.setSelected(true);
                break;
            case "HBA_170_230":
        input170230.setSelected(true);
                break;
            case "HBA_210_250":
        input210250.setSelected(true);
                break;
            default:
        input3070.setSelected(true);
                break;
      }

      inputClockMode.setSelectedItem(itsClockMode.limits);


      setStationLists(itsStationList.limits);
      checkSettings();
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
        return new AntennaConfigPanel();
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
      this.inputClockMode.setEnabled(enabled);
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

    public boolean isLBASelected() {
        return this.inputLBAAntennas.isSelected();
    }

     /**
     * Helper method that retrieves the child nodes for a given jOTDBnode,
     * and triggers setField() accordingly.
     * @param aNode the node to retrieve and display child data of.
     */
    private void retrieveAndDisplayChildDataForNode(jOTDBnode aNode){
        jOTDBparam aParam=null;
        try {
            ArrayList<jOTDBnode> HWchilds = new ArrayList(OtdbRmi.getRemoteMaintenance().getItemList(aNode.treeID(), aNode.nodeID(), 1));
            // get all the params per child
            for (jOTDBnode aHWNode:HWchilds) {
                aParam=null;
                // We need to keep all the params needed by this panel
                if (aHWNode.leaf) {
                    aParam = OtdbRmi.getRemoteMaintenance().getParam(aHWNode);
                }
                setField(aNode,aParam,aHWNode);
            }
        } catch (RemoteException ex) {
            String aS="Error during retrieveAndDisplayChildDataForNode: "+ ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
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
            String aS="Error during getParam: "+ ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            return;
        }

        if(parentName.equals("Observation")){
        // Observation Specific parameters
            switch (aKeyName) {
                case "antennaArray":
                inputLBAAntennas.setToolTipText(aParam.description);
                inputHBAAntennas.setToolTipText(aParam.description);
                itsAntennaArray=aNode;
                if (aNode.limits.equals("HBA")) {
                    inputHBAAntennas.setSelected(true);
                } else {
                    inputLBAAntennas.setSelected(true);
                }
                    break;
                case "antennaSet":
                inputInnerCircle.setToolTipText(aParam.description);
                inputOuterCircle.setToolTipText(aParam.description);
                inputSparseEven.setToolTipText(aParam.description);
                inputXPolesOnly.setToolTipText(aParam.description);
                inputYPolesOnly.setToolTipText(aParam.description);
                inputHBAZero.setToolTipText(aParam.description);
                inputHBAOne.setToolTipText(aParam.description);
                inputHBADual.setToolTipText(aParam.description);
                inputHBAJoined.setToolTipText(aParam.description);
                    inputHBAZeroInner.setToolTipText(aParam.description);
                    inputHBAOneInner.setToolTipText(aParam.description);
                    inputHBADualInner.setToolTipText(aParam.description);
                    inputHBAJoinedInner.setToolTipText(aParam.description);
                this.itsAntennaSet=aNode;
            switch (aNode.limits) {
                case "LBA_OUTER":
                    inputOuterCircle.setSelected(true);
                    break;
                case "LBA_SPARSE_EVEN":
                    inputSparseEven.setSelected(true);
                    break;
                case "LBA_SPARSE_ODD":
                    inputSparseOdd.setSelected(true);
                    break;
                case "LBA_X":
                    inputXPolesOnly.setSelected(true);
                    break;
                case "LBA_Y":
                    inputYPolesOnly.setSelected(true);
                    break;
                case "HBA_ZERO":
                    inputHBAZero.setSelected(true);
                    break;
                case "HBA_ONE":
                    inputHBAOne.setSelected(true);
                    break;
                case "HBA_DUAL":
                    inputHBADual.setSelected(true);
                    break;
                case "HBA_JOINED":
                    inputHBAJoined.setSelected(true);
                    break;
                case "HBA_ZERO_INNER":
                    inputHBAZeroInner.setSelected(true);
                    break;
                case "HBA_ONE_INNER":
                    inputHBAOneInner.setSelected(true);
                    break;
                case "HBA_DUAL_INNER":
                    inputHBADualInner.setSelected(true);
                    break;
                case "HBA_JOINED_INNER":
                    inputHBAJoinedInner.setSelected(true);
                    break;
                case "LBA_INNER":
                    inputInnerCircle.setSelected(true);
                    break;
                }
                    break;
                case "bandFilter":
                input3090.setToolTipText(aParam.description);
                input1090.setToolTipText(aParam.description);
                input110190.setToolTipText(aParam.description);
                input170230.setToolTipText(aParam.description);
                input210250.setToolTipText(aParam.description);
                this.itsBandFilter=aNode;
            switch (aNode.limits) {
                case "LBA_10_90":
                    if (inputClockMode.getSelectedItem().equals("<<Clock200")) {
                        input1090.setSelected(true);
                    } else {
                        input1070.setSelected(true);
                    }
                    break;
                case "HBA_110_190":
                    input110190.setSelected(true);
                    break;
                case "HBA_170_230":
                    input170230.setSelected(true);
                    break;
                case "HBA_210_250":
                    input210250.setSelected(true);
                    break;
                default:
                    if (inputClockMode.getSelectedItem().equals("<<Clock200")) {
                        input3090.setSelected(true);
                    } else {
                        input3070.setSelected(true);
                    }
                    break;
                }
                    break;
                case "clockMode":
                inputClockMode.setToolTipText(aParam.description);
                LofarUtils.setPopupComboChoices(inputClockMode,aParam.limits);
                if (!aNode.limits.equals("")) {
                    inputClockMode.setSelectedItem(aNode.limits);
                }
                itsClockMode=aNode;
                    break;
            }
        } else if(parentName.contains("VirtualInstrument")){
            // Observation Beamformer parameters
            if (aKeyName.equals("stationList")) {
                this.coreStationSelectionPanel.setToolTipText(aParam.description);
                this.remoteStationSelectionPanel.setToolTipText(aParam.description);
                this.europeStationSelectionPanel.setToolTipText(aParam.description);
                this.itsStationList = aNode;
                setStationLists(aNode.limits);
            }
        }
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
            this.coreStationSelectionPanel.setUsedStationList(new ArrayList(itsUsedCoreStations));
            this.remoteStationSelectionPanel.setUsedStationList(new ArrayList(itsUsedRemoteStations));
            this.europeStationSelectionPanel.setUsedStationList(new ArrayList(itsUsedEuropeStations));
        }
    }

    /** Checks all settings and enables/disables all what is needed.
     *
     */
    private void checkSettings() {

        // all stuff for Station Settings based on choices
        if (this.inputLBAAntennas.isSelected() ) {
            this.input1090.setEnabled(true);
            this.input3090.setEnabled(true);
            this.input1070.setEnabled(true);
            this.input3070.setEnabled(true);
            this.inputInnerCircle.setEnabled(true);
            this.inputOuterCircle.setEnabled(true);
            this.input110190.setEnabled(false);
            this.input170230.setEnabled(false);
            this.input210250.setEnabled(false);
            this.inputHBAZero.setEnabled(false);
            this.inputHBAOne.setEnabled(false);
            this.inputHBADual.setEnabled(false);
            this.inputHBAJoined.setEnabled(false);
            this.inputClockMode.setEnabled(false);
            this.inputHBAZeroInner.setEnabled(false);
            this.inputHBAOneInner.setEnabled(false);
            this.inputHBADualInner.setEnabled(false);
            this.inputHBAJoinedInner.setEnabled(false);

        } else {
            this.input1090.setEnabled(false);
            this.input3090.setEnabled(false);
            this.input1070.setEnabled(false);
            this.input3070.setEnabled(false);
            this.inputInnerCircle.setEnabled(false);
            this.inputOuterCircle.setEnabled(false);
            this.input110190.setEnabled(true);
            this.input170230.setEnabled(true);
            this.input210250.setEnabled(true);
            this.inputHBAZero.setEnabled(true);
            this.inputHBAOne.setEnabled(true);
            this.inputHBADual.setEnabled(true);
            this.inputHBAJoined.setEnabled(true);
            this.inputHBAZeroInner.setEnabled(true);
            this.inputHBAOneInner.setEnabled(true);
            this.inputHBADualInner.setEnabled(true);
            this.inputHBAJoinedInner.setEnabled(true);
            this.inputClockMode.setEnabled(true);

        }

        this.checkStationOverlap();

        // all stuff for CoreStationLayout Settings based on choices
        this.coreStationLayout.setHBALeftSquareEnabled(inputHBAZero.isEnabled()||inputHBADual.isEnabled()||inputHBAJoined.isEnabled()||inputHBAZeroInner.isEnabled()||inputHBADualInner.isEnabled()||inputHBAJoinedInner.isEnabled());
        this.coreStationLayout.setHBALeftSquareSelected(inputHBAZero.isSelected()||inputHBADual.isSelected()||inputHBAJoined.isSelected()||inputHBAZeroInner.isSelected()||inputHBADualInner.isSelected()||inputHBAJoinedInner.isSelected());
        this.coreStationLayout.setHBARightSquareEnabled(inputHBAOne.isEnabled()||inputHBADual.isEnabled()||inputHBAJoined.isEnabled()||inputHBAOneInner.isEnabled()||inputHBADualInner.isEnabled()||inputHBAJoinedInner.isEnabled());
        this.coreStationLayout.setHBARightSquareSelected(inputHBAOne.isSelected()||inputHBADual.isSelected()||inputHBAJoined.isSelected()||inputHBAOneInner.isSelected()||inputHBADualInner.isSelected()||inputHBAJoinedInner.isSelected());
        this.coreStationLayout.setLBAInnerCircleEnabled(inputInnerCircle.isEnabled());
        this.coreStationLayout.setLBAInnerCircleSelected(inputInnerCircle.isSelected());
        this.coreStationLayout.setLBAOuterCircleEnabled(inputOuterCircle.isEnabled());
        this.coreStationLayout.setLBAOuterCircleSelected(inputOuterCircle.isSelected());

        // all stuff for RemoteStationLayout Settings based on choices
        this.remoteStationLayout.setHBALeftOuterSquareEnabled(inputHBAZero.isEnabled()||inputHBAOne.isEnabled()||inputHBADual.isEnabled()||inputHBAJoined.isEnabled());
        this.remoteStationLayout.setHBALeftOuterSquareSelected(inputHBAZero.isSelected()||inputHBAOne.isSelected()||inputHBADual.isSelected()||inputHBAJoined.isSelected());
        this.remoteStationLayout.setHBALeftInnerSquareEnabled(inputHBAZero.isEnabled()||inputHBAOne.isEnabled()||inputHBADual.isEnabled()||inputHBAJoined.isEnabled()||inputHBAZeroInner.isEnabled()||inputHBAOneInner.isEnabled()||inputHBADualInner.isEnabled()||inputHBAJoinedInner.isEnabled());
        this.remoteStationLayout.setHBALeftInnerSquareSelected(inputHBAZero.isSelected()||inputHBAOne.isSelected()||inputHBADual.isSelected()||inputHBAJoined.isSelected()||inputHBAZeroInner.isSelected()||inputHBAOneInner.isSelected()||inputHBADualInner.isSelected()||inputHBAJoinedInner.isSelected());
        this.remoteStationLayout.setLBAOuterCircleEnabled(inputOuterCircle.isEnabled());
        this.remoteStationLayout.setLBAOuterCircleSelected(inputOuterCircle.isSelected());
        this.remoteStationLayout.setLBAInnerCircleEnabled(inputInnerCircle.isEnabled());
        this.remoteStationLayout.setLBAInnerCircleSelected(inputInnerCircle.isSelected());

        // all stuff for EuropeStationLayout Settings based on choices
        this.europeStationLayout.setHBALeftSquareEnabled(inputHBAZero.isEnabled()||inputHBAOne.isEnabled()||inputHBADual.isEnabled()||inputHBAJoined.isEnabled()||inputHBAZeroInner.isEnabled()||inputHBAOneInner.isEnabled()||inputHBADualInner.isEnabled()||inputHBAJoinedInner.isEnabled());
        this.europeStationLayout.setHBALeftSquareSelected(inputHBAZero.isSelected()||inputHBAOne.isSelected()||inputHBADual.isSelected()||inputHBAJoined.isSelected()||inputHBAZeroInner.isSelected()||inputHBAOneInner.isSelected()||inputHBADualInner.isSelected()||inputHBAJoinedInner.isSelected());
        this.europeStationLayout.setLBACircleEnabled(inputInnerCircle.isEnabled()||inputOuterCircle.isEnabled());
        this.europeStationLayout.setLBACircleSelected(inputInnerCircle.isSelected()||inputOuterCircle.isSelected());
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
                String aS="ObservationPanel: Error getting treeInfo/treetype" + ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                itsTreeType="";
            }
         } else {
            logger.debug("no node given");
        }

        // set defaults/initial settings
        restore();

        conflictText1.setVisible(false);
        conflictText2.setVisible(false);
        conflictText3.setVisible(false);
        conflictText4.setVisible(false);
        conflictText5.setVisible(false);
        conflictText6.setVisible(false);
        conflictText7.setVisible(false);
        conflictText8.setVisible(false);
        conflictWarning1.setVisible(false);
        conflictWarning2.setVisible(false);
        conflictWarning3.setVisible(false);
        conflictWarning4.setVisible(false);
        conflictWarning5.setVisible(false);
        conflictWarning6.setVisible(false);
        conflictWarning7.setVisible(false);
        conflictWarning8.setVisible(false);

        if (itsTreeType.equals("VHtree")) {
            this.setButtonsVisible(false);
            this.setAllEnabled(false);
        }


        itsMainFrame.setNormalCursor();

    }

    /** Check the overlap between the european stations and some of the HBA1 core stations
     *
     */
    private void checkStationOverlap() {
       String stations= getUsedStations();
       boolean error=false;
       // We only need to check stationOverlap in HAB mode, and only for
       if (this.inputHBAAntennas.isSelected() && (this.inputHBAOne.isSelected() || this.inputHBADual.isSelected() || this.inputHBAOneInner.isSelected() || this.inputHBADualInner.isSelected())) {
           if (stations.contains("DE601") && stations.contains("CS001")) {
               conflictText1.setVisible(true);
               conflictWarning1.setVisible(true);
               error=true;
           } else {
               conflictText1.setVisible(false);
               conflictWarning1.setVisible(false);
           }
           if (stations.contains("DE602") && stations.contains("CS031")) {
               conflictText2.setVisible(true);
               conflictWarning2.setVisible(true);
               error=true;
           } else {
               conflictText2.setVisible(false);
               conflictWarning2.setVisible(false);
           }
           if (stations.contains("DE603") && stations.contains("CS028")) {
               conflictText3.setVisible(true);
               conflictWarning3.setVisible(true);
               error=true;
           } else {
               conflictText3.setVisible(false);
               conflictWarning3.setVisible(false);
           }
           if (stations.contains("DE604") && stations.contains("CS011")) {
               conflictText4.setVisible(true);
               conflictWarning4.setVisible(true);
               error=true;
           } else {
               conflictText4.setVisible(false);
               conflictWarning4.setVisible(false);
           }
           if (stations.contains("DE605") && stations.contains("CS401")) {
               conflictText5.setVisible(true);
               conflictWarning5.setVisible(true);
               error=true;
           } else {
               conflictText5.setVisible(false);
               conflictWarning5.setVisible(false);
           }
           if (stations.contains("FR606") && stations.contains("CS030")) {
               conflictText6.setVisible(true);
               conflictWarning6.setVisible(true);
               error=true;
           } else {
               conflictText6.setVisible(false);
               conflictWarning6.setVisible(false);
           }
           if (stations.contains("SE607") && stations.contains("CS301")) {
               conflictText7.setVisible(true);
               conflictWarning7.setVisible(true);
               error=true;
           } else {
               conflictText7.setVisible(false);
               conflictWarning7.setVisible(false);
           }
           if (stations.contains("UK608") && stations.contains("CS013")) {
               conflictText8.setVisible(true);
               conflictWarning8.setVisible(true);
               error=true;
           } else {
               conflictText8.setVisible(false);
               conflictWarning8.setVisible(false);
           }
       } else {
           conflictText1.setVisible(false);
           conflictWarning1.setVisible(false);
           conflictText2.setVisible(false);
           conflictWarning2.setVisible(false);
           conflictText3.setVisible(false);
           conflictWarning3.setVisible(false);
           conflictText4.setVisible(false);
           conflictWarning4.setVisible(false);
           conflictText5.setVisible(false);
           conflictWarning5.setVisible(false);
           conflictText6.setVisible(false);
           conflictWarning6.setVisible(false);
           conflictText7.setVisible(false);
           conflictWarning7.setVisible(false);
           conflictText8.setVisible(false);
           conflictWarning8.setVisible(false);
       }
       String command="validInput";
       if (error) {
           command="invalidInput";
       }
       ActionEvent actionEvent = new ActionEvent(this,
          ActionEvent.ACTION_PERFORMED, command);
       this.fireActionListenerActionPerformed(actionEvent);

    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        AntennaLayoutGroup = new javax.swing.ButtonGroup();
        AntennaFilterGroup = new javax.swing.ButtonGroup();
        AntennaSelectionGroup = new javax.swing.ButtonGroup();
        jPanel1 = new javax.swing.JPanel();
        inputLBAAntennas = new javax.swing.JRadioButton();
        inputHBAAntennas = new javax.swing.JRadioButton();
        panelLBASelection = new javax.swing.JPanel();
        inputInnerCircle = new javax.swing.JRadioButton();
        inputOuterCircle = new javax.swing.JRadioButton();
        inputSparseEven = new javax.swing.JRadioButton();
        inputXPolesOnly = new javax.swing.JRadioButton();
        inputYPolesOnly = new javax.swing.JRadioButton();
        jLabel1 = new javax.swing.JLabel();
        input3090 = new javax.swing.JRadioButton();
        input1090 = new javax.swing.JRadioButton();
        input1070 = new javax.swing.JRadioButton();
        input3070 = new javax.swing.JRadioButton();
        inputSparseOdd = new javax.swing.JRadioButton();
        panelHBASelection1 = new javax.swing.JPanel();
        inputHBAZero = new javax.swing.JRadioButton();
        inputHBADual = new javax.swing.JRadioButton();
        jLabel2 = new javax.swing.JLabel();
        input110190 = new javax.swing.JRadioButton();
        input170230 = new javax.swing.JRadioButton();
        input210250 = new javax.swing.JRadioButton();
        inputHBAOne = new javax.swing.JRadioButton();
        inputHBAJoined = new javax.swing.JRadioButton();
        inputHBAZeroInner = new javax.swing.JRadioButton();
        inputHBAOneInner = new javax.swing.JRadioButton();
        inputHBAJoinedInner = new javax.swing.JRadioButton();
        inputHBADualInner = new javax.swing.JRadioButton();
        labelClockMode = new javax.swing.JLabel();
        inputClockMode = new javax.swing.JComboBox();
        jPanel2 = new javax.swing.JPanel();
        remoteStationSelectionPanel = new nl.astron.lofar.sas.otbcomponents.StationSelectionPanel();
        europeStationSelectionPanel = new nl.astron.lofar.sas.otbcomponents.StationSelectionPanel();
        coreStationSelectionPanel = new nl.astron.lofar.sas.otbcomponents.StationSelectionPanel();
        jPanel3 = new javax.swing.JPanel();
        remoteStationLayout = new nl.astron.lofar.sas.otbcomponents.RemoteStationLayout();
        europeStationLayout = new nl.astron.lofar.sas.otbcomponents.EuropeStationLayout();
        coreStationLayout = new nl.astron.lofar.sas.otbcomponents.CoreStationLayout();
        conflictText1 = new javax.swing.JLabel();
        conflictWarning1 = new javax.swing.JLabel();
        conflictText2 = new javax.swing.JLabel();
        conflictWarning2 = new javax.swing.JLabel();
        conflictText3 = new javax.swing.JLabel();
        conflictWarning3 = new javax.swing.JLabel();
        conflictText4 = new javax.swing.JLabel();
        conflictWarning4 = new javax.swing.JLabel();
        conflictText5 = new javax.swing.JLabel();
        conflictWarning5 = new javax.swing.JLabel();
        conflictText6 = new javax.swing.JLabel();
        conflictWarning6 = new javax.swing.JLabel();
        conflictText7 = new javax.swing.JLabel();
        conflictWarning7 = new javax.swing.JLabel();
        conflictText8 = new javax.swing.JLabel();
        conflictWarning8 = new javax.swing.JLabel();

        setLayout(new java.awt.BorderLayout());

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder("Station settings"));
        jPanel1.setPreferredSize(new java.awt.Dimension(300, 756));

        AntennaSelectionGroup.add(inputLBAAntennas);
        inputLBAAntennas.setSelected(true);
        inputLBAAntennas.setText("LBA antennas");
        inputLBAAntennas.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputLBAAntennasActionPerformed(evt);
            }
        });

        AntennaSelectionGroup.add(inputHBAAntennas);
        inputHBAAntennas.setText("HBA antennas");
        inputHBAAntennas.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputHBAAntennasActionPerformed(evt);
            }
        });

        panelLBASelection.setBorder(javax.swing.BorderFactory.createTitledBorder("Antenna Selection"));

        AntennaLayoutGroup.add(inputInnerCircle);
        inputInnerCircle.setText("Inner circle");
        inputInnerCircle.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputInnerCircleActionPerformed(evt);
            }
        });

        AntennaLayoutGroup.add(inputOuterCircle);
        inputOuterCircle.setText("Outer circle");
        inputOuterCircle.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputOuterCircleActionPerformed(evt);
            }
        });

        AntennaLayoutGroup.add(inputSparseEven);
        inputSparseEven.setText("Sparse Even");
        inputSparseEven.setEnabled(false);
        inputSparseEven.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputSparseEvenEvenActionPerformed(evt);
            }
        });

        AntennaLayoutGroup.add(inputXPolesOnly);
        inputXPolesOnly.setText("X poles only");
        inputXPolesOnly.setEnabled(false);
        inputXPolesOnly.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputXPolesOnlyActionPerformed(evt);
            }
        });

        AntennaLayoutGroup.add(inputYPolesOnly);
        inputYPolesOnly.setText("Y poles only");
        inputYPolesOnly.setEnabled(false);
        inputYPolesOnly.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputYPolesOnlyActionPerformed(evt);
            }
        });

        jLabel1.setFont(new java.awt.Font("Tahoma", 1, 11));
        jLabel1.setText("Freq. selection");

        AntennaFilterGroup.add(input3090);
        input3090.setSelected(true);
        input3090.setText("30-90 MHz");
        input3090.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                input3090ActionPerformed(evt);
            }
        });

        AntennaFilterGroup.add(input1090);
        input1090.setText("10-90 MHz");
        input1090.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                input1090ActionPerformed(evt);
            }
        });

        AntennaFilterGroup.add(input1070);
        input1070.setText("10-70 MHz");
        input1070.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                input1070ActionPerformed(evt);
            }
        });

        AntennaFilterGroup.add(input3070);
        input3070.setText("30-70 MHz");
        input3070.setActionCommand("30-70");
        input3070.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                input3070ActionPerformed(evt);
            }
        });

        AntennaLayoutGroup.add(inputSparseOdd);
        inputSparseOdd.setText("Sparse Odd");
        inputSparseOdd.setEnabled(false);
        inputSparseOdd.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputSparseOddActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout panelLBASelectionLayout = new javax.swing.GroupLayout(panelLBASelection);
        panelLBASelection.setLayout(panelLBASelectionLayout);
        panelLBASelectionLayout.setHorizontalGroup(
            panelLBASelectionLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(panelLBASelectionLayout.createSequentialGroup()
                .addGroup(panelLBASelectionLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(inputSparseEven)
                    .addComponent(inputOuterCircle)
                    .addComponent(inputInnerCircle)
                    .addComponent(jLabel1)
                    .addComponent(input1070)
                    .addComponent(input3070)
                    .addComponent(input1090)
                    .addComponent(input3090)
                    .addComponent(inputYPolesOnly)
                    .addComponent(inputXPolesOnly)
                    .addComponent(inputSparseOdd))
                .addContainerGap(22, Short.MAX_VALUE))
        );
        panelLBASelectionLayout.setVerticalGroup(
            panelLBASelectionLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(panelLBASelectionLayout.createSequentialGroup()
                .addComponent(inputInnerCircle)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(inputOuterCircle)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(inputSparseEven)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(inputSparseOdd)
                .addGap(2, 2, 2)
                .addComponent(inputXPolesOnly)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(inputYPolesOnly)
                .addGap(70, 70, 70)
                .addComponent(jLabel1)
                .addGap(7, 7, 7)
                .addComponent(input1070)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(input3070)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(input1090)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(input3090)
                .addContainerGap(51, Short.MAX_VALUE))
        );

        panelHBASelection1.setBorder(javax.swing.BorderFactory.createTitledBorder("Antenna Selection"));

        AntennaLayoutGroup.add(inputHBAZero);
        inputHBAZero.setText("HBA_ZERO");
        inputHBAZero.setEnabled(false);
        inputHBAZero.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputHBAZeroActionPerformed(evt);
            }
        });

        AntennaLayoutGroup.add(inputHBADual);
        inputHBADual.setText("HBA_DUAL");
        inputHBADual.setEnabled(false);
        inputHBADual.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputHBADualActionPerformed(evt);
            }
        });

        jLabel2.setFont(new java.awt.Font("Tahoma", 1, 11));
        jLabel2.setText("Freq. selection");

        AntennaFilterGroup.add(input110190);
        input110190.setText("110-190 MHz");
        input110190.setEnabled(false);
        input110190.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                input110190ActionPerformed(evt);
            }
        });

        AntennaFilterGroup.add(input170230);
        input170230.setText("170-230 MHz");
        input170230.setEnabled(false);
        input170230.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                input170230ActionPerformed(evt);
            }
        });

        AntennaFilterGroup.add(input210250);
        input210250.setText("210-250 MHz");
        input210250.setEnabled(false);
        input210250.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                input210250ActionPerformed(evt);
            }
        });

        AntennaLayoutGroup.add(inputHBAOne);
        inputHBAOne.setText("HBA_ONE");
        inputHBAOne.setEnabled(false);
        inputHBAOne.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputHBAOneActionPerformed(evt);
            }
        });

        AntennaLayoutGroup.add(inputHBAJoined);
        inputHBAJoined.setText("HBA_JOINED");
        inputHBAJoined.setEnabled(false);
        inputHBAJoined.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputHBAJoinedActionPerformed(evt);
            }
        });

        AntennaLayoutGroup.add(inputHBAZeroInner);
        inputHBAZeroInner.setSelected(true);
        inputHBAZeroInner.setText("HBA_ZERO_INNER");
        inputHBAZeroInner.setEnabled(false);
        inputHBAZeroInner.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputHBAZeroInnerActionPerformed(evt);
            }
        });

        AntennaLayoutGroup.add(inputHBAOneInner);
        inputHBAOneInner.setText("HBA_ONE_INNER");
        inputHBAOneInner.setEnabled(false);
        inputHBAOneInner.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputHBAOneInnerActionPerformed(evt);
            }
        });

        AntennaLayoutGroup.add(inputHBAJoinedInner);
        inputHBAJoinedInner.setText("HBA_JOINED_INNER");
        inputHBAJoinedInner.setEnabled(false);
        inputHBAJoinedInner.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputHBAJoinedInnerActionPerformed(evt);
            }
        });

        AntennaLayoutGroup.add(inputHBADualInner);
        inputHBADualInner.setText("HBA_DUAL_INNER");
        inputHBADualInner.setEnabled(false);
        inputHBADualInner.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputHBADualInnerActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout panelHBASelection1Layout = new javax.swing.GroupLayout(panelHBASelection1);
        panelHBASelection1.setLayout(panelHBASelection1Layout);
        panelHBASelection1Layout.setHorizontalGroup(
            panelHBASelection1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(panelHBASelection1Layout.createSequentialGroup()
                .addGroup(panelHBASelection1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(inputHBAZero)
                    .addComponent(inputHBAOne)
                    .addComponent(inputHBAJoined)
                    .addComponent(inputHBADual)
                    .addComponent(inputHBAZeroInner)
                    .addComponent(inputHBAOneInner)
                    .addComponent(inputHBAJoinedInner)
                    .addComponent(inputHBADualInner)
                    .addGroup(panelHBASelection1Layout.createSequentialGroup()
                        .addContainerGap()
                        .addGroup(panelHBASelection1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(jLabel2)
                            .addComponent(input110190)
                            .addComponent(input170230)
                            .addComponent(input210250))))
                .addContainerGap(44, Short.MAX_VALUE))
        );
        panelHBASelection1Layout.setVerticalGroup(
            panelHBASelection1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(panelHBASelection1Layout.createSequentialGroup()
                .addComponent(inputHBAZero)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(inputHBAOne)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(inputHBAJoined)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(inputHBADual)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(inputHBAZeroInner)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(inputHBAOneInner)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(inputHBAJoinedInner)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(inputHBADualInner)
                .addGap(18, 18, 18)
                .addComponent(jLabel2)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(input110190)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(input170230)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(input210250)
                .addContainerGap(73, Short.MAX_VALUE))
        );

        labelClockMode.setText("Clock Mode:");

        inputClockMode.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        inputClockMode.setEnabled(false);

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(inputLBAAntennas)
                            .addComponent(panelLBASelection, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGap(16, 16, 16)
                        .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(inputHBAAntennas)
                            .addComponent(panelHBASelection1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGap(26, 26, 26))
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addComponent(labelClockMode, javax.swing.GroupLayout.PREFERRED_SIZE, 86, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 72, Short.MAX_VALUE)
                        .addComponent(inputClockMode, javax.swing.GroupLayout.PREFERRED_SIZE, 131, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(53, 53, 53))))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(inputLBAAntennas)
                    .addComponent(inputHBAAntennas))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(panelHBASelection1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(panelLBASelection, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelClockMode)
                    .addComponent(inputClockMode, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(253, 253, 253))
        );

        add(jPanel1, java.awt.BorderLayout.WEST);

        jPanel2.setBorder(javax.swing.BorderFactory.createTitledBorder("Station Selections"));

        europeStationSelectionPanel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                europeStationSelectionPanelActionPerformed(evt);
            }
        });

        coreStationSelectionPanel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                coreStationSelectionPanelActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(remoteStationSelectionPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(europeStationSelectionPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(coreStationSelectionPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap(14, Short.MAX_VALUE))
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addComponent(coreStationSelectionPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(45, 45, 45)
                .addComponent(remoteStationSelectionPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(31, 31, 31)
                .addComponent(europeStationSelectionPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(78, Short.MAX_VALUE))
        );

        add(jPanel2, java.awt.BorderLayout.LINE_END);

        jPanel3.setBorder(javax.swing.BorderFactory.createTitledBorder("Station Layout"));

        remoteStationLayout.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                remoteStationLayoutActionPerformed(evt);
            }
        });

        europeStationLayout.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                europeStationLayoutActionPerformed(evt);
            }
        });

        coreStationLayout.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                coreStationLayoutActionPerformed(evt);
            }
        });

        conflictText1.setText("DE601  --> CS001 HBA1");

        conflictWarning1.setFont(new java.awt.Font("Tahoma", 1, 11));
        conflictWarning1.setForeground(new java.awt.Color(255, 0, 0));
        conflictWarning1.setText("Conflict !!");

        conflictText2.setText("DE602  --> CS031 HBA1");

        conflictWarning2.setFont(new java.awt.Font("Tahoma", 1, 11));
        conflictWarning2.setForeground(new java.awt.Color(255, 0, 0));
        conflictWarning2.setText("Conflict !!");

        conflictText3.setText("DE603  --> CS028 HBA1");

        conflictWarning3.setFont(new java.awt.Font("Tahoma", 1, 11));
        conflictWarning3.setForeground(new java.awt.Color(255, 0, 0));
        conflictWarning3.setText("Conflict !!");

        conflictText4.setText("DE604  --> CS011 HBA1");

        conflictWarning4.setFont(new java.awt.Font("Tahoma", 1, 11));
        conflictWarning4.setForeground(new java.awt.Color(255, 0, 0));
        conflictWarning4.setText("Conflict !!");

        conflictText5.setText("DE605  --> CS401 HBA1");

        conflictWarning5.setFont(new java.awt.Font("Tahoma", 1, 11));
        conflictWarning5.setForeground(new java.awt.Color(255, 0, 0));
        conflictWarning5.setText("Conflict !!");

        conflictText6.setText("FR606  --> CS030 HBA1");

        conflictWarning6.setFont(new java.awt.Font("Tahoma", 1, 11));
        conflictWarning6.setForeground(new java.awt.Color(255, 0, 0));
        conflictWarning6.setText("Conflict !!");

        conflictText7.setText("SE607  --> CS301 HBA1");

        conflictWarning7.setFont(new java.awt.Font("Tahoma", 1, 11));
        conflictWarning7.setForeground(new java.awt.Color(255, 0, 0));
        conflictWarning7.setText("Conflict !!");

        conflictText8.setText("UK608  --> CS013 HBA1");

        conflictWarning8.setFont(new java.awt.Font("Tahoma", 1, 11));
        conflictWarning8.setForeground(new java.awt.Color(255, 0, 0));
        conflictWarning8.setText("Conflict !!");

        javax.swing.GroupLayout jPanel3Layout = new javax.swing.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                    .addComponent(europeStationLayout, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(remoteStationLayout, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(coreStationLayout, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 308, Short.MAX_VALUE))
                .addGap(18, 18, 18)
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addComponent(conflictText1, javax.swing.GroupLayout.PREFERRED_SIZE, 158, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(conflictWarning1, javax.swing.GroupLayout.PREFERRED_SIZE, 58, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addComponent(conflictText2, javax.swing.GroupLayout.PREFERRED_SIZE, 158, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(conflictWarning2, javax.swing.GroupLayout.PREFERRED_SIZE, 58, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addComponent(conflictText3, javax.swing.GroupLayout.PREFERRED_SIZE, 158, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(conflictWarning3, javax.swing.GroupLayout.PREFERRED_SIZE, 58, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addComponent(conflictText4, javax.swing.GroupLayout.PREFERRED_SIZE, 158, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(conflictWarning4, javax.swing.GroupLayout.PREFERRED_SIZE, 58, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addComponent(conflictText5, javax.swing.GroupLayout.PREFERRED_SIZE, 158, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(conflictWarning5, javax.swing.GroupLayout.PREFERRED_SIZE, 58, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addComponent(conflictText6, javax.swing.GroupLayout.PREFERRED_SIZE, 158, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(conflictWarning6, javax.swing.GroupLayout.PREFERRED_SIZE, 58, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addComponent(conflictText7, javax.swing.GroupLayout.PREFERRED_SIZE, 158, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(conflictWarning7, javax.swing.GroupLayout.PREFERRED_SIZE, 58, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addComponent(conflictText8, javax.swing.GroupLayout.PREFERRED_SIZE, 158, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(conflictWarning8, javax.swing.GroupLayout.PREFERRED_SIZE, 58, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(54, Short.MAX_VALUE))
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addComponent(coreStationLayout, javax.swing.GroupLayout.PREFERRED_SIZE, 219, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(remoteStationLayout, javax.swing.GroupLayout.PREFERRED_SIZE, 203, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(europeStationLayout, javax.swing.GroupLayout.PREFERRED_SIZE, 203, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addGap(21, 21, 21)
                        .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(conflictText1)
                            .addComponent(conflictWarning1))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(conflictText2)
                            .addComponent(conflictWarning2))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(conflictText3)
                            .addComponent(conflictWarning3))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(conflictText4)
                            .addComponent(conflictWarning4))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(conflictText5)
                            .addComponent(conflictWarning5))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(conflictText6)
                            .addComponent(conflictWarning6))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(conflictText7)
                            .addComponent(conflictWarning7))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(conflictText8)
                            .addComponent(conflictWarning8))))
                .addContainerGap(96, Short.MAX_VALUE))
        );

        add(jPanel3, java.awt.BorderLayout.CENTER);
    }// </editor-fold>//GEN-END:initComponents

    private void inputLBAAntennasActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputLBAAntennasActionPerformed
        this.inputOuterCircle.setSelected(true);
        if (inputClockMode.getSelectedItem().equals("<<Clock200")) {
            this.input3090.setSelected(true);
        } else {
            this.input3070.setSelected(true);
        }
        checkSettings();
    }//GEN-LAST:event_inputLBAAntennasActionPerformed

    private void inputHBAAntennasActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputHBAAntennasActionPerformed
        this.inputHBAOne.setSelected(true);
        this.input110190.setSelected(true);
        this.inputClockMode.setSelectedItem("<<Clock200");
        checkSettings();
    }//GEN-LAST:event_inputHBAAntennasActionPerformed

    private void inputInnerCircleActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputInnerCircleActionPerformed
        checkSettings();
    }//GEN-LAST:event_inputInnerCircleActionPerformed

    private void inputOuterCircleActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputOuterCircleActionPerformed
        checkSettings();
    }//GEN-LAST:event_inputOuterCircleActionPerformed

    private void inputSparseEvenEvenActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputSparseEvenEvenActionPerformed
        checkSettings();
}//GEN-LAST:event_inputSparseEvenEvenActionPerformed

    private void inputXPolesOnlyActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputXPolesOnlyActionPerformed
        checkSettings();
    }//GEN-LAST:event_inputXPolesOnlyActionPerformed

    private void inputYPolesOnlyActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputYPolesOnlyActionPerformed
        checkSettings();
    }//GEN-LAST:event_inputYPolesOnlyActionPerformed

    private void input3090ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_input3090ActionPerformed
        inputClockMode.setSelectedItem("<<Clock200");
        checkSettings();
}//GEN-LAST:event_input3090ActionPerformed

    private void input1090ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_input1090ActionPerformed
        inputClockMode.setSelectedItem("<<Clock200");
        checkSettings();
    }//GEN-LAST:event_input1090ActionPerformed

    private void inputHBAZeroActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputHBAZeroActionPerformed
        checkSettings();
}//GEN-LAST:event_inputHBAZeroActionPerformed

    private void inputHBADualActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputHBADualActionPerformed
        checkSettings();
}//GEN-LAST:event_inputHBADualActionPerformed

    private void input110190ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_input110190ActionPerformed
        inputClockMode.setSelectedItem("<<Clock200");
        checkSettings();
    }//GEN-LAST:event_input110190ActionPerformed

    private void input170230ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_input170230ActionPerformed
        inputClockMode.setSelectedItem("<<Clock160");
        checkSettings();
    }//GEN-LAST:event_input170230ActionPerformed

    private void input210250ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_input210250ActionPerformed
        inputClockMode.setSelectedItem("<<Clock200");
        checkSettings();
    }//GEN-LAST:event_input210250ActionPerformed

    private void coreStationLayoutActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_coreStationLayoutActionPerformed
        switch (evt.getActionCommand()) {
            case "HBALeftSquare":
            if (this.inputHBAOne.isSelected()) {
                this.inputHBADual.setSelected(true);
            } else {
                this.inputHBAZero.setSelected(true);
            }
                break;
            case "HBARightSquare":
            if (this.inputHBAZero.isSelected()) {
                this.inputHBADual.setSelected(true);
            } else {
                this.inputHBAOne.setSelected(true);
            }
                break;
            case "LBAInnerCircle":
            this.inputInnerCircle.setSelected(!this.coreStationLayout.isLBAInnerCircleSelected());
                break;
            case "LBAOuterCircle":
            this.inputOuterCircle.setSelected(!this.coreStationLayout.isLBAOuterCircleSelected());
                break;
        }
        checkSettings();
    }//GEN-LAST:event_coreStationLayoutActionPerformed

    private void remoteStationLayoutActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_remoteStationLayoutActionPerformed
        switch (evt.getActionCommand()) {
            case "HBALeftInnerSquare":
                if (inputHBAZero.isSelected()) {
                    inputHBAZero.setSelected(false);
                    inputHBAZeroInner.setSelected(true);
                } else if (inputHBAOne.isSelected()) {
                    inputHBAOne.setSelected(false);
                    inputHBAOneInner.setSelected(true);
                } else if (inputHBADual.isSelected()) {
                    inputHBADual.setSelected(false);
                    inputHBADualInner.setSelected(true);
                } else if (inputHBAJoined.isSelected()) {
                    inputHBAJoined.setSelected(false);
                    inputHBAJoinedInner.setSelected(true);
                }
                break;
            case "HBALeftOuterSquare":
                if (inputHBAZeroInner.isSelected()) {
                    inputHBAZeroInner.setSelected(false);
                    inputHBAZero.setSelected(true);
                } else if (inputHBAOneInner.isSelected()) {
                    inputHBAOneInner.setSelected(false);
                    inputHBAOne.setSelected(true);
                } else if (inputHBADualInner.isSelected()) {
                    inputHBADualInner.setSelected(false);
                    inputHBADual.setSelected(true);
                } else if (inputHBAJoinedInner.isSelected()) {
                    inputHBAJoinedInner.setSelected(false);
                    inputHBAJoined.setSelected(true);
                }
                break;
            case "LBAInnerCircle":
            this.inputInnerCircle.setSelected(!this.remoteStationLayout.isLBAInnerCircleSelected());
                break;
            case "LBAOuterCircle":
            this.inputOuterCircle.setSelected(!this.remoteStationLayout.isLBAOuterCircleSelected());
                break;
        }
        checkSettings();
    }//GEN-LAST:event_remoteStationLayoutActionPerformed

    private void europeStationLayoutActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_europeStationLayoutActionPerformed
        switch (evt.getActionCommand()) {
            case "HBALeftSquare":
            this.inputHBADual.setSelected(true);
                break;
            case "LBACircle":
            this.inputOuterCircle.setSelected(this.europeStationLayout.isLBACircleSelected());
                break;
        }
        checkSettings();
}//GEN-LAST:event_europeStationLayoutActionPerformed

    private void input1070ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_input1070ActionPerformed
        inputClockMode.setSelectedItem("<<Clock160");
        checkSettings();
}//GEN-LAST:event_input1070ActionPerformed

    private void input3070ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_input3070ActionPerformed
        inputClockMode.setSelectedItem("<<Clock160");
        checkSettings();
}//GEN-LAST:event_input3070ActionPerformed

    private void inputHBAOneActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputHBAOneActionPerformed
        checkSettings();
}//GEN-LAST:event_inputHBAOneActionPerformed

    private void inputSparseOddActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputSparseOddActionPerformed
        // TODO add your handling code here:
}//GEN-LAST:event_inputSparseOddActionPerformed

    private void inputHBAJoinedActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputHBAJoinedActionPerformed
       checkSettings();
    }//GEN-LAST:event_inputHBAJoinedActionPerformed

    private void coreStationSelectionPanelActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_coreStationSelectionPanelActionPerformed
        checkStationOverlap();
    }//GEN-LAST:event_coreStationSelectionPanelActionPerformed

    private void europeStationSelectionPanelActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_europeStationSelectionPanelActionPerformed
        checkStationOverlap();
    }//GEN-LAST:event_europeStationSelectionPanelActionPerformed

    private void inputHBAZeroInnerActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputHBAZeroInnerActionPerformed
        checkSettings();
    }//GEN-LAST:event_inputHBAZeroInnerActionPerformed

    private void inputHBAOneInnerActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputHBAOneInnerActionPerformed
        checkSettings();
    }//GEN-LAST:event_inputHBAOneInnerActionPerformed

    private void inputHBAJoinedInnerActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputHBAJoinedInnerActionPerformed
        checkSettings();
    }//GEN-LAST:event_inputHBAJoinedInnerActionPerformed

    private void inputHBADualInnerActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputHBADualInnerActionPerformed
        checkSettings();
    }//GEN-LAST:event_inputHBADualInnerActionPerformed


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.ButtonGroup AntennaFilterGroup;
    private javax.swing.ButtonGroup AntennaLayoutGroup;
    private javax.swing.ButtonGroup AntennaSelectionGroup;
    private javax.swing.JLabel conflictText1;
    private javax.swing.JLabel conflictText2;
    private javax.swing.JLabel conflictText3;
    private javax.swing.JLabel conflictText4;
    private javax.swing.JLabel conflictText5;
    private javax.swing.JLabel conflictText6;
    private javax.swing.JLabel conflictText7;
    private javax.swing.JLabel conflictText8;
    private javax.swing.JLabel conflictWarning1;
    private javax.swing.JLabel conflictWarning2;
    private javax.swing.JLabel conflictWarning3;
    private javax.swing.JLabel conflictWarning4;
    private javax.swing.JLabel conflictWarning5;
    private javax.swing.JLabel conflictWarning6;
    private javax.swing.JLabel conflictWarning7;
    private javax.swing.JLabel conflictWarning8;
    private nl.astron.lofar.sas.otbcomponents.CoreStationLayout coreStationLayout;
    private nl.astron.lofar.sas.otbcomponents.StationSelectionPanel coreStationSelectionPanel;
    private nl.astron.lofar.sas.otbcomponents.EuropeStationLayout europeStationLayout;
    private nl.astron.lofar.sas.otbcomponents.StationSelectionPanel europeStationSelectionPanel;
    private javax.swing.JRadioButton input1070;
    private javax.swing.JRadioButton input1090;
    private javax.swing.JRadioButton input110190;
    private javax.swing.JRadioButton input170230;
    private javax.swing.JRadioButton input210250;
    private javax.swing.JRadioButton input3070;
    private javax.swing.JRadioButton input3090;
    private javax.swing.JComboBox inputClockMode;
    private javax.swing.JRadioButton inputHBAAntennas;
    private javax.swing.JRadioButton inputHBADual;
    private javax.swing.JRadioButton inputHBADualInner;
    private javax.swing.JRadioButton inputHBAJoined;
    private javax.swing.JRadioButton inputHBAJoinedInner;
    private javax.swing.JRadioButton inputHBAOne;
    private javax.swing.JRadioButton inputHBAOneInner;
    private javax.swing.JRadioButton inputHBAZero;
    private javax.swing.JRadioButton inputHBAZeroInner;
    private javax.swing.JRadioButton inputInnerCircle;
    private javax.swing.JRadioButton inputLBAAntennas;
    private javax.swing.JRadioButton inputOuterCircle;
    private javax.swing.JRadioButton inputSparseEven;
    private javax.swing.JRadioButton inputSparseOdd;
    private javax.swing.JRadioButton inputXPolesOnly;
    private javax.swing.JRadioButton inputYPolesOnly;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JLabel labelClockMode;
    private javax.swing.JPanel panelHBASelection1;
    private javax.swing.JPanel panelLBASelection;
    private nl.astron.lofar.sas.otbcomponents.RemoteStationLayout remoteStationLayout;
    private nl.astron.lofar.sas.otbcomponents.StationSelectionPanel remoteStationSelectionPanel;
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
