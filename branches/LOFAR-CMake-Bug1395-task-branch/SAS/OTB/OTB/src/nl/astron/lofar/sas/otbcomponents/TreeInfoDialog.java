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
import java.util.Calendar;
import java.util.Date;
import java.util.Iterator;
import java.util.Locale;
import java.util.TimeZone;
import java.util.TreeMap;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JOptionPane;
import nl.astron.lofar.lofarutils.DateTimeChooser;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBtree;
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
public class TreeInfoDialog extends javax.swing.JDialog {
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
        itsTreeIDs = treeIDs;
        try {
            // set selected Tree to first in the list
            itsTree=OtdbRmi.getRemoteOTDB().getTreeInfo(itsTreeIDs[0], false);
        } catch (RemoteException e) {
            logger.debug("Error getting the Treeinfo " + e);
            itsTree=null;
        }
        if (treeIDs.length > 1) {
            itsMultiple=true;
        }
        init();        
    }
    
    public void setTree(int [] treeIDs) {
        itsTreeIDs = treeIDs;
        if (treeIDs.length > 1) {
            itsMultiple=true;
        }
        try {
            // set selected Tree to first in the list
            itsTree=OtdbRmi.getRemoteOTDB().getTreeInfo(itsTreeIDs[0], false);
        } catch (RemoteException e) {
            logger.debug("Error getting the Treeinfo " + e);
            itsTree=null;
        }
        init();                
    }
    
    private void init() {
        this.durationValue.setValue(durationValue.getMinimum());
        this.durationValue.loopEnabled(true);
        this.durationValue.adjustWidthToMaximumValue();

        SimpleDateFormat id = new SimpleDateFormat("yyyy-MMM-d HH:mm",itsLocale);
        if (itsMultiple) {
            topLabel.setText("Tree Meta Data  -- MULTIPLE SELECTION -- Only first Tree's info is shown \n" +
                             "                Changes will be applied to all selections");            
        } else {
            topLabel.setText("Tree Meta Data");
        }
        
        isAdministrator=itsMainFrame.getUserAccount().isAdministrator();
        if (itsTree != null) {
            itsTreeType=OtdbRmi.getTreeType().get(itsTree.type);
            // keep the fields that can be changed
            itsTreeState=OtdbRmi.getTreeState().get(itsTree.state);
            itsClassification = OtdbRmi.getClassif().get(itsTree.classification);
            itsCampaign = itsTree.campaign;
            itsStarttime = itsTree.starttime;
            // try to set the dates
            if (itsStarttime.length() > 0 && !itsStarttime.equals("not-a-date-time")) {
                try {
                    itsStartDate = id.parse(itsStarttime);
                    setDurationButton.setEnabled(true);
                    setStopDateButton.setEnabled(true);
                } catch (ParseException ex) {
                    logger.debug("Error converting starttime "+itsStarttime);
                    itsStarttime="";
                    itsStartDate=null;
                }
            }
            itsStoptime = itsTree.stoptime;
            if (itsStoptime.length() > 0 && !itsStoptime.equals("not-a-date-time")) {
                try {
                    itsStopDate = id.parse(itsStoptime);
                } catch (ParseException ex) {
                    logger.debug("Error converting stoptime " + itsStoptime);
                    itsStoptime="";
                    itsStopDate=null;
                }
            }
            
            itsDescription = itsTree.description;     
            initComboLists();
            initView();
            initFocus();
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
        SimpleDateFormat id = new SimpleDateFormat("yyyy-MMM-d HH:mm",itsLocale);
        if (time.equals("start")) {
            if (itsStartDate != null) {
              startTimeInput.setText(id.format(itsStartDate));
              if (stopTimeInput.getText().equals("") || stopTimeInput.getText().equals("not-a-date-time")) {
                stopTimeInput.setText(id.format(itsStartDate));
              }
              setDurationButton.setEnabled(true);
              setStopDateButton.setEnabled(true);
            } else {
                startTimeInput.setText("not-a-date-time");
            }
            
              
        } else if (time.equals("stop")) {
            if (itsStopDate != null) {
                stopTimeInput.setText(id.format(itsStopDate));
                saveButton.setEnabled(true);
            } else {
                stopTimeInput.setText("not-a-date-time");
                saveButton.setEnabled(false);
            }
        }
    }

    private boolean checkTimes() {
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
            
            // check if date is further away then now(in GMT) + 4 minutes
            Date now = getGMTTime(new Date());
            Calendar cal = Calendar.getInstance();
            cal.setTime(now);
            cal.set(Calendar.MINUTE,cal.get(Calendar.MINUTE)+4);
            Date minTime = getGMTTime(cal.getTime());
            if (itsStartDate.before(minTime)) {
                anErrorMsg = "Start time needs to be minimal 4 minutes away from now (GMT)";
            }
            if (itsStopDate.before(itsStartDate)) {
                if (anErrorMsg.length() > 0) {
                    anErrorMsg+=", and ";
                }
                anErrorMsg = "Stop time BEFORE start time";
            }
        }
        
        if (anErrorMsg.length() > 0) {
            // create an Info popup.
            JOptionPane.showMessageDialog(this,anErrorMsg,"Warning",JOptionPane.WARNING_MESSAGE);
            return false;
        }
        return true;

    }
    
    private void initFocus() {
        
            
        // PIC
        if (itsTreeType.equals("hardware")) {
            momIDLabel.setVisible(false);
            momIDInput.setVisible(false);
            originalTreeIDLabel.setVisible(false);
            originalTreeIDInput.setVisible(false);
            campaignLabel.setVisible(false);
            campaignInput.setVisible(false);
            startTimeLabel.setVisible(false);
            startTimeInput.setVisible(false);
            durationLabel.setVisible(false);
            durationValue.setVisible(false);
            inputDurationType.setVisible(false);
            setDurationButton.setVisible(false);
            stopTimeLabel.setVisible(false);
            stopTimeInput.setVisible(false);
            setStartDateButton.setVisible(false);
            setStopDateButton.setVisible(false);
            descriptionInput.setEnabled(true);                
            // VICtemplate    
        } else if (itsTreeType.equals("VItemplate")) {
            if (itsTree.momID() < 1) {
                campaignLabel.setVisible(false);
                campaignInput.setVisible(false);        
            }
            startTimeLabel.setVisible(false);
            startTimeInput.setVisible(false);
            durationLabel.setVisible(false);
            durationValue.setVisible(false);
            inputDurationType.setVisible(false);
            setDurationButton.setVisible(false);
            stopTimeLabel.setVisible(false);
            stopTimeInput.setVisible(false);
            setStartDateButton.setVisible(false);
            setStopDateButton.setVisible(false);
            descriptionInput.setEnabled(true);                
            campaignInput.setEnabled(true);        
            
        // VIC
        } else if (itsTreeType.equals("VHtree")) {
            if (itsTree.momID() < 1) {
                campaignLabel.setVisible(false);
                campaignInput.setVisible(false);        
            }            
            startTimeLabel.setVisible(true);
            startTimeInput.setVisible(true);
            durationLabel.setVisible(true);
            durationValue.setVisible(true);
            inputDurationType.setVisible(true);
            setDurationButton.setVisible(true);
            stopTimeLabel.setVisible(true);
            stopTimeInput.setVisible(true);
            setStartDateButton.setVisible(true);
            setStopDateButton.setVisible(true);
            if (itsMultiple) {
                descriptionInput.setEnabled(false);
                campaignInput.setEnabled(false);        
            } else {
                descriptionInput.setEnabled(true);                
                campaignInput.setEnabled(true);        
            }
        }
        if (isAdministrator) {
            classificationInput.setEnabled(true);
        }
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
/*        // prepare a active/obsolete only combobox for special case state changes
        DefaultComboBoxModel aModel = (DefaultComboBoxModel)stateInput.getModel();
        aModel.removeAllElements();
        aModel.addElement("active");
        aModel.addElement("obsolete");
        
        if (!itsTreeState.equals("active") && !itsTreeState.equals("obsolete")) {
            aModel.addElement(itsTreeState);
        }
        
        if (itsTreeType.equals("hardware") || itsTreeType.equals("VItemplate")) {
            stateInput.setModel(aModel);
        }
*/
        treeIDInput.setText(String.valueOf(itsTree.treeID()));
        momIDInput.setText(String.valueOf(itsTree.momID()));
        classificationInput.setSelectedItem(itsClassification);
        creatorInput.setText(itsTree.creator);
        creationDateInput.setText(itsTree.creationDate);
        typeInput.setText((String)OtdbRmi.getTreeType().get(itsTree.type));
        stateInput.setSelectedItem(itsTreeState);
        originalTreeIDInput.setText(String.valueOf(itsTree.originalTree));
        campaignInput.setText(itsTree.campaign);
        startTimeInput.setText(itsTree.starttime);
        stopTimeInput.setText(itsTree.stoptime);
        descriptionInput.setText(itsDescription);
    }
    
    private boolean saveNewTree() {
        
        // make sure that if a VICtree is selected we check the start-end time first. If they are not correct, pop up a dialog.
        
        if (itsTreeType.equals("VHtree") && itsTreeState.equals("Scheduled")) {
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
                            logger.debug("Error during setTreeState: "+OtdbRmi.getRemoteMaintenance().errorMsg());                      
                        }
                    }

                    // Check treeClassification and alter in DB when changed
                    if ( !aClassification.equals(classificationInput.getSelectedItem().toString())) {
                        hasChanged=true;
                        aTree.classification=OtdbRmi.getRemoteTypes().getClassif(classificationInput.getSelectedItem().toString());
                        if (!OtdbRmi.getRemoteMaintenance().setClassification(aTree.treeID(), aTree.classification)) {
                            logger.debug("Error during setClassification: "+OtdbRmi.getRemoteMaintenance().errorMsg());
                        }
                    }
                    
                    // Check start & stoptimes and alter in DB when changed
                    if (!aTree.starttime.equals(startTimeInput.getText()) || !aTree.stoptime.equals(stopTimeInput.getText())) {
                        hasChanged=true;
                        aTree.starttime = startTimeInput.getText();
                        aTree.stoptime = stopTimeInput.getText();
                        if (OtdbRmi.getRemoteMaintenance().setSchedule(aTree.treeID(),aTree.starttime,aTree.stoptime)) {
                            logger.debug("Error during setSchedule: "+OtdbRmi.getRemoteMaintenance().errorMsg());                        
                        }
                    }
                }
            } else {

                // changable settings for PIC/VIC and templates
                if ( !itsClassification.equals(classificationInput.getSelectedItem().toString())) {
                    hasChanged=true;
                    itsTree.classification=OtdbRmi.getRemoteTypes().getClassif(classificationInput.getSelectedItem().toString());
                    if (!OtdbRmi.getRemoteMaintenance().setClassification(itsTree.treeID(), itsTree.classification)) {
                        logger.debug("Error during setClassification: "+OtdbRmi.getRemoteMaintenance().errorMsg());
                    }
                }
                if (!itsTreeState.equals(stateInput.getSelectedItem().toString())) {
                    hasChanged=true;
                    itsTree.state=OtdbRmi.getRemoteTypes().getTreeState(stateInput.getSelectedItem().toString());
                    if (!OtdbRmi.getRemoteMaintenance().setTreeState(itsTree.treeID(), itsTree.state)) {
                        logger.debug("Error during setTreeState: "+OtdbRmi.getRemoteMaintenance().errorMsg());                      
                    }
                }
                if (!itsDescription.equals(descriptionInput.getText())) {
                    hasChanged=true;
                    itsTree.description = descriptionInput.getText();
                    if (!OtdbRmi.getRemoteMaintenance().setDescription(itsTree.treeID(), itsTree.description)) {
                        logger.debug("Error during setDescription: "+OtdbRmi.getRemoteMaintenance().errorMsg());                        
                    }
                }
                // Next for VIC and Templates only
                if (itsTreeType.equals("VItemplate") || itsTreeType.equals("VHtree")) {
                    if (!itsCampaign.equals(campaignInput.getText()) && itsTree.momID() > 0) {
                        hasChanged=true;
                        itsTree.campaign = campaignInput.getText();
                        if (!OtdbRmi.getRemoteMaintenance().setMomInfo(itsTree.treeID(), itsTree.momID(),itsTree.campaign)) {
                            logger.debug("Error during setCampaign: "+OtdbRmi.getRemoteMaintenance().errorMsg());                        
                        }
                    }
                    // Next for VIC only
                    if (itsTreeType.equals("VHtree")) {
                        if (itsStarttime != null && itsStoptime != null &&
                                (!itsStarttime.equals(startTimeInput.getText()) || !itsStoptime.equals(stopTimeInput.getText()))) {
                            if (startTimeInput.getText().length() > 0 && stopTimeInput.getText().length() > 0 ) {
                               hasChanged=true;
                               itsTree.starttime = startTimeInput.getText();
                               itsTree.stoptime = stopTimeInput.getText();
                               if (OtdbRmi.getRemoteMaintenance().setSchedule(itsTree.treeID(),itsTree.starttime,itsTree.stoptime)) {
                                   logger.debug("Error during setSchedule: "+OtdbRmi.getRemoteMaintenance().errorMsg());                        
                               }
                            }
                        }
                    }
                }
            }
            return true;
        } catch (Exception e) {
          logger.debug("Changing metainfo via RMI and JNI failed"); 
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

        jLabel2 = new javax.swing.JLabel();
        stateInput = new javax.swing.JComboBox();
        jLabel3 = new javax.swing.JLabel();
        descriptionInput = new javax.swing.JTextArea();
        cancelButton = new javax.swing.JButton();
        saveButton = new javax.swing.JButton();
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
        jScrollPane1 = new javax.swing.JScrollPane();
        topLabel = new javax.swing.JTextArea();
        setStartDateButton = new javax.swing.JButton();
        setStopDateButton = new javax.swing.JButton();
        durationLabel = new javax.swing.JLabel();
        setDurationButton = new javax.swing.JButton();
        durationValue = new JSpinField(1,60);
        inputDurationType = new javax.swing.JComboBox();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("LOFAR View TreeInfo");
        setAlwaysOnTop(true);
        setModal(true);
        setName("loadFileDialog"); // NOI18N
        setResizable(false);
        getContentPane().setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jLabel2.setText("State:");
        getContentPane().add(jLabel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 230, -1, 20));

        stateInput.setToolTipText("State Selection");
        getContentPane().add(stateInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 230, 170, -1));

        jLabel3.setText("Description :");
        getContentPane().add(jLabel3, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 420, 130, 20));

        descriptionInput.setRows(3);
        descriptionInput.setToolTipText("Set Description for this tree");
        getContentPane().add(descriptionInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 440, 420, 60));

        cancelButton.setText("Cancel");
        cancelButton.setToolTipText("Cancel filesearch");
        cancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                cancelButtonActionPerformed(evt);
            }
        });
        getContentPane().add(cancelButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 520, -1, -1));

        saveButton.setText("Apply");
        saveButton.setToolTipText("Apply changes");
        saveButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                saveButtonActionPerformed(evt);
            }
        });
        getContentPane().add(saveButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(90, 520, -1, -1));

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
        getContentPane().add(stopTimeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 380, -1, 20));

        startTimeInput.setEditable(false);
        startTimeInput.setToolTipText("Start Time in GMT (YYYY-MMM-DD hh:mm)");
        startTimeInput.setDragEnabled(true);
        getContentPane().add(startTimeInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 320, 270, -1));

        stopTimeInput.setEditable(false);
        stopTimeInput.setToolTipText("Stop Time in GMT (YYYY-MMM-DD hh:mm)");
        getContentPane().add(stopTimeInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 380, 270, -1));

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

        jScrollPane1.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        jScrollPane1.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_NEVER);

        topLabel.setColumns(20);
        topLabel.setEditable(false);
        topLabel.setFont(new java.awt.Font("Tahoma", 1, 11));
        topLabel.setRows(2);
        topLabel.setText("Tree Meta Data");
        topLabel.setOpaque(false);
        jScrollPane1.setViewportView(topLabel);

        getContentPane().add(jScrollPane1, new org.netbeans.lib.awtextra.AbsoluteConstraints(0, 0, 450, 40));

        setStartDateButton.setText("set");
        setStartDateButton.setToolTipText("set Start Date");
        setStartDateButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                setStartDateButtonActionPerformed(evt);
            }
        });
        getContentPane().add(setStartDateButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(370, 320, 60, 20));

        setStopDateButton.setText("set");
        setStopDateButton.setEnabled(false);
        setStopDateButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                setStopDateButtonActionPerformed(evt);
            }
        });
        getContentPane().add(setStopDateButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(370, 380, 60, 20));

        durationLabel.setText("Duration:");
        getContentPane().add(durationLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 350, -1, 20));

        setDurationButton.setText("set");
        setDurationButton.setEnabled(false);
        setDurationButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                setDurationButtonActionPerformed(evt);
            }
        });
        getContentPane().add(setDurationButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(370, 350, 60, 20));
        getContentPane().add(durationValue, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 350, 40, 20));

        inputDurationType.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Minute(s)", "Hour(s)", "Day(s)" }));
        inputDurationType.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputDurationTypeActionPerformed(evt);
            }
        });
        getContentPane().add(inputDurationType, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 350, 110, -1));

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private Date getGMTTime(Date aDate) {
        SimpleDateFormat aD   = new SimpleDateFormat("yyyy-MMM-d HH:mm",itsLocale);
        SimpleDateFormat aGMT = new SimpleDateFormat("yyyy-MMM-d HH:mm",itsLocale);
        aGMT.setTimeZone(TimeZone.getTimeZone("GMT"));
        String  aS = aGMT.format(aDate);
        
        Date aGMTDate = null;
        try {
            return aD.parse(aS);
        } catch (ParseException ex) {
            ex.printStackTrace();
        }        
        return aGMTDate;
        
    }
    
    private void setStopDateButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_setStopDateButtonActionPerformed
       Date initialDate = getGMTTime(new Date());
       String aS = stopTimeInput.getText(); 
       if (!aS.equals("")  && !aS.equals("not-a-date-time") ) {
            try {
                SimpleDateFormat aD = new SimpleDateFormat("yyyy-MMM-d HH:mm", itsLocale);
                initialDate = aD.parse(aS);
            } catch (ParseException ex) {
                ex.printStackTrace();
            }
        }
        DateTimeChooser chooser = new DateTimeChooser(initialDate);
        itsStopDate = DateTimeChooser.showDialog(this,"StopTime",chooser);
        composeTimeString("stop");
    }//GEN-LAST:event_setStopDateButtonActionPerformed

    private void setStartDateButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_setStartDateButtonActionPerformed
        Date initialDate = getGMTTime(new Date());
        String aS = startTimeInput.getText();
        if (!aS.equals("")  && ! aS.equals("not-a-date-time") ) {
            try {
                SimpleDateFormat aD = new SimpleDateFormat("yyyy-MMM-d HH:mm", itsLocale);
                initialDate = aD.parse(aS);
            } catch (ParseException ex) {
                ex.printStackTrace();
            }
        } else {
        }
        DateTimeChooser chooser = new DateTimeChooser(initialDate);
        itsStartDate = DateTimeChooser.showDialog(this,"StartTime",chooser);
        composeTimeString("start");
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
            // create new date based on olddate + duration
            Date now = itsStartDate;
            Calendar cal = Calendar.getInstance();
            cal.setTime(now);
            if (inputDurationType.getSelectedItem().equals("Minute(s)")){
                cal.set(Calendar.MINUTE,cal.get(Calendar.MINUTE)+durationValue.getValue());
            } else if (inputDurationType.getSelectedItem().equals("Hour(s)")){
                cal.set(Calendar.HOUR,cal.get(Calendar.HOUR)+durationValue.getValue());
            } else if (inputDurationType.getSelectedItem().equals("Day(s)")){
                cal.set(Calendar.DAY_OF_MONTH,cal.get(Calendar.DAY_OF_MONTH)+durationValue.getValue());
            }

            itsStopDate = cal.getTime();
            SimpleDateFormat id = new SimpleDateFormat("yyyy-MMM-d HH:mm",itsLocale);
            stopTimeInput.setText(id.format(itsStopDate));
}//GEN-LAST:event_setDurationButtonActionPerformed

    private void inputDurationTypeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputDurationTypeActionPerformed
        if (inputDurationType.getSelectedItem().equals("Minute(s)")) {
            this.durationValue.setMinimum(1);
            this.durationValue.setMaximum(60);
        } else if (inputDurationType.getSelectedItem().equals("Hour(s)")) {
            this.durationValue.setMinimum(1);
            this.durationValue.setMaximum(24);
        } else if (inputDurationType.getSelectedItem().equals("Day(s)")) {
            this.durationValue.setMinimum(1);
            this.durationValue.setMaximum(31);
        }

        this.durationValue.setValue(this.durationValue.getMinimum());
    }//GEN-LAST:event_inputDurationTypeActionPerformed


    private MainFrame itsMainFrame = null;
    private jOTDBtree itsTree = null;
    private int []    itsTreeIDs=null;
    private boolean   isAdministrator;
    private boolean   hasChanged=false;
    private boolean   itsMultiple=false;
    private String    itsClassification = "";
    private String    itsTreeState = "";
    private String    itsTreeType = "";
    private String    itsCampaign = "";
    private String    itsStarttime = "";
    private String    itsStoptime = "";
    private String    itsDescription = "";
    private Date      itsStartDate = null;
    private Date      itsStopDate = null;
    private Locale    itsLocale = new Locale("en");
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JTextField campaignInput;
    private javax.swing.JLabel campaignLabel;
    private javax.swing.JButton cancelButton;
    private javax.swing.JComboBox classificationInput;
    private javax.swing.JTextField creationDateInput;
    private javax.swing.JTextField creatorInput;
    private javax.swing.JTextArea descriptionInput;
    private javax.swing.JLabel durationLabel;
    private com.toedter.components.JSpinField durationValue;
    private javax.swing.JComboBox inputDurationType;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel10;
    private javax.swing.JLabel jLabel11;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JTextField momIDInput;
    private javax.swing.JLabel momIDLabel;
    private javax.swing.JTextField originalTreeIDInput;
    private javax.swing.JLabel originalTreeIDLabel;
    private javax.swing.JButton saveButton;
    private javax.swing.JButton setDurationButton;
    private javax.swing.JButton setStartDateButton;
    private javax.swing.JButton setStopDateButton;
    private javax.swing.JTextField startTimeInput;
    private javax.swing.JLabel startTimeLabel;
    private javax.swing.JComboBox stateInput;
    private javax.swing.JTextField stopTimeInput;
    private javax.swing.JLabel stopTimeLabel;
    private javax.swing.JTextArea topLabel;
    private javax.swing.JTextField treeIDInput;
    private javax.swing.JTextField typeInput;
    // End of variables declaration//GEN-END:variables

}
