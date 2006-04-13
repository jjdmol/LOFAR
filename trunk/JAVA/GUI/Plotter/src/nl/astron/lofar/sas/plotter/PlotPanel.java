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
import javax.swing.JComponent;
import javax.swing.JPanel;

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

	public void createPlot(String constraint){
            currentDataConstraint = constraint;
            plot = m_PlotController.createPlot(constraint);
            this.add(plot);
        }

	public Image exportImage(){
		return null;
	}

	public void exportRoot(){

	}

	public void modifyDataSelection(){

	}

	public PrintJob printPlot(){
		return null;
	}

}