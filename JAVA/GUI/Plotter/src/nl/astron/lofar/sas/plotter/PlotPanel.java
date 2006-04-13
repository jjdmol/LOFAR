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

/**
 * @version $Id$
 * @created 11-04-2006, 15:00
 * @author pompert
 */
public class PlotPanel {

	public PlotController m_PlotController;

	public PlotPanel(){

	}

	public void finalize() throws Throwable {

	}

	public JComponent createPlot(String constraint){
		return null;
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