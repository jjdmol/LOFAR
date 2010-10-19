/*
 * NodeViewPanel.java
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

import java.awt.Component;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.ListSelectionModel;
import nl.astron.lofar.lofarutils.DateTimeChooser;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.SharedVars;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.tablemodels.LogParamTableModel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import org.apache.log4j.Logger;

/**
 * @created 26-01-2006, 15:47
 *
 * @author  coolen
 *
 * @version $Id$
 */
public class LogParamPanel extends javax.swing.JPanel implements IViewPanel {
    
    static Logger logger = Logger.getLogger(LogParamPanel.class);
    static String name="Log";

   
    /** Creates new form BeanForm based upon aNode
     *
     * @params  aNode   node to obtain the info from
     *
     */
    public LogParamPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode = aNode;
        itsOtdbRmi=SharedVars.getOTDBrmi();

        initializeTabs();
        initPanel();
    }
    
    /** Creates new form BeanForm */
    public LogParamPanel() {
        initComponents();
        initializeTabs();
    }
    
    private void initializeTabs() {
        LogParamTableModel aModel = new LogParamTableModel();
        tablePanel1.setTableModel(aModel);
    }
    
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
            itsOtdbRmi=SharedVars.getOTDBrmi();
 
        } else {
            logger.debug("No Mainframe supplied");
        }
    }
    
    public String getShortName() {
        return name;
    }
    
    public void setContent(Object anObject) {
        itsNode = (jOTDBnode)anObject;
       
        initPanel();
    }
    public boolean isSingleton() {
        return false;
    }
    
    public JPanel getInstance() {
        return new LogParamPanel();
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
        
        //  Fill in menu as in the example above        
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
    
    private void initPanel() {
        // check access
        UserAccount userAccount = itsMainFrame.getUserAccount();
        if(userAccount.isAdministrator()) {
            // enable/disable certain controls
        }
        if(userAccount.isAstronomer()) {
            // enable/disable certain controls
        }
        if(userAccount.isInstrumentScientist()) {
            // enable/disable certain controls
        }
        
        tablePanel1.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        LogParamTableModel model = new LogParamTableModel();
        tablePanel1.setTableModel(model);
        if (itsNode == null ) {
            logger.debug("ERROR:  empty node supplied");
            LogParamNameText.setText("");
        } else {
            LogParamNameText.setText(itsNode.name);
        }
        setTime();
        String aS = Integer.toString(itsMainFrame.getSharedVars().getLogParamLevel());
        LogParamLevelComboBox.setSelectedItem(aS);
        LogParamRecentOnlyCheckbox.setSelected(itsMainFrame.getSharedVars().getLogParamMostRecent());
        fillTable();
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
        this.logParamApplyButton.setVisible(visible);
        this.logParamCancelButton.setVisible(visible);
    }
    
    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
        this.logParamApplyButton.setEnabled(enabled);
        this.logParamCancelButton.setEnabled(enabled);
    }
    
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
        enableParamName(enabled);
        enableStartTime(enabled);
        enableEndTime(enabled);
        enableRecentOnly(enabled);
        enableButtons(enabled);
    }
    
    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableParamName(boolean enabled) {
        this.LogParamNameText.setEnabled(enabled);
    }
    
    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableStartTime(boolean enabled) {
        this.LogParamStartTimeText.setEnabled(enabled);
    }
    
    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableEndTime(boolean enabled) {
        this.LogParamEndTimeText.setEnabled(enabled);
    }
    
    /** Enables/disables this inputfield
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableRecentOnly(boolean enabled) {
        this.LogParamRecentOnlyCheckbox.setEnabled(enabled);
    }
    
    
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        titleText = new javax.swing.JLabel();
        tablePanel1 = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        jPanel1 = new javax.swing.JPanel();
        LogParamNameLabel = new javax.swing.JLabel();
        LogParamStartTimeLabel = new javax.swing.JLabel();
        LogParamEndTimeLabel = new javax.swing.JLabel();
        logParamCancelButton = new javax.swing.JButton();
        logParamApplyButton = new javax.swing.JButton();
        LogParamEndTimeText = new javax.swing.JTextField();
        LogParamStartTimeText = new javax.swing.JTextField();
        LogParamNameText = new javax.swing.JTextField();
        LogParamRecentOnlyCheckbox = new javax.swing.JCheckBox();
        logParamRefreshButton = new javax.swing.JButton();
        setStartDateButton = new javax.swing.JButton();
        setStopDateButton = new javax.swing.JButton();
        LogParamLevelComboBox = new javax.swing.JComboBox();
        LogParamLevelLabel = new javax.swing.JLabel();

        titleText.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        titleText.setText("no Title");
        titleText.setVisible(false);

        LogParamNameLabel.setText("Name");

        LogParamStartTimeLabel.setText("StartTime");

        LogParamEndTimeLabel.setText("EndTime");

        logParamCancelButton.setText("Cancel");
        logParamCancelButton.setToolTipText("restore defaults");
        logParamCancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                logParamCancelButtonActionPerformed(evt);
            }
        });

        logParamApplyButton.setText("Apply");
        logParamApplyButton.setToolTipText("Apply changes to logform");
        logParamApplyButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                logParamApplyButtonActionPerformed(evt);
            }
        });

        LogParamEndTimeText.setEditable(false);
        LogParamEndTimeText.setToolTipText("Give end date/time for log search");

        LogParamStartTimeText.setEditable(false);
        LogParamStartTimeText.setToolTipText("Give Start date/time for logging");

        LogParamNameText.setEditable(false);
        LogParamNameText.setText("None");

        LogParamRecentOnlyCheckbox.setText("Most Recent Only");
        LogParamRecentOnlyCheckbox.setToolTipText("Select to get only the most recent log val");

        logParamRefreshButton.setText("Refresh");
        logParamRefreshButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                logParamRefreshButtonActionPerformed(evt);
            }
        });

        setStartDateButton.setText("set");
        setStartDateButton.setToolTipText("set Start Date");
        setStartDateButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                setStartDateButtonActionPerformed(evt);
            }
        });

        setStopDateButton.setText("set");
        setStopDateButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                setStopDateButtonActionPerformed(evt);
            }
        });

        LogParamLevelComboBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "0", "1", "2", "3", "4", "5", "6", "7" }));

        LogParamLevelLabel.setText("Level");
        LogParamLevelLabel.setToolTipText("Level to descent ");

        org.jdesktop.layout.GroupLayout jPanel1Layout = new org.jdesktop.layout.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel1Layout.createSequentialGroup()
                        .add(logParamCancelButton)
                        .add(10, 10, 10)
                        .add(logParamApplyButton)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(logParamRefreshButton)
                        .addContainerGap(729, Short.MAX_VALUE))
                    .add(jPanel1Layout.createSequentialGroup()
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(LogParamStartTimeLabel)
                            .add(LogParamNameLabel)
                            .add(LogParamEndTimeLabel)
                            .add(LogParamLevelLabel))
                        .add(29, 29, 29)
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(jPanel1Layout.createSequentialGroup()
                                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                                    .add(LogParamNameText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 851, Short.MAX_VALUE)
                                    .add(jPanel1Layout.createSequentialGroup()
                                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                                            .add(org.jdesktop.layout.GroupLayout.LEADING, LogParamEndTimeText)
                                            .add(org.jdesktop.layout.GroupLayout.LEADING, LogParamStartTimeText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 275, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                                            .add(setStopDateButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 60, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                            .add(setStartDateButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 60, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                                        .add(51, 51, 51)))
                                .add(14, 14, 14))
                            .add(jPanel1Layout.createSequentialGroup()
                                .add(LogParamLevelComboBox, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 45, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .addContainerGap())))
                    .add(jPanel1Layout.createSequentialGroup()
                        .add(LogParamRecentOnlyCheckbox)
                        .addContainerGap(829, Short.MAX_VALUE))))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(LogParamNameLabel)
                    .add(LogParamNameText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(6, 6, 6)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(LogParamStartTimeLabel)
                    .add(LogParamStartTimeText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(setStartDateButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 20, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(5, 5, 5)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(LogParamEndTimeLabel)
                    .add(LogParamEndTimeText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(setStopDateButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 20, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(7, 7, 7)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(LogParamLevelLabel)
                    .add(LogParamLevelComboBox, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                .add(LogParamRecentOnlyCheckbox)
                .add(29, 29, 29)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(logParamCancelButton)
                    .add(logParamApplyButton)
                    .add(logParamRefreshButton))
                .addContainerGap())
        );

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, jPanel1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(titleText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 950, Short.MAX_VALUE)
                    .add(tablePanel1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 950, Short.MAX_VALUE))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(titleText)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(tablePanel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );
    }// </editor-fold>//GEN-END:initComponents

    private void logParamApplyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_logParamApplyButtonActionPerformed
        fillTable();
    }//GEN-LAST:event_logParamApplyButtonActionPerformed

    private void logParamCancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_logParamCancelButtonActionPerformed
        LogParamNameText.setText(itsNode.name);
        composeTimeString("start");
        composeTimeString("stop");
        LogParamRecentOnlyCheckbox.setSelected(itsMainFrame.getSharedVars().getLogParamMostRecent());
    }//GEN-LAST:event_logParamCancelButtonActionPerformed

    private void logParamRefreshButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_logParamRefreshButtonActionPerformed
        setTime();
        fillTable();
    }//GEN-LAST:event_logParamRefreshButtonActionPerformed

    private void fillTable() {
        itsMainFrame.getSharedVars().setLogParamStartTime(LogParamStartTimeText.getText());
        itsMainFrame.getSharedVars().setLogParamEndTime(LogParamEndTimeText.getText());
        itsMainFrame.getSharedVars().setLogParamMostRecent(LogParamRecentOnlyCheckbox.isSelected());
        itsMainFrame.getSharedVars().setLogParamLevel(Integer.parseInt((String)LogParamLevelComboBox.getSelectedItem()));

        itsMainFrame.setHourglassCursor();
        if (!((LogParamTableModel)tablePanel1.getTableModel()).fillTable(itsMainFrame,itsNode.nodeID())) {
            logger.debug("Error filling LogParamTableMode");
        }
        itsMainFrame.setNormalCursor();        
    }
    
    private void setStartDateButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_setStartDateButtonActionPerformed
        try {
            SimpleDateFormat aD = new SimpleDateFormat("yyyy-MMM-d HH:mm", itsLocale);
            Date initialDate = aD.parse(LogParamStartTimeText.getText());
            DateTimeChooser chooser = new DateTimeChooser(initialDate);
            itsStartTime = DateTimeChooser.showDialog(this, "StartTime", chooser);
            composeTimeString("start");//GEN-LAST:event_setStartDateButtonActionPerformed
        } catch (ParseException ex) {
            ex.printStackTrace();
        }
    }

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
        try {
            SimpleDateFormat aD = new SimpleDateFormat("yyyy-MMM-d HH:mm", itsLocale);
            Date initialDate = aD.parse(LogParamEndTimeText.getText());
            DateTimeChooser chooser = new DateTimeChooser(initialDate);
            itsStopTime = DateTimeChooser.showDialog(this, "StopTime", chooser);
            composeTimeString("stop");//GEN-LAST:event_setStopDateButtonActionPerformed
        } catch (ParseException ex) {
           ex.printStackTrace();
        }
    }
    
     /*
     * Compose the timestring from a given date
     */
    public void composeTimeString(String time) {
        // Set the dateformat OTDB takes
        SimpleDateFormat id = new SimpleDateFormat("yyyy-MMM-d HH:mm",itsLocale);
        if (time.equals("start")) {
            if (itsStartTime != null) {
              LogParamStartTimeText.setText(id.format(itsStartTime));
            } else {
                LogParamStartTimeText.setText("not-a-date-time");
            }
              
        } else if (time.equals("stop")) {
            if (itsStopTime != null) {
                LogParamEndTimeText.setText(id.format(itsStopTime));
            } else {
                LogParamEndTimeText.setText("not-a-date-time");
            }
        }
    }   
    private void setTime() {
        
        SimpleDateFormat aDate = new SimpleDateFormat("yyyy-MMM-d HH:mm",itsLocale);
        SimpleDateFormat aGMT = new SimpleDateFormat("yyyy-MMM-d HH:mm",itsLocale);
        aGMT.setTimeZone(TimeZone.getTimeZone("GMT"));
        Calendar aC = Calendar.getInstance(itsLocale);
        try {
            if (itsMainFrame.getSharedVars().getLogParamEndTime().equals("")) {
                Date endDate = aC.getTime();
                aC.add(Calendar.MONTH, -1);
                String  aS = aGMT.format(endDate);
                LogParamEndTimeText.setText(aS);
            } else {
                String  aS = itsMainFrame.getSharedVars().getLogParamEndTime();
                LogParamEndTimeText.setText(aS); 
                itsStopTime = aDate.parse(LogParamEndTimeText.getText());
            }
            if (itsMainFrame.getSharedVars().getLogParamStartTime().equals("")) {
                Date startDate = aC.getTime();
                String aS = aGMT.format(startDate);
                LogParamStartTimeText.setText(aS);
            } else {
                String  aS = itsMainFrame.getSharedVars().getLogParamStartTime();
                LogParamStartTimeText.setText(aS); 
                itsStartTime = aDate.parse(LogParamStartTimeText.getText());
            }
        } catch (ParseException ex) {
            ex.printStackTrace();
        }
    }
    
    private jOTDBnode  itsNode = null;
    private MainFrame  itsMainFrame;
    private OtdbRmi    itsOtdbRmi; 
    private Date       itsStartTime = null;
    private Date       itsStopTime = null;
    private Locale     itsLocale = new Locale("en");

    
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel LogParamEndTimeLabel;
    private javax.swing.JTextField LogParamEndTimeText;
    private javax.swing.JComboBox LogParamLevelComboBox;
    private javax.swing.JLabel LogParamLevelLabel;
    private javax.swing.JLabel LogParamNameLabel;
    private javax.swing.JTextField LogParamNameText;
    private javax.swing.JCheckBox LogParamRecentOnlyCheckbox;
    private javax.swing.JLabel LogParamStartTimeLabel;
    private javax.swing.JTextField LogParamStartTimeText;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JButton logParamApplyButton;
    private javax.swing.JButton logParamCancelButton;
    private javax.swing.JButton logParamRefreshButton;
    private javax.swing.JButton setStartDateButton;
    private javax.swing.JButton setStopDateButton;
    private nl.astron.lofar.sas.otbcomponents.TablePanel tablePanel1;
    private javax.swing.JLabel titleText;
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

    /**
     * Holds value of property title.
     */
    private String itsTitle;

    /**
     * Setter for property title.
     * @param title New value of property title.
     */
    public void setTitle(String title) {
        this.itsTitle = title;
        titleText.setText(itsTitle);
    }
}
