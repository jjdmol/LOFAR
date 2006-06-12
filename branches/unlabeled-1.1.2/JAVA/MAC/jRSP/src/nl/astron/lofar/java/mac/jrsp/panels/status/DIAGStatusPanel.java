/*
 * DIAGStatusPanel.java
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

package nl.astron.lofar.mac.apl.gui.jrsp.panels.status;

import nl.astron.lofar.mac.apl.gui.jrsp.BoardStatus;

/**
 * A panel that displays the DIAG status data. This panel is used by the
 * StatusPanel.
 *
 * @author  balken
 */
public class DIAGStatusPanel extends javax.swing.JPanel {
    
    /** 
     * Creates new form RSPStatusPanel.
     */
    public DIAGStatusPanel() 
    {
        initComponents();
    }

    public void setFields(BoardStatus boardStatus)
    {
        if (boardStatus != null) {
            txtIfUnderTest.setText(Integer.toString(boardStatus.ifUnderTest));
            txtMode.setText(Integer.toString(boardStatus.mode));
            txtRiErrors.setText(Integer.toString(boardStatus.riErrors));
            txtRcuxErrors.setText(Integer.toString(boardStatus.rcuxErrors));
            txtRcuyErrors.setText(Integer.toString(boardStatus.rcuyErrors));
            txtLcuErrors.setText(Integer.toString(boardStatus.lcuErrors));
            txtCepErrors.setText(Integer.toString(boardStatus.cepErrors));
            txtSerdesErrors.setText(Integer.toString(boardStatus.serdesErrors));
            txtAp0RiErrors.setText(Integer.toString(boardStatus.ap0RiErrors));
            txtAp1RiErrors.setText(Integer.toString(boardStatus.ap1RiErrors));
            txtAp2RiErrors.setText(Integer.toString(boardStatus.ap2RiErrors));
            txtAp3RiErrors.setText(Integer.toString(boardStatus.ap3RiErrors));
        } else {
            txtIfUnderTest.setText("");
            txtMode.setText("");
            txtRiErrors.setText("");
            txtRcuxErrors.setText("");
            txtRcuyErrors.setText("");
            txtLcuErrors.setText("");
            txtCepErrors.setText("");
            txtSerdesErrors.setText("");
            txtAp0RiErrors.setText("");
            txtAp1RiErrors.setText("");
            txtAp2RiErrors.setText("");
            txtAp3RiErrors.setText("");
        }
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        lblIfUnderTest = new javax.swing.JLabel();
        lblMode = new javax.swing.JLabel();
        lblRiErrors = new javax.swing.JLabel();
        lblRcuxErrors = new javax.swing.JLabel();
        lblRcuyErrors = new javax.swing.JLabel();
        lblLcuErrors = new javax.swing.JLabel();
        lblCepErrors = new javax.swing.JLabel();
        lblSerdesErrors = new javax.swing.JLabel();
        lblAp0RiErrors = new javax.swing.JLabel();
        txtIfUnderTest = new javax.swing.JTextField();
        txtMode = new javax.swing.JTextField();
        txtRiErrors = new javax.swing.JTextField();
        txtRcuxErrors = new javax.swing.JTextField();
        txtRcuyErrors = new javax.swing.JTextField();
        txtLcuErrors = new javax.swing.JTextField();
        txtCepErrors = new javax.swing.JTextField();
        txtSerdesErrors = new javax.swing.JTextField();
        txtAp0RiErrors = new javax.swing.JTextField();
        lblAp1RiErrors = new javax.swing.JLabel();
        txtAp1RiErrors = new javax.swing.JTextField();
        lblAp2RiErrors = new javax.swing.JLabel();
        txtAp2RiErrors = new javax.swing.JTextField();
        lblAp3RiErrors = new javax.swing.JLabel();
        txtAp3RiErrors = new javax.swing.JTextField();

        setBorder(javax.swing.BorderFactory.createTitledBorder("DIAG"));
        lblIfUnderTest.setText("Interface");

        lblMode.setText("Mode");

        lblRiErrors.setText("RI Errors");

        lblRcuxErrors.setText("RCUX Errors");

        lblRcuyErrors.setText("RCUY Errors");

        lblLcuErrors.setText("LCU Errors");

        lblCepErrors.setText("CEP Errors");

        lblSerdesErrors.setText("SERDES Errors");

        lblAp0RiErrors.setText("AP0 RI Errors");

        txtIfUnderTest.setEditable(false);
        txtIfUnderTest.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtMode.setEditable(false);
        txtMode.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtRiErrors.setEditable(false);
        txtRiErrors.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtRcuxErrors.setEditable(false);
        txtRcuxErrors.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtRcuyErrors.setEditable(false);
        txtRcuyErrors.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtLcuErrors.setEditable(false);
        txtLcuErrors.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtCepErrors.setEditable(false);
        txtCepErrors.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtSerdesErrors.setEditable(false);
        txtSerdesErrors.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        txtAp0RiErrors.setEditable(false);
        txtAp0RiErrors.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        lblAp1RiErrors.setText("AP1 RI Errors");

        txtAp1RiErrors.setEditable(false);
        txtAp1RiErrors.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        lblAp2RiErrors.setText("AP2 RI Errors");

        txtAp2RiErrors.setEditable(false);
        txtAp2RiErrors.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        lblAp3RiErrors.setText("AP3 RI Errors");

        txtAp3RiErrors.setEditable(false);
        txtAp3RiErrors.setHorizontalAlignment(javax.swing.JTextField.RIGHT);

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap(27, Short.MAX_VALUE)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                            .add(lblAp0RiErrors)
                            .add(lblSerdesErrors)
                            .add(lblCepErrors)
                            .add(lblLcuErrors)
                            .add(lblRcuyErrors)
                            .add(lblRcuxErrors)
                            .add(lblRiErrors)
                            .add(lblMode)
                            .add(lblIfUnderTest))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                                    .add(txtIfUnderTest, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(txtMode, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(txtRiErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                                .add(txtRcuxErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .add(txtRcuyErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .add(txtLcuErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .add(txtCepErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                .add(txtSerdesErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                            .add(txtAp0RiErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                        .add(lblAp1RiErrors)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(txtAp1RiErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                        .add(lblAp2RiErrors)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(txtAp2RiErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                        .add(lblAp3RiErrors)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(txtAp3RiErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(txtIfUnderTest, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(lblIfUnderTest))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblMode)
                    .add(txtMode, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblRiErrors)
                    .add(txtRiErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblRcuxErrors)
                    .add(txtRcuxErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblRcuyErrors)
                    .add(txtRcuyErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblLcuErrors)
                    .add(txtLcuErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblCepErrors)
                    .add(txtCepErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblSerdesErrors)
                    .add(txtSerdesErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblAp0RiErrors)
                    .add(txtAp0RiErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(txtAp1RiErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(lblAp1RiErrors))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(txtAp2RiErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(lblAp2RiErrors))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(txtAp3RiErrors, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(lblAp3RiErrors))
                .addContainerGap(14, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel lblAp0RiErrors;
    private javax.swing.JLabel lblAp1RiErrors;
    private javax.swing.JLabel lblAp2RiErrors;
    private javax.swing.JLabel lblAp3RiErrors;
    private javax.swing.JLabel lblCepErrors;
    private javax.swing.JLabel lblIfUnderTest;
    private javax.swing.JLabel lblLcuErrors;
    private javax.swing.JLabel lblMode;
    private javax.swing.JLabel lblRcuxErrors;
    private javax.swing.JLabel lblRcuyErrors;
    private javax.swing.JLabel lblRiErrors;
    private javax.swing.JLabel lblSerdesErrors;
    private javax.swing.JTextField txtAp0RiErrors;
    private javax.swing.JTextField txtAp1RiErrors;
    private javax.swing.JTextField txtAp2RiErrors;
    private javax.swing.JTextField txtAp3RiErrors;
    private javax.swing.JTextField txtCepErrors;
    private javax.swing.JTextField txtIfUnderTest;
    private javax.swing.JTextField txtLcuErrors;
    private javax.swing.JTextField txtMode;
    private javax.swing.JTextField txtRcuxErrors;
    private javax.swing.JTextField txtRcuyErrors;
    private javax.swing.JTextField txtRiErrors;
    private javax.swing.JTextField txtSerdesErrors;
    // End of variables declaration//GEN-END:variables
    
}
