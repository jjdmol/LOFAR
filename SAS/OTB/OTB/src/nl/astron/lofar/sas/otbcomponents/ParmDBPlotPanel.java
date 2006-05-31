/*
 * ParamViewPanel.java
 *
 * Created on 26 januari 2006, 15:47
 */

package nl.astron.lofar.sas.otbcomponents;

import java.awt.Component;
import java.awt.Dimension;
import java.util.Vector;
import javax.swing.JComboBox;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.SharedVars;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.jParmDBnode;
import org.apache.log4j.Logger;

/**
 *
 * @author  pompert
 */
public class ParmDBPlotPanel extends javax.swing.JPanel implements IViewPanel{
    
    static Logger logger = Logger.getLogger(ParmDBPlotPanel.class);
    static String name="Plotter";
    private int successfulNumberOfSlots;
    private MainFrame  itsMainFrame;
    private String itsParamName;
    
    /** Creates new form BeanForm based upon aParameter
     *
     * @params  aParam   Param to obtain the info from
     *
     */
    public ParmDBPlotPanel(MainFrame aMainFrame,String paramName) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsParamName = paramName;
        successfulNumberOfSlots=4;
        initPanel(paramName);
    }
    
    /** Creates new form BeanForm */
    public ParmDBPlotPanel() {
        initComponents();
        
    }
    
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
            
            
        } else {
            logger.debug("No Mainframe supplied");
        }
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
        // build up the menu
        JPopupMenu aPopupMenu = new JPopupMenu();
        int[] availableSlots = itsSlotsPanel.getAvailableSlotIndexes();
        for(int i = 0; i < availableSlots.length; i++){
            JMenuItem aMenuItem=new JMenuItem("Add to slot "+availableSlots[i]);
            aMenuItem.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    popupMenuHandler(evt);
                }
            });
            aMenuItem.setActionCommand("Add to slot " +availableSlots[i]);
            aPopupMenu.add(aMenuItem);
            aPopupMenu.setOpaque(true);
        }
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
        logger.debug("PopUp menu Selection made: "+evt.getActionCommand().toString());
        int slotSelected = Integer.parseInt(evt.getActionCommand().toString().substring(12));
        logger.debug("Plot Slot extrapolated: "+slotSelected);
        itsSlotsPanel.addPlotToSlot(slotSelected,constructPlotterConstraints(itsParamName));
    }
    
    private Object constructPlotterConstraints(String aParamName){
        String[] passToDataAccess = null;
        if (aParamName != null) {
            String cloneParamName = aParamName.toString();
            if(cloneParamName.equalsIgnoreCase("ParmDB")){
                cloneParamName = "*";
            }else{
                cloneParamName=cloneParamName.substring(7);
                cloneParamName += "*";
            }
            try{
                passToDataAccess = new String[7];
                
                Vector paramValues;
                paramValues = SharedVars.getJParmFacade().getRange(cloneParamName);
                double startx = Double.parseDouble(paramValues.get(0).toString());
                double endx =Double.parseDouble(paramValues.get(1).toString());
                double starty = Double.parseDouble(paramValues.get(2).toString());
                double endy = Double.parseDouble(paramValues.get(3).toString());
                int numx = Integer.parseInt("5");
                int numy = Integer.parseInt("5");
                
                passToDataAccess[0] = cloneParamName;
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
        return passToDataAccess;
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
    }
    public void setParam(String aParam) {
        if (aParam != null) {
            itsParamName=aParam;
            initPanel(aParam);
        } else {
            logger.debug("No param supplied");
        }
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
        logger.trace("ParmDB name selected : "+itsParamName);
        int panelWidth = itsMainFrame.getWidth();
        int panelHeight = itsMainFrame.getHeight();
        slotsPane.setMinimumSize(new Dimension(640,480));
        slotsPane.setPreferredSize(new Dimension(panelWidth-400,panelHeight-300));
        slotsPane.setSize(new Dimension(panelWidth-400,panelHeight-300));
        slotsPane.getViewport().setPreferredSize(new Dimension(640,480));
        itsSlotsPanel.setMinimumSize(new Dimension(640,480));
        itsSlotsPanel.setPreferredSize(new Dimension(panelWidth-440,panelHeight-340));
        //itsSlotsPanel.repaint();
        //initPanel(itsParamName);
    }
    
    private void saveInput() {
        // Just check all possible fields that CAN change. The enabled method will take care if they COULD be changed.
        // this way we keep this panel general for multiple use
        boolean hasChanged = false;
        
    }
    
    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
        
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
        
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        java.awt.GridBagConstraints gridBagConstraints;

        contentPanel = new javax.swing.JPanel();
        lSlotsAmount = new javax.swing.JLabel();
        cSlotsAmount = new javax.swing.JComboBox();
        slotsPane = new javax.swing.JScrollPane();
        itsSlotsPanel = new nl.astron.lofar.sas.otb.util.plotter.PlotSlotsPanel();
        helpPane = new javax.swing.JScrollPane();
        tHelp = new javax.swing.JTextArea();

        setLayout(new java.awt.BorderLayout());

        contentPanel.setLayout(new java.awt.GridBagLayout());

        lSlotsAmount.setHorizontalAlignment(javax.swing.SwingConstants.RIGHT);
        lSlotsAmount.setLabelFor(cSlotsAmount);
        lSlotsAmount.setText("Number of Slots");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        contentPanel.add(lSlotsAmount, gridBagConstraints);

        cSlotsAmount.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "1", "4", "9", "16", "25" }));
        cSlotsAmount.setSelectedIndex(1);
        cSlotsAmount.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent evt) {
                cSlotsAmountItemStateChanged(evt);
            }
        });

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        contentPanel.add(cSlotsAmount, gridBagConstraints);

        slotsPane.setViewportBorder(javax.swing.BorderFactory.createEtchedBorder());
        slotsPane.setAutoscrolls(true);
        slotsPane.setMinimumSize(new java.awt.Dimension(640, 480));
        slotsPane.setPreferredSize(new java.awt.Dimension(640, 480));
        itsSlotsPanel.setMinimumSize(new java.awt.Dimension(640, 480));
        itsSlotsPanel.setPreferredSize(new java.awt.Dimension(640, 480));
        slotsPane.setViewportView(itsSlotsPanel);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        contentPanel.add(slotsPane, gridBagConstraints);

        tHelp.setBackground(javax.swing.UIManager.getDefaults().getColor("Viewport.background"));
        tHelp.setColumns(20);
        tHelp.setEditable(false);
        tHelp.setRows(5);
        tHelp.setText("To zoom: Click and hold the left mouse button and select a rectangle.\nTo reset the zoom: Press CTRL-LeftMouseButton to reset the zoom.\nTo change colors/etc: Double-Click on a line in the legend.\nTo change plot/axis labels and tics/etc: Click on an axis or title and press the right mouse button.");
        tHelp.setMinimumSize(new java.awt.Dimension(565, 75));
        helpPane.setViewportView(tHelp);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 2;
        gridBagConstraints.gridwidth = 2;
        gridBagConstraints.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        contentPanel.add(helpPane, gridBagConstraints);

        add(contentPanel, java.awt.BorderLayout.CENTER);

    }// </editor-fold>//GEN-END:initComponents
    
    private void cSlotsAmountItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_cSlotsAmountItemStateChanged
        int wannaHaveSlots = Integer.parseInt(evt.getItem().toString());
        
        System.out.println("current slots: "+ successfulNumberOfSlots);
        System.out.println("want to have slots: "+ wannaHaveSlots);
        if(wannaHaveSlots != successfulNumberOfSlots){
            try {
                itsSlotsPanel.setAmountOfSlots(wannaHaveSlots,false);
            } catch (NumberFormatException ex) {
                //TODO log!
                ex.printStackTrace();
            } catch (IllegalArgumentException ex) {
                //TODO log!
                //ex.printStackTrace();
                String[] buttons = {"Clear Slots","Cancel"};
                int choice =  JOptionPane.showOptionDialog(this,ex.getMessage(), "Plots detected in to be deleted slots", JOptionPane.DEFAULT_OPTION, JOptionPane.QUESTION_MESSAGE, null,buttons,buttons[0]);
                if(choice == 0){
                    itsSlotsPanel.setAmountOfSlots(wannaHaveSlots,true);
                }else{
                    double squareRoot = Math.sqrt(Double.parseDouble(""+itsSlotsPanel.getAmountOfSlots()));
                    System.out.println("cancelling : slots was "+ itsSlotsPanel.getAmountOfSlots());
                    int wishedIndex = (Integer.parseInt(""+(int)squareRoot))-1;
                    cSlotsAmount.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "1", "4", "9", "16", "25" }));
                    System.out.println("cancelling : setting combobox to index "+ wishedIndex +" / value "+squareRoot);
                    cSlotsAmount.addItemListener(new java.awt.event.ItemListener() {
                        public void itemStateChanged(java.awt.event.ItemEvent evt) {
                            cSlotsAmountItemStateChanged(evt);
                        }
                    });
                    System.out.println("cancelling : slots will be "+ itsSlotsPanel.getAmountOfSlots());
                   
                    cSlotsAmount.setSelectedItem(new String(""+itsSlotsPanel.getAmountOfSlots()));
                 }
            }
        }
        successfulNumberOfSlots = itsSlotsPanel.getAmountOfSlots();
    }//GEN-LAST:event_cSlotsAmountItemStateChanged
    
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JComboBox cSlotsAmount;
    private javax.swing.JPanel contentPanel;
    private javax.swing.JScrollPane helpPane;
    private nl.astron.lofar.sas.otb.util.plotter.PlotSlotsPanel itsSlotsPanel;
    private javax.swing.JLabel lSlotsAmount;
    private javax.swing.JScrollPane slotsPane;
    private javax.swing.JTextArea tHelp;
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
