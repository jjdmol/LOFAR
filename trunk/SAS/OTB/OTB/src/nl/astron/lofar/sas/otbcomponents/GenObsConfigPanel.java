/*
 * GenObsConfigPanel.java
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
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.tablemodels.RSPMACTableModel;
import nl.astron.lofar.sas.otb.util.tablemodels.StationPositionsTableModel;
import nl.astron.lofar.sas.otb.util.tablemodels.SubbandFreqTableModel;
import org.apache.log4j.Logger;

/**
 *Panel to set some general Observation configuration values
 *
 * @created 18-05-2006, 09:47
 *
 * @author  coolen
 *
 * @version $Id$
 */
public class GenObsConfigPanel extends javax.swing.JPanel implements IViewPanel{
    
    static Logger logger = Logger.getLogger(GenObsConfigPanel.class);
    static String name = "GenObsConfig";
    
    
    /** Creates new form BeanForm based upon aNode
     *
     * @params  aNode   node to obtain the info from
     *
     */
    public GenObsConfigPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode=aNode;
        itsOtdbRmi=itsMainFrame.getSharedVars().getOTDBrmi();
        initializeTabs();
        initPanel();
    }
    
    /** Creates new form BeanForm */
    public GenObsConfigPanel() {
        initComponents();
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
        initPanel();
    }
    public boolean isSingleton() {
        return false;
    }
    
    public JPanel getInstance() {
        return new GenObsConfigPanel();
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
        
        // for now:
        // set fields that can be changed here for now, later in the useraccount check
        
        
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
            setNodeName(itsNode.name);
        } else {
            logger.debug("ERROR:  no node given");
        }
    }
    
    /** 
     * Initializes the tab-panels. Each tab has a specific table model that
     * contains the data for the table in the tab
     */
    private void initializeTabs() {
        StationPositionsTableModel SPModel = new StationPositionsTableModel(itsMainFrame);
        StationPositionsPanel.setTableModel(SPModel);
        StationPositionsPanel.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

        SubbandFreqTableModel SFModel = new SubbandFreqTableModel(itsMainFrame);
        SubbandFrequenciesPanel.setTableModel(SFModel);
        SubbandFrequenciesPanel.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        
        RSPMACTableModel RMAModel = new RSPMACTableModel(itsMainFrame);
        RSPMACAddressPanel.setTableModel(RMAModel);
        RSPMACAddressPanel.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        
    }
    
    private String getNodeName() {
//        return this.NodeNameText.getText();
        return "";
    }
    
    private void setNodeName(String aS) {
//        this.NodeNameText.setText(aS);
    }
    
    private void enableNodeName(boolean enabled) {
//        this.NodeNameText.setEnabled(enabled);
    }
    
    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
        this.NodeApplyButton.setEnabled(enabled);
        this.NodeCancelButton.setEnabled(enabled);
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
        this.NodeApplyButton.setVisible(visible);
        this.NodeCancelButton.setVisible(visible);
    }
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
        enableNodeName(enabled);
        enableButtons(enabled);
    }
    
    private void saveInput() {
        // Just check all possible fields that CAN change. The enabled method will take care if they COULD be changed.
        // this way we keep this panel general for multiple use
        boolean hasChanged = false;
        if (itsNode != null) {
            
        } else {
            logger.debug("ERROR:  no Param given");
        }
    }
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        jTabbedPane1 = new javax.swing.JTabbedPane();
        GenericPanel = new javax.swing.JPanel();
        jLabel11 = new javax.swing.JLabel();
        jLabel12 = new javax.swing.JLabel();
        NrSubbandsText1 = new javax.swing.JTextField();
        jLabel13 = new javax.swing.JLabel();
        NrChannelsText1 = new javax.swing.JTextField();
        jLabel14 = new javax.swing.JLabel();
        NrStationsText1 = new javax.swing.JTextField();
        jLabel15 = new javax.swing.JLabel();
        NrRSPBoardsText1 = new javax.swing.JTextField();
        jLabel16 = new javax.swing.JLabel();
        SampleRateText1 = new javax.swing.JTextField();
        jLabel17 = new javax.swing.JLabel();
        StartTimeText1 = new javax.swing.JTextField();
        jLabel18 = new javax.swing.JLabel();
        StopTimeText1 = new javax.swing.JTextField();
        jLabel19 = new javax.swing.JLabel();
        MeasurementSetNameText1 = new javax.swing.JTextField();
        jLabel20 = new javax.swing.JLabel();
        NrSamplesFrameText = new javax.swing.JTextField();
        NodeCancelButton = new javax.swing.JButton();
        NodeApplyButton = new javax.swing.JButton();
        StationPositionsPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        SubbandFrequenciesPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        RSPMACAddressPanel = new nl.astron.lofar.sas.otbcomponents.TablePanel();

        jLabel11.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel11.setText("Generic Observation Configuration Panel");

        jLabel12.setText("#Nr Subbands:");

        NrSubbandsText1.setToolTipText("Number of Subbands for this Observation");

        jLabel13.setText("#Nr Channels/Subband:");

        NrChannelsText1.setToolTipText("Number of Channels per Subband for this Observation");

        jLabel14.setText("#Nr Stations:");

        NrStationsText1.setToolTipText("Number of Stations for this Observation");

        jLabel15.setText("#Nr RSPBoards:");

        NrRSPBoardsText1.setToolTipText("Number of RSPBoards for this Observation");

        jLabel16.setText("SampleRate:");

        SampleRateText1.setToolTipText("SampleRate for this Observation");

        jLabel17.setText("StartTime:");

        StartTimeText1.setToolTipText("StartTime for this Observation");

        jLabel18.setText("StopTime:");

        StopTimeText1.setToolTipText("StopTime for this Observation");

        jLabel19.setText("MeasurementSet Name:");

        MeasurementSetNameText1.setToolTipText("Name for the MeasurementSet for this Observation");

        jLabel20.setText("#Nr Samples/Frame:");

        NrSamplesFrameText.setToolTipText("Number of samples per frame for this observation");

        NodeCancelButton.setText("Cancel");
        NodeCancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                NodeCancelButtonActionPerformed(evt);
            }
        });

        NodeApplyButton.setText("Apply");
        NodeApplyButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                NodeApplyButtonActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout GenericPanelLayout = new org.jdesktop.layout.GroupLayout(GenericPanel);
        GenericPanel.setLayout(GenericPanelLayout);
        GenericPanelLayout.setHorizontalGroup(
            GenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(GenericPanelLayout.createSequentialGroup()
                .add(GenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(GenericPanelLayout.createSequentialGroup()
                        .add(70, 70, 70)
                        .add(jLabel11, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 460, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(GenericPanelLayout.createSequentialGroup()
                        .add(47, 47, 47)
                        .add(GenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(GenericPanelLayout.createSequentialGroup()
                                .add(NodeCancelButton)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(NodeApplyButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 70, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                            .add(GenericPanelLayout.createSequentialGroup()
                                .add(GenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                                    .add(jLabel12)
                                    .add(jLabel13)
                                    .add(jLabel14)
                                    .add(jLabel15)
                                    .add(jLabel17)
                                    .add(jLabel16)
                                    .add(jLabel18)
                                    .add(jLabel19)
                                    .add(jLabel20))
                                .add(16, 16, 16)
                                .add(GenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                                    .add(MeasurementSetNameText1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 375, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(GenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                                        .add(StopTimeText1)
                                        .add(StartTimeText1)
                                        .add(SampleRateText1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 104, Short.MAX_VALUE)
                                        .add(NrChannelsText1)
                                        .add(NrSubbandsText1)
                                        .add(NrRSPBoardsText1)
                                        .add(NrStationsText1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 186, Short.MAX_VALUE))
                                    .add(NrSamplesFrameText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 186, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))))))
                .addContainerGap(232, Short.MAX_VALUE))
        );
        GenericPanelLayout.setVerticalGroup(
            GenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(GenericPanelLayout.createSequentialGroup()
                .add(11, 11, 11)
                .add(jLabel11, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 20, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(23, 23, 23)
                .add(GenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel12)
                    .add(NrSubbandsText1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(GenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel13)
                    .add(NrChannelsText1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(GenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel14)
                    .add(NrStationsText1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(GenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel15)
                    .add(NrRSPBoardsText1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(GenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(SampleRateText1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(jLabel16))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(GenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel17)
                    .add(StartTimeText1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(GenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel18)
                    .add(StopTimeText1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(GenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel19)
                    .add(MeasurementSetNameText1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(GenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel20)
                    .add(NrSamplesFrameText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(36, 36, 36)
                .add(GenericPanelLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(NodeCancelButton)
                    .add(NodeApplyButton))
                .add(219, 219, 219))
        );
        jTabbedPane1.addTab("Generic", GenericPanel);

        jTabbedPane1.addTab("Station Positions", StationPositionsPanel);

        jTabbedPane1.addTab("Subband Frequencies", SubbandFrequenciesPanel);

        jTabbedPane1.addTab("RSP MAC Addresses", RSPMACAddressPanel);

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                .add(18, 18, 18)
                .add(jTabbedPane1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 791, Short.MAX_VALUE)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                .addContainerGap()
                .add(jTabbedPane1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 574, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents

    
    private void NodeApplyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NodeApplyButtonActionPerformed
// TODO add your handling code here:
    }//GEN-LAST:event_NodeApplyButtonActionPerformed
    
    private void NodeCancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NodeCancelButtonActionPerformed
// TODO add your handling code here:
    }//GEN-LAST:event_NodeCancelButtonActionPerformed
    
    private jOTDBnode itsNode = null;
    private MainFrame  itsMainFrame;
    private OtdbRmi    itsOtdbRmi;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel GenericPanel;
    private javax.swing.JTextField MeasurementSetNameText1;
    private javax.swing.JButton NodeApplyButton;
    private javax.swing.JButton NodeCancelButton;
    private javax.swing.JTextField NrChannelsText1;
    private javax.swing.JTextField NrRSPBoardsText1;
    private javax.swing.JTextField NrSamplesFrameText;
    private javax.swing.JTextField NrStationsText1;
    private javax.swing.JTextField NrSubbandsText1;
    private nl.astron.lofar.sas.otbcomponents.TablePanel RSPMACAddressPanel;
    private javax.swing.JTextField SampleRateText1;
    private javax.swing.JTextField StartTimeText1;
    private nl.astron.lofar.sas.otbcomponents.TablePanel StationPositionsPanel;
    private javax.swing.JTextField StopTimeText1;
    private nl.astron.lofar.sas.otbcomponents.TablePanel SubbandFrequenciesPanel;
    private javax.swing.JLabel jLabel11;
    private javax.swing.JLabel jLabel12;
    private javax.swing.JLabel jLabel13;
    private javax.swing.JLabel jLabel14;
    private javax.swing.JLabel jLabel15;
    private javax.swing.JLabel jLabel16;
    private javax.swing.JLabel jLabel17;
    private javax.swing.JLabel jLabel18;
    private javax.swing.JLabel jLabel19;
    private javax.swing.JLabel jLabel20;
    private javax.swing.JTabbedPane jTabbedPane1;
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
        listenerList.add(java.awt.event.ActionListener.class, listener);
    }
    
    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeActionListener(java.awt.event.ActionListener listener) {
        
        listenerList.remove(java.awt.event.ActionListener.class, listener);
    }
    
    /**
     * Notifies all registered listeners about the event.
     *
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {
        
        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed(event);
            }
        }
    }
    
    
    
    
    
}
