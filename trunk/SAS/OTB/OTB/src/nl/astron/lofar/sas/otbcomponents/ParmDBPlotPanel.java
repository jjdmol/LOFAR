/*
 * ParamViewPanel.java
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
import java.awt.Dimension;
import java.rmi.RemoteException;
import java.util.HashMap;
import java.util.Vector;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import nl.astron.lofar.java.gui.plotter.PlotConstants;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.SharedVars;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.jParmDBnode;
import org.apache.log4j.Logger;

/**
 * This panel contains the ParmDB Plotter and all related functionality
 *
 * @created 23-04-2006, 15:47
 * @author  pompert
 * @version $Id$
 */
public class ParmDBPlotPanel extends javax.swing.JPanel implements IViewPanel{
    
    private static ParmDBPlotPanel instance = null;
    static Logger logger = Logger.getLogger(ParmDBPlotPanel.class);
    static String name="Plotter";
    private int successfulNumberOfSlots;
    private MainFrame  itsMainFrame;
    private jParmDBnode itsParam;
    private String itsParamName;
    private String itsParamTableName;
    
    /** Creates new form ParmDBPlotPanel
     *
     * @param  aMainFrame   The mainframe to be associated with this panel
     * @param  paramName   a Parameter name to be associated with this panel
     */
    public ParmDBPlotPanel(MainFrame aMainFrame,String paramName) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsParamName = paramName;
        successfulNumberOfSlots=4;
        initPanel(paramName);
    }
    
    /** Creates new form ParmDBPlotPanel */
    public ParmDBPlotPanel() {
        
    }
    /**
     * Sets the mainframe for this panel
     * @param aMainFrame The mainframe to be associated with this panel
     */
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
            int panelWidth = itsMainFrame.getWidth();
            int panelHeight = itsMainFrame.getHeight();
            slotsPane.setMinimumSize(new Dimension(640,480));
            slotsPane.setPreferredSize(new Dimension(panelWidth-400,panelHeight-300));
            //slotsPane.setSize(new Dimension(panelWidth-400,panelHeight-300));
            slotsPane.getViewport().setPreferredSize(new Dimension(panelWidth-400,panelHeight-300));
            itsSlotsPanel.setMinimumSize(new Dimension(640,480));
            itsSlotsPanel.setPreferredSize(new Dimension(panelWidth-440,panelHeight-340));
            
            successfulNumberOfSlots=4;
            
        } else {
            logger.debug("No Mainframe supplied");
        }
    }
    /**
     * Returns true as this panel needs a Popupmenu in the tree to get its data.
     * @return true
     */
    public boolean hasPopupMenu() {
        return true;
    }
    /**
     * Returns true as this panel needs support for multiple ParmDB table data selections.
     * @return true
     */
    public boolean isSingleton() {
        return true;
    }
    /**
     * Returns the static instance of ParmDBPlotPanel, if it does not exist, this method will create one.
     * @return the ParmDBPlotPanel instance
     */
    public JPanel getInstance() {
        if(instance == null){
            instance = this;
            initComponents();
        }
        return instance;
    }
    
    /** create popup menu for this panel
     *
     * This popupmenu will show options in which slot the user would like to make a plot in <br>
     * or add data to an existing plot.
     */
    public void createPopupMenu(Component aComponent,int x, int y) {
        // build up the menu
        JPopupMenu aPopupMenu = new JPopupMenu();
        
        int[] availableSlots = itsSlotsPanel.getAvailableSlotIndexes();
        if(availableSlots.length > 0){
            JMenu addSlotMenu=new JMenu("Add to slot");
            
            logger.trace("Available slots to put in popup menu: "+ availableSlots.length);
            for(int i = 0; i < availableSlots.length; i++){
                JMenuItem aMenuItem=new JMenuItem(""+availableSlots[i]);
                aMenuItem.setActionCommand("Add to slot "+availableSlots[i]);
                aMenuItem.addActionListener(new java.awt.event.ActionListener() {
                    public void actionPerformed(java.awt.event.ActionEvent evt) {
                        popupMenuHandler(evt);
                    }
                });
                
                addSlotMenu.add(aMenuItem);
                
            }
            aPopupMenu.add(addSlotMenu);
        }
        int[] occupiedSlots = itsSlotsPanel.getOccupiedSlotIndexes();
        if(occupiedSlots.length > 0){
            JMenu addSlotMenu=new JMenu("Add to plot in slot");
            
            logger.trace("Occupied slots to put in popup menu: "+ availableSlots.length);
            for(int i = 0; i < occupiedSlots.length; i++){
                JMenuItem aMenuItem=new JMenuItem(""+occupiedSlots[i]);
                aMenuItem.setActionCommand("Add to plot in slot "+occupiedSlots[i]);
                aMenuItem.addActionListener(new java.awt.event.ActionListener() {
                    public void actionPerformed(java.awt.event.ActionEvent evt) {
                        popupMenuHandler(evt);
                    }
                });
                
                addSlotMenu.add(aMenuItem);
                
            }
            aPopupMenu.add(addSlotMenu);
        }
        aPopupMenu.setOpaque(true);
        aPopupMenu.show(aComponent, x, y );
    }
    
    /** handles the choice from the popupmenu
     *
     * This method handles the choice between Adding a plot or Adding data to an existing plot.<br>
     * It then triggers the PlotSlotsPanel to act on the information.
     */
    public void popupMenuHandler(java.awt.event.ActionEvent evt) {
        logger.debug("PopUp menu Selection made: "+evt.getActionCommand().toString());
        if(evt.getActionCommand().startsWith("Add to slot")){
            int slotSelected = Integer.parseInt(evt.getActionCommand().toString().substring(12));
            logger.debug("Plot Slot extrapolated: "+slotSelected);
            itsSlotsPanel.addPlotToSlot(slotSelected,constructPlotterConstraints(itsParamName,itsParamTableName));
        } else if(evt.getActionCommand().startsWith("Add to plot in slot")){
            int slotSelected = Integer.parseInt(evt.getActionCommand().toString().substring(20));
            logger.debug("Plot Slot extrapolated: "+slotSelected);
            Object parameterConstraints =  constructPlotterConstraints(itsParamName,itsParamTableName);            
            itsSlotsPanel.alterDataInPlot(slotSelected,parameterConstraints,PlotConstants.DATASET_OPERATOR_ADD);
        }
    }
    /**
     * This helper method constructs a HashMap with constraints, <br>
     * compatible with the retrieveData method and the ADD updateData command used in PlotDataAccessParmDBImpl.
     *
     * @param aParamName The parameter name filter to search for in ParmDB
     * @param itsParamTableName The parameter table name that will be put in front of every ParmDB value for ID purposes.
     * @see nl.astron.lofar.sas.otb.util.plotter.PlotDataAccessParmDBImpl
     * @return HashMap<String,Object> object that can be passed on to PlotDataAccessParmDBImpl.
     */
    private Object constructPlotterConstraints(String aParamName,String itsParamTableName){
        HashMap<String,Object> parameterConstraints = new HashMap<String,Object>();
        parameterConstraints.put(new String("PARMDBINTERFACE"),SharedVars.getJParmFacade());
        
        String[] passToDataAccess = null;
        if (aParamName != null) {
            String cloneParamName = aParamName.toString();
            if(cloneParamName.equalsIgnoreCase(itsParamTableName)){
                cloneParamName = "*";
            }else{
                cloneParamName=cloneParamName.substring(itsParamTableName.length()+1);
                if(itsParam.isLeaf()){
                    cloneParamName += "*";
                }else{
                    cloneParamName += ":*";
                }
                
            }
            try{
                passToDataAccess = new String[8];
                
                Vector paramValues;
                paramValues = SharedVars.getJParmFacade().getRange(cloneParamName);
                //paramValues = SharedVars.getJParmFacade().getRange("*");
                
                double startx = Double.parseDouble(paramValues.get(0).toString());
                double endx =Double.parseDouble(paramValues.get(1).toString());
                double starty = Double.parseDouble(paramValues.get(2).toString());
                double endy = Double.parseDouble(paramValues.get(3).toString());
                
                if(startx==0.0 && endx==1.0 && starty==0.0 && endy==1.0){
                    logger.debug("Bypassing getRange() for ParmDB as invalid range was returned for "+cloneParamName+". Getting range for all parms");
                    paramValues = SharedVars.getJParmFacade().getRange("*");
                    startx = Double.parseDouble(paramValues.get(0).toString());
                    endx =Double.parseDouble(paramValues.get(1).toString());
                    starty = Double.parseDouble(paramValues.get(2).toString());
                    endy = Double.parseDouble(paramValues.get(3).toString());
                }
                int numx = Integer.parseInt("64");
                int numy = Integer.parseInt("1");
                
                passToDataAccess[0] = cloneParamName;
                passToDataAccess[1] = ""+startx;
                passToDataAccess[2] = ""+endx;
                passToDataAccess[3] = ""+numx;
                passToDataAccess[4] = ""+starty;
                passToDataAccess[5] = ""+endy;
                passToDataAccess[6] = ""+numy;
                passToDataAccess[7] = itsParamTableName;
            }catch(Exception ex){
                JOptionPane.showMessageDialog(itsMainFrame, ex.getMessage(),
                        "Error detected",
                        JOptionPane.ERROR_MESSAGE);
                logger.error("Plotter created an exception :"+ex.getMessage(),ex);
            }
        } else {
            logger.debug("ERROR:  no Param Name given");
        }
        parameterConstraints.put("PARMDBCONSTRAINTS",passToDataAccess);
        return parameterConstraints;
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
    /**
     * This method reads the ParmDB location set in the TreeNode that triggers this method.
     * It then updates the JParmFacade location string so the plotter can find the data it needs.
     */
    public void setContent(Object anObject) {
        
        jParmDBnode node = (jParmDBnode)anObject;
        try {
            SharedVars.getJParmFacade().setParmFacadeDB(node.getParmDBLocation());
            
        } catch (RemoteException ex) {
            logger.error("setContent() - jParmFacade RMI error while updating table name ",ex);
        }
        itsParamName = node.getNodeID();
        itsParam=node;
        itsParamTableName = node.getParmDBIdentifier();
        logger.trace("ParmDB name selected : "+itsParamName);
        
        
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
        bClearSlots = new javax.swing.JButton();
        bHelp = new javax.swing.JButton();

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
        itsSlotsPanel.setMinimumSize(new java.awt.Dimension(640, 480));
        itsSlotsPanel.setPreferredSize(null);
        slotsPane.setViewportView(itsSlotsPanel);

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.gridwidth = 4;
        gridBagConstraints.fill = java.awt.GridBagConstraints.BOTH;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 2, 2);
        contentPanel.add(slotsPane, gridBagConstraints);

        bClearSlots.setText("Clear Slots");
        bClearSlots.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                bClearSlotsActionPerformed(evt);
            }
        });

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.anchor = java.awt.GridBagConstraints.WEST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 0, 0);
        contentPanel.add(bClearSlots, gridBagConstraints);

        bHelp.setText("Help");
        bHelp.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                bHelpActionPerformed(evt);
            }
        });

        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.ipadx = 10;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.EAST;
        gridBagConstraints.insets = new java.awt.Insets(2, 2, 0, 20);
        contentPanel.add(bHelp, gridBagConstraints);

        add(contentPanel, java.awt.BorderLayout.CENTER);

    }// </editor-fold>//GEN-END:initComponents
    
    private void bClearSlotsActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_bClearSlotsActionPerformed
        String message = "Are you sure you want to clear all slots?";
        String[] buttons = {"Clear Slots","Cancel"};
        int choice =  JOptionPane.showOptionDialog(this,message, "Please confirm", JOptionPane.DEFAULT_OPTION, JOptionPane.QUESTION_MESSAGE, null,buttons,buttons[0]);
        if(choice == 0){
            itsSlotsPanel.clearSlots();
        }
        
    }//GEN-LAST:event_bClearSlotsActionPerformed
    
    private void bHelpActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_bHelpActionPerformed
        String message = "Slots:\nRight-click on the light-gray header of a slot";
        message+=" to see what you can do with the plot in it.\n\nPlots:\n";
        message+="To zoom: Click and hold the left mouse button and select a rectangle.";
        message+="\nTo reset the zoom: Press CTRL-LeftMouseButton to reset the zoom.";
        message+="\nTo change colors/etc: Double-Click on a line in the legend.\n";
        message+="To change plot/axis labels and tics/etc: Click on an axis or title";
        message+=" and press the right mouse button.";
        
        JOptionPane.showMessageDialog(null,message, "Help",JOptionPane.INFORMATION_MESSAGE);
        
    }//GEN-LAST:event_bHelpActionPerformed
    
    private void cSlotsAmountItemStateChanged(java.awt.event.ItemEvent evt) {//GEN-FIRST:event_cSlotsAmountItemStateChanged
        int wannaHaveSlots = Integer.parseInt(evt.getItemSelectable().getSelectedObjects()[0].toString());
        if (wannaHaveSlots != successfulNumberOfSlots){
            try {
                wannaHaveSlots = Integer.parseInt(evt.getItem().toString());
                itsSlotsPanel.setAmountOfSlots(wannaHaveSlots,false);
                successfulNumberOfSlots = itsSlotsPanel.getAmountOfSlots();
            } catch (NumberFormatException ex) {
                logger.error(ex);
            } catch (IllegalArgumentException ex) {
                //TODO log!
                String[] buttons = {"Clear Slots","Cancel"};
                String exceptionString = ex.getMessage();
                exceptionString+="\n\nPlease clear or move ";
                exceptionString+= "these plots manually by pressing cancel,\nor let ";
                exceptionString+= "the application delete them by pressing Clear Slots.";
                int choice =  JOptionPane.showOptionDialog(this,exceptionString, "Plots detected in to be deleted slots", JOptionPane.DEFAULT_OPTION, JOptionPane.QUESTION_MESSAGE, null,buttons,buttons[0]);
                if(choice == 0){
                    itsSlotsPanel.setAmountOfSlots(Integer.parseInt(evt.getItem().toString()),true);
                    successfulNumberOfSlots = itsSlotsPanel.getAmountOfSlots();
                }else{
                    double squareRoot = Math.sqrt(Double.parseDouble(""+successfulNumberOfSlots));
                    int wishedIndex = (Integer.parseInt(""+(int)squareRoot))-1;
                    cSlotsAmount.setSelectedItem(new String(""+successfulNumberOfSlots));
                }
            }
            
        }
    }//GEN-LAST:event_cSlotsAmountItemStateChanged
    
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton bClearSlots;
    private javax.swing.JButton bHelp;
    private javax.swing.JComboBox cSlotsAmount;
    private javax.swing.JPanel contentPanel;
    private nl.astron.lofar.sas.otb.util.plotter.PlotSlotsPanel itsSlotsPanel;
    private javax.swing.JLabel lSlotsAmount;
    private javax.swing.JScrollPane slotsPane;
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
