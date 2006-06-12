/*
 * RSUStatusPanel.java
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
 * A panel that displays the ADO status data. This panel is used by the
 * StatusPanel.
 *
 * @author  balken
 */
public class RSUStatusPanel extends javax.swing.JPanel {
    
    /** The messages that can be displayed in the fields */
    private String[] trigMessages = {"Board reset",
			             "User reconfiguration request",
			             "User reset request",
			             null, // 3 / 011b isn't used.
			             "Watchdog timer timeout"};

    private String[] imMessages = {"Factory image",
                        	   "Application image"};

    private String[] fpgaMessages = {"BP was reconfigured",
                                     "AP was reconfigured"};

    private String[] errMessages = {"Configuration was succesfull",
                                    "Error occured during configuration"};

    private String[] rdyMessages = {"Configuration ongoing",
                                    "Configuration done"};
        
    /** 
     * Creates new form ETHStatusPanel.
     */
    public RSUStatusPanel() 
    {
        initComponents();        
    }

    public void setFields(BoardStatus boardStatus)
    {
        if (boardStatus != null) {
            txtRdy.setText(boardStatus.cpRdy ? rdyMessages[1] : rdyMessages[0]);
            txtErr.setText(boardStatus.cpErr ? errMessages[1] : errMessages[0]);
            txtFpga.setText(boardStatus.cpFpga ? fpgaMessages[1] : fpgaMessages[0]);
            txtIm.setText(boardStatus.cpIm ? imMessages[1] : imMessages[0]);
            txtTrig.setText(trigMessages[boardStatus.cpTrig]);
        } else {
            txtRdy.setText("");
            txtErr.setText("");
            txtFpga.setText("");
            txtIm.setText("");
            txtTrig.setText("");
        }
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        lblRdy = new javax.swing.JLabel();
        lblErr = new javax.swing.JLabel();
        lblFpga = new javax.swing.JLabel();
        txtRdy = new javax.swing.JTextField();
        txtErr = new javax.swing.JTextField();
        txtFpga = new javax.swing.JTextField();
        lblIm = new javax.swing.JLabel();
        txtIm = new javax.swing.JTextField();
        lblTrig = new javax.swing.JLabel();
        txtTrig = new javax.swing.JTextField();

        setBorder(javax.swing.BorderFactory.createTitledBorder("RSU"));
        lblRdy.setText("Ready");

        lblErr.setText("Conf. result");

        lblFpga.setText("FPGA type");

        txtRdy.setEditable(false);

        txtErr.setEditable(false);

        txtFpga.setEditable(false);

        lblIm.setText("Image type");

        txtIm.setEditable(false);

        lblTrig.setText("Reconf. trigger");

        txtTrig.setEditable(false);

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                .addContainerGap(23, Short.MAX_VALUE)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(lblTrig)
                    .add(lblFpga)
                    .add(lblErr)
                    .add(lblRdy)
                    .add(lblIm))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                    .add(txtFpga)
                    .add(txtIm)
                    .add(txtErr, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 105, Short.MAX_VALUE)
                    .add(txtRdy)
                    .add(txtTrig, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 282, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblRdy)
                    .add(txtRdy, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblErr)
                    .add(txtErr, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblFpga)
                    .add(txtFpga, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblIm)
                    .add(txtIm, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblTrig)
                    .add(txtTrig, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(18, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel lblErr;
    private javax.swing.JLabel lblFpga;
    private javax.swing.JLabel lblIm;
    private javax.swing.JLabel lblRdy;
    private javax.swing.JLabel lblTrig;
    private javax.swing.JTextField txtErr;
    private javax.swing.JTextField txtFpga;
    private javax.swing.JTextField txtIm;
    private javax.swing.JTextField txtRdy;
    private javax.swing.JTextField txtTrig;
    // End of variables declaration//GEN-END:variables
    
}
