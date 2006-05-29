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
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import nl.astron.lofar.java.gui.plotter.PlotConstants;
import nl.astron.lofar.java.gui.plotter.PlotPanel;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;

/**
 * @version $Id$
 * @created May 24, 2006, 11:12 AM
 * @author pompert
 */
public class PlotSlot extends JPanel{
    
    public static final String EMPTY_SLOT = "Empty Slot";
    private PlotGroup itsPlotGroup;
    private PlotPanel itsPlot;
    private String slotLabel;
    
    /** Creates a new instance of PlotSlot */
    public PlotSlot() {
        itsPlotGroup = null;
        itsPlot = new PlotPanel();
        setLabel(EMPTY_SLOT);
        setPreferredSize(new Dimension(320,240));
        setBackground(Color.WHITE);
        
        setLayout(new BorderLayout());
        
    }
    /** 
     * Creates a new instance of PlotSlot using a supplied label
     * @param name The name to be given to the slot.
     * 
     */
    public PlotSlot(String label) {
        this();
        setLabel(label);
       
    }
    
    public void setLabel(String label){
        slotLabel = label;
    }
    
    public String getLabel(){
        return slotLabel;
    }
    
    public PlotPanel getPlot(){
        return itsPlot;
    }
    
    public void setPlot(PlotPanel aPlot){
        itsPlot = aPlot;
    }
    
    public void addPlot(Object constraints){
        removeAll();
        try {            
            itsPlot.createPlot(PlotConstants.PLOT_XYLINE,true,constraints);
            add(itsPlot,BorderLayout.CENTER);
        } catch (PlotterException ex) {
            JTextArea error = new JTextArea(ex.getMessage());
            error.setColumns(50);
            add(new JTextArea(ex.getMessage()),BorderLayout.CENTER);
            ex.printStackTrace();
        }
    }
    
    public void modifyPlot(Object constraints){
        if(containsPlot()){
         
            try {
                itsPlot.modifyPlot(constraints);
                add(itsPlot,BorderLayout.CENTER);
            } catch (PlotterException ex) {
                JTextArea error = new JTextArea(ex.getMessage());
                error.setColumns(50);
                add(new JTextArea(ex.getMessage()),BorderLayout.CENTER);
                ex.printStackTrace();
            }

        }else{
            addPlot(constraints);
        }
    }
    
    private void removePlot(){
        if(containsPlot()){
            this.remove(itsPlot);
        }
        itsPlot = null;
    }
    
    private boolean containsPlot(){
        return itsPlot != null;
    }
    
    public PlotGroup getPlotGroup(){
       return itsPlotGroup; 
    }
    
    public void setPlotGroup(PlotGroup aPlotGroup){
        itsPlotGroup = aPlotGroup;
    }
    
    public void removePlotGroup(){
        itsPlotGroup = null;
    }
    
    public boolean hasPlotGroup(){
        return itsPlotGroup != null;
    }
    public void clearSlot(){
        setLabel(EMPTY_SLOT);
        removePlot();
    }
    public boolean isEmpty(){
        return containsPlot();
    }
   
}
