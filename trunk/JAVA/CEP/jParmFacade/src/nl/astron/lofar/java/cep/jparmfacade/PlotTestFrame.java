/*
 * PlotTestFrame.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter.test;

import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import nl.astron.lofar.sas.plotter.PlotConstants;
import nl.astron.lofar.sas.plotter.PlotPanel;

/**
 * @created 13-04-2006, 13:00
 * @author pompert
 * @version $Id$
 */
public class PlotTestFrame extends javax.swing.JFrame {
    
    private PlotPanel testPanel;
    
    /** Creates new form PlotTestFrame */
    public PlotTestFrame() {
        testPanel = new PlotPanel();
        PlotPanel testPanel2 = new PlotPanel();
        
        //For use with PlotDataAccessTestImpl
        //testPanel.createPlot(PlotConstants.PLOT_XYLINE,"line");
        //testPanel2.createPlot(PlotConstants.PLOT_GRID,"grid");
        
        //For use with PlotDataAccessParmDBImpl class
        testPanel.createPlot(PlotConstants.PLOT_XYLINE,"parm*");
        //testPanel2.createPlot(PlotConstants.PLOT_GRID,"parm2");
        
        this.setLayout(new GridBagLayout());
        
        this.getContentPane().add(testPanel.getPlot(),new GridBagConstraints(0,0,1,1,1,1,GridBagConstraints.CENTER,GridBagConstraints.BOTH,new Insets(0,0,0,0),1,1));
        this.getContentPane().add(testPanel.getLegendForPlot(),new GridBagConstraints(0,1,1,1,1,1,GridBagConstraints.CENTER,GridBagConstraints.BOTH,new Insets(0,0,0,0),1,1));
        //this.getContentPane().add(testPanel2.getPlot(),new GridBagConstraints(1,0,1,1,1,1,GridBagConstraints.CENTER,GridBagConstraints.BOTH,new Insets(0,0,0,0),1,1));
        //this.getContentPane().add(testPanel2.getLegendForPlot(),new GridBagConstraints(1,1,1,1,1,1,GridBagConstraints.CENTER,GridBagConstraints.BOTH,new Insets(0,0,0,0),1,1));
        
        
        this.setSize(new Dimension(640,480));
        initComponents();
             
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        pack();
    }// </editor-fold>//GEN-END:initComponents
    
    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
       PlotTestFrame af = new PlotTestFrame();
       af.setVisible(true);
    }
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    // End of variables declaration//GEN-END:variables
    
}
