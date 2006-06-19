/*
 * PlotSlotManager.java
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

import java.awt.event.ActionEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.LinkedList;
import javax.swing.JComponent;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.SwingUtilities;

/**
 * @version $Id$
 * @created May 24, 2006, 11:12 AM
 * @author pompert
 */
public class PlotSlotManager{
    
    public static final String REFRESH_FULL = "REFRESHFULL";
    public static final String REFRESH_SINGLE = "REFRESHSINGLE";
   
    private LinkedList<PlotSlot> itsPlotSlots;
    private PlotSlot selectedSlot;
    
    /** Creates a new instance of PlotSlotManager */
    public PlotSlotManager(int amountOfSlots) {
        itsPlotSlots = new LinkedList<PlotSlot>();
        setAmountOfSlots(amountOfSlots,true);
        selectedSlot = null;
    }
    public int getAmountOfSlots(){
        return itsPlotSlots.size();
    }
    public void setAmountOfSlots(int amount, boolean force) throws IllegalArgumentException{
        if(amount > 0){
            if(itsPlotSlots.size() < amount){
                int difference = amount - itsPlotSlots.size();
                int oldAmount = itsPlotSlots.size();
                for(int i = 1; i <= difference; i++){
                    PlotSlot aNewSlot = new PlotSlot();
                    aNewSlot.setLabel(""+(oldAmount+i));
                    itsPlotSlots.add(aNewSlot);
                }
                this.fireSlotsUpdated(-1); 
            }else if (itsPlotSlots.size() > amount){
                int difference = itsPlotSlots.size() - amount;
                boolean plotInTheWay = false;
                int plotsInTheWay = 0;
                for(int i = 1; i <= difference; i++){
                    PlotSlot checkPlot = getSlot(amount+i);
                    if(!checkPlot.isEmpty()){
                        plotInTheWay = true;
                        plotsInTheWay++;
                    }
                }
                if(plotInTheWay && !force){
                    String exceptionString = "There is/are "+plotsInTheWay+" plot(s) present in the ";
                    exceptionString+= "last "+difference+" slots.\n\nPlease clear or move ";
                    exceptionString+= "these plots manually by pressing cancel,\nor let ";
                    exceptionString+= "the application delete them by pressing Clear Slots.";
                    throw new IllegalArgumentException(exceptionString);
                }else{
                    for(int i = 1; i <= difference; i++){
                        itsPlotSlots.removeLast();
                    }
                    this.fireSlotsUpdated(-1); 
                }
            }
        }
    }
    public LinkedList<PlotSlot> getSlots(){
        return itsPlotSlots;
    }
    public PlotSlot getSlot(int index) throws IllegalArgumentException{
        if(index > 0 && index <= itsPlotSlots.size()){
            return itsPlotSlots.get(index-1);
        }else{
            throw new IllegalArgumentException("There is no PlotSlot in the list(size:"+getAmountOfSlots()+") at index "+index);
        }
    }
    
    public int getAnAvailableSlotIndex(){
        int availableSlot = 0;
        for(PlotSlot slot : itsPlotSlots){
            if(slot.isEmpty()){
                availableSlot = itsPlotSlots.indexOf(slot)+1;
            }
        }
        return availableSlot;
    }
    
    public int[] getAvailableSlotIndexes(){
        int[] availableSlots = null;
        availableSlots = new int[countAvailableSlots()];
        int index = 0;
        for(PlotSlot slot : itsPlotSlots){
            if(slot.isEmpty()){
                availableSlots[index] = itsPlotSlots.indexOf(slot)+1;
                index++;
            }
        }
        
        return availableSlots;
    }
    public int[] getOccupiedSlotIndexes(){
        int[] occupiedSlots = null;
        occupiedSlots = new int[countOccupiedSlots()];
        int index = 0;
        for(PlotSlot slot : itsPlotSlots){
            if(!slot.isEmpty()){
                occupiedSlots[index] = itsPlotSlots.indexOf(slot)+1;
                index++;
            }
        }
        
        return occupiedSlots;
    }
    
    public boolean areSlotsAvailable(){
        boolean slotsAvailable = false;
        for(PlotSlot slot : itsPlotSlots){
            if(slot.isEmpty()){
                slotsAvailable = true;
            }
        }
        return slotsAvailable;
    }
    public int countAvailableSlots(){
        int numberOfAvailableSlots = 0;
        for(PlotSlot slot : itsPlotSlots){
            if(slot.isEmpty()){
                numberOfAvailableSlots++;
            }
        }
        return numberOfAvailableSlots;
    }
    public int countOccupiedSlots(){
        int numberOfOccupiedSlots = 0;
        for(PlotSlot slot : itsPlotSlots){
            if(!slot.isEmpty()){
                numberOfOccupiedSlots++;
            }
        }
        return numberOfOccupiedSlots;
    }
    
    public void createPlotInSlot(int index, Object constraints){
        getSlot(index).addPlot(constraints);
        int currentPlots = 0;
        for(PlotSlot slot : itsPlotSlots){
            if(slot.containsPlot()) currentPlots++;
        }
        if(currentPlots > 1){
            fireSlotsUpdated(index);
        }else{
            fireSlotsUpdated(-1);
        }
    }
    public void createLegendInSlot(int index){
        getSlot(index).addLegend();
    }
    public void removeLegendInSlot(int index){
        getSlot(index).removeLegend();
    }
    public JComponent getLegendForSlot(int index){
        return getSlot(index).getLegend();
    }
    
    public void createPlotInAnyAvailableSlot(Object constraints){
        createPlotInSlot(getAnAvailableSlotIndex(),constraints);
    }
    
    public void modifyPlotInSlot(int index, Object constraints){
        getSlot(index).modifyPlot(constraints);
        int currentPlots = 0;
        for(PlotSlot slot : itsPlotSlots){
            if(slot.containsPlot()) currentPlots++;
        }
        if(currentPlots > 1){
            fireSlotsUpdated(index);
        }else{
            fireSlotsUpdated(-1);
        }
    }
    
    public void movePlot(int indexFromSlot, int indexToSlot){
        getSlot(indexToSlot).clearSlot();
        getSlot(indexToSlot).setPlot(getSlot(indexFromSlot).getPlot());
        if(getSlot(indexFromSlot).containsLegend()){
            getSlot(indexToSlot).addLegend();
        }
        getSlot(indexFromSlot).clearSlot();
        this.fireSlotsUpdated(indexFromSlot);
        this.fireSlotsUpdated(indexToSlot);
    }
    public void clearSlots(){
        for(int i = 1; i <= getAmountOfSlots(); i++){
            PlotSlot aSlot = getSlot(i);
            aSlot.clearSlot();
           
        }
        fireSlotsUpdated(-1);
    }
    
    public void clearSlot(int index){
        getSlot(index).clearSlot();
        this.fireSlotsUpdated(index);
    }
    
    public boolean isSlotAvailable(int index){
        return getSlot(index).isEmpty();
    }
    public boolean isSlotOccupied(int index){
        return !getSlot(index).isEmpty();
    }
    
    /**
     * Utility field used by event firing mechanism.
     */
    private javax.swing.event.EventListenerList listenerList =  null;
    
    /**
     * Registers ActionListener to receive events.
     *
     * @param listener The listener to register.
     */
    public void addActionListener(java.awt.event.ActionListener listener) {
        
        if (listenerList == null ) {
            listenerList = new javax.swing.event.EventListenerList();
        }
        listenerList.add(java.awt.event.ActionListener.class, listener);
    }
    
    /**
     * Removes ActionListener from the list of listeners.
     *
     * @param listener The listener to remove.
     */
    public void removeActionListener(java.awt.event.ActionListener listener) {
        
        listenerList.remove(java.awt.event.ActionListener.class, listener);
    }
    
    /**
     * Notifies all registered listeners about the event.
     *
     * @param event The event to be fired
     */
    private void fireSlotsUpdated(int id) {
        
        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList();
        ActionEvent action = null;
        if(id == -1){
            action = new ActionEvent(this,id,this.REFRESH_FULL);
        }else{
            action = new ActionEvent(this,id,this.REFRESH_SINGLE);
        }
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed(action);
            }
        }
    }
    
}

