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
import java.util.LinkedList;
import org.apache.log4j.Logger;

/**
 *
 * This manager class takes care of a collection of PlotSlot instances, <br>
 * being able to increase or decrease the size of the collection, and <br> also
 * being able to fire events to listeners if modifications took place inside <br>
 * the collection that would require some refresh action in a user interface.
 *
 * @version $Id$
 * @created May 24, 2006, 11:12 AM
 * @author pompert
 */
public class PlotSlotManager{
    
    public static final String REFRESH_FULL = "REFRESHFULL";
    public static final String REFRESH_SINGLE = "REFRESHSINGLE";
    
    private static Logger logger = Logger.getLogger(PlotSlotManager.class);
    
    private LinkedList<PlotSlot> itsPlotSlots;
    
    /**
     * Creates a new instance of PlotSlotManager and setting the limit of the collection <br>
     * to the value specified
     * @param amountOfSlots The amount of PlotSlots to be managed in the PlotSlot collection.
     */
    public PlotSlotManager(int amountOfSlots) {
        itsPlotSlots = new LinkedList<PlotSlot>();
        setAmountOfSlots(amountOfSlots,true);
    }
    /**
     * Gets the amount of PlotSlots in the PlotSlot collection being managed by this class
     * @return the amount of PlotSlots in the collection
     */
    public int getAmountOfSlots(){
        return itsPlotSlots.size();
    }
    /**
     * This method sets the amount of PlotSlots in the collection. <br><br>
     * As this collection may be filled with PlotSlots that are not empty, <br>
     * some logic has been built in to prevent you from deleting PlotSlots without your permission:
     * <br><br>
     * <li>If the amount specified is larger than currently managed by the PlotSlotManager,<br>
     * the new size is applied immediately without further consequences. The force argument is ignored.<br>
     * <li>If the amount specified is lower than currently managed by the PlotSlotManager, <br>
     * and the force argument is TRUE, the new size is applied immediately, <br>
     * regardless if PlotSlots are in the way (which is the case if their index(es)<br>
     * are greater than the new amount specified, and the PlotSlots' isEmpty() method returns false).<br>
     * <li>If the amount specified is lower than currently managed by the PlotSlotManager, <br>
     * and the force argument is FALSE, the new size is applied ONLY if there are no PlotSlots in the way, <br>
     * (which is the case if their index(es) are greater than the new amount specified, <br>
     * and the PlotSlots' isEmpty() method returns false).If a PlotSlot is in the way,<br>
     * an IllegalArgumentException is thrown containing a message that tells<br>
     * how many PlotSlots are in the way in the to be deleted PlotSlots.
     *
     * @param amount The new amount of slots to be managed in the PlotSlot collection.
     * @param force Force the amount on the collection, and delete any PlotSlot in the way.
     * @throws IllegalArgumentException will be thrown if one or more PlotSlots <br>
     * are in the way and the force argument is false.
     */
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
                    exceptionString+= "last "+difference+" slots. ";
                    
                    logger.info(exceptionString);
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
    /**
     * This method gets the PlotSlot at the index specified
     *
     * @param index The index of the slot in the PlotSlot collection.
     * @throws IllegalArgumentException will be thrown if the index specified does not contain a PlotSlot.
     */
    public PlotSlot getSlot(int index) throws IllegalArgumentException{
        if(index > 0 && index <= itsPlotSlots.size()){
            return itsPlotSlots.get(index-1);
        }else{
            logger.error("There is no PlotSlot in the list(size:"+getAmountOfSlots()+") at index "+index);
            throw new IllegalArgumentException("There is no PlotSlot in the list(size:"+getAmountOfSlots()+") at index "+index);
        }
    }
    /**
     * This method gets the first index in the collection where an empty PlotSlot is present.
     * @return the index of the first available slot.
     */
    public int getAnAvailableSlotIndex(){
        int availableSlot = 0;
        for(PlotSlot slot : itsPlotSlots){
            if(slot.isEmpty() && availableSlot == 0){
                availableSlot = itsPlotSlots.indexOf(slot)+1;
            }
        }
        return availableSlot;
    }
    /**
     * This method gets all the indexes in the collection where an empty PlotSlot is present.
     * @return the indexes of the available slots.
     */
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
    /**
     * This method gets all the indexes in the collection where an occupied/filled PlotSlot is present.
     * @return the indexes of the occupied/filled/non-empty slots.
     */
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
    /**
     * This method answers if the PlotSlot collection contains available PlotSlot(s).
     * @return true if there is/are PlotSlot(s) available
     * @return false if there are no PlotSlots available
     */
    public boolean areSlotsAvailable(){
        boolean slotsAvailable = false;
        for(PlotSlot slot : itsPlotSlots){
            if(slot.isEmpty()){
                slotsAvailable = true;
            }
        }
        return slotsAvailable;
    }
    /**
     * This method gets the amount of PlotSlots in the collection which are available/not filled.
     * @return the amount of available slots.
     */
    public int countAvailableSlots(){
        int numberOfAvailableSlots = 0;
        for(PlotSlot slot : itsPlotSlots){
            if(slot.isEmpty()){
                numberOfAvailableSlots++;
            }
        }
        return numberOfAvailableSlots;
    }
    /**
     * This method gets the amount of PlotSlots in the collection which are occupied/filled.
     * @return the amount of occupied/filled/not empty slots.
     */
    public int countOccupiedSlots(){
        int numberOfOccupiedSlots = 0;
        for(PlotSlot slot : itsPlotSlots){
            if(!slot.isEmpty()){
                numberOfOccupiedSlots++;
            }
        }
        return numberOfOccupiedSlots;
    }
    /**
     * This method answers if the PlotSlot specified is available/empty.
     * @param index the PlotSlot index to check
     * @return true if the PlotSlot is available
     * @return false if the PlotSlot is not available
     */
    public boolean isSlotAvailable(int index){
        return getSlot(index).isEmpty();
    }
    /**
     * This method answers if the PlotSlot specified is occupied/not empty.
     * @param index the PlotSlot index to check
     * @return true if the PlotSlot is occupied
     * @return false if the PlotSlot is not occupied
     */
    public boolean isSlotOccupied(int index){
        return !getSlot(index).isEmpty();
    }
    
    /**
     * This method will create a plot in the PlotSlot specified using the constraints.<br>
     * It will trigger a fireSlotsUpdated event for the PlotSlot specified.
     * @param index the PlotSlot index in the collection
     * @param constraints the constraints for the LOFAR Plotter framework to build the plot.
     */
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
    /**
     * This method will create a plot in the first available PlotSlot using the constraints.
     * @param constraints the constraints for the LOFAR Plotter framework to build the plot.
     */
    public void createPlotInAnyAvailableSlot(Object constraints){
        createPlotInSlot(getAnAvailableSlotIndex(),constraints);
    }
    /**
     * This method will modify the plot in the PlotSlot specified using the constraints.
     * @param index the PlotSlot index in the collection
     * @param constraints the constraints for the LOFAR Plotter framework to update the plot.
     */
    public void modifyPlotInSlot(int index, Object constraints){
        getSlot(index).modifyPlot(constraints);
    }
    /**
     * This method will move the plot+legend in the PlotSlot specified to another PlotSlot specified.
     * <br>
     * It will trigger a fireSlotsUpdated event for BOTH the PlotSlots specified.
     *
     * @param indexFromSlot the PlotSlot index in the collection which contains the plot to be moved.
     * @param indexToSlot the PlotSlot index in the collection which should contain the plot moved.
     */
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
    /**
     * This method will clear all PlotSlots in the collection. <br>
     * It removes every plot being managed!<br>
     * It will trigger a fireSlotsUpdated event that will mean a full refresh<br>
     * of the visualisation of the collection of PlotSlots.
     */
    public void clearSlots(){
        for(int i = 1; i <= getAmountOfSlots(); i++){
            PlotSlot aSlot = getSlot(i);
            aSlot.clearSlot();
            
        }
        fireSlotsUpdated(-1);
    }
    /**
     * This method will clear the PlotSlot specified<br>
     * It removes the plot inside this PlotSlot if present!<br>
     * It will trigger a fireSlotsUpdated for the PlotSlot specified.
     * @param index the index of the PlotSlot to clear.
     */
    public void clearSlot(int index){
        getSlot(index).clearSlot();
        this.fireSlotsUpdated(index);
    }
    
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
     * Notifies all registered listeners about an update on (a) slot(s) event(s).
     *
     * @param id The event to be fired (REFRESH_FULL if a full refresh is probably needed, <br>
     * or REFRESH_SINGLE if a specific slot has to be updated.
     * 
     */
    private void fireSlotsUpdated(int id) {
        
        if (myListenerList == null) return;
        Object[] listeners = myListenerList.getListenerList();
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

