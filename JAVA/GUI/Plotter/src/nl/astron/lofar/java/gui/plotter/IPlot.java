/*
 * IPlot.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl

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

package nl.astron.lofar.java.gui.plotter;

import java.util.HashMap;
import javax.swing.JComponent;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;

/**
 * @created 11-04-2006, 15:00
 * @author pompert
 * @version $Id$
 * @updated 13-apr-2006 11:19:45
 */
public interface IPlot{

	/**
         * Creates a plot using several key arguments
	 * @param type Type of plot as dictated by PlotConstants.PLOT_*
	 * @param name Name to be given to the plot
	 * @param data The dataset to be used to create the plot
	 * @param separateLegend Indicates the user's need for a separate legend 
	 */
	public JComponent createPlot(int type, String name, HashMap data, boolean separateLegend) throws PlotterException;
        
        /**
	 * Returns the current dataset used in the plot 
	 */
        public HashMap getData();
        /**
         * Sets the dataset used in the plot 
         * @param newData A new set of data
	 */
        public void setData(HashMap newData);
        /**
         * Create a legend/key using the plot specified
         * @param aPlot A plot JComponent
	 */
        public JComponent getLegend(JComponent aPlot) throws PlotterException;
}

