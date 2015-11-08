/*
 * RSPStatusPanel.java
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

import java.text.NumberFormat;

import nl.astron.lofar.sas.otb.jrsp.BoardStatus;

/**
 * A panel that displays the ADO status data. This panel is used by the
 * StatusPanel.
 *
 * @author  balken
 */
public class RSPStatusPanel extends javax.swing.JPanel {
    
    /** 
     * Creates new form RSPStatusPanel.
     */
    public RSPStatusPanel() 
    {
        initComponents();
    }

    public void setFields(BoardStatus boardStatus)
    {
        if (boardStatus != null) {
            NumberFormat nf = NumberFormat.getNumberInstance();
            nf.setMinimumFractionDigits(2);
            nf.setMaximumFractionDigits(2);
            txtVoltage1V2.setText(nf.format(boardStatus.voltage1V2));
            txtVoltage2V5.setText(nf.format(boardStatus.voltage2V5));
            txtVoltage3V3.setText(nf.format(boardStatus.voltage3V3));
            txtPcbTemp.setText(Integer.toString(boardStatus.pcbTemp));
            txtBpTemp.setText(Integer.toString(boardStatus.bpTemp));
            txtAp0Temp.setText(Integer.toString(boardStatus.ap0Temp));
            txtAp1Temp.setText(Integer.toString(boardStatus.ap1Temp));
            txtAp2Temp.setText(Integer.toString(boardStatus.ap2Temp));
            txtAp3Temp.setText(Integer.toString(boardStatus.ap3Temp));
            txtBpClock.setText(Integer.toString(boardStatus.bpClock));
        } else {
            txtVoltage1V2.setText("");
            txtVoltage2V5.setText("");
            txtVoltage3V3.setText("");
            txtPcbTemp.setText("");
            txtBpTemp.setText("");
            txtAp0Temp.setText("");
            txtAp1Temp.setText("");
            txtAp2Temp.setText("");
            txtAp3Temp.setText("");
            txtBpClock.setText("");
        }
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        lblVoltage1V2 = new javax.swing.JLabel();
        lblVoltage2V5 = new javax.swing.JLabel();
        lblVoltage3V3 = new javax.swing.JLabel();
        lblPcbTemp = new javax.swing.JLabel();
        lblBpTemp = new javax.swing.JLabel();
        lblAp0Temp = new javax.swing.JLabel();
        lblAp1Temp = new javax.swing.JLabel();
        lblAp2Temp = new javax.swing.JLabel();
        lblAp3Temp = new javax.swing.JLabel();
        lblBpClock = new javax.swing.JLabel();
        txtVoltage1V2 = new javax.swing.JTextField();
        txtVoltage2V5 = new javax.swing.JTextField();
        txtVoltage3V3 = new javax.swing.JTextField();
        txtPcbTemp = new javax.swing.JTextField();
        txtBpTemp = new javax.swing.JTextField();
        txtAp0Temp = new javax.swing.JTextField();
        txtAp1Temp = new javax.swing.JTextField();
        txtAp2Temp = new javax.swing.JTextField();
        txtAp3Temp = new javax.swing.JTextField();
        txtBpClock = new javax.swing.JTextField();

        setBorder(javax.swing.BorderFactory.createTitledBorder("RSP"));
        lblVoltage1V2.setText("Voltage 1.2V");

        lblVoltage2V5.setText("Voltage 2.5V");

        lblVoltage3V3.setText("Voltage 3.3V");

        lblPcbTemp.setText("Board temp.");

        lblBpTemp.setText("BP temp.");

        lblAp0Temp.setText("AP0 temp.");

        lblAp1Temp.setText("AP1 temp.");

        lblAp2Temp.setText("AP2 temp.");

        lblAp3Temp.setText("AP3 temp.");

        lblBpClock.setText("BP clock spd.");

        txtVoltage1V2.setEditable(false);
        txtVoltage1V2.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtVoltage2V5.setEditable(false);
        txtVoltage2V5.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtVoltage3V3.setEditable(false);
        txtVoltage3V3.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtPcbTemp.setEditable(false);
        txtPcbTemp.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtBpTemp.setEditable(false);
        txtBpTemp.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtAp0Temp.setEditable(false);
        txtAp0Temp.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtAp1Temp.setEditable(false);
        txtAp1Temp.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtAp2Temp.setEditable(false);
        txtAp2Temp.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtAp3Temp.setEditable(false);
        txtAp3Temp.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtBpClock.setEditable(false);
        txtBpClock.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                .addContainerGap(33, Short.MAX_VALUE)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(lblBpClock)
                    .add(lblAp3Temp)
                    .add(lblAp2Temp)
                    .add(lblAp1Temp)
                    .add(lblAp0Temp)
                    .add(lblBpTemp)
                    .add(lblPcbTemp)
                    .add(lblVoltage3V3)
                    .add(lblVoltage2V5)
                    .add(lblVoltage1V2))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                            .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                                .add(txtVoltage1V2, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .add(txtVoltage2V5, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .add(txtVoltage3V3, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                            .add(txtPcbTemp, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(txtBpTemp, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(txtAp0Temp, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(txtAp1Temp, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(txtAp2Temp, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                        .add(txtAp3Temp, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(txtBpClock, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(txtVoltage1V2, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(lblVoltage1V2))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblVoltage2V5)
                    .add(txtVoltage2V5, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblVoltage3V3)
                    .add(txtVoltage3V3, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblPcbTemp)
                    .add(txtPcbTemp, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblBpTemp)
                    .add(txtBpTemp, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblAp0Temp)
                    .add(txtAp0Temp, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblAp1Temp)
                    .add(txtAp1Temp, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblAp2Temp)
                    .add(txtAp2Temp, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblAp3Temp)
                    .add(txtAp3Temp, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblBpClock)
                    .add(txtBpClock, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(17, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel lblAp0Temp;
    private javax.swing.JLabel lblAp1Temp;
    private javax.swing.JLabel lblAp2Temp;
    private javax.swing.JLabel lblAp3Temp;
    private javax.swing.JLabel lblBpClock;
    private javax.swing.JLabel lblBpTemp;
    private javax.swing.JLabel lblPcbTemp;
    private javax.swing.JLabel lblVoltage1V2;
    private javax.swing.JLabel lblVoltage2V5;
    private javax.swing.JLabel lblVoltage3V3;
    private javax.swing.JTextField txtAp0Temp;
    private javax.swing.JTextField txtAp1Temp;
    private javax.swing.JTextField txtAp2Temp;
    private javax.swing.JTextField txtAp3Temp;
    private javax.swing.JTextField txtBpClock;
    private javax.swing.JTextField txtBpTemp;
    private javax.swing.JTextField txtPcbTemp;
    private javax.swing.JTextField txtVoltage1V2;
    private javax.swing.JTextField txtVoltage2V5;
    private javax.swing.JTextField txtVoltage3V3;
    // End of variables declaration//GEN-END:variables
    
}
