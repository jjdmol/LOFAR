/*
 * PlotPanel.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Image;
import java.awt.PrintJob;
import javax.swing.JComponent;
import javax.swing.JPanel;
import nl.astron.lofar.sas.plotter.exceptions.PlotterException;

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
