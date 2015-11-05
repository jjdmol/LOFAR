/*
 *  PlotSlotsPanel.java
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
 *
 */

package nl.astron.lofar.sas.otb.util.plotter;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.util.HashMap;
import java.util.LinkedList;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.SwingConstants;
import nl.astron.lofar.java.gui.plotter.PlotConstants;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;
import nl.astron.lofar.sas.otb.SharedVars;
import org.apache.log4j.Logger;

/**
 * This class visually represents the collection of PlotSlots available from the<br>
 * PlotSlotManager. On top of that the user of this panel can perform standard operations <br>
 * and operations needed in the LOFAR BBS/ParmDB Plotter system
 *
 * @version $Id$
 * @created May 29, 2006, 14:12
 * @author pompert
 * @see PlotSlotManager
 */
public class PlotSlotsPanel extends javax.swing.JPanel {
    
    static Logger logger = Logger.getLogger(PlotSlotsPanel.class);
    static String name = "PlotSlotsPanel";
    private PlotSlotManager itsSlotManager;
    private PlotSlot selectedSlot;
    
    
    /**
     * Creates a new PlotSlotsPanel. It connects itself to the PlotSlotManager<br>
     * and adds listeners to that manager so this panel is refreshed whenever something<br>
     * changes in the PlotSlot collection.
     */
    public PlotSlotsPanel() {
        initComponents();
        int numberOfSlots = 4;
        itsSlotManager = new PlotSlotManager(numberOfSlots);
        itsSlotManager.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent evt){
                if(evt.getActionCommand().equals(PlotSlotManager.REFRESH_FULL)){
                    rearrangeSlotGrid();
                }
                if(evt.getActionCommand().equals(PlotSlotManager.REFRESH_SINGLE)){
                    int slotId = evt.getID();
                    itsSlotManager.getSlot(slotId).validate();
                }
            }
        });
        rearrangeSlotGrid();
    }
    /**
     * This method will create a plot in the PlotSlot specified using the constraints.<br>
     * @param index the PlotSlot index in the PlotSlotManager's collection
     * @param constraints the constraints for the LOFAR Plotter framework to build the plot.
     * @throws IllegalArgumentException is thrown if a plot already exists in the PlotSlot specified.
     */
    public void addPlotToSlot(int index,Object constraints) throws IllegalArgumentException{
        if(itsSlotManager.isSlotAvailable(index)){
            itsSlotManager.createPlotInSlot(index,constraints);
        }else{
            throw new IllegalArgumentException("A plot already exists in slot "+index);
        }
    }
    /**
     * This method will alter a plot in the PlotSlot specified using the update constraints.<br>
     * @param index the PlotSlot index in PlotSlotManager's collection
     * @param constraints the constraints for the LOFAR Plotter framework to update the plot.
     * @operation A string representation of the operation that should be passed to the LOFAR Plotter framework.
     * @throws IllegalArgumentException is thrown if a plot is not found in the PlotSlot specified.
     */
    @SuppressWarnings("unchecked")
    public void alterDataInPlot(int slotIndex,Object constraints, String operation) throws IllegalArgumentException{
        if(itsSlotManager.isSlotOccupied(slotIndex)){
            double offset = itsSlotManager.getSlot(slotIndex).getOffset();
            String[] offsetS = new String[1];
            offsetS[0] = ""+offset;
            //Remove offsets if present to prevent gaps in the dataset.
            if(offset != 0.0 && !operation.equalsIgnoreCase("DATASET_OPERATOR_REMOVE_Y_OFFSET")){
                HashMap<String,Object> alterOffset = new HashMap<String,Object>();
                alterOffset.put(new String("PARMDBINTERFACE"),SharedVars.getJParmFacade());
                alterOffset.put("DATASET_OPERATOR_REMOVE_Y_OFFSET",offsetS);
                itsSlotManager.modifyPlotInSlot(slotIndex,alterOffset);
                //update the data identifiers passed as the offset has been removed...Not necessary for additions!
                if(!operation.equalsIgnoreCase(PlotConstants.DATASET_OPERATOR_ADD)){
                    
                    String[] dataIdentifiers = (String[])constraints;
                    String[] newDataIdentifiers = new String[dataIdentifiers.length];
                    int i=0;
                    for(String oldString : dataIdentifiers){
                        String newString = null;
                        if(oldString.contains(" OFFSET")){
                            newString = oldString.substring(0, oldString.lastIndexOf(" OFFSET"));
                        }else{
                            newString = oldString;
                        }
                        newDataIdentifiers[i]=newString;
                        i++;
                    }
                    constraints = newDataIdentifiers;
                }
                
            }
            //Perform the actual addition or removal of data in the plot.
            HashMap<String,Object> alterData = new HashMap<String,Object>();
            alterData.put(new String("PARMDBINTERFACE"),SharedVars.getJParmFacade());
            alterData.put(operation,constraints);
            
            itsSlotManager.modifyPlotInSlot(slotIndex,alterData);
            
            //Reapply the offsets using the new values. Skips this step if the user wants to remove the offset.
            if(offset != 0.0 && !operation.equalsIgnoreCase("DATASET_OPERATOR_REMOVE_Y_OFFSET")){
                HashMap<String,Object> alterOffset = new HashMap<String,Object>();
                alterOffset.put(new String("PARMDBINTERFACE"),SharedVars.getJParmFacade());
                alterOffset.put("DATASET_OPERATOR_ADD_Y_OFFSET",offsetS);
                itsSlotManager.modifyPlotInSlot(slotIndex,alterOffset);
            }
            //Remove any offset if a delete action results in only one value in the plot being left.
            if(operation.equalsIgnoreCase(PlotConstants.DATASET_OPERATOR_DELETE)){
                LinkedList<HashMap> currentValuesInPlot;
                try {
                    currentValuesInPlot = (LinkedList<HashMap>) itsSlotManager.getSlot(slotIndex).getPlot().getDataForPlot().get(PlotConstants.DATASET_VALUES);
                    if(offset != 0.0 && currentValuesInPlot.size()==1){
                        
                        String[] valueArray = new String[1];
                        valueArray[0] = ""+itsSlotManager.getSlot(slotIndex).getOffset();
                        HashMap<String,Object> alterOffset = new HashMap<String,Object>();
                        alterOffset.put(new String("PARMDBINTERFACE"),SharedVars.getJParmFacade());
                        alterOffset.put("DATASET_OPERATOR_REMOVE_Y_OFFSET",valueArray);
                        itsSlotManager.modifyPlotInSlot(slotIndex,alterOffset);
                        itsSlotManager.getSlot(slotIndex).setOffset(0.0);
                    }
                } catch (PlotterException ex) {
                    ex.printStackTrace();
                }
            }
            
        }else{
            throw new IllegalArgumentException("A plot was not found in slot "+slotIndex);
        }
    }
    /**
     * This method clears all the PlotSlots in PlotSlotManager's collection
     */
    public void clearSlots(){
        itsSlotManager.clearSlots();
        repaint();
    }
    /**
     * Gets the amount of PlotSlots in PlotSlotManager's collection
     * @return the amount of PlotSlots in PlotSlotManager's collection
     */
    public int getAmountOfSlots(){
        return itsSlotManager.getAmountOfSlots();
    }
    /**
     * This method sets the amount of PlotSlots in PlotSlotManager's collection. <br><br>
     * See PlotSlotManager.setAmountOfSlots() for further information.
     * @param amount The new amount of slots to be managed in PlotSlotManager's collection.
     * @param force Force the amount on the collection, and delete any PlotSlot in the way.
     * @throws IllegalArgumentException will be thrown if one or more PlotSlots <br>
     * are in the way and the force argument is false.
     * @see PlotSlotManager
     */
    public void setAmountOfSlots(int amount, boolean force) throws IllegalArgumentException{
        itsSlotManager.setAmountOfSlots(amount,force);
    }
    /**
     * This method answers if the PlotSlot specified is available/empty.
     * @param index the PlotSlot index to check
     * @return true if the PlotSlot is available
     * @return false if the PlotSlot is occupied
     */
    public boolean isSlotAvailable(int index){
        return itsSlotManager.isSlotAvailable(index);
    }
    /**
     * This method gets all the indexes in PlotSlotManager's collection where an empty PlotSlot is present.
     * @return the indexes of the available slots.
     */
    public int[] getAvailableSlotIndexes(){
        return itsSlotManager.getAvailableSlotIndexes();
    }
    /**
     * This method gets all the indexes in PlotSlotManager's collection where a occupied PlotSlot is present.
     * @return the indexes of the occupied/filled/non-empty slots.
     */
    public int[] getOccupiedSlotIndexes(){
        return itsSlotManager.getOccupiedSlotIndexes();
    }
    /**
     * This helper method refreshes the entire Panel by removing all currently displayed PlotSlots and <br>
     * retrieving them again from the PlotSlotManager. It then displays them.
     */
    private void rearrangeSlotGrid(){
        slotsPanel.removeAll();
        
        //determine the amount of rows and columns to display the slots. Only supports
        //amounts that can be square-rooted to a positive integer!
        double squareRoot = Math.sqrt(Double.parseDouble(""+itsSlotManager.getAmountOfSlots()));
        int columnsAndRows = Integer.parseInt(""+(int)squareRoot);
        GridBagConstraints gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridwidth = columnsAndRows;
        gridBagConstraints.gridheight = columnsAndRows;
        gridBagConstraints.anchor = java.awt.GridBagConstraints.NORTHWEST;
        gridBagConstraints.insets = new java.awt.Insets(0, 0, 0, 0);
        GridBagLayout layout = new GridBagLayout();
        layout.setConstraints(slotsPanel,gridBagConstraints);
        slotsPanel.setSize(this.getSize());
        logger.trace("getting "+itsSlotManager.getAmountOfSlots()+" slots from PlotSlotManager");
        int x = 0;
        int y = 0;
        for(int i = 1; i <= itsSlotManager.getAmountOfSlots(); i++){
            logger.trace("Setting Slot to grid coordinate ("+x+","+y+")");
            gridBagConstraints = new java.awt.GridBagConstraints();
            gridBagConstraints.gridx = x;
            gridBagConstraints.gridy = y;
            gridBagConstraints.ipadx = 0;
            gridBagConstraints.ipady = 0;
            gridBagConstraints.weightx = 1.0;
            gridBagConstraints.weighty = 1.0;
            gridBagConstraints.anchor = java.awt.GridBagConstraints.CENTER;
            PlotSlot newSlot = itsSlotManager.getSlot(i);
            if(!newSlot.isViewedExternally()){
                //Display the slot normally in the panel
                //Add a slotListener to the slot
                newSlot.addSlotListener(new SlotMouseAdapter());
                newSlot.setMinimumSize(new Dimension(getWidth()/columnsAndRows,getHeight()/columnsAndRows));
                newSlot.setPreferredSize(new Dimension(getWidth()/columnsAndRows,getHeight()/columnsAndRows));
                slotsPanel.add(newSlot,gridBagConstraints);
            }else{
                //Show a temporary panel as the slot is displayed elsewhere and
                //could cause display problems if displayed again here.
                JPanel tempPanel = new JPanel();
                tempPanel.setBackground(Color.WHITE);
                tempPanel.setLayout(new BorderLayout());
                tempPanel.setBorder(javax.swing.BorderFactory.createEtchedBorder());
                tempPanel.setMinimumSize(new Dimension(getWidth()/columnsAndRows,getHeight()/columnsAndRows));
                tempPanel.setPreferredSize(new Dimension(getWidth()/columnsAndRows,getHeight()/columnsAndRows));
                JLabel tempLabel = new JLabel("External viewer active");
                tempLabel.setHorizontalAlignment(SwingConstants.CENTER);
                tempPanel.add(tempLabel,BorderLayout.CENTER);
                JPanel northPanel = new JPanel();
                northPanel.setLayout(new BorderLayout());
                northPanel.setBackground(Color.LIGHT_GRAY);
                JLabel slotNumber = new JLabel(""+i);
                slotNumber.setForeground(Color.WHITE);
                northPanel.add(slotNumber,BorderLayout.CENTER);
                tempPanel.add(northPanel,BorderLayout.NORTH);
                
                slotsPanel.add(tempPanel,gridBagConstraints);
            }
            
            x++;
            if (x == columnsAndRows){
                y++;
                x = 0;
            }
        }
        //Reset the GridBagLayout so a repaint is invoked.
        slotsPanel.validate();
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        slotsPanel = new javax.swing.JPanel();

        setLayout(new java.awt.BorderLayout());

        slotsPanel.setLayout(new java.awt.GridBagLayout());

        slotsPanel.setBackground(new java.awt.Color(255, 255, 255));
        slotsPanel.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        slotsPanel.setMinimumSize(null);
        slotsPanel.setPreferredSize(null);
        add(slotsPanel, java.awt.BorderLayout.CENTER);

    }// </editor-fold>//GEN-END:initComponents
            // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel slotsPanel;
    // End of variables declaration//GEN-END:variables
    
    /**
     * Utility field used by event firing mechanism.
     */
    private javax.swing.event.EventListenerList myListenerList =  null;
    
    /**
     * Registers ActionListener to receive events.
     *
     * @param listener The listener to register.
     */
    public void addActionListener(java.awt.event.ActionListener listener) {
        
        if (myListenerList == null ) {
            myListenerList = new javax.swing.event.EventListenerList();
        }
        myListenerList.add(java.awt.event.ActionListener.class, listener);
    }
    
    /**
     * Removes ActionListener from the list of listeners.
     *
     * @param listener The listener to remove.
     */
    public void removeActionListener(java.awt.event.ActionListener listener) {
        
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
    
    /**
     * This inner class provides the plotter with the functionality of moving slots etc
     *
     */
    
    class SlotMouseAdapter extends PlotSlotListener{
        
        private boolean externalLegendActive = false;
        private boolean externalViewerActive = false;
        //set the default offset to be displayed in the offset dialog
        private String previousOffset = "1.0";
        /**
         * This method builds up the Context menu when you right-click on the slot header.<br>
         *
         * @param aSlot the slot that was triggered by the user.
         * @e the MouseEvent itself as it is needed to pinpoint the location on the screen where to display the menu.
         */
        @SuppressWarnings("unchecked")
        @Override
        public void slotContextMenuTriggered(PlotSlot aSlot, MouseEvent e){
            if(aSlot instanceof PlotSlot){
                selectedSlot = aSlot;
                if(selectedSlot.containsPlot()){
                    
                    JPopupMenu aPopupMenu = new JPopupMenu();
                    
                    if(selectedSlot.containsLegend() && !externalLegendActive){
                        JMenuItem aMenuItem=new JMenuItem("Hide Legend");
                        aMenuItem.addActionListener(new java.awt.event.ActionListener() {
                            public void actionPerformed(java.awt.event.ActionEvent evt) {
                                selectedSlot.removeLegend();
                                selectedSlot.validate();
                                
                            }
                        });
                        aPopupMenu.add(aMenuItem);
                    }else if (!externalLegendActive && !externalViewerActive){
                        //Show menu to display a legend inside the slot or in
                        //a separate window.
                        JMenu aMenuItem=new JMenu("Show legend");
                        JMenuItem subMenuItem=new JMenuItem("Inside the slot");
                        
                        subMenuItem.addActionListener(new java.awt.event.ActionListener() {
                            public void actionPerformed(java.awt.event.ActionEvent evt) {
                                selectedSlot.addLegend();
                                selectedSlot.validate();
                            }
                        });
                        JMenuItem subMenuItem2=new JMenuItem("In separate window");
                        subMenuItem2.addActionListener(new java.awt.event.ActionListener() {
                            public void actionPerformed(java.awt.event.ActionEvent evt) {
                                PlotSlotViewFrame dialog = new PlotSlotViewFrame(itsSlotManager,Integer.parseInt(selectedSlot.getLabel()),"Legend viewer for Plot "+selectedSlot.getLabel(),true);
                                dialog.addWindowListener(new WindowListener(){
                                    public void windowDeactivated(WindowEvent e){}
                                    public void windowActivated(WindowEvent e){}
                                    public void windowClosed(WindowEvent e){}
                                    public void windowDeiconified(WindowEvent e){}
                                    public void windowIconified(WindowEvent e){}
                                    public void windowClosing(WindowEvent e){
                                        externalLegendActive = false;
                                    }
                                    public void windowOpened(WindowEvent e){}
                                });
                                dialog.setVisible(true);
                                externalLegendActive = true;
                            }
                        });
                        aMenuItem.add(subMenuItem);
                        aMenuItem.add(subMenuItem2);
                        aPopupMenu.add(aMenuItem);
                    }else if(externalLegendActive){
                        //Show a warning that you will need to close the legend viewer first.
                        JMenuItem subMenuItem=new JMenuItem("Please close the legend viewer first");
                        subMenuItem.setEnabled(false);
                        aPopupMenu.add(subMenuItem);
                    }else if(externalViewerActive){
                        //Show the option of showing the legend in the separately viewed PlotSlot
                        JMenuItem subMenuItem=new JMenuItem("Show Legend");
                        subMenuItem.addActionListener(new java.awt.event.ActionListener() {
                            public void actionPerformed(java.awt.event.ActionEvent evt) {
                                selectedSlot.addLegend();
                                selectedSlot.validate();
                            }
                        });
                        aPopupMenu.add(subMenuItem);
                    }
                    
                    if(!externalLegendActive){
                        //Show menu options that work if no external legend is active for this slot.
                        if(!externalViewerActive){
                            //Show menu options that work if no external window is active for this slot
                            int[] availableSlots = itsSlotManager.getAvailableSlotIndexes();
                            if(availableSlots.length > 0){
                                aPopupMenu.addSeparator();
                                
                                JMenu aMenuItem2=new JMenu("Move to slot");
                                
                                for(int i = 0; i < availableSlots.length; i++){
                                    JMenuItem subItem=new JMenuItem(""+availableSlots[i]);
                                    subItem.setActionCommand(""+availableSlots[i]);
                                    subItem.addActionListener(new java.awt.event.ActionListener() {
                                        public void actionPerformed(java.awt.event.ActionEvent evt) {
                                            
                                            itsSlotManager.movePlot(Integer.parseInt(selectedSlot.getLabel()),Integer.parseInt(evt.getActionCommand()));
                                            repaint();
                                            
                                        }
                                    });
                                    aMenuItem2.add(subItem);
                                }
                                aPopupMenu.add(aMenuItem2);
                            }
                            aPopupMenu.addSeparator();
                            JMenuItem aMenuItem=new JMenuItem("Show in separate window");
                            aMenuItem.addActionListener(new java.awt.event.ActionListener() {
                                public void actionPerformed(java.awt.event.ActionEvent evt) {
                                    PlotSlotViewFrame dialog = new PlotSlotViewFrame(itsSlotManager,Integer.parseInt(selectedSlot.getLabel()),"Viewer for Plot "+selectedSlot.getLabel(),false);
                                    //Add a listener so the grid of PlotSlots get refreshed when a separate viewer is closed.
                                    dialog.addWindowListener(new WindowListener(){
                                        public void windowDeactivated(WindowEvent e){}
                                        public void windowActivated(WindowEvent e){
                                            PlotSlotViewFrame sourceFrame = (PlotSlotViewFrame)e.getSource();
                                            itsSlotManager.getSlot(sourceFrame.plotIndex).setViewedExternally(true);
                                            rearrangeSlotGrid();}
                                        public void windowClosed(WindowEvent e){
                                            PlotSlotViewFrame sourceFrame = (PlotSlotViewFrame)e.getSource();
                                            itsSlotManager.getSlot(sourceFrame.plotIndex).setViewedExternally(false);
                                            rearrangeSlotGrid();
                                            externalLegendActive = false;}
                                        public void windowDeiconified(WindowEvent e){}
                                        public void windowIconified(WindowEvent e){}
                                        public void windowClosing(WindowEvent e){}
                                        public void windowOpened(WindowEvent e){}
                                    });
                                    dialog.setVisible(true);
                                    externalViewerActive=true;
                                }
                            });
                            aPopupMenu.add(aMenuItem);
                        }
                        aPopupMenu.addSeparator();
                        JMenuItem aMenuItem2=new JMenuItem("Clear Slot");
                        aMenuItem2.addActionListener(new java.awt.event.ActionListener() {
                            public void actionPerformed(java.awt.event.ActionEvent evt) {
                                itsSlotManager.clearSlot(Integer.parseInt(selectedSlot.getLabel()));
                                selectedSlot.repaint();
                            }
                        });
                        aPopupMenu.add(aMenuItem2);
                        JMenuItem aMenuItem3=new JMenuItem("Print Slot");
                        aMenuItem3.addActionListener(new java.awt.event.ActionListener() {
                            public void actionPerformed(java.awt.event.ActionEvent evt) {
                                selectedSlot.printSlot();
                                selectedSlot.validate();
                            }
                        });
                        aPopupMenu.add(aMenuItem3);
                        
                        try {
                            LinkedList<HashMap> currentValuesInPlot = (LinkedList<HashMap>)selectedSlot.getPlot().getDataForPlot().get(PlotConstants.DATASET_VALUES);
                            if(currentValuesInPlot.size()>0){
                                
                                aPopupMenu.addSeparator();
                                double offset = selectedSlot.getOffset();
                                if(offset==0.0){
                                    JMenuItem offSet=new JMenuItem("Add Y-Axis offset");
                                    offSet.addActionListener(new java.awt.event.ActionListener() {
                                        public void actionPerformed(java.awt.event.ActionEvent evt) {
                                            String[] valueArray = new String[1];
                                            String offsetSpecifier = JOptionPane.showInputDialog("Please specify a valid double as an offset. Example: 1 or 2.5",previousOffset);
                                            if(offsetSpecifier!=null){
                                                double offset = 0.0;
                                                try {
                                                    
                                                    offset = Double.parseDouble(offsetSpecifier);
                                                    valueArray[0] = offsetSpecifier;
                                                    alterDataInPlot(Integer.parseInt(selectedSlot.getLabel()),valueArray,"DATASET_OPERATOR_ADD_Y_OFFSET");
                                                    selectedSlot.setOffset(offset);
                                                    previousOffset = offsetSpecifier;
                                                } catch (NumberFormatException ex) {
                                                    JOptionPane.showMessageDialog(null, "Invalid offset specified!",
                                                            "Invalid input",
                                                            JOptionPane.ERROR_MESSAGE);
                                                    logger.error("User failed to specify a double offset: "+ex.getMessage());
                                                }
                                            }
                                        }
                                    });
                                    aPopupMenu.add(offSet);
                                }else{
                                    JMenuItem offSet=new JMenuItem("Remove Y-Axis offset");
                                    offSet.setActionCommand(""+offset);
                                    offSet.addActionListener(new java.awt.event.ActionListener() {
                                        public void actionPerformed(java.awt.event.ActionEvent evt) {
                                            String[] valueArray = new String[1];
                                            valueArray[0] = evt.getActionCommand();
                                            alterDataInPlot(Integer.parseInt(selectedSlot.getLabel()),valueArray,"DATASET_OPERATOR_REMOVE_Y_OFFSET");
                                            selectedSlot.setOffset(0.0);
                                        }
                                    });
                                    aPopupMenu.add(offSet);
                                }
                                JMenu aMenu=new JMenu("Modify value(s)");
                                if(currentValuesInPlot.size()>1){
                                    
                                    JMenu subtractMenu=new JMenu("Subtract operations");
                                    
                                    JMenuItem subtractMean=new JMenuItem("Subtract mean(all values) from all values");
                                    subtractMean.setActionCommand("Subtract mean(all values) from all values");
                                    subtractMean.addActionListener(new java.awt.event.ActionListener() {
                                        public void actionPerformed(java.awt.event.ActionEvent evt) {
                                            String[] valueArray = new String[1];
                                            valueArray[0] = evt.getActionCommand();
                                            alterDataInPlot(Integer.parseInt(selectedSlot.getLabel()),valueArray,"DATASET_OPERATOR_SUBTRACT_MEAN_ALL_LINES_FROM_ALL_LINES");
                                        }
                                    });
                                    subtractMenu.add(subtractMean);
                                    /*
                                    JMenu subtractSingleMenu=new JMenu("Subtract mean(all values) from value");
                                     
                                    for(HashMap aValue : currentValuesInPlot){
                                        String dataValueLabel = (String)aValue.get(PlotConstants.DATASET_VALUELABEL);
                                        JMenuItem subtractValueItem=new JMenuItem(dataValueLabel);
                                        subtractValueItem.setActionCommand(dataValueLabel);
                                        subtractValueItem.addActionListener(new java.awt.event.ActionListener() {
                                            public void actionPerformed(java.awt.event.ActionEvent evt) {
                                                String[] valueArray = new String[1];
                                                valueArray[0] = evt.getActionCommand();
                                                alterDataInPlot(Integer.parseInt(selectedSlot.getLabel()),valueArray,"DATASET_OPERATOR_SUBTRACT_MEAN_ALL_LINES_FROM_LINE");
                                     
                                            }
                                        });
                                        subtractSingleMenu.add(subtractValueItem);
                                     
                                    }
                                    subtractMenu.add(subtractSingleMenu);
                                     */
                                    JMenu subtractValueMenu=new JMenu("Subtract one value from all other values");
                                    
                                    for(HashMap aValue : currentValuesInPlot){
                                        String dataValueLabel = (String)aValue.get(PlotConstants.DATASET_VALUELABEL);
                                        JMenuItem subtractValueItem=new JMenuItem(dataValueLabel);
                                        subtractValueItem.setActionCommand(dataValueLabel);
                                        subtractValueItem.addActionListener(new java.awt.event.ActionListener() {
                                            public void actionPerformed(java.awt.event.ActionEvent evt) {
                                                String[] valueArray = new String[1];
                                                valueArray[0] = evt.getActionCommand();
                                                alterDataInPlot(Integer.parseInt(selectedSlot.getLabel()),valueArray,"DATASET_OPERATOR_SUBTRACT_LINE");
                                                
                                            }
                                        });
                                        subtractValueMenu.add(subtractValueItem);
                                        
                                    }
                                    subtractMenu.add(subtractValueMenu);
                                    aMenu.add(subtractMenu);
                                }
                                JMenu aMenu2=new JMenu("Remove value from plot");
                                for(HashMap aValue : currentValuesInPlot){
                                    String dataValueLabel = (String)aValue.get(PlotConstants.DATASET_VALUELABEL);
                                    JMenuItem clearValueItem=new JMenuItem(dataValueLabel);
                                    clearValueItem.setActionCommand(dataValueLabel);
                                    clearValueItem.addActionListener(new java.awt.event.ActionListener() {
                                        public void actionPerformed(java.awt.event.ActionEvent evt) {
                                            String[] valueArray = new String[1];
                                            valueArray[0] = evt.getActionCommand();
                                            alterDataInPlot(Integer.parseInt(selectedSlot.getLabel()),valueArray,PlotConstants.DATASET_OPERATOR_DELETE);
                                            //selectedSlot.repaint();
                                        }
                                    });
                                    aMenu2.add(clearValueItem);
                                    
                                }
                                
                                aMenu.add(aMenu2);
                                aPopupMenu.add(aMenu);
                            }
                        } catch (NumberFormatException ex) {
                            logger.error("Could not delete data in slot "+selectedSlot.getLabel()+".",ex);
                        } catch (PlotterException ex) {
                            logger.error("Could not retrieve the data for slot "+selectedSlot.getLabel()+" to delete specific value(s).",ex);
                        }
                        
                    }
                    aPopupMenu.setOpaque(true);
                    aPopupMenu.show(selectedSlot, e.getX(), e.getY());
                }
            }
        }
    }
    
}
