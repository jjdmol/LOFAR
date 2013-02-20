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
 * This interface forms the communications contract between the plotter classes
 * and any possible plotter graphics framework using a class that implements
 * this interface.
 *
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
         * @return the JComponent plot generated
         * @throws PlotterException will be thrown if the plot could not be generated for any reason.
	 */
	public JComponent createPlot(int type, String name, HashMap<String,Object> data, boolean separateLegend) throws PlotterException;
        
        /**
         * Modifies a given plot using a given dataset.
	 * @param aPlot A plot JComponent
         * @param data The data to be displayed in the plot.
         * @return A legend JComponent of plot aPlot
         * @return the JComponent plot with the new dataset embedded.
         * @throws PlotterException will be thrown if the plot could not be generated for any reason.
	 */
	public JComponent modifyPlot(JComponent aPlot, HashMap<String,Object> data) throws PlotterException;
        
        /**
	 * Returns the current dataset used in the plot
         * @return the dataset currently in use.
	 */
        public HashMap getData();
        /**
         * Sets the dataset used in the plot 
         * @param newData A new set of data
	 */
        public void setData(HashMap<String,Object> newData);
        /**
         * Create a legend/key using the plot specified.
         * @param aPlot A plot JComponent
         * @return A legend JComponent of plot aPlot
         * @throws PlotterException will be thrown if the legend could not be generated for the given JComponent.
	 */
        public JComponent getLegend(JComponent aPlot) throws PlotterException;
}

