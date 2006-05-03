/*
 * PlotPanel.java
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

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Image;
import java.awt.PrintJob;
import javax.swing.JComponent;
import javax.swing.JPanel;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;

/**
 * @created 11-04-2006, 15:00
 * @author pompert
 * @version $Id$
 * @updated 13-apr-2006 11:19:47
 */
public class PlotPanel extends JPanel{

	private PlotController m_PlotController;
        private JComponent plot;
        private JComponent legend;
        private String[] currentDataConstraint;

	public PlotPanel(){
            m_PlotController = new PlotController();
            this.setBackground(Color.WHITE);
            this.setLayout(new BorderLayout());
        }

	public void finalize() throws Throwable {
            plot = null;
            legend = null;
            m_PlotController = null;
            currentDataConstraint = null;
	}

	public void createPlot(int type, String[] constraints) throws PlotterException{
            this.removeAll();
            plot = null;
            legend = null;
            currentDataConstraint = constraints;
            
            plot = m_PlotController.createPlot(type,constraints);
            this.add(plot,BorderLayout.CENTER);
        }
        
	public Image exportImage(){
		return null;
	}

	public void exportData(){

	}

	public void modifyDataSelection(){

	}

	public PrintJob printPlot(){
            
            return null;
	}
        
        public JComponent getPlot(){
            return plot;
        }
        public JComponent getLegendForPlot() throws PlotterException{
            if(legend == null && plot != null){
               legend = m_PlotController.getLegendForPlot(plot);
            }
            return legend;
            
        }
        
 }
