/*
 * SetServerDialog.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

package nl.astron.lofar.sas.otbcomponents;

import nl.astron.lofar.sas.otb.MainFrame;

/**
 * @created 24-04-2006, 11:33
 *
 * @author  coolen
 *
 * @version $Id$
 */
public class SetServerDialog extends javax.swing.JDialog {
    
    /** Creates new form BeanForm
     * 
     * @param   parent      Frame where this dialog belongs to
     * @param   modal       Should the Dialog be modal or not
     * @param   aType       PIC-tree or VIC-component
     */
    public SetServerDialog(MainFrame parent, boolean modal) {
        super(parent, modal);
        itsServer=parent.getServer();
        itsPort=parent.getPort();
        initComponents();
        setTopLabel();
        getRootPane().setDefaultButton(applyButton);
        jTextField1.setText(itsServer);
        jTextField2.setText(itsPort);
        
        ok=true;
    }
    
    /* Sets the top label with the right information */
    private void setTopLabel() {
        this.topLabelInput.setText("Set the server + port you want to connect to");
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        topLabelInput = new javax.swing.JLabel();
        cancelButton = new javax.swing.JButton();
        applyButton = new javax.swing.JButton();
        jLabel1 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        jTextField1 = new javax.swing.JTextField();
        jTextField2 = new javax.swing.JTextField();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("LOFAR set server");
        setAlwaysOnTop(true);
        setModal(true);
        setName("loadFileDialog"); // NOI18N
        setResizable(false);
        getContentPane().setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        topLabelInput.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        topLabelInput.setText("Set Server");
        getContentPane().add(topLabelInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 11, 384, 20));

        cancelButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_cancel.png"))); // NOI18N
        cancelButton.setText("Cancel");
        cancelButton.setToolTipText("Cancel filesearch");
        cancelButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        cancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                cancelButtonActionPerformed(evt);
            }
        });
        getContentPane().add(cancelButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 150, -1, -1));

        applyButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_apply.png"))); // NOI18N
        applyButton.setText("Apply");
        applyButton.setToolTipText("Apply these changes");
        applyButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        applyButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                applyButtonActionPerformed(evt);
            }
        });
        getContentPane().add(applyButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 150, -1, -1));

        jLabel1.setText("Server");
        getContentPane().add(jLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 64, -1, -1));

        jLabel2.setText("Port");
        getContentPane().add(jLabel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 105, -1, -1));
        getContentPane().add(jTextField1, new org.netbeans.lib.awtextra.AbsoluteConstraints(60, 60, 282, 20));
        getContentPane().add(jTextField2, new org.netbeans.lib.awtextra.AbsoluteConstraints(60, 100, 79, -1));

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void applyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_applyButtonActionPerformed
        ok = true;
        if (!jTextField1.getText().equals("")) {
            itsServer = jTextField1.getText();
        }
        if (!jTextField2.getText().equals("")) {
            itsPort = jTextField2.getText();
        }
        setVisible(false);
        dispose();
    }//GEN-LAST:event_applyButtonActionPerformed

    private void cancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_cancelButtonActionPerformed
        ok = false;
        setVisible(false);
        itsServer="lofar17.astron.nl";
        itsPort="1091";
        dispose();
    }//GEN-LAST:event_cancelButtonActionPerformed


    private String itsServer="lofar17.astron.nl";
    private String itsPort="1091";
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton applyButton;
    private javax.swing.JButton cancelButton;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JTextField jTextField1;
    private javax.swing.JTextField jTextField2;
    private javax.swing.JLabel topLabelInput;
    // End of variables declaration//GEN-END:variables

    /**
     * Getter for property Server.
     * @return Value of property Server.
     */
    public String getServer()    {

        return this.itsServer;
    }

    /**
     * Holds value of property status.
     */
    private String itsStatus;


    /**
     * Getter for property itsPort.
     * @return Value of property itsPort.
     */
    public String getPort()  {

        return this.itsPort;
    }

    /**
     * Holds value of property ok.
     */
    private boolean ok;

    /**
     * Getter for property ok.
     * @return Value of property ok.
     */
    public boolean isOk() {

        return this.ok;
    }
    
}
