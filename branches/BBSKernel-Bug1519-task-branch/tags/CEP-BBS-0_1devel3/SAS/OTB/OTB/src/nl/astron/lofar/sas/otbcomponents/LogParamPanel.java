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
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.ListSelectionModel;
import nl.astron.lofar.sas.otb.MainFrame;
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
        itsOtdbRmi=itsMainFrame.getSharedVars().getOTDBrmi();

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
            itsOtdbRmi=itsMainFrame.getSharedVars().getOTDBrmi();
 
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
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
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

        LogParamEndTimeText.setText("2005-12-31 59:59:59.000");
        LogParamEndTimeText.setToolTipText("Give end date/time for log search");

        LogParamStartTimeText.setText("2005-01-01 00:00:00.000");
        LogParamStartTimeText.setToolTipText("Give Start date/time for logging");

        LogParamNameText.setEditable(false);
        LogParamNameText.setText("None");

        LogParamRecentOnlyCheckbox.setText("Most Recent Only");
        LogParamRecentOnlyCheckbox.setToolTipText("Select to get only the most recent log val");

        org.jdesktop.layout.GroupLayout jPanel1Layout = new org.jdesktop.layout.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel1Layout.createSequentialGroup()
                .add(14, 14, 14)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(logParamCancelButton)
                    .add(LogParamStartTimeLabel)
                    .add(LogParamNameLabel))
                .add(10, 10, 10)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                        .add(jPanel1Layout.createSequentialGroup()
                            .add(LogParamStartTimeText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(40, 40, 40)
                            .add(LogParamEndTimeLabel)
                            .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                            .add(LogParamEndTimeText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(13, 13, 13)
                            .add(LogParamRecentOnlyCheckbox))
                        .add(LogParamNameText))
                    .add(logParamApplyButton))
                .addContainerGap(258, Short.MAX_VALUE))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel1Layout.createSequentialGroup()
                .add(9, 9, 9)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(LogParamNameLabel)
                    .add(LogParamNameText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(6, 6, 6)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(LogParamStartTimeLabel)
                    .add(LogParamStartTimeText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(LogParamEndTimeLabel)
                    .add(LogParamEndTimeText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(LogParamRecentOnlyCheckbox))
                .add(30, 30, 30)
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(logParamCancelButton)
                    .add(logParamApplyButton))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(titleText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 950, Short.MAX_VALUE)
                    .add(tablePanel1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 950, Short.MAX_VALUE)
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, jPanel1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(titleText)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(tablePanel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jPanel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
    }// </editor-fold>//GEN-END:initComponents

    private void logParamApplyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_logParamApplyButtonActionPerformed
        itsStartTime=LogParamStartTimeText.getText();
        itsEndTime=LogParamEndTimeText.getText();
        setMostRecent=LogParamRecentOnlyCheckbox.isSelected();
        itsMainFrame.setHourglassCursor();
        if (!((LogParamTableModel)tablePanel1.getTableModel()).fillTable(itsMainFrame,itsNode.nodeID(),itsStartTime,itsEndTime,setMostRecent)) {
            logger.debug("Error filling LogParamTableMode");
        }
        itsMainFrame.setNormalCursor();
    }//GEN-LAST:event_logParamApplyButtonActionPerformed

    private void logParamCancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_logParamCancelButtonActionPerformed
        LogParamNameText.setText(itsNode.name);
        LogParamStartTimeText.setText(itsStartTime);
        LogParamEndTimeText.setText(itsEndTime);
        LogParamRecentOnlyCheckbox.setSelected(setMostRecent);
    }//GEN-LAST:event_logParamCancelButtonActionPerformed
    
    private jOTDBnode itsNode = null;
    private MainFrame  itsMainFrame;
    private OtdbRmi    itsOtdbRmi; 
    private String itsStartTime = "2005-01-01 00:00:00.000";
    private String itsEndTime = "2005-01-01 00:00:00.000";
    private boolean setMostRecent = false;
    
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel LogParamEndTimeLabel;
    private javax.swing.JTextField LogParamEndTimeText;
    private javax.swing.JLabel LogParamNameLabel;
    private javax.swing.JTextField LogParamNameText;
    private javax.swing.JCheckBox LogParamRecentOnlyCheckbox;
    private javax.swing.JLabel LogParamStartTimeLabel;
    private javax.swing.JTextField LogParamStartTimeText;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JButton logParamApplyButton;
    private javax.swing.JButton logParamCancelButton;
    private nl.astron.lofar.sas.otbcomponents.TablePanel tablePanel1;
    private javax.swing.JLabel titleText;
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
