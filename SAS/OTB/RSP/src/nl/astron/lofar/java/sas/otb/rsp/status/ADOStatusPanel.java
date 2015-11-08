/*
 * ADOStatusPanel.java
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

package nl.astron.lofar.java.sas.otb.rsp.status;

import javax.swing.border.TitledBorder;

import nl.astron.lofar.sas.otb.jrsp.ADOStatus;

/**
 * A panel that displays the ADO status data. This panel is used by the
 * StatusPanel.
 *
 * @author  balken
 */
public class ADOStatusPanel extends javax.swing.JPanel 
{
    /** 
     * Creates new form ETHStatusPanel.
     */
    public ADOStatusPanel() 
    {
        initComponents();
    }
    
    /**
     * Changes the title as it is displayed in the border of this panel.
     * @param   title   The title to be displayed in the border.
     */
    public void setTitle(String title)
    {
        ((TitledBorder)getBorder()).setTitle(title);
    }
    
    public void setFields(ADOStatus adoStatus)
    {
        if (adoStatus != null) {
            txtAdcOffsetX.setText(Integer.toString(adoStatus.adcOffsetX));
            txtAdcOffsetY.setText(Integer.toString(adoStatus.adcOffsetY));
        } else {
            txtAdcOffsetX.setText("");
            txtAdcOffsetY.setText("");
        }
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        lblAdcOffsetX = new javax.swing.JLabel();
        lblAdcOffsetY = new javax.swing.JLabel();
        txtAdcOffsetX = new javax.swing.JTextField();
        txtAdcOffsetY = new javax.swing.JTextField();

        setBorder(javax.swing.BorderFactory.createTitledBorder("ADO"));
        lblAdcOffsetX.setText("ADC offset X");

        lblAdcOffsetY.setText("ADC offset Y");

        txtAdcOffsetX.setEditable(false);
        txtAdcOffsetX.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtAdcOffsetY.setEditable(false);
        txtAdcOffsetY.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                .addContainerGap(33, Short.MAX_VALUE)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(lblAdcOffsetY)
                    .add(lblAdcOffsetX))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(txtAdcOffsetX, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(txtAdcOffsetY, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(txtAdcOffsetX, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(lblAdcOffsetX))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblAdcOffsetY)
                    .add(txtAdcOffsetY, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(16, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel lblAdcOffsetX;
    private javax.swing.JLabel lblAdcOffsetY;
    private javax.swing.JTextField txtAdcOffsetX;
    private javax.swing.JTextField txtAdcOffsetY;
    // End of variables declaration//GEN-END:variables
    
}
