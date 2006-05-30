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

import java.util.LinkedList;
import nl.astron.lofar.java.gui.plotter.PlotPanel;

/**
 * @version $Id$
 * @created May 24, 2006, 11:12 AM
 * @author pompert
 */
public class PlotSlotManager{
    
    private LinkedList<PlotSlot> itsPlotSlots;
    
    /** Creates a new instance of PlotSlotManager */
    public PlotSlotManager(int amountOfSlots) {
        itsPlotSlots = new LinkedList<PlotSlot>();
        for(int i = 1;i<=amountOfSlots;i++){
            itsPlotSlots.add(new PlotSlot());
        }
    }
    public int getAmountOfSlots(){
        return itsPlotSlots.size();
    }
    public void setAmountOfSlots(int amount, boolean force) throws IllegalArgumentException{
        if(itsPlotSlots.size() < amount){
            int difference = amount - itsPlotSlots.size();
            for(int i = 1; i <= difference; i++){
                itsPlotSlots.add(new PlotSlot());
            }
        }else if (itsPlotSlots.size() > amount){
            int difference = itsPlotSlots.size() - amount;
            boolean plotInTheWay = false;
            for(int i = 1; i <= difference; i++){
                PlotSlot checkPlot = getSlot(amount+i);
                if(!checkPlot.getLabel().equalsIgnoreCase(PlotSlot.EMPTY_SLOT)){
                    plotInTheWay = true;
                }
            }
            if(plotInTheWay && !force){
                String exceptionString = "There is/are plot(s) present in the ";
                exceptionString+= "last "+difference+" slots. Please clear ";
                exceptionString+= "these slots manually or use the force argument to ";
                exceptionString+= "let the application delete them";
                throw new IllegalArgumentException(exceptionString);
            }else{
                for(int i = 1; i <= difference; i++){
                    itsPlotSlots.removeLast();
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
    
    public void createPlotInSlot(int index, Object constraints){
        getSlot(index).addPlot(constraints);
    }
    
    public void createPlotInAnyAvailableSlot(Object constraints){
        getSlot(getAnAvailableSlotIndex()).addPlot(constraints);
    }
    
    public void modifyPlotInSlot(int index, Object constraints){
        getSlot(index).modifyPlot(constraints);
    }
    
    public void clearSlot(int index){
        getSlot(index).clearSlot();
    }
    
    public boolean isSlotAvailable(int index){
        return getSlot(index).isEmpty();
    }
    
    
}
