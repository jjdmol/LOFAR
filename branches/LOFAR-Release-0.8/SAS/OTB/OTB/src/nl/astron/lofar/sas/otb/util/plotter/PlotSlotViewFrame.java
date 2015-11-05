/*
 * PlotSlotViewFrame.java
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
import javax.swing.JDialog;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;

/**
 * This class represents a dialog with which you can display a PlotSlot or legend.
 *
 * @version $Id$
 * @created June 1, 2006, 2:56 PM
 * @author pompert
 */
public class PlotSlotViewFrame extends JDialog{
    
    /**
     *The index of the PlotSlot currently displayed
     */
    protected int plotIndex;
    
    /** 
     * Creates a new instance of PlotSlotViewFrame, with several mandatory parameters:<br><br>
     * @param parent The PlotSlotManager that handles the PlotSlot collection<br>
     * @param index The index of the PlotSlot this dialog has to display (or its legend)<br>
     * @param title The title to be displayed in the window bar<br>
     * @param showLegendOnly Should the dialog only show the legend (true), or the entire slot (false)<br><br>
     */
    public PlotSlotViewFrame(PlotSlotManager parent, int index, String title, boolean showLegendOnly) {
        super();
        plotIndex = index;
        this.getContentPane().setLayout(new BorderLayout());
        setTitle(title);
        //this.setAlwaysOnTop(true);
        PlotSlot viewSlot = parent.getSlot(index);
        if(!showLegendOnly){
            setTitle(title);
            this.getContentPane().add(viewSlot,BorderLayout.CENTER);
            pack();
        }else{
            setTitle(title);
            try {
                this.getContentPane().add(viewSlot.getPlot().getLegendForPlot(),BorderLayout.CENTER);
                pack();
            } catch (PlotterException ex) {
                ex.printStackTrace();
            }
        }
        
        this.setDefaultCloseOperation(PlotSlotViewFrame.DISPOSE_ON_CLOSE);
        
    }
}
