/*
 * SubbandStatsPanel.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

package nl.astron.lofar.java.gui.mac.jrsp.subbandstats;

import com.sun.org.apache.xerces.internal.parsers.JAXPConfiguration;
import java.awt.event.ActionEvent;
import java.util.ArrayList;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import nl.astron.lofar.java.mac.jrsp.RCUMask;
import nl.astron.lofar.java.mac.jrsp.WGRegisterType;
import nl.astron.lofar.java.gui.mac.jrsp.ITabPanel;
import nl.astron.lofar.java.gui.mac.jrsp.MainPanel;
import org.apache.log4j.Logger;

public class SubbandStatsPanel extends JPanel implements ITabPanel 
{
    private MainPanel mainPanel;
        
    /** Logger */
    private Logger itsLogger;
    
    /**
     * Default Constructor.
     */
    public SubbandStatsPanel()
    {
        itsLogger = Logger.getLogger(getClass());
        itsLogger.info("Constructor");
        
        initComponents();       
        
        /*
         * ActionListeners WaveformSettings.
         */
        settingsPanel.getInputPanel().addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                inputPanelActionPerformed(evt);                   
            }                
        });
        
        settingsPanel.getListPanel().addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                listPanelActionPerformed(evt);
            }
        });
        
        /*
         * Disables all the user input because there is no need to push a button
         * or select a filter when there is no mask.
         */
        enablePanel(false);
    }

    /**
     * Used to initialize the ITabPanel and give it a refrence to the main panel.
     * 
     * @param mainPanel   The MainPanel.
     */
    public void init(MainPanel mainPanel)
    {
        this.mainPanel = mainPanel;
    }
    
    /**
     * Method that can be called by the main panel to update this panel.
     */
    public void update(int updateType)
    {
        /*
         * Board should be connected.
         */
        if (!mainPanel.getBoard().isConnected())
        {
            enablePanel(false); // mainPanel.getBoard.isConnected() == false
            return;
        }
        
        /*
         * There are no breaks in this switch. With this construction 
         * ITabPanel.REFRESH_UPDATE is also processed when 
         * ITabPanel.REQUIRED_UPDATE is "called".
         */
        switch(updateType) {
            case ITabPanel.REQUIRED_UPDATE:
                enablePanel(true); // mainPanel.getBoard.isConnected() == true
                updateSettingsPanel();
                plotLeft.updateBoardSelectionBox( mainPanel.getBoard().getNrRSPBoards() );
                plotRight.updateBoardSelectionBox( mainPanel.getBoard().getNrRSPBoards() );
            case ITabPanel.REFRESH_UPDATE:
                updatePlotLeft();                
                updatePlotRight();                
        }       
    }

    /**
     * Retrieve new data for the left plot based on it's RCU masks.
     */
    public void updatePlotLeft()
    {
        plotLeft.updatePlot( mainPanel.getBoard().getSubbandStats( plotLeft.getRCUMask() ) );
    }
    
    /**
     * Retrieve new data for the right plot based on it's RCU masks.
     */
    public void updatePlotRight()
    {
        plotRight.updatePlot( mainPanel.getBoard().getSubbandStats( plotRight.getRCUMask() ) );
    }
    
    /**
     * Update Settings (List) Panel.
     */
    public void updateSettingsPanel()
    {        
        ArrayList<WGRegisterType> temp = new ArrayList<WGRegisterType>();
        // retrieve number of rcu's'
        int nofRCUs = mainPanel.getBoard().getNrRCUs();
        RCUMask mask = new RCUMask();
        for (int i = 0; i < nofRCUs; i++) {
            // set a bit for every rcu
            mask.setBit(i);
        }
        
        /*
         * Get data and process it, so only the data with mode != 0 is kept.
         */
        WGRegisterType[] arrTemp = mainPanel.getBoard().getWaveformSettings(mask);
                
        int j = 0;
        for (int i = 0; i < mask.size() && j < arrTemp.length; i++) {
            // if bit i is set, add entry to list.
            if (mask.getBit(i)) {
                if (arrTemp[j].mode != 0) {
                    /*
                     * Only add entry if mode is not 0.
                     */
                    arrTemp[j].board = i / 8;   // i/8 is the number of the board
                    arrTemp[j].antenna = i % 8; // i%8 is the number of the antenna
                    temp.add(arrTemp[j]);
                }
                j++;
            }
        }
        
        settingsPanel.getListPanel().updateTableModel(temp);
    }
    
    /**
     * Enables or disables the buttons on this panel.
     * @param   b       Boolean value used to determine to enable (true) or
     *                  disable (false).
     */
    public void enablePanel(boolean b) {
        plotLeft.enablePanel(b);
        plotRight.enablePanel(b);
        settingsPanel.getInputPanel().enablePanel(b);
        settingsPanel.getListPanel().enablePanel(b);
    }    
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        plotRight = new nl.astron.lofar.java.gui.mac.jrsp.subbandstats.SubbandStatsPlotPanel();
        plotLeft = new nl.astron.lofar.java.gui.mac.jrsp.subbandstats.SubbandStatsPlotPanel();
        settingsPanel = new nl.astron.lofar.java.gui.mac.jrsp.subbandstats.WaveformSettingsPanel();

        plotRight.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                plotRightActionPerformed(evt);
            }
        });

        plotLeft.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                plotLeftActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                .add(org.jdesktop.layout.GroupLayout.LEADING, settingsPanel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .add(org.jdesktop.layout.GroupLayout.LEADING, layout.createSequentialGroup()
                    .add(plotLeft, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                    .add(plotRight, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(plotLeft, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(plotRight, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(settingsPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
    }// </editor-fold>//GEN-END:initComponents

    /**
     * Invoked when the OK-button on the input panel is pressed.
     */    
    private void inputPanelActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_inputPanelActionPerformed
itsLogger.debug("inputPanelActionPerformed");
        /*
         * Make a RCUMask that combines teh RCUMasks of the both plotters.
         */
        RCUMask mask = new RCUMask(plotLeft.getRCUMask(), plotRight.getRCUMask());
        
        /*
         * Call setWaveformSettings. But first parse the mode, freq. and ampl
         * to int's and a double.
         */
        try
        {
            int mode = 1; // default value for mode.
            short phase = Short.parseShort(settingsPanel.getInputPanel().getPhase());
            double frequency = Double.parseDouble(settingsPanel.getInputPanel().getFrequency());
            int amplitude = Integer.parseInt(settingsPanel.getInputPanel().getAmplitude());
            
            /**
             * Display a error message when a inputfield contains a invalid value.
             */
            if (frequency < 0 || frequency > 80) {
                JOptionPane.showMessageDialog(null, "Frequency should be a value between 0 and 80.", "Error", JOptionPane.ERROR_MESSAGE);
                return;
            } else if (phase < 0 || phase > 360) {
                JOptionPane.showMessageDialog(null, "Phase should be a value between 0 and 360.", "Error", JOptionPane.ERROR_MESSAGE);
                return;                
            } else if (amplitude < 0 || amplitude > 100) {
                JOptionPane.showMessageDialog(null, "Amplitude should be a value between 0 and 100.", "Error", JOptionPane.ERROR_MESSAGE);
                return;
            }            
            
            if (!mainPanel.getBoard().setWaveformSettings(mask, mode, frequency, phase, amplitude))
            {
                JOptionPane.showMessageDialog(null, "Failed to change the waveform settings.", "Error", JOptionPane.ERROR_MESSAGE);
            }
        }
        catch(NumberFormatException nfe)
        {
            JOptionPane.showMessageDialog(null, "Incorrect value entered. All fields should contain a positive value.", "Error", JOptionPane.ERROR_MESSAGE);
        }

        /*
         * Update whole panel.
         */
        update(ITabPanel.REQUIRED_UPDATE);
    }//GEN-LAST:event_inputPanelActionPerformed
 
    /**
     * Invoked when the Remove-button on the input panel is pressed.
     */
    private void listPanelActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_listPanelActionPerformed
        /*
         * Get the selected rows.
         */
        WGRegisterType[] data = settingsPanel.getListPanel().getSelectedRows();
        
        /*
         * If there wasn't a row selected, display a error and exit method.
         */
        if (data.length == 0) {
            JOptionPane.showMessageDialog(this, "Please select a antenna to remove it's settings.", "Error", JOptionPane.ERROR_MESSAGE);
        }
        
        /*
         * Make a RCUMask for the selected antenna.
         */
        RCUMask mask = new RCUMask();
        for (int i=0 ;i < data.length; i++) {
            mask.setBit((data[i].board * 8) + data[i].antenna);
        }
        
        /*
         * SetWaveFormSettings to 0, 0, 0 and update the ssPanel.
         */
        mainPanel.getBoard().setWaveformSettings(mask, 0, 0.0, (short)0, 0);
        update(ITabPanel.REQUIRED_UPDATE);
    }//GEN-LAST:event_listPanelActionPerformed
    
    /**
     * Invoked when a board is switched with the combobox or a antenna was
     * selected on the left panel.
     *
     * Seperate methods for the left and right plot are needed, because you 
     * can't get the panel with the ActionEvent.
     */
    private void plotLeftActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_plotLeftActionPerformed
        // The board must be connected.
        if (!mainPanel.getBoard().isConnected()) {
            return;
        }
        plotActionPerformed(plotLeft, evt);
        updatePlotLeft();
    }//GEN-LAST:event_plotLeftActionPerformed
    
    /**
     * Invoked when a board is switched with the combobox or a antenna was
     * selected on the left panel.
     */
    private void plotRightActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_plotRightActionPerformed
        // The board must be connected.
        if (!mainPanel.getBoard().isConnected()) {
            return;
        }
        plotActionPerformed(plotRight, evt);
        updatePlotRight();
    }//GEN-LAST:event_plotRightActionPerformed
    
    /**
     * The action required for the plotLeftActionPerformed and 
     * plotRightActionPerformed are equal.
     * @param   plot        The plot that generated the actionPerformed.
     * @param   evt         The actionevent.
     */
    private void plotActionPerformed(SubbandStatsPlotPanel plot, ActionEvent evt) {
        /*
         * If another board was selected: reset everything. If a antenna was
         * (un)checked there aren't any actions to perform.
         */
        if (evt.getSource().getClass().equals(javax.swing.JComboBox.class))
        {
            plot.reset();
            /*
             * @todo: this needs to be tested!
             */
            plot.getMaskSelectionPanel().setBoard( plot.getSelectedBoard() );
        }
    }    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.java.gui.mac.jrsp.subbandstats.SubbandStatsPlotPanel plotLeft;
    private nl.astron.lofar.java.gui.mac.jrsp.subbandstats.SubbandStatsPlotPanel plotRight;
    private nl.astron.lofar.java.gui.mac.jrsp.subbandstats.WaveformSettingsPanel settingsPanel;
    // End of variables declaration//GEN-END:variables
}
