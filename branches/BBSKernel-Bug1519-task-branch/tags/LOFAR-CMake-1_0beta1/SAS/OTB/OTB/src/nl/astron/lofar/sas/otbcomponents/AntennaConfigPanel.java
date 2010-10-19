/*
 * AntennaConfigPanel.java
 *
 * Created on 18 maart 2009, 7:38
 */

package nl.astron.lofar.sas.otbcomponents;

import java.awt.Component;
import java.rmi.RemoteException;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.JPanel;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBparam;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otbcomponents.bbs.BBSStrategyPanel;
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
    private Vector<jOTDBparam> itsParamList;
    private String             itsTreeType="";

    // AntennaConfig parameters
    private jOTDBnode itsAntennaArray;
    private jOTDBnode itsAntennaSet;
    private jOTDBnode itsBandFilter;
    private jOTDBnode itsLongBaselines;
    private jOTDBnode itsStationList;

    // Clockmode
    private jOTDBnode itsClockMode;


    private Vector<String>    itsUsedCoreStations      = new Vector<String>();
    private Vector<String>    itsUsedRemoteStations    = new Vector<String>();
    private Vector<String>    itsUsedEuropeStations    = new Vector<String>();


    /**
     * Creates new BBSStrategyPanel instance
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
     * <br><br>
     * <b>Important</b>: The jOTDBnode to be passed on should always be the 'BBS.Strategy' node.
     * @param anObject the BBS Strategy jOTDBnode to be displayed in the GUI.
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
                }else if (LofarUtils.keyName(aNode.name).equals("VirtualInstrument")) {
                    this.retrieveAndDisplayChildDataForNode(aNode);
                }
           }
        } catch (RemoteException ex) {
            logger.debug("Error during getComponentParam: "+ ex);
            itsParamList=null;
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
            logger.debug("Error: saveNode failed : " + ex);
        }
    }

    public boolean saveInput() {

        //StationList
        if (this.itsStationList != null && !getUsedStations().equals(itsStationList.limits)) {
            itsStationList.limits = getUsedStations();
            logger.trace("Variable VirtualInstrumenst ("+itsStationList.name+"//"+itsStationList.treeID()+"//"+itsStationList.nodeID()+"//"+itsStationList.parentID()+"//"+itsStationList.paramDefID()+") updating to :"+itsStationList.limits);
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
        } else if (this.inputSparse.isSelected()) {
          antennaSet = "LBA_SPARSE";
        } else if (this.inputXPolesOnly.isSelected()) {
          antennaSet = "LBA_X";
        } else if (this.inputYPolesOnly.isSelected()) {
          antennaSet = "LBA_Y";
        } else if (this.inputHBAOne.isSelected()) {
          antennaSet = "HBA_ONE";
        } else if (this.inputHBABoth.isSelected()) {
          antennaSet = "HBA_BOTH";
        }
        if (itsAntennaSet != null && !antennaSet.equals(itsAntennaSet.limits)) {
            itsAntennaSet.limits = antennaSet;
            saveNode(itsAntennaSet);
        }

        //BandFilter
        String bandFilter="LBA_30_80";
        if (this.input1090.isSelected()) {
            bandFilter="LBA_10_90";
        } else if (this.input1070.isSelected()) {
            bandFilter="LBA_10_90";
        } else if (this.input3070.isSelected()) {
            bandFilter="LBA_30_80";
        } else if (this.input110190.isSelected()) {
            bandFilter="HBA_110_190";
        } else if (this.input170230.isSelected()) {
            bandFilter="HBA_170_230";
        } else if (this.input210250.isSelected()) {
            bandFilter="HBA_210_250";
        }
        if (itsBandFilter != null && !bandFilter.equals(itsBandFilter.limits)) {
            itsBandFilter.limits = bandFilter;
            saveNode(itsBandFilter);
        }

        //LongBaselines
        String longBaselines="False";
        if (this.inputLongBaselines.isSelected()) {
            longBaselines="True";
        }
        if (itsLongBaselines != null && !longBaselines.equals(itsLongBaselines.limits)) {
            itsLongBaselines.limits = longBaselines;
            saveNode(itsLongBaselines);
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
      if (itsAntennaSet.limits.equals("LBA_OUTER")) {
        inputOuterCircle.setSelected(true);
      } else if (itsAntennaSet.limits.equals("LBA_SPARSE")) {
        inputSparse.setSelected(true);
      } else if (itsAntennaSet.limits.equals("LBA_X")) {
        inputXPolesOnly.setSelected(true);
      } else if (itsAntennaSet.limits.equals("LBA_Y")) {
        inputYPolesOnly.setSelected(true);
      } else if (itsAntennaSet.limits.equals("HBA_ONE")) {
        inputHBAOne.setSelected(true);
      } else if (itsAntennaSet.limits.equals("HBA_BOTH")) {
        inputHBABoth.setSelected(true);
      } else {
        inputInnerCircle.setSelected(true);
      }

      if (itsBandFilter.limits.equals("LBA_10_90")) {
        input1090.setSelected(true);
      } else if (itsBandFilter.limits.equals("HBA_110_190")) {
        input110190.setSelected(true);
      } else if (itsBandFilter.limits.equals("HBA_170_230")) {
        input170230.setSelected(true);
      } else if (itsBandFilter.limits.equals("HBA_210_250")) {
        input210250.setSelected(true);
      } else {
        input3080.setSelected(true);
      }

      if (itsLongBaselines.limits.toLowerCase().equals("true")) {
        inputLongBaselines.setSelected(true);
      } else {
        inputLongBaselines.setSelected(false);
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
        return new BBSStrategyPanel();
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
            if (aKeyName.equals("antennaArray")) {
                inputLBAAntennas.setToolTipText(aParam.description);
                inputHBAAntennas.setToolTipText(aParam.description);
                itsAntennaArray=aNode;
                if (aNode.limits.equals("HBA")) {
                    inputHBAAntennas.setSelected(true);
                } else {
                    inputLBAAntennas.setSelected(true);
                }
            } else if (aKeyName.equals("antennaSet")) {
                inputInnerCircle.setToolTipText(aParam.description);
                inputOuterCircle.setToolTipText(aParam.description);
                inputSparse.setToolTipText(aParam.description);
                inputXPolesOnly.setToolTipText(aParam.description);
                inputYPolesOnly.setToolTipText(aParam.description);
                inputHBAOne.setToolTipText(aParam.description);
                inputHBABoth.setToolTipText(aParam.description);
                this.itsAntennaSet=aNode;
                if (aNode.limits.equals("LBA_OUTER")) {
                    inputOuterCircle.setSelected(true);
                } else if (aNode.limits.equals("LBA_SPARSE")) {
                    inputSparse.setSelected(true);
                } else if (aNode.limits.equals("LBA_X")) {
                    inputXPolesOnly.setSelected(true);
                } else if (aNode.limits.equals("LBA_Y")) {
                    inputYPolesOnly.setSelected(true);
                } else if (aNode.limits.equals("HBA_ONE")) {
                    inputHBAOne.setSelected(true);
                } else if (aNode.limits.equals("HBA_BOTH")) {
                    inputHBABoth.setSelected(true);
                } else {
                    inputInnerCircle.setSelected(true);
                }
            } else if (aKeyName.equals("bandFilter")) {
                input3080.setToolTipText(aParam.description);
                input1090.setToolTipText(aParam.description);
                input110190.setToolTipText(aParam.description);
                input170230.setToolTipText(aParam.description);
                input210250.setToolTipText(aParam.description);
                this.itsBandFilter=aNode;
                if (aNode.limits.equals("LBA_10_90")) {
                    if (inputClockMode.getSelectedItem().equals("<<Clock200")) {
                        input1090.setSelected(true);
                    } else {
                        input1070.setSelected(true);
                    }
                } else if (aNode.limits.equals("HBA_110_190")) {
                    input110190.setSelected(true);
                } else if (aNode.limits.equals("HBA_170_230")) {
                    input170230.setSelected(true);
                } else if (aNode.limits.equals("HBA_210_250")) {
                    input210250.setSelected(true);
                } else {
                    if (inputClockMode.getSelectedItem().equals("<<Clock200")) {
                        input3080.setSelected(true);
                    } else {
                        input3070.setSelected(true);
                    }
                }
            } else if (aKeyName.equals("longBaselines")) {
                this.itsLongBaselines=aNode;
                if (aNode.limits.toLowerCase().equals("true")) {
                    this.inputLongBaselines.setSelected(true);
                } else {
                    this.inputLongBaselines.setSelected(false);
                }
            } else if (aKeyName.equals("clockMode")) {
                inputClockMode.setToolTipText(aParam.description);
                LofarUtils.setPopupComboChoices(inputClockMode,aParam.limits);
                if (!aNode.limits.equals("")) {
                    inputClockMode.setSelectedItem(aNode.limits);
                }
                itsClockMode=aNode;
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
                if (aS[i].substring(0,2).equals("CS") ) {
                    itsUsedCoreStations.add(aS[i]);
                } else if (aS[i].substring(0,2).equals("RS")) {
                    itsUsedRemoteStations.add(aS[i]);
                } else {
                    itsUsedEuropeStations.add(aS[i]);
                }
            }
            this.coreStationSelectionPanel.setUsedStationList(itsUsedCoreStations);
            this.remoteStationSelectionPanel.setUsedStationList(itsUsedRemoteStations);
            this.europeStationSelectionPanel.setUsedStationList(itsUsedEuropeStations);
        }
    }

    /** Checks all settings and enables/disables all what is needed.
     *
     */
    private void checkSettings() {

        // all stuff for Station Settings based on choices
        if (this.inputLBAAntennas.isSelected() ) {
            this.input1090.setEnabled(true);
            this.input3080.setEnabled(true);
            this.input1070.setEnabled(true);
            this.input3070.setEnabled(true);
            this.inputLongBaselines.setEnabled(true);
            this.inputInnerCircle.setEnabled(true);
            this.inputOuterCircle.setEnabled(true);
            this.input110190.setEnabled(false);
            this.input170230.setEnabled(false);
            this.input210250.setEnabled(false);
            this.inputHBAOne.setEnabled(false);
            this.inputHBATwo.setEnabled(false);
            this.inputHBABoth.setEnabled(false);
            this.inputClockMode.setEnabled(false);
            if (this.inputInnerCircle.isSelected()) {
                this.inputLongBaselines.setEnabled(true);
            } else {
                this.inputLongBaselines.setEnabled(false);
            }

        } else {
            this.input1090.setEnabled(false);
            this.input3080.setEnabled(false);
            this.input1070.setEnabled(false);
            this.input3070.setEnabled(false);
            this.inputLongBaselines.setEnabled(false);
            this.inputInnerCircle.setEnabled(false);
            this.inputOuterCircle.setEnabled(false);
            this.input110190.setEnabled(true);
            this.input170230.setEnabled(true);
            this.input210250.setEnabled(true);
            this.inputHBAOne.setEnabled(true);
            this.inputHBATwo.setEnabled(true);
            this.inputHBABoth.setEnabled(true);
            this.inputClockMode.setEnabled(true);

        }


        // all stuff for CoreStationLayout Settings based on choices
        this.coreStationLayout.setHBALeftSquareEnabled(inputHBAOne.isEnabled()||inputHBABoth.isSelected());
        this.coreStationLayout.setHBALeftSquareSelected(inputHBAOne.isSelected()||inputHBABoth.isSelected());
        this.coreStationLayout.setHBARightSquareEnabled(inputHBATwo.isEnabled()||inputHBABoth.isSelected());
        this.coreStationLayout.setHBARightSquareSelected(inputHBATwo.isSelected()||inputHBABoth.isSelected());
        this.coreStationLayout.setLBAInnerCircleEnabled(inputInnerCircle.isEnabled());
        this.coreStationLayout.setLBAInnerCircleSelected(inputInnerCircle.isSelected());
        this.coreStationLayout.setLBAOuterCircleEnabled(inputOuterCircle.isEnabled());
        this.coreStationLayout.setLBAOuterCircleSelected(inputOuterCircle.isSelected());
        this.coreStationLayout.setLBALongBaselineLeftEnabled(inputLongBaselines.isEnabled());
        this.coreStationLayout.setLBALongBaselineLeftSelected(inputLongBaselines.isSelected());
        this.coreStationLayout.setLBALongBaselineRightEnabled(inputLongBaselines.isEnabled());
        this.coreStationLayout.setLBALongBaselineRightSelected(inputLongBaselines.isSelected());

        // all stuff for RemoteStationLayout Settings based on choices
        this.remoteStationLayout.setHBALeftSquareEnabled(inputHBAOne.isEnabled()||inputHBATwo.isEnabled()||inputHBABoth.isEnabled());
        this.remoteStationLayout.setHBALeftSquareSelected(inputHBAOne.isSelected()||inputHBATwo.isSelected()||inputHBABoth.isSelected());
        this.remoteStationLayout.setLBAInnerCircleEnabled(inputInnerCircle.isEnabled());
        this.remoteStationLayout.setLBAInnerCircleSelected(inputInnerCircle.isSelected());
        this.remoteStationLayout.setLBAOuterCircleEnabled(inputOuterCircle.isEnabled());
        this.remoteStationLayout.setLBAOuterCircleSelected(inputOuterCircle.isSelected());

        // all stuff for EuropeStationLayout Settings based on choices
        this.europeStationLayout.setHBALeftSquareEnabled(inputHBAOne.isEnabled()||inputHBATwo.isEnabled()||inputHBABoth.isEnabled());
        this.europeStationLayout.setHBALeftSquareSelected(inputHBAOne.isSelected()||inputHBATwo.isSelected()||inputHBABoth.isSelected());
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
                logger.debug("ObservationPanel: Error getting treeInfo/treetype" + ex);
                itsTreeType="";
            }
         } else {
            logger.debug("ERROR:  no node given");
        }

        // set defaults/initial settings
        restore();



        if (itsTreeType.equals("VHtree")) {
            this.setButtonsVisible(false);
            this.setAllEnabled(false);
        }


        itsMainFrame.setNormalCursor();

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
        inputSparse = new javax.swing.JRadioButton();
        inputXPolesOnly = new javax.swing.JRadioButton();
        inputYPolesOnly = new javax.swing.JRadioButton();
        inputLongBaselines = new javax.swing.JCheckBox();
        jLabel1 = new javax.swing.JLabel();
        input3080 = new javax.swing.JRadioButton();
        input1090 = new javax.swing.JRadioButton();
        input1070 = new javax.swing.JRadioButton();
        input3070 = new javax.swing.JRadioButton();
        panelHBASelection1 = new javax.swing.JPanel();
        inputHBAOne = new javax.swing.JRadioButton();
        inputHBABoth = new javax.swing.JRadioButton();
        jLabel2 = new javax.swing.JLabel();
        input110190 = new javax.swing.JRadioButton();
        input170230 = new javax.swing.JRadioButton();
        input210250 = new javax.swing.JRadioButton();
        inputHBATwo = new javax.swing.JRadioButton();
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

        AntennaLayoutGroup.add(inputSparse);
        inputSparse.setText("Sparse");
        inputSparse.setEnabled(false);
        inputSparse.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputSparseActionPerformed(evt);
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

        inputLongBaselines.setText("long baselines");
        inputLongBaselines.setEnabled(false);
        inputLongBaselines.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputLongBaselinesActionPerformed(evt);
            }
        });

        jLabel1.setFont(new java.awt.Font("Tahoma", 1, 11));
        jLabel1.setText("Freq. selection");

        AntennaFilterGroup.add(input3080);
        input3080.setSelected(true);
        input3080.setText("30-80 MHz");
        input3080.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                input3080ActionPerformed(evt);
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

        javax.swing.GroupLayout panelLBASelectionLayout = new javax.swing.GroupLayout(panelLBASelection);
        panelLBASelection.setLayout(panelLBASelectionLayout);
        panelLBASelectionLayout.setHorizontalGroup(
            panelLBASelectionLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(panelLBASelectionLayout.createSequentialGroup()
                .addGroup(panelLBASelectionLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(input3080)
                    .addComponent(input1090)
                    .addComponent(input1070)
                    .addComponent(input3070)
                    .addComponent(jLabel1)
                    .addComponent(inputLongBaselines)
                    .addComponent(inputYPolesOnly)
                    .addComponent(inputXPolesOnly)
                    .addComponent(inputSparse)
                    .addComponent(inputOuterCircle)
                    .addComponent(inputInnerCircle))
                .addContainerGap(14, Short.MAX_VALUE))
        );
        panelLBASelectionLayout.setVerticalGroup(
            panelLBASelectionLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(panelLBASelectionLayout.createSequentialGroup()
                .addComponent(inputInnerCircle)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(inputOuterCircle)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(inputSparse)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(inputXPolesOnly)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(inputYPolesOnly)
                .addGap(18, 18, 18)
                .addComponent(inputLongBaselines)
                .addGap(18, 18, 18)
                .addComponent(jLabel1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(input3080)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(input1090)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(input1070)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(input3070)
                .addContainerGap())
        );

        panelHBASelection1.setBorder(javax.swing.BorderFactory.createTitledBorder("Antenna Selection"));

        AntennaLayoutGroup.add(inputHBAOne);
        inputHBAOne.setSelected(true);
        inputHBAOne.setText("Square One");
        inputHBAOne.setEnabled(false);
        inputHBAOne.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputHBAOneActionPerformed(evt);
            }
        });

        AntennaLayoutGroup.add(inputHBABoth);
        inputHBABoth.setText("Both Squares");
        inputHBABoth.setEnabled(false);
        inputHBABoth.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputHBABothActionPerformed(evt);
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

        AntennaLayoutGroup.add(inputHBATwo);
        inputHBATwo.setText("Square Two");
        inputHBATwo.setEnabled(false);
        inputHBATwo.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputHBATwoActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout panelHBASelection1Layout = new javax.swing.GroupLayout(panelHBASelection1);
        panelHBASelection1.setLayout(panelHBASelection1Layout);
        panelHBASelection1Layout.setHorizontalGroup(
            panelHBASelection1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(panelHBASelection1Layout.createSequentialGroup()
                .addGroup(panelHBASelection1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(inputHBAOne)
                    .addComponent(jLabel2)
                    .addComponent(input110190)
                    .addComponent(input170230)
                    .addComponent(input210250)
                    .addComponent(inputHBATwo)
                    .addComponent(inputHBABoth))
                .addContainerGap(10, Short.MAX_VALUE))
        );
        panelHBASelection1Layout.setVerticalGroup(
            panelHBASelection1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(panelHBASelection1Layout.createSequentialGroup()
                .addComponent(inputHBAOne)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(inputHBATwo)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(inputHBABoth)
                .addGap(80, 80, 80)
                .addComponent(jLabel2)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(input110190)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(input170230)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(input210250)
                .addContainerGap(55, Short.MAX_VALUE))
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
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(inputLBAAntennas)
                            .addComponent(panelLBASelection, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGap(16, 16, 16)
                        .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(panelHBASelection1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(inputHBAAntennas)))
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addComponent(labelClockMode, javax.swing.GroupLayout.PREFERRED_SIZE, 86, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(inputClockMode, javax.swing.GroupLayout.PREFERRED_SIZE, 131, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addGap(80, 80, 80))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(inputLBAAntennas)
                    .addComponent(inputHBAAntennas))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(panelLBASelection, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(panelHBASelection1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(labelClockMode)
                    .addComponent(inputClockMode, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(338, Short.MAX_VALUE))
        );

        add(jPanel1, java.awt.BorderLayout.WEST);

        jPanel2.setBorder(javax.swing.BorderFactory.createTitledBorder("Station Selections"));

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addContainerGap()
                        .addComponent(coreStationSelectionPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(remoteStationSelectionPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(europeStationSelectionPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
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
                .addContainerGap(113, Short.MAX_VALUE))
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

        javax.swing.GroupLayout jPanel3Layout = new javax.swing.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(europeStationLayout, javax.swing.GroupLayout.DEFAULT_SIZE, 411, Short.MAX_VALUE)
            .addComponent(remoteStationLayout, javax.swing.GroupLayout.DEFAULT_SIZE, 411, Short.MAX_VALUE)
            .addComponent(coreStationLayout, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 411, Short.MAX_VALUE)
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addComponent(coreStationLayout, javax.swing.GroupLayout.PREFERRED_SIZE, 219, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(remoteStationLayout, javax.swing.GroupLayout.PREFERRED_SIZE, 203, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(europeStationLayout, javax.swing.GroupLayout.PREFERRED_SIZE, 203, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(89, 89, 89))
        );

        add(jPanel3, java.awt.BorderLayout.CENTER);
    }// </editor-fold>//GEN-END:initComponents

    private void inputLBAAntennasActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputLBAAntennasActionPerformed
        this.inputOuterCircle.setSelected(true);
        if (inputClockMode.getSelectedItem().equals("<<Clock200")) {
            this.input3080.setSelected(true);
        } else {
            this.input3070.setSelected(true);
        }
        checkSettings();
    }//GEN-LAST:event_inputLBAAntennasActionPerformed

    private void inputHBAAntennasActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputHBAAntennasActionPerformed
        this.inputHBAOne.setSelected(true);
        this.inputLongBaselines.setSelected(false);
        this.input110190.setSelected(true);
        this.inputClockMode.setSelectedItem("<<Clock200");
        checkSettings();
    }//GEN-LAST:event_inputHBAAntennasActionPerformed

    private void inputInnerCircleActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputInnerCircleActionPerformed
        this.inputLongBaselines.setSelected(true);
        this.inputLongBaselines.setEnabled(true);
        checkSettings();
    }//GEN-LAST:event_inputInnerCircleActionPerformed

    private void inputOuterCircleActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputOuterCircleActionPerformed
        this.inputLongBaselines.setSelected(false);
        this.inputLongBaselines.setEnabled(false);
        checkSettings();
    }//GEN-LAST:event_inputOuterCircleActionPerformed

    private void inputSparseActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputSparseActionPerformed
        checkSettings();
}//GEN-LAST:event_inputSparseActionPerformed

    private void inputXPolesOnlyActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputXPolesOnlyActionPerformed
        checkSettings();
    }//GEN-LAST:event_inputXPolesOnlyActionPerformed

    private void inputYPolesOnlyActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputYPolesOnlyActionPerformed
        checkSettings();
    }//GEN-LAST:event_inputYPolesOnlyActionPerformed

    private void inputLongBaselinesActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputLongBaselinesActionPerformed
        checkSettings();
    }//GEN-LAST:event_inputLongBaselinesActionPerformed

    private void input3080ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_input3080ActionPerformed
        inputClockMode.setSelectedItem("<<Clock200");
        checkSettings();
    }//GEN-LAST:event_input3080ActionPerformed

    private void input1090ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_input1090ActionPerformed
        inputClockMode.setSelectedItem("<<Clock200");
        checkSettings();
    }//GEN-LAST:event_input1090ActionPerformed

    private void inputHBAOneActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputHBAOneActionPerformed
        checkSettings();
}//GEN-LAST:event_inputHBAOneActionPerformed

    private void inputHBABothActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputHBABothActionPerformed
        checkSettings();
}//GEN-LAST:event_inputHBABothActionPerformed

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
        if (evt.getActionCommand().equals("HBALeftSquare")) {
            if (this.inputHBATwo.isSelected()) {
                this.inputHBABoth.setSelected(true);
            } else {
                this.inputHBAOne.setSelected(true);
            }
        } else if (evt.getActionCommand().equals("HBARightSquare")) {
            if (this.inputHBAOne.isSelected()) {
                this.inputHBABoth.setSelected(true);
            } else {
                this.inputHBATwo.setSelected(true);
            }
        } else if (evt.getActionCommand().equals("LBAInnerCircle")) {
            this.inputInnerCircle.setSelected(!this.coreStationLayout.isLBAInnerCircleSelected());
            this.inputLongBaselines.setSelected(true);
        } else if (evt.getActionCommand().equals("LBAOuterCircle")) {
            this.inputOuterCircle.setSelected(!this.coreStationLayout.isLBAOuterCircleSelected());
            this.inputLongBaselines.setSelected(false);
        } else if (evt.getActionCommand().equals("LBALongBaselineLeft")) {
            this.inputLongBaselines.setSelected(!this.coreStationLayout.isLBALongBaselineLeftSelected());
        } else if (evt.getActionCommand().equals("LBALongBaselineRight")) {
            this.inputLongBaselines.setSelected(!this.coreStationLayout.isLBALongBaselineRightSelected());
        }
        checkSettings();
    }//GEN-LAST:event_coreStationLayoutActionPerformed

    private void remoteStationLayoutActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_remoteStationLayoutActionPerformed
        if (evt.getActionCommand().equals("HBALeftSquare")) {
            this.inputHBABoth.setSelected(true);
        } else if (evt.getActionCommand().equals("LBAInnerCircle")) {
            this.inputLongBaselines.setSelected(true);
            this.inputInnerCircle.setSelected(!this.remoteStationLayout.isLBAInnerCircleSelected());
        } else if (evt.getActionCommand().equals("LBAOuterCircle")) {
            this.inputLongBaselines.setSelected(false);
            this.inputOuterCircle.setSelected(!this.remoteStationLayout.isLBAOuterCircleSelected());
        }
        checkSettings();
    }//GEN-LAST:event_remoteStationLayoutActionPerformed

    private void europeStationLayoutActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_europeStationLayoutActionPerformed
        if (evt.getActionCommand().equals("HBALeftSquare")) {
            this.inputHBABoth.setSelected(true);
        } else if (evt.getActionCommand().equals("LBACircle")) {
            this.inputOuterCircle.setSelected(this.europeStationLayout.isLBACircleSelected());
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

    private void inputHBATwoActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputHBATwoActionPerformed
        checkSettings();
}//GEN-LAST:event_inputHBATwoActionPerformed


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.ButtonGroup AntennaFilterGroup;
    private javax.swing.ButtonGroup AntennaLayoutGroup;
    private javax.swing.ButtonGroup AntennaSelectionGroup;
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
    private javax.swing.JRadioButton input3080;
    private javax.swing.JComboBox inputClockMode;
    private javax.swing.JRadioButton inputHBAAntennas;
    private javax.swing.JRadioButton inputHBABoth;
    private javax.swing.JRadioButton inputHBAOne;
    private javax.swing.JRadioButton inputHBATwo;
    private javax.swing.JRadioButton inputInnerCircle;
    private javax.swing.JRadioButton inputLBAAntennas;
    private javax.swing.JCheckBox inputLongBaselines;
    private javax.swing.JRadioButton inputOuterCircle;
    private javax.swing.JRadioButton inputSparse;
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
