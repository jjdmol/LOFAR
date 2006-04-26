/*
 * PlotTestFrame.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O. Box 2, 7990AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 */

package nl.astron.lofar.sas.plotter.test;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JLabel;
import nl.astron.lofar.sas.plotter.PlotConstants;
import nl.astron.lofar.sas.plotter.PlotPanel;
import nl.astron.lofar.sas.plotter.exceptions.PlotterException;

/**
 * @created 13-04-2006, 13:00
 * @author pompert
 * @version $Id$
 */
public class PlotTestFrame extends javax.swing.JFrame {
    
    private PlotPanel testPanel;
    private JLabel exceptionLabel;
    private boolean plotPresent;
    
    /** Creates new form PlotTestFrame */
    public PlotTestFrame() {
        testPanel = new PlotPanel();
        plotPresent = false;
        //For use with PlotDataAccessTestImpl
        //testPanel.createPlot(PlotConstants.PLOT_XYLINE,"line");
        //testPanel2.createPlot(PlotConstants.PLOT_GRID,"grid");
        
        //For use with PlotDataAccessParmDBImpl class
        //testPanel.createPlot(PlotConstants.PLOT_XYLINE,"parm*");
        //testPanel2.createPlot(PlotConstants.PLOT_GRID,"parm2");
        
         
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
        java.awt.GridBagConstraints gridBagConstraints;

        plotModPanel = new javax.swing.JPanel();
        lparameterConstraint = new javax.swing.JLabel();
        tparameterConstraint = new javax.swing.JTextField();
        lstartX = new javax.swing.JLabel();
        tstartX = new javax.swing.JTextField();
        lendX = new javax.swing.JLabel();
        tendX = new javax.swing.JTextField();
        lnumX = new javax.swing.JLabel();
        tnumX = new javax.swing.JTextField();
        lstartY = new javax.swing.JLabel();
        tstartY = new javax.swing.JTextField();
        lendY = new javax.swing.JLabel();
        tendY = new javax.swing.JTextField();
        lnumY = new javax.swing.JLabel();
        tnumY = new javax.swing.JTextField();
        cLegend = new javax.swing.JCheckBox();
        jSeparator1 = new javax.swing.JSeparator();
        bplotButton = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setTitle("LOFAR | JAVA Plotter Test Application");
        setIconImage(getIconImage());
        plotModPanel.setLayout(new java.awt.GridBagLayout());

        lparameterConstraint.setText("Parameter");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridwidth = 15;
        plotModPanel.add(lparameterConstraint, gridBagConstraints);

        tparameterConstraint.setColumns(10);
        tparameterConstraint.setText("parm*");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridwidth = 15;
        plotModPanel.add(tparameterConstraint, gridBagConstraints);

        lstartX.setText("startx");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridwidth = 15;
        plotModPanel.add(lstartX, gridBagConstraints);

        tstartX.setColumns(3);
        tstartX.setText("0");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridwidth = 15;
        plotModPanel.add(tstartX, gridBagConstraints);

        lendX.setText("endx");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridwidth = 15;
        plotModPanel.add(lendX, gridBagConstraints);

        tendX.setColumns(3);
        tendX.setText("5");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridwidth = 15;
        plotModPanel.add(tendX, gridBagConstraints);

        lnumX.setText("numx");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridwidth = 15;
        plotModPanel.add(lnumX, gridBagConstraints);

        tnumX.setColumns(3);
        tnumX.setText("5");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridwidth = 15;
        plotModPanel.add(tnumX, gridBagConstraints);

        lstartY.setText("starty");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridwidth = 15;
        plotModPanel.add(lstartY, gridBagConstraints);

        tstartY.setColumns(3);
        tstartY.setText("0");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridwidth = 15;
        plotModPanel.add(tstartY, gridBagConstraints);

        lendY.setText("endy");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridwidth = 15;
        plotModPanel.add(lendY, gridBagConstraints);

        tendY.setColumns(3);
        tendY.setText("5");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridwidth = 15;
        plotModPanel.add(tendY, gridBagConstraints);

        lnumY.setText("numy");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridwidth = 15;
        plotModPanel.add(lnumY, gridBagConstraints);

        tnumY.setColumns(3);
        tnumY.setText("5");
        gridBagConstraints = new java.awt.GridBagConstraints();
        gridBagConstraints.gridwidth = 15;
        plotModPanel.add(tnumY, gridBagConstraints);

        cLegend.setText("Legend");
        cLegend.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        cLegend.setHorizontalTextPosition(javax.swing.SwingConstants.LEADING);
        cLegend.setMargin(new java.awt.Insets(0, 0, 0, 0));
        plotModPanel.add(cLegend, new java.awt.GridBagConstraints());

        jSeparator1.setOrientation(javax.swing.SwingConstants.VERTICAL);
        jSeparator1.setMinimumSize(new java.awt.Dimension(50, 10));
        plotModPanel.add(jSeparator1, new java.awt.GridBagConstraints());

        bplotButton.setText("Plot");
        bplotButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                handlePlotButton(evt);
            }
        });

        plotModPanel.add(bplotButton, new java.awt.GridBagConstraints());

        getContentPane().add(plotModPanel, java.awt.BorderLayout.NORTH);

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void handlePlotButton(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_handlePlotButton
        JButton source = (JButton)evt.getSource();
        if(source == bplotButton){
            if(checkBoxes() == true){
                
                String[] argsForController = new String[7];
                argsForController[0] = tparameterConstraint.getText();
                argsForController[1] = tstartX.getText();
                argsForController[2] = tendX.getText();
                argsForController[3] = tnumX.getText();
                argsForController[4] = tstartY.getText();
                argsForController[5] = tendY.getText();
                argsForController[6] = tnumY.getText();
               
               
                //plotPane.add(testPanel.getPlot(),new GridBagConstraints(0,0,1,1,1,1,GridBagConstraints.CENTER,GridBagConstraints.BOTH,new Insets(0,0,0,0),1,1));
                //plotPane.add(testPanel.getLegendForPlot(),new GridBagConstraints(0,1,1,1,1,1,GridBagConstraints.CENTER,GridBagConstraints.BOTH,new Insets(0,0,0,0),1,1));
                if(plotPresent){
                    this.getContentPane().remove(testPanel.getPlot());
                    try {
                        this.getContentPane().remove(testPanel.getLegendForPlot());
                    } catch (Exception ex) {
                        ex.printStackTrace();
                    }
                }
                try {
                    if(exceptionLabel !=null){
                        this.getContentPane().remove(exceptionLabel);
                        exceptionLabel = null;
                    }
                    testPanel.createPlot(PlotConstants.PLOT_XYLINE,argsForController);
                    this.add(testPanel.getPlot(),BorderLayout.CENTER);
                    if(cLegend.isSelected()){
                        this.add(testPanel.getLegendForPlot(),BorderLayout.SOUTH); 
                    }
                   
                    plotPresent = true;
                    
                } catch (PlotterException ex) {
                    exceptionLabel = new JLabel(ex.getMessage());
                    this.add(exceptionLabel,BorderLayout.CENTER);
                    plotPresent = false;
                }
                
                this.pack();
                
            }
           }
    }//GEN-LAST:event_handlePlotButton
    
    /**
     * Checks if all input boxes are numerical
     * @return boolean Result of input validation
     */
    private boolean checkBoxes(){
        boolean numbersOK = true;            
        double startx = 0;
        double endx = 0;
        int numx = 0;
        double starty = 0;
        double endy = 0;
        int numy = 0;
        try {
            startx = Double.parseDouble(tstartX.getText());
            tstartX.setForeground(Color.BLACK);
        } catch (NumberFormatException ex) {
            tstartX.setForeground(Color.RED);
            numbersOK = false;
        }
        try {
            endx = Double.parseDouble(tendX.getText());
            tendX.setForeground(Color.BLACK);
        } catch (NumberFormatException ex) {
            tendX.setForeground(Color.RED);
            numbersOK = false;
        }
        try {
            numx = Integer.parseInt(tnumX.getText());
            tnumX.setForeground(Color.BLACK);
        } catch (NumberFormatException ex) {
            tnumX.setForeground(Color.RED);
            numbersOK = false;
        }
        try {
            starty = Double.parseDouble(tstartY.getText());
            tstartY.setForeground(Color.BLACK);
        } catch (NumberFormatException ex) {
            tstartY.setForeground(Color.RED);
            numbersOK = false;
        }
        try {
            endy = Double.parseDouble(tendY.getText());  
            tendY.setForeground(Color.BLACK);
        } catch (NumberFormatException ex) {
            tendY.setForeground(Color.RED);
            numbersOK = false;
        }
        try {
            numy = Integer.parseInt(tnumY.getText()); 
            tnumY.setForeground(Color.BLACK);
        } catch (NumberFormatException ex) {
            tnumY.setForeground(Color.RED);
            numbersOK = false;
        }
        return numbersOK;
    }
    
    
    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
       PlotTestFrame af = new PlotTestFrame();
       af.setVisible(true);
    }
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton bplotButton;
    private javax.swing.JCheckBox cLegend;
    private javax.swing.JSeparator jSeparator1;
    private javax.swing.JLabel lendX;
    private javax.swing.JLabel lendY;
    private javax.swing.JLabel lnumX;
    private javax.swing.JLabel lnumY;
    private javax.swing.JLabel lparameterConstraint;
    private javax.swing.JLabel lstartX;
    private javax.swing.JLabel lstartY;
    private javax.swing.JPanel plotModPanel;
    private javax.swing.JTextField tendX;
    private javax.swing.JTextField tendY;
    private javax.swing.JTextField tnumX;
    private javax.swing.JTextField tnumY;
    private javax.swing.JTextField tparameterConstraint;
    private javax.swing.JTextField tstartX;
    private javax.swing.JTextField tstartY;
    // End of variables declaration//GEN-END:variables
    
}
