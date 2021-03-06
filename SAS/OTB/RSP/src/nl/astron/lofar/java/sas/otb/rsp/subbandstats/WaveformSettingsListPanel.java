/*
 * WaveformSettingsListPanel.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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



package nl.astron.lofar.java.sas.otb.rsp.subbandstats;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;

import javax.swing.event.EventListenerList;

import nl.astron.lofar.sas.otb.jrsp.WGRegisterType;

/**
 *
 * @author  balken
 */
public class WaveformSettingsListPanel extends javax.swing.JPanel 
{    
    /** Used to store the listeners to this class */
    private EventListenerList listenerList;
    
    /**
     * The TableModel that is used by the jTable
     */
    private WaveformTableModel tableModel;
    
    /**
     * Creates new form WaveformSettingsListPanel
     */
    public WaveformSettingsListPanel() 
    {
        listenerList = new EventListenerList();
                
        initComponents();
        tableModel =  (WaveformTableModel) jTable.getModel();        
    }

    /**
     * Update the model of tableModel with this data.
     */
    public void updateTableModel(ArrayList<WGRegisterType> data)
    {
        tableModel.updateList(data);
    }
    
    /**
     * Enables or disables the buttons on this panel.
     * @param   b       Boolean value used to determine to enable (true) or
     *                  disable (false).
     */
    public void enablePanel(boolean b) {
        jTable.setEnabled(b);        
        btnRemove.setEnabled(b);
    }

    /**
     * Return the selected rows as WGRegisterTypes
     */
    public WGRegisterType[] getSelectedRows()
    {
        return tableModel.getRows(jTable.getSelectedRows());
    }
    
    /**
     * Invoked when a action occurs; when btnRemove is pressed.
     */
    public void actionPerformed(ActionEvent e)
    {
        fireActionPerformed(e);
    }
    
    /**
     * Adds listener to the listeners list.
     */
    public void addActionListener(ActionListener l)
    {
        listenerList.add(ActionListener.class, l);
    }
    
    /**
     * Removes listener from the listeners list.
     */
    public void removeActionListener(ActionListener l)
    {
        listenerList.remove(ActionListener.class, l);
    }
    
    /**
     * Notify all listeners that a action had been performed.
     */
    public void fireActionPerformed(ActionEvent e) 
    {
            // Guaranteed to return a non-null array
            Object[] listeners = listenerList.getListenerList();
    
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
        jScrollPane = new javax.swing.JScrollPane();
        jTable = new javax.swing.JTable();
        btnRemove = new javax.swing.JButton();

        setBorder(javax.swing.BorderFactory.createTitledBorder("List"));
        jTable.setModel(new nl.astron.lofar.java.sas.otb.rsp.subbandstats.WaveformTableModel());
        jScrollPane.setViewportView(jTable);

        btnRemove.setText("Remove");
        btnRemove.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                fireActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                .addContainerGap()
                .add(jScrollPane, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 419, Short.MAX_VALUE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(btnRemove)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(btnRemove)
                    .add(jScrollPane, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 128, Short.MAX_VALUE))
                .addContainerGap())
        );
    }// </editor-fold>//GEN-END:initComponents
   
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton btnRemove;
    private javax.swing.JScrollPane jScrollPane;
    private javax.swing.JTable jTable;
    // End of variables declaration//GEN-END:variables
    
}
