/*
 * SubbandStatsPlotPanel.java
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

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.HashMap;
import javax.swing.JPanel;
import javax.swing.event.EventListenerList;
import nl.astron.lofar.java.gui.plotter.PlotConstants;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;
import nl.astron.lofar.java.mac.jrsp.RCUMask;
import nl.astron.lofar.java.gui.mac.jrsp.MaskSelectionPanel;
import nl.astron.lofar.java.gui.mac.jrsp.PlotDataModel;
import org.apache.log4j.Logger;

public class SubbandStatsPlotPanel extends JPanel
{
    /** log4j logger. */
    private Logger itsLogger;
    
    /** SubbandStatsPanel. */
    private SubbandStatsPanel itsSubbandStatsPanel;
     
    /** Used to store the listeners to this class */
    private EventListenerList itsListenerList;
        
    /**
     * Default Constructor.
     */
    public SubbandStatsPlotPanel()
    {
        itsLogger = Logger.getLogger(SubbandStatsPlotPanel.class);
        itsLogger.info("Constructor");
        itsListenerList = new EventListenerList();
        
        initComponents();
    }
       
    /**
     * Updates the plot with the given data. The old data is removed and the
     * new data is added.
     * @param   ssData  The Subband Statistics Data that is used for the plot.
     */
    public void updatePlot(double[] ssData)
    {
        // create a hashmap to store the type of the graph and the data
        HashMap<String,Object> hm = new HashMap<String,Object>();
        hm.put("type",PlotDataModel.SUBBAND_STATS);
        hm.put("data", ssData);
        
        itsPlotContainer.updatePlot(hm);
    }
    
    /**
     * Updates the board selection combobox.
     * @param   nofBoards   Int value of the number of boards.
     */
    public void updateBoardSelectionBox(int nofBoards)
    {
        /*
         * All the items of the combobox are removed and for every board there
         * is a new string added as 
         */
        cmbBoard.removeAllItems();
        for (int i=0; i < nofBoards; i++) {
            cmbBoard.addItem(Integer.toString(i));
        }
    }    
    
    /**
     * Enables or disables the buttons on this panel.
     * @param   b       Boolean value used to determine to enable (true) or
     *                  disable (false).
     */
    public void enablePanel(boolean b) {
        /*
         * Only executed when the panel is to be disabled.
         * ( enablePanel ( false ) )
         */
        if (!b) {
            updatePlot(new double[0]);
            updateBoardSelectionBox(0);
            itsMaskSelectionPanel.resetCheckBoxes();            
        }
        cmbBoard.setEnabled(b);
        itsMaskSelectionPanel.enablePanel(b);
    }
    
    /**
     * Resets all checkboxes, turning them off.
     */
    public void reset()
    {
        /*
         * Deselect all checkboxes.
         */
        itsMaskSelectionPanel.resetCheckBoxes();
    }
    
    /**
     * Selected Index is the same as the Board number.
     * @return  Number of the selected board.
     */
    public int getSelectedBoard()
    {
        return cmbBoard.getSelectedIndex();
    }
    
    /**
     * Returns the RCUMask. The selected board and antennas can be determined
     * based on the set bits.
     * @return  RCUMask
     */
    public RCUMask getRCUMask()
    {
        return itsMaskSelectionPanel.getRCUMask();
    }    
    
    /**
     * Return the mask selection panel. 
     * (Default access - only in this package)
     * @return  itsMaskSelectionPanel
     */
    MaskSelectionPanel getMaskSelectionPanel()
    {
        return itsMaskSelectionPanel;
    }    
    
    /**
     * Adds listener to the listeners list.
     * @param   l   ActionListener to be added.
     */
    public void addActionListener(ActionListener l)
    {
        itsListenerList.add(ActionListener.class, l);
    }
    
    /**
     * Removes listener from the listeners list.
     * @param   l   ActionListener to be removed.
     */
    public void removeActionListener(ActionListener l)
    {
        itsListenerList.remove(ActionListener.class, l);
    }
    
    /**
     * Notify all listeners that a action had been performed.
     * @param   e   ActionEvent
     */
    public void fireActionPerformed(ActionEvent e) 
    {
            // Guaranteed to return a non-null array
            Object[] listeners = itsListenerList.getListenerList();
    
            // Process the listeners last to first, notifying
            // those that are interested in this event
            for (int i = listeners.length-2; i>=0; i-=2) 
            {
                    if (listeners[i]==ActionListener.class) 
                    {
                            ((ActionListener)listeners[i+1]).actionPerformed(e);
                    }
            }
    }    
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        cmbBoard = new javax.swing.JComboBox();
        itsMaskSelectionPanel = new nl.astron.lofar.java.gui.mac.jrsp.MaskSelectionPanel();
        itsPlotContainer = new nl.astron.lofar.java.gui.mac.jrsp.PlotContainer();

        setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createTitledBorder("Subband Statistics Plot")));
        cmbBoard.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                cmbBoardActionPerformed(evt);
            }
        });

        itsMaskSelectionPanel.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                itsMaskSelectionPanelActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(50, 50, 50)
                .add(cmbBoard, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 48, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(21, 21, 21)
                .add(itsMaskSelectionPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
            .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .add(itsPlotContainer, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                .add(itsPlotContainer, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, 20, Short.MAX_VALUE)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(itsMaskSelectionPanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(cmbBoard, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );
    }// </editor-fold>//GEN-END:initComponents

    private void cmbBoardActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_cmbBoardActionPerformed
        fireActionPerformed(evt);
    }//GEN-LAST:event_cmbBoardActionPerformed

    private void itsMaskSelectionPanelActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_itsMaskSelectionPanelActionPerformed
        fireActionPerformed(evt);
    }//GEN-LAST:event_itsMaskSelectionPanelActionPerformed
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JComboBox cmbBoard;
    private nl.astron.lofar.java.gui.mac.jrsp.MaskSelectionPanel itsMaskSelectionPanel;
    private nl.astron.lofar.java.gui.mac.jrsp.PlotContainer itsPlotContainer;
    // End of variables declaration//GEN-END:variables
}
