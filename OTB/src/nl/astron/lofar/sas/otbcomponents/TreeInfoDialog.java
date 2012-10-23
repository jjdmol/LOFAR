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
import com.toedter.components.JSpinField;
import java.rmi.RemoteException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.Iterator;
import java.util.Locale;
import java.util.TimeZone;
import java.util.TreeMap;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JOptionPane;
import nl.astron.lofar.lofarutils.DateTimeChooser;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb3.jDefaultTemplate;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBtree;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import org.apache.log4j.Logger;


/**
 *
 * @created 24-03-2006, 12:34
 *
 * @author  coolen
 *
 * @version $Id$
 */
public final class TreeInfoDialog extends javax.swing.JDialog {
    static Logger logger = Logger.getLogger(TreeInfoDialog.class);
    static String name = "TreeInfoDialog";
    
    /** Creates new form BeanForm
     * 
     * @param   parent      Frame where this dialog belongs
     * @param   modal       Should the Dialog be modal or not
     * @param   aTree       the Tree we work with
     * @param   aMainFrame  the Mainframe we are part off
     */
    public TreeInfoDialog(boolean modal, int[] treeIDs , MainFrame aMainFrame) {
        super(aMainFrame, modal);
        initComponents();
        itsMainFrame = aMainFrame;
        setTree(treeIDs);
    }
    
    public void setTree(int [] treeIDs) {
        itsTreeIDs = treeIDs;
        if (treeIDs.length > 1) {
            itsMultiple=true;
            topLabel.setText("Tree Meta Data  -- MULTIPLE SELECTION -- Only first Tree's info is shown \n" +
                             "                Changes will be applied to all selections");
        } else {
            itsMultiple=false;
            topLabel.setText("Tree Meta Data");
        }
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
    
    private void init() {

        timeWarningLabel.setVisible(false);
        nameLabel.setVisible(false);
        nameInput.setVisible(false);
        setNameButton.setVisible(false);

        isInitialized=false;
        SimpleDateFormat id = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss",itsLocale);
        inputDurationDays.loopEnabled(true);
        inputDurationHours.loopEnabled(true);
        inputDurationMinutes.loopEnabled(true);
        inputDurationSeconds.loopEnabled(true);

        isAdministrator=itsMainFrame.getUserAccount().isAdministrator();
        if (itsTree != null) {
            itsTreeType=OtdbRmi.getTreeType().get(itsTree.type);
            // keep the fields that can be changed
            itsTreeState=OtdbRmi.getTreeState().get(itsTree.state);
            itsClassification = OtdbRmi.getClassif().get(itsTree.classification);
            if(itsTreeType.equals("VHtree")) {
                itsStarttime = itsTree.starttime;
                try {
                    // Get all Beams (if any) from this observation and try to determine the longest duration
                    // try to set the dates
                    itsMaxBeamDuration=0;
                    ArrayList<jOTDBnode> beams = new ArrayList(OtdbRmi.getRemoteMaintenance().getItemList(itsTree.treeID(), "%.Beam[%.duration"));
                    for (jOTDBnode aNode: beams) {
                        try {
                            if (Integer.parseInt(aNode.limits) > itsMaxBeamDuration) {
                                itsMaxBeamDuration=Integer.parseInt(aNode.limits);
                            }
                        } catch (NumberFormatException ex) {
                            logger.error("Integer Conversion error on duration " + aNode.limits + " - " + ex);
                        }
                    }
                    if (itsMaxBeamDuration > 0 ) {
                        //calcDuration takes miliseconds, and beamdurations are in secs.
                        calcDuration(itsMaxBeamDuration*1000);
                    } else {
                        calcDuration(0);
                    }
                } catch (RemoteException e) {
                    String aS="Error getting the Beams " + e;
                    logger.error(aS);
                    LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                }

                if (itsStarttime.length() > 0 && !itsStarttime.equals("not-a-date-time")) {
                    try {
                        itsStartDate = id.parse(itsStarttime);
                    } catch (ParseException ex) {
                        logger.error("Error converting starttime "+itsStarttime);
                        itsStarttime="";
                        itsStartDate=null;
                    }
                } else {
                    itsStartDate=null;
                }
                itsStoptime = itsTree.stoptime;
                if (itsStoptime.length() > 0 && !itsStoptime.equals("not-a-date-time")) {
                    try {
                        itsStopDate = id.parse(itsStoptime);

                    } catch (ParseException ex) {
                        logger.error("Error converting stoptime " + itsStoptime);
                        itsStoptime="";
                        itsStopDate=null;
                    }
                } else {
                    itsStopDate=null;
                }
            }
            if (itsTree.description != null) itsDescription = itsTree.description;
            if (itsTree.processType != null) itsProcessType=itsTree.processType;
            if (itsTree.processSubtype != null) itsProcessSubType=itsTree.processSubtype;
            if (itsTree.strategy != null) itsStrategy=itsTree.strategy;
            initComboLists();
            initFocus();
            initView();
            setDuration();

            isInitialized=true;
            getRootPane().setDefaultButton(saveButton);
        } else {
            logger.debug("No tree found to work with");
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
    
    /*
     * Compose the timestring from a given date
     */
    public void composeTimeString(String time) {
        // Set the dateformat OTDB takes
        SimpleDateFormat id = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss",itsLocale);
        switch (time) {
            case "start":
                if (itsStartDate != null) {
                  startTimeInput.setText(id.format(itsStartDate));
                  if (stopTimeInput.getText().equals("") ||
                          stopTimeInput.getText().equals("not-a-date-time") ||
                          itsStartDate.after(itsStopDate)) {
                    stopTimeInput.setText(id.format(itsStartDate));
                    itsStopDate=itsStartDate;
                  }
                  itsStarttime=startTimeInput.getText();
                } else {
                    startTimeInput.setText("not-a-date-time");
                }
                break;
            case "stop":
                if (itsStopDate != null) {
                    stopTimeInput.setText(id.format(itsStopDate));
                    itsStoptime=stopTimeInput.getText();
                    saveButton.setEnabled(true);
                } else {
                    stopTimeInput.setText("not-a-date-time");
                    saveButton.setEnabled(false);
                }
                if (itsStartDate != null && itsStartDate.after(itsStopDate)) {
                    startTimeInput.setText(id.format(itsStopDate));
                    itsStartDate=itsStopDate;

                }
                break;
        }
    }

    // Calculate days/hours/minutes/secs from milisecs
    // @param secs = time in miliseconds
    private void calcDuration(long msecs) {
        if (msecs > 0) {
            long days=msecs/86400000;
            msecs-=days*86400000;
            long hours=msecs/3600000;
            msecs-=hours*3600000;
            long minutes=msecs/60000;
            msecs-=minutes*60000;
            long seconds=msecs/1000;

            if (days > 365) {
                timeWarningLabel.setVisible(true);
                while (days >= 365) {
                    days-=365;
                }
            }
            inputDurationDays.setValue((int)days);
            inputDurationHours.setValue((int)hours);
            inputDurationMinutes.setValue((int)minutes);
            inputDurationSeconds.setValue((int)seconds);
        } else {
            inputDurationDays.setValue(0);
            inputDurationHours.setValue(0);
            inputDurationMinutes.setValue(0);
            inputDurationSeconds.setValue(0);
        }
    }

    private void setStartTime() {
        if (itsStopDate == null) {
            return;
        }
        // create new startdate based on stopdate - duration
        Calendar cal = Calendar.getInstance();
        cal.setTime(itsStopDate);
        cal.set(Calendar.DAY_OF_YEAR,cal.get(Calendar.DAY_OF_YEAR)-Integer.valueOf(inputDurationDays.getValue()));
        cal.set(Calendar.HOUR,cal.get(Calendar.HOUR)-Integer.valueOf(inputDurationHours.getValue()));
        cal.set(Calendar.MINUTE,cal.get(Calendar.MINUTE)-Integer.valueOf(inputDurationMinutes.getValue()));
        cal.set(Calendar.SECOND,cal.get(Calendar.SECOND)-Integer.valueOf(inputDurationSeconds.getValue()));

        itsStartDate = cal.getTime();
        SimpleDateFormat id = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss",itsLocale);
        itsStarttime=id.format(itsStartDate);
        startTimeInput.setText(itsStarttime);
    }

    private void setStopTime() {
        // create new stopdate based on startdate + duration
        if (itsStartDate == null) {
            return;
        }
        Calendar cal = Calendar.getInstance();
        cal.setTime(itsStartDate);
        cal.set(Calendar.DAY_OF_YEAR,cal.get(Calendar.DAY_OF_YEAR)+Integer.valueOf(inputDurationDays.getValue()));
        cal.set(Calendar.HOUR,cal.get(Calendar.HOUR)+Integer.valueOf(inputDurationHours.getValue()));
        cal.set(Calendar.MINUTE,cal.get(Calendar.MINUTE)+Integer.valueOf(inputDurationMinutes.getValue()));
        cal.set(Calendar.SECOND,cal.get(Calendar.SECOND)+Integer.valueOf(inputDurationSeconds.getValue()));

        itsStopDate = cal.getTime();
        SimpleDateFormat id = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss",itsLocale);
        itsStoptime=id.format(itsStopDate);
        stopTimeInput.setText(itsStoptime);
    }

    // try to calculate the duration from the difference between start/stoptime
    private void setDuration() {
        if (itsStartDate != null && itsStopDate!=null && itsStartDate.before(itsStopDate)) {
            long dur= itsStopDate.getTime()-itsStartDate.getTime();
            calcDuration(dur);
        } else {
            calcDuration(itsMaxBeamDuration*1000);
        }
   }

    private boolean checkTimes() {

        // for now return true because Arno needs to do some testing.
        return true;
/*
        // start the checking. Both dates need to be set.
        // the startdate needs to be at least 4 minutes away from NOW
        // the enddate needs to be further in the future then the starttime/.
        String anErrorMsg = "";
        if (itsStartDate == null || startTimeInput.getText().length() == 0 || startTimeInput.getText().equals("not-a-date-time")) {
            anErrorMsg = "Start time not set";
        } else if (itsStopDate == null || stopTimeInput.getText().length() == 0 || stopTimeInput.getText().equals("not-a-date-time")) {
            anErrorMsg = "Stop time not set";
        } else {
            
            if (itsStopDate.before(itsStartDate)) {
               anErrorMsg = "Stop time BEFORE start time";
            }
            
            // check if date is further away then now(in GMT) + 1 minutes
            Date now = getGMTTime(new Date());
            Calendar cal = Calendar.getInstance();
            cal.setTime(now);
            cal.set(Calendar.MINUTE,cal.get(Calendar.MINUTE)+1);
            Date minTime = getGMTTime(cal.getTime());
            if (itsStartDate.after(now)) {
                anErrorMsg = "That date allready passed I'm afraid.";
            } else if (itsStartDate.before(minTime)) {
                    anErrorMsg = "Start time needs to be minimal 1 minutes away from now (GMT)";
                    // create an Info popup.
                    JOptionPane.showMessageDialog(this,anErrorMsg,"Warning",JOptionPane.WARNING_MESSAGE);
                    return true;            }
            if (itsStopDate.before(itsStartDate)) {
                if (anErrorMsg.length() > 0) {
                    anErrorMsg+=", and ";
                }
                anErrorMsg = "Stop time BEFORE start time";
            }
        }
        if (!anErrorMsg.equals("") ) {
            // create an Error popup.
            JOptionPane.showMessageDialog(this,anErrorMsg,"Error",JOptionPane.ERROR_MESSAGE);
            return false;
        } else {
            return true;
        }
 * 
 */
    }
    
    private void initFocus() {
        // PIC
        switch (itsTreeType) {
            case "hardware":
                momIDLabel.setVisible(false);
                momIDInput.setVisible(false);
                nameLabel.setVisible(false);
                nameInput.setVisible(false);
                setNameButton.setVisible(false);
                originalTreeIDLabel.setVisible(false);
                originalTreeIDInput.setVisible(false);
                campaignLabel.setVisible(false);
                campaignInput.setVisible(false);
                showCampaignButton.setVisible(false);
                startTimeLabel.setVisible(false);
                startTimeInput.setVisible(false);
                durationLabel.setVisible(false);
                durationDayLabel.setVisible(false);
                durationHourLabel.setVisible(false);
                durationMinuteLabel.setVisible(false);
                durationSecondLabel.setVisible(false);
                inputDurationDays.setVisible(false);
                inputDurationHours.setVisible(false);
                inputDurationMinutes.setVisible(false);
                inputDurationSeconds.setVisible(false);
                setDurationButton.setVisible(false);
                stopTimeLabel.setVisible(false);
                stopTimeInput.setVisible(false);
                setStartDateButton.setVisible(false);
                setStopDateButton.setVisible(false);
                descriptionInput.setEnabled(true);
                // VICtemplate
                break;
            case "VItemplate":
                campaignLabel.setVisible(false);
                campaignInput.setVisible(false);
                showCampaignButton.setVisible(false);
                startTimeLabel.setVisible(false);
                startTimeInput.setVisible(false);
                durationLabel.setVisible(false);
                durationDayLabel.setVisible(false);
                durationHourLabel.setVisible(false);
                durationMinuteLabel.setVisible(false);
                durationSecondLabel.setVisible(false);
                inputDurationDays.setVisible(false);
                inputDurationHours.setVisible(false);
                inputDurationMinutes.setVisible(false);
                inputDurationSeconds.setVisible(false);
                setDurationButton.setVisible(false);
                stopTimeLabel.setVisible(false);
                stopTimeInput.setVisible(false);
                setStartDateButton.setVisible(false);
                setStopDateButton.setVisible(false);
                descriptionInput.setEnabled(true);
                
            // VIC
                break;
            case "VHtree":
                nameLabel.setVisible(false);
                nameInput.setVisible(false);
                setNameButton.setVisible(false);
                campaignLabel.setVisible(true);
                campaignInput.setVisible(true);
                showCampaignButton.setVisible(true);
                startTimeLabel.setVisible(true);
                startTimeInput.setVisible(true);
                durationLabel.setVisible(true);
                durationDayLabel.setVisible(true);
                durationHourLabel.setVisible(true);
                durationMinuteLabel.setVisible(true);
                durationSecondLabel.setVisible(true);
                inputDurationDays.setVisible(true);
                inputDurationHours.setVisible(true);
                inputDurationMinutes.setVisible(true);
                inputDurationSeconds.setVisible(true);
                setDurationButton.setVisible(true);
                stopTimeLabel.setVisible(true);
                stopTimeInput.setVisible(true);
                setStartDateButton.setVisible(true);
                setStopDateButton.setVisible(true);
                if (itsMultiple) {
                    descriptionInput.setEnabled(false);
                    inputDurationDays.setEnabled(false);
                    inputDurationHours.setEnabled(false);
                    inputDurationMinutes.setEnabled(false);
                    inputDurationSeconds.setEnabled(false);
                    setDurationButton.setEnabled(false);
                    setStartDateButton.setEnabled(false);
                    setStopDateButton.setEnabled(false);
                    showCampaignButton.setEnabled(false);

                } else {
                    descriptionInput.setEnabled(true);
                    inputDurationDays.setEnabled(true);
                    inputDurationHours.setEnabled(true);
                    inputDurationMinutes.setEnabled(true);
                    inputDurationSeconds.setEnabled(true);
                    setDurationButton.setEnabled(true);
                    setStartDateButton.setEnabled(true);
                    setStopDateButton.setEnabled(true);
                    showCampaignButton.setEnabled(true);
                }
                break;
        }
        if (isAdministrator) {
            classificationInput.setEnabled(true);
        }
        this.groupIDInput.setEditable(false);
        this.processTypeInput.setEditable(false);
        this.processSubTypeInput.setEditable(false);
        this.strategyInput.setEditable(false);
    }
    
    private void initComboLists() {
        DefaultComboBoxModel aClassifModel = new DefaultComboBoxModel();
        TreeMap aClassifMap=OtdbRmi.getClassif();
        Iterator classifIt = aClassifMap.keySet().iterator();
        while (classifIt.hasNext()) {
            aClassifModel.addElement((String)aClassifMap.get(classifIt.next()));
        }
        classificationInput.setModel(aClassifModel);
        
        DefaultComboBoxModel aStateModel = new DefaultComboBoxModel();
        TreeMap aStateMap=OtdbRmi.getTreeState();
        Iterator stateIt = aStateMap.keySet().iterator();
        while (stateIt.hasNext()) {
            aStateModel.addElement((String)aStateMap.get(stateIt.next()));
        }
        stateInput.setModel(aStateModel);
    }
    
    /* Fill the view */
    private void initView() {
        processTypeInput.setEnabled(false);
        processSubTypeInput.setEnabled(false);
        strategyInput.setEnabled(false);
        processTypeInput.setEditable(false);
        processSubTypeInput.setEditable(false);
        strategyInput.setEditable(false);
        // check if the found tree is a defaulttree
        if (itsTreeType.equals("VItemplate")) {
            try {
                itsDefaultTemplateList = new ArrayList(OtdbRmi.getRemoteOTDB().getDefaultTemplates());
                Iterator<jDefaultTemplate> anI = itsDefaultTemplateList.iterator();
                while (anI.hasNext()) {
                    // found DefaultTemplate
                    jDefaultTemplate aT = anI.next();
                    if (aT.treeID() == itsTree.treeID()) {
                        itsName=aT.name;
                        nameInput.setText(aT.name);
                        nameLabel.setVisible(true);
                        nameInput.setVisible(true);
                        setNameButton.setVisible(true);
                        processTypeInput.setEnabled(true);
                        processSubTypeInput.setEnabled(true);
                        strategyInput.setEnabled(true);
                        processTypeInput.setEditable(true);
                        processSubTypeInput.setEditable(true);
                        strategyInput.setEditable(true);
                    }
                }
            }  catch (RemoteException ex) {
                String aS="Error retrieving defaultTemplates: "+ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            }
        } else {
            nameLabel.setVisible(false);
            nameInput.setVisible(false);
            setNameButton.setVisible(false);
        }
        treeIDInput.setText(String.valueOf(itsTree.treeID()));
        momIDInput.setText(String.valueOf(itsTree.momID()));
        classificationInput.setSelectedItem(itsClassification);
        creatorInput.setText(itsTree.creator);
        creationDateInput.setText(itsTree.creationDate.replace("T", " "));
        typeInput.setText((String)OtdbRmi.getTreeType().get(itsTree.type));
        stateInput.setSelectedItem(itsTreeState);
        originalTreeIDInput.setText(String.valueOf(itsTree.originalTree));
        campaignInput.setText(itsTree.campaign);
        startTimeInput.setText(itsTree.starttime.replace("T", " "));
        stopTimeInput.setText(itsTree.stoptime.replace("T", " "));
        descriptionInput.setText(itsTree.description);
        groupIDInput.setText(String.valueOf(itsTree.groupID));
        processTypeInput.setText(itsTree.processType);
        processSubTypeInput.setText(itsTree.processSubtype);
        strategyInput.setText(itsTree.strategy);
    }
    
    private boolean saveNewTree() {
        
        // make sure that if a VICtree is selected we check the start-end time first. If they are not correct, pop up a dialog.

        boolean succes = true;

        if (itsTreeType.equals("VHtree") && stateInput.getSelectedItem().toString().equalsIgnoreCase("Scheduled")) {
            if ( ! checkTimes()) {
                return false;
            }
        }
        try {
            
            // if multiple selections have been made, then the changes should be set to all the selected trees
            if (itsMultiple) {
                for (int i=0; i < itsTreeIDs.length; i++) {
                    jOTDBtree aTree=OtdbRmi.getRemoteOTDB().getTreeInfo(itsTreeIDs[i], false); 
                    String aTreeState=OtdbRmi.getTreeState().get(aTree.state);
                    String aClassification = OtdbRmi.getClassif().get(aTree.classification);
                    // Check treeState and alter in DB when changed
                    if (!aTreeState.equals(stateInput.getSelectedItem().toString())) {
                        aTree.state=OtdbRmi.getRemoteTypes().getTreeState(stateInput.getSelectedItem().toString());
                        hasChanged=true;
                        if (!OtdbRmi.getRemoteMaintenance().setTreeState(aTree.treeID(), aTree.state)) {
                            String aS="Error during setTreeState("+aTree.treeID()+","+aTree.state+"): "+OtdbRmi.getRemoteMaintenance().errorMsg();
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                            succes=false;
                        }
                    }

                    // Check treeClassification and alter in DB when changed
                    if ( !aClassification.equals(classificationInput.getSelectedItem().toString())) {
                        hasChanged=true;
                        aTree.classification=OtdbRmi.getRemoteTypes().getClassif(classificationInput.getSelectedItem().toString());
                        if (!OtdbRmi.getRemoteMaintenance().setClassification(aTree.treeID(), aTree.classification)) {
                            String aS="Error during setClassification("+aTree.treeID()+","+aTree.classification+"): "+OtdbRmi.getRemoteMaintenance().errorMsg();
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                            succes=false;
                        }
                    }
                }
            } else {

                // changable settings for PIC/VIC and templates
                if ( !itsClassification.equals(classificationInput.getSelectedItem().toString())) {
                    hasChanged=true;
                    itsTree.classification=OtdbRmi.getRemoteTypes().getClassif(classificationInput.getSelectedItem().toString());
                    if (!OtdbRmi.getRemoteMaintenance().setClassification(itsTree.treeID(), itsTree.classification)) {
                        String aS="Error during setClassification("+itsTree.treeID()+","+ itsTree.classification+"): "+OtdbRmi.getRemoteMaintenance().errorMsg();
                        logger.error(aS);
                        LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        succes=false;
                    }
                }
                if (!itsTreeState.equals(stateInput.getSelectedItem().toString())) {
                    hasChanged=true;
                    itsTree.state=OtdbRmi.getRemoteTypes().getTreeState(stateInput.getSelectedItem().toString());
                    if (!OtdbRmi.getRemoteMaintenance().setTreeState(itsTree.treeID(), itsTree.state)) {
                        String aS="Error during setTreeState("+itsTree.treeID()+","+itsTree.state+"): "+OtdbRmi.getRemoteMaintenance().errorMsg();
                        logger.error(aS);
                        LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        succes=false;
                    }
                }
                if (!itsDescription.equals(descriptionInput.getText())) {
                    hasChanged=true;
                    itsTree.description = descriptionInput.getText();
                    if (!OtdbRmi.getRemoteMaintenance().setDescription(itsTree.treeID(), itsTree.description)) {
                        String aS="Error during setDescription("+itsTree.treeID()+","+itsTree.description+"): "+OtdbRmi.getRemoteMaintenance().errorMsg();
                        logger.error(aS);
                        LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        succes=false;
                    }
                }
                if (!itsProcessType.equals(processTypeInput.getText()) ||
                    !itsProcessSubType.equals(processSubTypeInput.getText()) ||
                    !itsStrategy.equals(strategyInput.getText())) {
                    hasChanged=true;
                    itsTree.processType = processTypeInput.getText();
                    itsTree.processSubtype = processSubTypeInput.getText();
                    itsTree.strategy = strategyInput.getText();
                    if (!OtdbRmi.getRemoteMaintenance().assignProcessType(itsTree.treeID(), itsTree.processType,itsTree.processSubtype,itsTree.strategy)) {
                        String aS="Error during assignProcessType("+itsTree.treeID()+","+itsTree.processType+","+itsTree.processSubtype+","+itsTree.strategy                                +"): "+OtdbRmi.getRemoteMaintenance().errorMsg();
                        logger.error(aS);
                        LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                        succes=false;
                    }
                }
                // Next for VIC only
                if (itsTreeType.equals("VHtree")) {
                    if (itsStarttime != null && itsStoptime != null &&
                            (!itsStarttime.replace("T", " ").equals(itsTree.starttime.replace("T", " ")) || !itsStoptime.replace("T", " ").equals(itsTree.stoptime.replace("T", " ")))) {
                        if (startTimeInput.getText().length() > 0 && stopTimeInput.getText().length() > 0 ) {
                           hasChanged=true;
                           itsTree.starttime = startTimeInput.getText();
                           itsTree.stoptime = stopTimeInput.getText();
                           if (!OtdbRmi.getRemoteMaintenance().setSchedule(itsTree.treeID(),itsTree.starttime,itsTree.stoptime)) {
                               String aS="Error during setSchedule("+itsTree.treeID()+","+itsTree.starttime+","+itsTree.stoptime+"): "+OtdbRmi.getRemoteMaintenance().errorMsg();
                               logger.error(aS);
                               LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                               succes=false;
                           }
                        }
                    }
                }
                // Next for template only
                if (itsTreeType.equals("VItemplate")) {
                    try {
                        if (itsName != null && !itsName.equals(nameInput.getText())) {
                            itsName=nameInput.getText();
                            if (!OtdbRmi.getRemoteMaintenance().assignTemplateName(itsTree.treeID(),itsName)){
                                String aS="Error during assignTemplateName("+itsTree.treeID()+","+itsName+"): "+OtdbRmi.getRemoteMaintenance().errorMsg();
                                logger.error(aS);
                                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                                succes=false;
                            } else {
                                hasChanged=true;
                            }
                        }
                    } catch (RemoteException ex) {
                        try {
                            String aS="RemoteExceptionError while setting TemplateName " + OtdbRmi.getRemoteMaintenance().errorMsg();
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                            succes=false;
                        } catch (RemoteException ex1) {
                            String aS="Remote Exception Error getting the remote errorMessage";
                            logger.error(aS);
                            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                            succes=false;
                        }
                    }
                }
            }
            return succes;
        } catch (Exception e) {
          String aS="Exception changing metainfo via RMI and JNI failed";
          logger.error(aS);
          LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
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

        jTextField1 = new javax.swing.JTextField();
        jLabel2 = new javax.swing.JLabel();
        stateInput = new javax.swing.JComboBox();
        jLabel3 = new javax.swing.JLabel();
        cancelButton = new javax.swing.JButton();
        saveButton = new javax.swing.JButton();
        jLabel1 = new javax.swing.JLabel();
        nameInput = new javax.swing.JTextField();
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
        jScrollPane1 = new javax.swing.JScrollPane();
        topLabel = new javax.swing.JTextArea();
        setStartDateButton = new javax.swing.JButton();
        setStopDateButton = new javax.swing.JButton();
        durationLabel = new javax.swing.JLabel();
        setDurationButton = new javax.swing.JButton();
        inputDurationDays = new JSpinField(0,364);
        inputDurationDays.adjustWidthToMaximumValue();
        inputDurationHours = new JSpinField(0,23);
        inputDurationHours.adjustWidthToMaximumValue();
        inputDurationSeconds = new JSpinField(0,59);
        inputDurationSeconds.adjustWidthToMaximumValue();
        inputDurationMinutes = new JSpinField(0,59);
        inputDurationMinutes.adjustWidthToMaximumValue();
        durationSecondLabel = new javax.swing.JLabel();
        durationDayLabel = new javax.swing.JLabel();
        durationHourLabel = new javax.swing.JLabel();
        durationMinuteLabel = new javax.swing.JLabel();
        timeWarningLabel = new javax.swing.JLabel();
        timeWarningLabel.setVisible(false);
        showCampaignButton = new javax.swing.JButton();
        nameLabel = new javax.swing.JLabel();
        treeIDInput = new javax.swing.JTextField();
        setNameButton = new javax.swing.JButton();
        groupIDLabel = new javax.swing.JLabel();
        groupIDInput = new javax.swing.JTextField();
        processTypeLabel = new javax.swing.JLabel();
        processTypeInput = new javax.swing.JTextField();
        processSubTypeLabel = new javax.swing.JLabel();
        strategyLabel = new javax.swing.JLabel();
        strategyInput = new javax.swing.JTextField();
        descriptionInput = new javax.swing.JTextArea();
        processSubTypeInput = new javax.swing.JTextField();

        jTextField1.setText("jTextField1");

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("LOFAR View TreeInfo");
        setModal(true);
        setName("loadFileDialog"); // NOI18N
        setResizable(false);
        getContentPane().setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jLabel2.setText("State:");
        getContentPane().add(jLabel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 350, -1, 20));

        stateInput.setToolTipText("State Selection");
        getContentPane().add(stateInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 350, 170, -1));

        jLabel3.setText("Description :");
        getContentPane().add(jLabel3, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 560, 130, 20));

        cancelButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_cancel.png"))); // NOI18N
        cancelButton.setText("Cancel");
        cancelButton.setToolTipText("Cancel filesearch");
        cancelButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        cancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                cancelButtonActionPerformed(evt);
            }
        });
        getContentPane().add(cancelButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 650, 100, -1));

        saveButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_apply.png"))); // NOI18N
        saveButton.setText("Apply");
        saveButton.setToolTipText("Apply changes");
        saveButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        saveButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                saveButtonActionPerformed(evt);
            }
        });
        getContentPane().add(saveButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 650, 90, -1));

        jLabel1.setText("ID:");
        getContentPane().add(jLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 50, -1, 20));

        nameInput.setToolTipText("Give Name for DefaultTree.\n!!!!!! Keep in mind that only Default templates who's names are known to MoM can be used by MoM !!!!!!!\n");
        nameInput.setEnabled(false);
        getContentPane().add(nameInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(250, 50, 200, 20));

        jLabel4.setText("Classification:");
        getContentPane().add(jLabel4, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 230, -1, 20));

        classificationInput.setToolTipText("Select Classification");
        classificationInput.setEnabled(false);
        getContentPane().add(classificationInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 230, 170, -1));

        jLabel6.setText("Creator:");
        getContentPane().add(jLabel6, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 260, -1, 20));

        creatorInput.setToolTipText("Owner for this TreeNode");
        creatorInput.setEnabled(false);
        getContentPane().add(creatorInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 260, 440, -1));

        startTimeLabel.setText("StartTime:");
        getContentPane().add(startTimeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 440, -1, 20));

        stopTimeLabel.setText("StopTime:");
        getContentPane().add(stopTimeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 530, -1, 20));

        startTimeInput.setEditable(false);
        startTimeInput.setToolTipText("Start Time in GMT (YYYY-MMM-DD hh:mm:ss)");
        startTimeInput.setDragEnabled(true);
        getContentPane().add(startTimeInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 440, 340, -1));

        stopTimeInput.setEditable(false);
        stopTimeInput.setToolTipText("Stop Time in GMT (YYYY-MMM-DD hh:mm:ss)");
        getContentPane().add(stopTimeInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 530, 340, -1));

        momIDLabel.setText("MoMID:");
        getContentPane().add(momIDLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 80, -1, 20));

        momIDInput.setToolTipText("MoMID");
        momIDInput.setEnabled(false);
        getContentPane().add(momIDInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 80, 90, -1));

        jLabel10.setText("CreationDate:");
        getContentPane().add(jLabel10, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 290, -1, 20));

        creationDateInput.setToolTipText("Date this entry was created");
        creationDateInput.setEnabled(false);
        getContentPane().add(creationDateInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 290, 440, -1));

        jLabel11.setText("Type:");
        getContentPane().add(jLabel11, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 320, -1, 20));

        typeInput.setEnabled(false);
        getContentPane().add(typeInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 320, 440, -1));

        originalTreeIDLabel.setText("OriginalTree:");
        getContentPane().add(originalTreeIDLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 380, -1, 20));

        originalTreeIDInput.setToolTipText("Original Tree ID");
        originalTreeIDInput.setEnabled(false);
        getContentPane().add(originalTreeIDInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 380, 90, -1));

        campaignLabel.setText("Campaign:");
        getContentPane().add(campaignLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 410, -1, 20));

        campaignInput.setEditable(false);
        campaignInput.setToolTipText("Campaign name");
        getContentPane().add(campaignInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 410, 340, -1));

        jScrollPane1.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        jScrollPane1.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_NEVER);

        topLabel.setColumns(20);
        topLabel.setEditable(false);
        topLabel.setFont(new java.awt.Font("Tahoma", 1, 11));
        topLabel.setRows(2);
        topLabel.setText("Tree Meta Data");
        topLabel.setOpaque(false);
        jScrollPane1.setViewportView(topLabel);

        getContentPane().add(jScrollPane1, new org.netbeans.lib.awtextra.AbsoluteConstraints(0, 0, 570, 40));

        setStartDateButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_edit.gif"))); // NOI18N
        setStartDateButton.setText("set");
        setStartDateButton.setToolTipText("set Start Date");
        setStartDateButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        setStartDateButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                setStartDateButtonActionPerformed(evt);
            }
        });
        getContentPane().add(setStartDateButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(460, 440, 90, -1));

        setStopDateButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_edit.gif"))); // NOI18N
        setStopDateButton.setText("set");
        setStopDateButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        setStopDateButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                setStopDateButtonActionPerformed(evt);
            }
        });
        getContentPane().add(setStopDateButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(470, 530, 90, -1));

        durationLabel.setText("Duration:");
        getContentPane().add(durationLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 480, -1, 20));

        setDurationButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_apply.png"))); // NOI18N
        setDurationButton.setText("set");
        setDurationButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        setDurationButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                setDurationButtonActionPerformed(evt);
            }
        });
        getContentPane().add(setDurationButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(460, 470, 90, -1));

        inputDurationDays.setName("Days"); // NOI18N
        getContentPane().add(inputDurationDays, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 480, 50, -1));

        inputDurationHours.setName("Hours"); // NOI18N
        inputDurationHours.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
            public void propertyChange(java.beans.PropertyChangeEvent evt) {
                inputDurationHoursPropertyChange(evt);
            }
        });
        getContentPane().add(inputDurationHours, new org.netbeans.lib.awtextra.AbsoluteConstraints(180, 480, 40, -1));

        inputDurationSeconds.setName("Seconds\n"); // NOI18N
        inputDurationSeconds.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
            public void propertyChange(java.beans.PropertyChangeEvent evt) {
                inputDurationSecondsPropertyChange(evt);
            }
        });
        getContentPane().add(inputDurationSeconds, new org.netbeans.lib.awtextra.AbsoluteConstraints(300, 480, 40, -1));

        inputDurationMinutes.setName("Minutes"); // NOI18N
        inputDurationMinutes.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
            public void propertyChange(java.beans.PropertyChangeEvent evt) {
                inputDurationMinutesPropertyChange(evt);
            }
        });
        getContentPane().add(inputDurationMinutes, new org.netbeans.lib.awtextra.AbsoluteConstraints(240, 480, 40, -1));

        durationSecondLabel.setText("seconds:");
        getContentPane().add(durationSecondLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(300, 460, -1, -1));

        durationDayLabel.setText("days:");
        getContentPane().add(durationDayLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 460, -1, -1));

        durationHourLabel.setText("hours:");
        getContentPane().add(durationHourLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(180, 460, -1, -1));

        durationMinuteLabel.setText("minutes:");
        getContentPane().add(durationMinuteLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(240, 460, -1, -1));

        timeWarningLabel.setForeground(new java.awt.Color(255, 0, 0));
        timeWarningLabel.setText("WARNING: Observation exceeds maximum length !!!!!!!!!");
        getContentPane().add(timeWarningLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 510, 440, -1));

        showCampaignButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_folder.png"))); // NOI18N
        showCampaignButton.setText("show");
        showCampaignButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        showCampaignButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                showCampaignButtonMouseClicked(evt);
            }
        });
        getContentPane().add(showCampaignButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(460, 410, 90, -1));

        nameLabel.setText("Name:");
        getContentPane().add(nameLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(200, 50, -1, 20));

        treeIDInput.setToolTipText("Tree ID in database");
        treeIDInput.setEnabled(false);
        getContentPane().add(treeIDInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 50, 90, 20));

        setNameButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_apply.png"))); // NOI18N
        setNameButton.setText("set");
        setNameButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        setNameButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                setNameButtonActionPerformed(evt);
            }
        });
        getContentPane().add(setNameButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(470, 50, 100, -1));

        groupIDLabel.setText("GroupID:");
        getContentPane().add(groupIDLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 110, -1, 20));

        groupIDInput.setToolTipText("GroupID");
        groupIDInput.setEnabled(false);
        getContentPane().add(groupIDInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 110, 90, 20));

        processTypeLabel.setText("ProcessType:");
        getContentPane().add(processTypeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 140, -1, 20));

        processTypeInput.setToolTipText("processType");
        getContentPane().add(processTypeInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 140, 130, -1));

        processSubTypeLabel.setText("ProcessSubType:");
        getContentPane().add(processSubTypeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 170, -1, 20));

        strategyLabel.setText("Strategy:");
        getContentPane().add(strategyLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 200, -1, 20));

        strategyInput.setToolTipText("strategy");
        getContentPane().add(strategyInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 200, 210, -1));

        descriptionInput.setLineWrap(true);
        descriptionInput.setRows(3);
        descriptionInput.setToolTipText("Set Description for this tree");
        getContentPane().add(descriptionInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 580, 550, 60));
        getContentPane().add(processSubTypeInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 170, 450, -1));

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private Date getGMTTime(Date aDate) {
        SimpleDateFormat aD   = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss",itsLocale);
        SimpleDateFormat aGMT = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss",itsLocale);
        aGMT.setTimeZone(TimeZone.getTimeZone("GMT"));
        String  aS = aGMT.format(aDate);
        
        Date aGMTDate = null;
        try {
            return aD.parse(aS);
        } catch (ParseException ex) {
            logger.error("Parse Exception in time: ", ex);
        }        
        return aGMTDate;
        
    }
    
    private void setStopDateButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_setStopDateButtonActionPerformed
       Date initialDate = getGMTTime(new Date());
       String aS = stopTimeInput.getText(); 
       if (!aS.equals("")  && !aS.equals("not-a-date-time") ) {
            try {
                SimpleDateFormat aD = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", itsLocale);
                initialDate = aD.parse(aS);
            } catch (ParseException ex) {
                logger.error("Parse Exception in time: ", ex);
            }
        }
        DateTimeChooser chooser = new DateTimeChooser(initialDate);
        itsStopDate = DateTimeChooser.showDialog(this,"StopTime",chooser);
        composeTimeString("stop");
        if (itsStarttime.equals("not-a-date-time") || itsStarttime.equals("") ) {
           setStartTime();
        } else {
           setDuration();
        }
    }//GEN-LAST:event_setStopDateButtonActionPerformed

    private void setStartDateButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_setStartDateButtonActionPerformed
        Date initialDate = getGMTTime(new Date());
        String aS = startTimeInput.getText().replace("T", " ");
        if (!aS.equals("")  && ! aS.equals("not-a-date-time") ) {
            try {
                SimpleDateFormat aD = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss",itsLocale);
                initialDate = aD.parse(aS);
            } catch (ParseException ex) {
                logger.error("Parse Exception in time: ", ex);
            }
        } else {
        }
        DateTimeChooser chooser = new DateTimeChooser(initialDate);
        itsStartDate = DateTimeChooser.showDialog(this,"StartTime",chooser);
        composeTimeString("start");
        setDuration();
        setStopTime();
    }//GEN-LAST:event_setStartDateButtonActionPerformed

    private void saveButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_saveButtonActionPerformed
        // if state has been changed to scheduled, only apply possible when description field is not empty
        if (stateInput.getSelectedItem().toString().equals("scheduled") && descriptionInput.getText().isEmpty()) {
            String errorMsg = "When changing to scheduled, description needs to be filled";
            JOptionPane.showMessageDialog(this,errorMsg,"description error",JOptionPane.ERROR_MESSAGE);
            logger.error(errorMsg );
        } else if (itsTreeType.equals("VHtree") && itsStartDate!= null && itsStopDate != null && itsStartDate.after(itsStopDate)) {
            String errorMsg = "StartDate after stopdate!!!!";
            JOptionPane.showMessageDialog(this,errorMsg,"date error",JOptionPane.ERROR_MESSAGE);
            logger.error(errorMsg );  
        } else {
           if (saveNewTree()) {

                setVisible(false);
                dispose();
            }
        }
    }//GEN-LAST:event_saveButtonActionPerformed

    private void cancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_cancelButtonActionPerformed
        setVisible(false);
        dispose();
    }//GEN-LAST:event_cancelButtonActionPerformed

    private void setDurationButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_setDurationButtonActionPerformed

        // set initialized to false to holt durationupdates from evaluating setValue command
        boolean b=isInitialized;
        isInitialized=false;
        // possibilities:
        // if startDate AND stopdate valid,OR only startDate valid, set will add duration to startTime to set stopTime
        // if only stopDate set, set will subtract duration from stopDate to calculate the startTime

        if ((itsStopDate!=null && !itsStoptime.equals("not-a-date-time"))&& (itsStartDate==null||itsStarttime.equals("not-a-date-time"))) {
            setStartTime();
        } else {
            setStopTime();
        }


        isInitialized=b;
}//GEN-LAST:event_setDurationButtonActionPerformed

    private void inputDurationSecondsPropertyChange(java.beans.PropertyChangeEvent evt) {//GEN-FIRST:event_inputDurationSecondsPropertyChange
         if (isInitialized && evt.getPropertyName().equals("value")) {
            if (((Integer)evt.getOldValue()).intValue() == 0 && ((Integer)evt.getNewValue()).intValue() == 59) {
                int min = inputDurationMinutes.getValue();
                min -= 1;
                if (min < 0) {
                    min=59;
                }
                inputDurationMinutes.setValue(min);
            } else if(((Integer)evt.getOldValue()).intValue() == 59 && ((Integer)evt.getNewValue()).intValue() == 0 ) {
                int min = inputDurationMinutes.getValue();
                min += 1;
                if (min > 59) {
                    min=0;
                }
                inputDurationMinutes.setValue(min);
            }
        }
    }//GEN-LAST:event_inputDurationSecondsPropertyChange

    private void inputDurationMinutesPropertyChange(java.beans.PropertyChangeEvent evt) {//GEN-FIRST:event_inputDurationMinutesPropertyChange
        if (isInitialized && evt.getPropertyName().equals("value")) {
            if (((Integer)evt.getOldValue()).intValue() == 0 && ((Integer)evt.getNewValue()).intValue() == 59) {
                int hr = inputDurationHours.getValue();
                hr -= 1;
                if (hr < 0) {
                    hr=23;
                }
                inputDurationHours.setValue(hr);
            } else if(((Integer)evt.getOldValue()).intValue() == 59 && ((Integer)evt.getNewValue()).intValue() == 0 ) {
                int hr = inputDurationHours.getValue();
                hr +=1;
                if (hr > 23) {
                    hr=0;
                }
                inputDurationHours.setValue(hr);
            }
        }
    }//GEN-LAST:event_inputDurationMinutesPropertyChange

    private void inputDurationHoursPropertyChange(java.beans.PropertyChangeEvent evt) {//GEN-FIRST:event_inputDurationHoursPropertyChange
        if (isInitialized && evt.getPropertyName().equals("value")) {
            if (((Integer)evt.getOldValue()).intValue() == 0 && ((Integer)evt.getNewValue()).intValue() == 23) {
                int day = inputDurationDays.getValue();
                day-=1;
                if (day < 0) {
                    day=364;
                }
                inputDurationDays.setValue(day);
            } else if(((Integer)evt.getOldValue()).intValue() == 23 && ((Integer)evt.getNewValue()).intValue() == 0 ) {
                int day = inputDurationDays.getValue();
                day+=1;
                if (day > 364) {
                    day=0;
                }
                inputDurationDays.setValue(day);
            }
        }
    }//GEN-LAST:event_inputDurationHoursPropertyChange

    private void showCampaignButtonMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_showCampaignButtonMouseClicked
        // show campaignInfo dialog
        if (campaignInfoDialog == null ) {
            campaignInfoDialog = new CampaignInfoDialog(true,itsMainFrame,true);
        } else {
            campaignInfoDialog.refresh();
        }
        campaignInfoDialog.setLocationRelativeTo(this);
        campaignInfoDialog.setVisible(true);
    }//GEN-LAST:event_showCampaignButtonMouseClicked

    private void setNameButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_setNameButtonActionPerformed
        String aName = (String)JOptionPane.showInputDialog(this, 
                "Give Name for DefaultTree.\n\n !!!!!! Keep in mind that only Default templates who's names are known to MoM can be used by MoM !!!!!!! \n\n",
                "DefaultTree Name", 
                JOptionPane.QUESTION_MESSAGE,
                null,
                null,
                nameInput.getText());
        
        // cancelled
        if (aName == null) {
            return;
        }
        
        if (aName.equals("")) {
            nameInput.setText("") ;
            return;
        }

        if (!aName.equals(nameInput.getText())) {
            boolean found=false;
            try {
                ArrayList<jDefaultTemplate> aDFList = new ArrayList(OtdbRmi.getRemoteOTDB().getDefaultTemplates());
                for (jDefaultTemplate it: aDFList) {
                    if (it.name.equals(aName)) {
                        found=true;
                    }
                }
                if (found) {
                    JOptionPane.showMessageDialog(this,"This name has been used allready.", "Duplicate name error", JOptionPane.ERROR_MESSAGE);
                } else {
                    nameInput.setText(aName);
                }
            } catch (RemoteException ex) {
                try {
                    String aS= "Remote exception Error while getting default template list " + OtdbRmi.getRemoteMaintenance().errorMsg();
                    logger.error(aS);
                    LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                } catch (RemoteException ex1) {
                    String aS="Remote Exception Error getting the remote errorMessage";
                    logger.error(aS);
                    LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                }
            }
        }
    }//GEN-LAST:event_setNameButtonActionPerformed


    private MainFrame itsMainFrame = null;
    private jOTDBtree itsTree = null;
    private int []    itsTreeIDs=null;
    private String    itsName=null;
    private boolean   isAdministrator;
    private boolean   hasChanged=false;
    private boolean   itsMultiple=false;
    private String    itsClassification = "";
    private String    itsTreeState = "";
    private String    itsTreeType = "";
    private String    itsStarttime = "";
    private String    itsStoptime = "";
    private String    itsDescription = "";
    private String    itsProcessType = "";
    private String    itsProcessSubType = "";
    private String    itsStrategy = "";
    private Date      itsStartDate = null;
    private Date      itsStopDate = null;
    private Locale    itsLocale = new Locale("en");
    private boolean   isInitialized=false;
    private CampaignInfoDialog campaignInfoDialog=null;
    private int       itsMaxBeamDuration=0;
    private ArrayList<jDefaultTemplate> itsDefaultTemplateList=null;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JTextField campaignInput;
    private javax.swing.JLabel campaignLabel;
    private javax.swing.JButton cancelButton;
    private javax.swing.JComboBox classificationInput;
    private javax.swing.JTextField creationDateInput;
    private javax.swing.JTextField creatorInput;
    private javax.swing.JTextArea descriptionInput;
    private javax.swing.JLabel durationDayLabel;
    private javax.swing.JLabel durationHourLabel;
    private javax.swing.JLabel durationLabel;
    private javax.swing.JLabel durationMinuteLabel;
    private javax.swing.JLabel durationSecondLabel;
    private javax.swing.JTextField groupIDInput;
    private javax.swing.JLabel groupIDLabel;
    private com.toedter.components.JSpinField inputDurationDays;
    private com.toedter.components.JSpinField inputDurationHours;
    private com.toedter.components.JSpinField inputDurationMinutes;
    private com.toedter.components.JSpinField inputDurationSeconds;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel10;
    private javax.swing.JLabel jLabel11;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JTextField jTextField1;
    private javax.swing.JTextField momIDInput;
    private javax.swing.JLabel momIDLabel;
    private javax.swing.JTextField nameInput;
    private javax.swing.JLabel nameLabel;
    private javax.swing.JTextField originalTreeIDInput;
    private javax.swing.JLabel originalTreeIDLabel;
    private javax.swing.JTextField processSubTypeInput;
    private javax.swing.JLabel processSubTypeLabel;
    private javax.swing.JTextField processTypeInput;
    private javax.swing.JLabel processTypeLabel;
    private javax.swing.JButton saveButton;
    private javax.swing.JButton setDurationButton;
    private javax.swing.JButton setNameButton;
    private javax.swing.JButton setStartDateButton;
    private javax.swing.JButton setStopDateButton;
    private javax.swing.JButton showCampaignButton;
    private javax.swing.JTextField startTimeInput;
    private javax.swing.JLabel startTimeLabel;
    private javax.swing.JComboBox stateInput;
    private javax.swing.JTextField stopTimeInput;
    private javax.swing.JLabel stopTimeLabel;
    private javax.swing.JTextField strategyInput;
    private javax.swing.JLabel strategyLabel;
    private javax.swing.JLabel timeWarningLabel;
    private javax.swing.JTextArea topLabel;
    private javax.swing.JTextField treeIDInput;
    private javax.swing.JTextField typeInput;
    // End of variables declaration//GEN-END:variables

}
