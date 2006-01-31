/*
 * ModelTypeDialog.java
 *
 * Created on 31 januari 2006, 12:08
 */

package nl.astron.lofar.cep.bb.bb_gui;

/**
 *
 * @author  Coolen
 */
public class ModelTypeDialog extends javax.swing.JDialog {
    
    /** Creates new form ModelTypeDialog */
    public ModelTypeDialog() {
        initComponents();
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        jPanel1 = new javax.swing.JPanel();
        TotalEJ = new javax.swing.JRadioButton();
        PatchEJ = new javax.swing.JRadioButton();
        RealImag = new javax.swing.JRadioButton();
        BandPass = new javax.swing.JRadioButton();
        Dipole = new javax.swing.JRadioButton();
        jButton1 = new javax.swing.JButton();
        jButton2 = new javax.swing.JButton();
        SelectionText = new javax.swing.JTextField();

        getContentPane().setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        jPanel1.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jPanel1.setBorder(new javax.swing.border.TitledBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 2), "Select all models you want"));
        TotalEJ.setText("TotalEJ");
        TotalEJ.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                TotalEJActionPerformed(evt);
            }
        });

        jPanel1.add(TotalEJ, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 30, -1, -1));

        PatchEJ.setText("PatchEJ");
        PatchEJ.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                PatchEJActionPerformed(evt);
            }
        });

        jPanel1.add(PatchEJ, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 60, -1, -1));

        RealImag.setText("RealImag");
        RealImag.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                RealImagActionPerformed(evt);
            }
        });

        jPanel1.add(RealImag, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 90, -1, -1));

        BandPass.setText("BandPass");
        BandPass.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                BandPassActionPerformed(evt);
            }
        });

        jPanel1.add(BandPass, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 120, -1, -1));

        Dipole.setText("Dipole");
        Dipole.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                DipoleActionPerformed(evt);
            }
        });

        jPanel1.add(Dipole, new org.netbeans.lib.awtextra.AbsoluteConstraints(30, 150, -1, -1));

        getContentPane().add(jPanel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 20, 380, 210));

        jButton1.setText("OK");
        jButton1.setToolTipText("accept models");
        jButton1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButton1ActionPerformed(evt);
            }
        });

        getContentPane().add(jButton1, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 290, -1, -1));

        jButton2.setText("Cancel");
        jButton2.setToolTipText("Cancel selection");
        jButton2.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButton2ActionPerformed(evt);
            }
        });

        getContentPane().add(jButton2, new org.netbeans.lib.awtextra.AbsoluteConstraints(70, 290, -1, -1));

        SelectionText.setToolTipText("Composed selection");
        SelectionText.setEnabled(false);
        getContentPane().add(SelectionText, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 250, 380, -1));

        pack();
    }
    // </editor-fold>//GEN-END:initComponents

    private void jButton2ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButton2ActionPerformed
        this.itsSelection=itsOldSelection;
        this.itsOldSelection="";
        setSelection(itsSelection); 
        this.dispose();       
    }//GEN-LAST:event_jButton2ActionPerformed

    private void DipoleActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DipoleActionPerformed
        createSelection();
    }//GEN-LAST:event_DipoleActionPerformed

    private void BandPassActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_BandPassActionPerformed
        createSelection();
    }//GEN-LAST:event_BandPassActionPerformed

    private void RealImagActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_RealImagActionPerformed
        createSelection();  
    }//GEN-LAST:event_RealImagActionPerformed

    private void PatchEJActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_PatchEJActionPerformed
        createSelection();
    }//GEN-LAST:event_PatchEJActionPerformed

    private void TotalEJActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_TotalEJActionPerformed
        createSelection();
    }//GEN-LAST:event_TotalEJActionPerformed

    private void jButton1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButton1ActionPerformed
        this.dispose();
    }//GEN-LAST:event_jButton1ActionPerformed
    
    
    private void createSelection() {
        String aP="";
        String aS="";
        if (TotalEJ.isSelected()){
            aS+=aP+"TotalEJ";
            aP=".";
        }
        if (PatchEJ.isSelected()){
            aS+=aP+"PatchEJ";
            aP=".";
        }
        if (RealImag.isSelected()){
            aS+=aP+"RealImag";
           aP=".";
        }
        if (BandPass.isSelected()){
            aS+=aP+"BandPass";
            aP=".";
        }
        if (Dipole.isSelected()){
            aS+=aP+"Dipole";
            aP=".";
        }
        
        itsSelection=aS;
        SelectionText.setText(itsSelection);
    }
    
    public void setSelection(String aSelection) {
        TotalEJ.setSelected(false);
        PatchEJ.setSelected(false);
        RealImag.setSelected(false);
        BandPass.setSelected(false);
        Dipole.setSelected(false);

        String [] aS=aSelection.split(".");

        for (int i=0;i<aS.length;i++) {
            if (aS[i].equals("TotalEJ")) {
                TotalEJ.setSelected(true);
            }
            if (aS[i].equals("PatchEJ")) {
                PatchEJ.setSelected(true);
            }
            if (aS[i].equals("RealImag")) {
                RealImag.setSelected(true);
            }
            if (aS[i].equals("BandPass")) {
                BandPass.setSelected(true);
            }
            if (aS[i].equals("Dipole")) {
                Dipole.setSelected(true);
            }
        }
        
        this.itsOldSelection=this.itsSelection;
        this.itsSelection=aSelection;
        
        SelectionText.setText(itsSelection);
    }
    
    public String getSelection() {
        return this.itsSelection;
    }
    
    private String itsSelection="";
    private String itsOldSelection="";
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JRadioButton BandPass;
    private javax.swing.JRadioButton Dipole;
    private javax.swing.JRadioButton PatchEJ;
    private javax.swing.JRadioButton RealImag;
    private javax.swing.JTextField SelectionText;
    private javax.swing.JRadioButton TotalEJ;
    private javax.swing.JButton jButton1;
    private javax.swing.JButton jButton2;
    private javax.swing.JPanel jPanel1;
    // End of variables declaration//GEN-END:variables
    
}
