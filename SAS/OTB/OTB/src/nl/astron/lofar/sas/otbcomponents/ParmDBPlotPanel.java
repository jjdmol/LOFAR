/*
 * ParamViewPanel.java
 *
 * Created on 26 januari 2006, 15:47
 */

package nl.astron.lofar.sas.otbcomponents;

import java.awt.Component;
import java.awt.GridBagConstraints;
import java.util.Vector;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.SharedVars;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.jParmDBnode;
import nl.astron.lofar.sas.otb.util.plotter.PlotSlotsPanel;
import org.apache.log4j.Logger;

/**
 *
 * @author  pompert
 */
public class ParmDBPlotPanel extends javax.swing.JPanel implements IViewPanel{
    
    static Logger logger = Logger.getLogger(ParmDBPlotPanel.class);
    static String name="Plotter";
    
    private MainFrame  itsMainFrame;
    private String itsParamName;
    private PlotSlotsPanel itsSlotsPanel;
    
    /** Creates new form BeanForm based upon aParameter
     *
     * @params  aParam   Param to obtain the info from
     *
     */
    public ParmDBPlotPanel(MainFrame aMainFrame,String paramName) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsParamName = paramName;
        
        initPanel(paramName);
    }
    
    /** Creates new form BeanForm */
    public ParmDBPlotPanel() {
        initComponents();
        itsSlotsPanel = new PlotSlotsPanel();
        
        GridBagConstraints gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.ipadx = 2;
        gridBagConstraints.ipady = 0;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.insets = new java.awt.Insets(1, 1, 0, 0);
        add(itsSlotsPanel, gridBagConstraints);
        
    }
    
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
            
            
        } else {
            logger.debug("No Mainframe supplied");
        }
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
    
    private void initPanel(String aParamName) {
        // check access
        UserAccount userAccount = itsMainFrame.getUserAccount();
        
        // For now:
        // give enabled/disabled fields here, can be changed later to choices stored in the database
        
        if(userAccount.isAdministrator()) {
            // enable/disable certain controls
        }
        if(userAccount.isAstronomer()) {
            // enable/disable certain controls
        }
        if(userAccount.isInstrumentScientist()) {
            // enable/disable certain controls
        }       
        
        if (aParamName != null) {
            if(aParamName.equalsIgnoreCase("ParmDB")){
                itsParamName = "*";
            }else{
                itsParamName=aParamName.substring(7);
                itsParamName += "*";
            }
            try{
                String[] passToDataAccess = new String[7];
                
                Vector paramValues;
                paramValues = SharedVars.getJParmFacade().getRange(itsParamName);
                double startx = Double.parseDouble(paramValues.get(0).toString());
                double endx =Double.parseDouble(paramValues.get(1).toString());
                double starty = Double.parseDouble(paramValues.get(2).toString());
                double endy = Double.parseDouble(paramValues.get(3).toString());
                int numx = Integer.parseInt("5");
                int numy = Integer.parseInt("5");
                
                passToDataAccess[0] = itsParamName;
                passToDataAccess[1] = ""+startx;
                passToDataAccess[2] = ""+endx;
                passToDataAccess[3] = ""+numx;
                passToDataAccess[4] = ""+starty;
                passToDataAccess[5] = ""+endy;
                passToDataAccess[6] = ""+numy;
            }catch(Exception ex){
                JOptionPane.showMessageDialog(itsMainFrame, ex.getMessage(),
                        "Error detected",
                        JOptionPane.ERROR_MESSAGE);
                logger.error("Plotter created an exception :"+ex.getMessage(),ex);
            }
        } else {
            logger.debug("ERROR:  no Param Name given");
        }
    }
    public void setParam(String aParam) {
        if (aParam != null) {
            itsParamName=aParam;
            initPanel(aParam);
        } else {
            logger.debug("No param supplied");
        }
    }
    
    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
        this.ParamApplyButton.setEnabled(enabled);
        this.ParamCancelButton.setEnabled(enabled);
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
        this.ParamApplyButton.setVisible(visible);
        this.ParamCancelButton.setVisible(visible);
    }
    
    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
       
    }
    
    public String getShortName() {
        return name;
    }
    public void setContent(Object anObject) {
        jParmDBnode node = (jParmDBnode)anObject;
        itsParamName = node.nodeID();
        
        initPanel(itsParamName);
    }
    
    private void saveInput() {
        // Just check all possible fields that CAN change. The enabled method will take care if they COULD be changed.
        // this way we keep this panel general for multiple use
        boolean hasChanged = false;
        
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        java.awt.GridBagConstraints gridBagConstraints;

        jScrollPane1 = new javax.swing.JScrollPane();
        jTextArea1 = new javax.swing.JTextArea();
        ParamCancelButton = new javax.swing.JButton();
        ParamApplyButton = new javax.swing.JButton();
        jLabel1 = new javax.swing.JLabel();
        jScrollPane2 = new javax.swing.JScrollPane();
        jTextArea2 = new javax.swing.JTextArea();

        jTextArea1.setColumns(20);
        jTextArea1.setRows(5);
        jScrollPane1.setViewportView(jTextArea1);

        setLayout(new java.awt.GridBagLayout());

        ParamCancelButton.setText("Cancel");
        ParamCancelButton.setEnabled(false);
        ParamCancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ParamCancelButtonActionPerformed(evt);
            }
        });

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 2;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.insets = new java.awt.Insets(30, 36, 7, 0);
        add(ParamCancelButton, gridBagConstraints);

        ParamApplyButton.setText("Apply");
        ParamApplyButton.setEnabled(false);
        ParamApplyButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ParamApplyButtonActionPerformed(evt);
            }
        });

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 3;
        gridBagConstraints.gridy = 3;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.insets = new java.awt.Insets(30, 5, 7, 9);
        add(ParamApplyButton, gridBagConstraints);

        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("ParmDB Plot Panel");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 0;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.ipadx = 206;
        gridBagConstraints.ipady = 5;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.insets = new java.awt.Insets(10, 114, 0, 0);
        add(jLabel1, gridBagConstraints);

        jTextArea2.setBackground(javax.swing.UIManager.getDefaults().getColor("scrollbar"));
        jTextArea2.setColumns(20);
        jTextArea2.setEditable(false);
        jTextArea2.setRows(5);
        jTextArea2.setText("To zoom: Click and hold the left mouse button and select a rectangle.\nTo reset the zoom: Press CTRL-LeftMouseButton to reset the zoom.\nTo change colors/etc: Double-Click on a line in the legend.\nTo change plot/axis labels and tics/etc: Click on an axis or title and press the right mouse button.");
        jScrollPane2.setViewportView(jTextArea2);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridwidth = 4;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.ipadx = 588;
        gridBagConstraints.ipady = 58;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.weightx = 1.0;
        gridBagConstraints.weighty = 1.0;
        gridBagConstraints.insets = new java.awt.Insets(400, 10, 0, 9);
        add(jScrollPane2, gridBagConstraints);

    }// </editor-fold>//GEN-END:initComponents
    
    private void ParamApplyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ParamApplyButtonActionPerformed
        saveInput();
    }//GEN-LAST:event_ParamApplyButtonActionPerformed
    
    private void ParamCancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ParamCancelButtonActionPerformed
        initPanel(itsParamName);
    }//GEN-LAST:event_ParamCancelButtonActionPerformed
    
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton ParamApplyButton;
    private javax.swing.JButton ParamCancelButton;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JTextArea jTextArea1;
    private javax.swing.JTextArea jTextArea2;
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
