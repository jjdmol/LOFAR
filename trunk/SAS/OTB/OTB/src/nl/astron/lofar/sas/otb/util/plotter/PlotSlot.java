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
import java.awt.Font;
import javax.swing.JComponent;
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
    private boolean hasLegend;
    private JLabel rightClickFacade;
    
    /** Creates a new instance of PlotSlot */
    public PlotSlot() {
        itsPlotGroup = null;
        itsPlot = null;
        hasLegend = false;
        setBackground(Color.WHITE);
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
    }
    
    public void setLabel(String label){
        slotLabel = label;
        rightClickFacade.setText(" "+label);
    }
    
    public String getLabel(){
        return slotLabel;
    }
    public PlotPanel getPlot(){
        return itsPlot;
    }
    public void setPlot(PlotPanel aPlot){
         removeAll();
         itsPlot = aPlot;
         addOptionLabel();
         add(itsPlot,BorderLayout.CENTER);
         if(containsLegend()){
            addLegend(); 
         }
    }
    
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
            JTextArea error = new JTextArea(ex.getMessage());
            error.setColumns(50);
            add(new JTextArea(ex.getMessage()),BorderLayout.CENTER);
            ex.printStackTrace();
        }
    }
    
    public void modifyPlot(Object constraints){
        if(containsPlot()){
            removeAll();
            addOptionLabel();
            try {
                itsPlot.modifyPlot(constraints);
                add(itsPlot,BorderLayout.CENTER);
                if(containsLegend()){
                    addLegend(); 
                }
                
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
            this.removeAll();
            addOptionLabel();
        }
        itsPlot = null;
        hasLegend = false;
    }
    
    private void addOptionLabel(){
        JPanel northPanel = new JPanel();
        northPanel.setLayout(new BorderLayout());
        northPanel.setToolTipText("Right-Click here to customize this slot");
        northPanel.setBackground(Color.LIGHT_GRAY);
        rightClickFacade.setForeground(Color.WHITE);
        northPanel.add(rightClickFacade,BorderLayout.CENTER);
        add(northPanel,BorderLayout.NORTH);
    }
    
    public void addLegend(){
        if(itsPlot != null && itsPlot.getPlot() != null){
            if(!containsLegend()){
                try {
                    add(itsPlot.getLegendForPlot(),BorderLayout.SOUTH);
                    
                } catch (PlotterException ex) {
                    JTextArea error = new JTextArea(ex.getMessage());
                    error.setColumns(50);
                    add(new JTextArea(ex.getMessage()),BorderLayout.CENTER);
                    ex.printStackTrace();
                }
                hasLegend = true;
            }else{
                removeLegend();
                addLegend();
            }
        }
    }
    public void removeLegend(){
        if(containsLegend() && itsPlot != null && itsPlot.getPlot()!= null){
            removeAll();
            if(containsPlot()) addOptionLabel();
            add(itsPlot,BorderLayout.CENTER);
            hasLegend = false;
        }
    }
    public JComponent getLegend(){
        JComponent legend = null;
        if(containsLegend() && itsPlot != null && itsPlot.getPlot()!= null){
            try{
                legend = itsPlot.getLegendForPlot();
            } catch (PlotterException ex) {
                JTextArea error = new JTextArea(ex.getMessage());
                error.setColumns(50);
                legend = error;
                ex.printStackTrace();
            }
        }
        return legend;
    }
    
    public boolean containsPlot(){
        return itsPlot != null;
    }
    
    public boolean containsLegend(){
        return hasLegend;
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
        removePlot();
    }
    public boolean isEmpty(){
        return !containsPlot();
    }
    
}
