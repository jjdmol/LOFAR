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
            //addComponentListener(resizeListener);
        }

	public void finalize() throws Throwable {

	}

	public void createPlot(String constraint){
            currentDataConstraint = constraint;
            try {
                plot = m_PlotController.createPlot(constraint);
                
                add(plot);
            } catch (PlotterException ex) {
                add(new JLabel(ex.getMessage()));
                //ex.printStackTrace();
            }
        }
        
        private ComponentListener resizeListener = new ComponentListener(){
            public void componentHidden(ComponentEvent e){}
            public void componentMoved(ComponentEvent e){}
            public void componentResized(ComponentEvent e){
                int height = getHeight();
                int width = getWidth();
                int height2 = plot.getHeight();
                int width2 = plot.getWidth();
                System.out.println("Width/Height of PlotPanel: "+width+"/"+height);
                plot.setSize(width,height);
                //plot.validate();
                System.out.println("Width/Height of PlotLayout: "+width2+"/"+height2);
                //plot.firePropertyChange("Height",height,height2);
                //plot.firePropertyChange("Width",width,width2);
                
            }
            public void componentShown(ComponentEvent e){}
        };

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
        
        public JComponent getPlot(){
            return plot;
        }
        
 }