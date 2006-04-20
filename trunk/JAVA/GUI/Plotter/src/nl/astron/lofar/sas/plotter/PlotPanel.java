/*
 * PlotPanel.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter;

import java.awt.Image;
import java.awt.PrintJob;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import javax.swing.JComponent;
import javax.swing.JLabel;
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
        private String currentDataConstraint;

	public PlotPanel(){
            m_PlotController = new PlotController();
        }

	public void finalize() throws Throwable {

	}

	public void createPlot(int type, String constraint){
            currentDataConstraint = constraint;
            try {
                
                plot = m_PlotController.createPlot(type,constraint);
                
                add(plot);
            } catch (PlotterException ex) {
                plot = new JPanel();
                plot.add(new JLabel(ex.getMessage()));
                add(plot);
                //ex.printStackTrace();
            }
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
        public JComponent getLegendForPlot(){
            JComponent legend = null;
            try {
                legend = m_PlotController.getLegendForPlot(plot);
            } catch (PlotterException ex) {
                legend = new JLabel(ex.getMessage());
            }
            return legend;
        }
        
 }