/*
 * PlotSlot.java
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
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.LinkedList;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import javax.swing.SwingUtilities;
import nl.astron.lofar.java.gui.plotter.PlotConstants;
import nl.astron.lofar.java.gui.plotter.PlotPanel;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterPrintException;
import org.apache.log4j.Logger;

/**
 * This class serves as a wrapper for a plot made using the LOFAR Plotter Framework.
 *
 * @version $Id$
 * @created May 24, 2006, 11:12 AM
 * @author pompert
 * @see nl.astron.lofar.java.gui.plotter.*
 */
public class PlotSlot extends JPanel{
    
    public static final String EMPTY_SLOT = "Empty Slot";
    private static Logger logger = Logger.getLogger(PlotSlot.class);
    private PlotGroup itsPlotGroup;
    private PlotPanel itsPlot;
    private String slotLabel;
    private boolean hasLegend;
    private double offset;
    private boolean isViewedExternally;
    private JLabel rightClickFacade;
    private LinkedList<PlotSlotListener> myListenerList =  null;
    private MouseAdapter plotMouseListener;
    
    /** Creates a new instance of PlotSlot */    
    public PlotSlot() {
        itsPlotGroup = null;
        itsPlot = null;
        hasLegend = false;
        offset = 0.0;
        setBackground(Color.WHITE);
        plotMouseListener = new MouseAdapter(){
            @Override
            public void mouseReleased(MouseEvent evt){
                fireContextMenuEvent(evt);
            }
        };
        setBorder(javax.swing.BorderFactory.createEtchedBorder());
        setSize(new Dimension(320,240));
        setPreferredSize(new Dimension(320,240));
        setLayout(new BorderLayout());
        rightClickFacade = new JLabel("Empty");
        setLabel(EMPTY_SLOT);
        addOptionLabel();
    }
    /**
     * Creates a new instance of PlotSlot using a supplied label
     * @param name The name to be given to the slot.
     *
     */
    public PlotSlot(String label) {
        this();
        setLabel(label);
        rightClickFacade.setText(" "+label);
    }
    /**
     * Sets the label of the slot to the given String.
     * @param label The label or name to be given to the slot.
     *
     */
    public void setLabel(String label){
        slotLabel = label;
        rightClickFacade.setText(" "+label);
    }
    /**
     * Gets the label of the slot.
     * @return the Slot label
     */
    public String getLabel(){
        return slotLabel;
    }
    /**
     * Sets a helper boolean that shows if this slot is viewed in an external dialog instead of a main panel.
     * Suggested usage: within PlotSlotsPanel only.
     * @param viewedExternally Sets the value that can help determine if the slot is viewed externally.
     * @see PlotSlotsPanel
     */
    protected void setViewedExternally(boolean viewedExternally){
        this.isViewedExternally = viewedExternally;
    }
    /**
     * Gets the value that shows if the slot is viewed externally.
     * @return true if the PlotSlot is viewed externally 
     * @return false if the PlotSlot is not viewed externally.
     */
    protected boolean isViewedExternally(){
        return isViewedExternally;
    }
    /**
     * Gets the LOFAR Plotter framework plot contained in this slot.
     * @return the PlotPanel inside this PlotSlot
     */
    public PlotPanel getPlot(){
        return itsPlot;
    }
    /**
     * Sets the LOFAR Plotter framework plot to be contained in this slot. You are urged
     * only to use this method if addPlot() does not suit your needs.
     * @param aPlot a LOFAR Plotter framework plot.
     * @see nl.astron.lofar.java.gui.plotter.PlotPanel
     */
    public void setPlot(PlotPanel aPlot){
        removeAll();
        itsPlot = aPlot;
        addOptionLabel();
        add(itsPlot,BorderLayout.CENTER);
        if(containsLegend()){
            addLegend();
        }
    }
    /**
     * Adds a LOFAR Plotter framework plot to be displayed in this slot.
     * @param constraints an Object that will be passed to the plotter framework with constraints.
     * @see nl.astron.lofar.java.gui.plotter.PlotPanel
     * @see nl.astron.lofar.java.gui.plotter.PlotConstants
     * @see nl.astron.lofar.java.gui.plotter.IPlotDataAccess
     */
    public void addPlot(Object constraints){
        removeAll();
        addOptionLabel();
        try {
            if(itsPlot == null){
                itsPlot = new PlotPanel();
            }
            itsPlot.createPlot(PlotConstants.PLOT_XYLINE,true,constraints);
            add(itsPlot,BorderLayout.CENTER);
        } catch (PlotterException ex) {
            JOptionPane.showMessageDialog(null, ex.getMessage(),
                    "Error detected while making the plot",
                    JOptionPane.ERROR_MESSAGE);
            
            logger.error(ex);
            itsPlot = null;
            removeAll();
            addOptionLabel();
            
        }
    }
    /**
     * Modifies the LOFAR Plotter framework plot displayed in this slot.
     * @param constraints an Object that will be passed to the plotter framework update method with constraints.
     * @see nl.astron.lofar.java.gui.plotter.PlotPanel
     * @see nl.astron.lofar.java.gui.plotter.PlotConstants
     * @see nl.astron.lofar.java.gui.plotter.IPlotDataAccess
     */
    public void modifyPlot(Object constraints){
        if(containsPlot()){
            try {
                itsPlot.modifyPlot(constraints);
                validate();
                
            } catch (PlotterException ex) {
                JOptionPane.showMessageDialog(null, ex.getMessage(),
                        "Error detected while updating the plot",
                        JOptionPane.ERROR_MESSAGE);
                
                logger.error(ex);
            }
        }else{
            addPlot(constraints);
        }
    }
    /**
     * Removes the LOFAR Plotter framework plot contained in this slot
     */
    private void removePlot(){
        if(containsPlot()){
            this.removeAll();
            addOptionLabel();
        }
        itsPlot = null;
        hasLegend = false;
        offset=0.0;
    }
    /**
     * This helper method adds the Slot number header on top of this slot, and attaches a mouse listener.
     */
    private void addOptionLabel(){
        JPanel northPanel = new JPanel();
        northPanel.setLayout(new BorderLayout());
        northPanel.setToolTipText("Right-Click here to customize this slot");
        northPanel.setBackground(Color.LIGHT_GRAY);
        rightClickFacade.setForeground(Color.WHITE);
        northPanel.add(rightClickFacade,BorderLayout.CENTER);
        northPanel.addMouseListener(plotMouseListener);
        
        add(northPanel,BorderLayout.NORTH);
    }
    /**
     * Adds a legend for the LOFAR Plotter framework plot displayed in this slot.
     */
    public void addLegend(){
        if(itsPlot != null && itsPlot.getPlot() != null){
            if(!containsLegend()){
                try {
                    add(itsPlot.getLegendForPlot(),BorderLayout.SOUTH);
                    
                } catch (PlotterException ex) {
                    JTextArea error = new JTextArea(ex.getMessage());
                    error.setColumns(50);
                    add(new JTextArea(ex.getMessage()),BorderLayout.CENTER);
                    
                    logger.error(ex);
                }
                hasLegend = true;
            }else{
                removeLegend();
                addLegend();
            }
        }
    }
    
    /**
     * Removes the legend from the slot if present.
     */
    public void removeLegend(){
        if(containsLegend() && itsPlot != null && itsPlot.getPlot()!= null){
            removeAll();
            addOptionLabel();
            add(itsPlot,BorderLayout.CENTER);
            hasLegend = false;
        }
    }
    /**
     * Gets the legend for the LOFAR Plotter framework plot displayed in this slot.
     * @return the Legend for this PlotSlot's contained plot.
     */
    public JComponent getLegend(){
        JComponent legend = null;
        if(itsPlot != null && itsPlot.getPlot()!= null){
            try{
                legend = itsPlot.getLegendForPlot();
                hasLegend = true;
            } catch (PlotterException ex) {
                JTextArea error = new JTextArea(ex.getMessage());
                error.setColumns(50);
                legend = error;
                
                logger.error(ex);
            }
        }
        return legend;
    }
    /**
     * This method tells you if a plot is active in this slot.
     * @return true if the PlotSlot contains a plot
     * @return false if the PlotSlot does not contain a plot
     */
    public boolean containsPlot(){
        return itsPlot != null;
    }
    /**
     * This method tells you if a plot legend is active in this slot.
     * @return true if the PlotSlot contains a legend
     * @return false if the PlotSlot does not contain a legend
     */
    public boolean containsLegend(){
        return hasLegend;
    }
    /**
     * This method returns the active Plot Group linked to this slot.
     * @return the PlotGroup associated with this PlotSlot
     */
    public PlotGroup getPlotGroup(){
        return itsPlotGroup;
    }
    /**
     * This method sets the active Plot Group to be linked to this slot.
     * @param aPlotGroup the Plot Group to be linked to this slot.
     */
    public void setPlotGroup(PlotGroup aPlotGroup){
        itsPlotGroup = aPlotGroup;
    }
    /**
     * This method removes the link between the current Plot Group and this slot
     */
    public void removePlotGroup(){
        itsPlotGroup = null;
    }
    /**
     * This method will tell you if there is a link between a Plot Group and this slot
     */
    public boolean hasPlotGroup(){
        return itsPlotGroup != null;
    }
    /**
     * This method clears the slot of the plot, legend if present. The slot will then be available for new ones.
     */
    public void clearSlot(){
        removePlot();
        offset=0.0;
    }
     /**
     * This method tells you if the slot is empty (eg: no plot or legend).
     * @return true if the PlotSlot is empty
     * @return false if the PlotSlot is not empty
     */
    public boolean isEmpty(){
        return !containsPlot();
    }
    /**
     * This method will attempt to print the contents of the slot by calling the LOFAR Plotter framework's printing function.
     * @see nl.astron.lofar.java.gui.plotter.PlotPanel
     */
    public void printSlot(){
        try {
            itsPlot.printPlot(this.hasLegend);
        } catch (PlotterPrintException ex) {
            logger.error(ex);
        }
    }
    /**
     * This method returns the current offset amount (0.0 if none specified)
     * @return the offset in double format
     */
    public double getOffset(){
        return offset;
    }
    /**
     * This method sets the current offset amount (0.0 if you like to disable the offset)
     * @param offset The offset to be set in the plot.
     */
    public void setOffset(double offset){
        this.offset = offset;
    }
    /**
     * Registers PlotSlotListener to receive events in this slot.
     *
     * @param listener The listener to register.
     */
    public void addSlotListener(PlotSlotListener listener) {
        
        if (myListenerList == null ) {
            myListenerList = new LinkedList<PlotSlotListener>();
        }
        myListenerList.add(listener);
    }
    
    /**
     * Removes ActionListener from the list of listeners.
     *
     * @param listener The listener to remove.
     */
    public void removeSlotListener(PlotSlotListener listener) {
        
        myListenerList.remove(listener);
    }
    
    /**
     * Notifies all registered listeners about the event.
     *
     * @param event The event to be fired
     */
    private void fireContextMenuEvent(MouseEvent evt) {
        if (myListenerList != null) {
            if(SwingUtilities.isRightMouseButton(evt)){
                for(PlotSlotListener listener : myListenerList){
                    ((PlotSlotListener)listener).slotContextMenuTriggered(this,evt);
                }
            }
        }
    }
}
