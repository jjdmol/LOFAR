/*
 * PlotContainer.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

package nl.astron.lofar.mac.apl.gui.jrsp.panels;

import java.awt.BorderLayout;
import java.awt.Color;
import javax.swing.JPanel;
import nl.astron.lofar.java.gui.plotter.PlotConstants;
import nl.astron.lofar.java.gui.plotter.PlotPanel;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;
import nl.astron.lofar.mac.apl.gui.jrsp.PlotDataModel;

/**
 * This class is used to make the Plotter scale. It's actually a hack...
 *
 * When we would add itsPlotPanel directly to the SubbandStatsPlotPanel it will
 * not scale to desired (smaller) size. By adding it to this panel it adepts
 * to the borderlayout.
 *
 * @author balken
 */
public class PlotContainer extends JPanel {
    
    /** The plotter */
    private PlotPanel itsPlotPanel;
            
    /** Creates a new instance of PlotContainer */
    public PlotContainer() {
        setBackground(new Color(255, 255, 255)); // make background white        
        setLayout(new BorderLayout());  
        
        itsPlotPanel = new nl.astron.lofar.java.gui.plotter.PlotPanel();
        
        try {
            itsPlotPanel.createPlot(PlotConstants.PLOT_XYLINE, true, new double[0]);
        } catch (PlotterException ex) {
            ex.printStackTrace();
        }
        
        /*
         * In case code below doesn't work:
         * add(itsPlotPanel.getPlot(), BorderLayout.CENTER);
         */
        add(itsPlotPanel, BorderLayout.CENTER);
    }
    
    /**
     * Updates the plot by sending the /data/ to the plotDataModel and then
     * calling validate() to make the plot change/show.
     * @param   data    Subband statistics data to be shown in the plot.
     */
    public void updatePlot(double[] data) {                      
        try {
            itsPlotPanel.createPlot(PlotConstants.PLOT_XYLINE, true, data);            
        } catch (PlotterException ex) {
            ex.printStackTrace();
        }
        
        validate();
    }
}
