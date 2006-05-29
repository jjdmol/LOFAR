/*
 * GenObsConfigPanel.java
 *
 *Panel to set some general Observation configuration values
 *
 * Created on 18 May 2006, 09:47
 */

package nl.astron.lofar.sas.otbcomponents;

import java.awt.Component;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import org.apache.log4j.Logger;

/**
 *
 * @author  coolen
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
        NodeCancelButton = new javax.swing.JButton();
        NodeApplyButton = new javax.swing.JButton();
        jLabel1 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        NrSubbandsText = new javax.swing.JTextField();
        jLabel3 = new javax.swing.JLabel();
        NrChannelsText = new javax.swing.JTextField();
        jLabel4 = new javax.swing.JLabel();
        NrStationsText = new javax.swing.JTextField();
        jLabel5 = new javax.swing.JLabel();
        NrRSPBoardsText = new javax.swing.JTextField();
        jLabel6 = new javax.swing.JLabel();
        SampleRateText = new javax.swing.JTextField();
        jLabel7 = new javax.swing.JLabel();
        StartTimeText = new javax.swing.JTextField();
        jLabel8 = new javax.swing.JLabel();
        StopTimeText = new javax.swing.JTextField();
        jLabel9 = new javax.swing.JLabel();
        MeasurementSetNameText = new javax.swing.JTextField();
        jLabel10 = new javax.swing.JLabel();

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

        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("Generic Observation Configuration Panel");

        jLabel2.setText("#Nr Subbands:");

        NrSubbandsText.setToolTipText("Number of Subbands for this Observation");

        jLabel3.setText("#Nr Channels/Subband:");

        NrChannelsText.setToolTipText("Number of Channels per Subband for this Observation");

        jLabel4.setText("#Nr Stations:");

        NrStationsText.setToolTipText("Number of Stations for this Observation");

        jLabel5.setText("#Nr RSPBoards:");

        NrRSPBoardsText.setToolTipText("Number of RSPBoards for this Observation");

        jLabel6.setText("SampleRate:");

        SampleRateText.setToolTipText("SampleRate for this Observation");

        jLabel7.setText("StartTime:");

        StartTimeText.setToolTipText("StartTime for this Observation");

        jLabel8.setText("StopTime:");

        StopTimeText.setToolTipText("StopTime for this Observation");

        jLabel9.setText("MeasurementSet Name:");

        MeasurementSetNameText.setToolTipText("Name for the MeasurementSet for this Observation");

        jLabel10.setText("jLabel10");

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(70, 70, 70)
                .add(jLabel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 460, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(117, Short.MAX_VALUE))
            .add(layout.createSequentialGroup()
                .add(47, 47, 47)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jLabel10)
                    .add(layout.createSequentialGroup()
                        .add(NodeCancelButton)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(NodeApplyButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 70, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(layout.createSequentialGroup()
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(jLabel2)
                            .add(jLabel3)
                            .add(jLabel4)
                            .add(jLabel5)
                            .add(jLabel7)
                            .add(jLabel6)
                            .add(jLabel8)
                            .add(jLabel9))
                        .add(16, 16, 16)
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(MeasurementSetNameText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 375, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                                .add(StopTimeText)
                                .add(StartTimeText)
                                .add(SampleRateText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 104, Short.MAX_VALUE)
                                .add(NrChannelsText)
                                .add(NrSubbandsText)
                                .add(NrRSPBoardsText)
                                .add(NrStationsText, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 186, Short.MAX_VALUE)))))
                .addContainerGap(93, Short.MAX_VALUE))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(11, 11, 11)
                .add(jLabel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 20, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(23, 23, 23)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel2)
                    .add(NrSubbandsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel3)
                    .add(NrChannelsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel4)
                    .add(NrStationsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel5)
                    .add(NrRSPBoardsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(SampleRateText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(jLabel6))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel7)
                    .add(StartTimeText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel8)
                    .add(StopTimeText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(jLabel9)
                    .add(MeasurementSetNameText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jLabel10)
                .add(18, 18, 18)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(NodeCancelButton)
                    .add(NodeApplyButton))
                .add(166, 166, 166))
        );
    }// </editor-fold>//GEN-END:initComponents

    private void NodeApplyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NodeApplyButtonActionPerformed
        saveInput();
    }//GEN-LAST:event_NodeApplyButtonActionPerformed

    private void NodeCancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NodeCancelButtonActionPerformed
        initPanel();
    }//GEN-LAST:event_NodeCancelButtonActionPerformed
    
    private jOTDBnode itsNode = null;
    private MainFrame  itsMainFrame;
    private OtdbRmi    itsOtdbRmi;   
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JTextField MeasurementSetNameText;
    private javax.swing.JButton NodeApplyButton;
    private javax.swing.JButton NodeCancelButton;
    private javax.swing.JTextField NrChannelsText;
    private javax.swing.JTextField NrRSPBoardsText;
    private javax.swing.JTextField NrStationsText;
    private javax.swing.JTextField NrSubbandsText;
    private javax.swing.JTextField SampleRateText;
    private javax.swing.JTextField StartTimeText;
    private javax.swing.JTextField StopTimeText;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel10;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JLabel jLabel8;
    private javax.swing.JLabel jLabel9;
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
