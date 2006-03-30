package nl.astron.lofar.mac.apl.gui.jrsp.panels;

import nl.astron.lofar.mac.apl.gui.jrsp.BoardStatus;

/**
 *
 * @author  balken
 */
public class MEPStatusPanel extends javax.swing.JPanel {
    
    /** 
     * Creates new form MEPStatusPanel.
     */
    public MEPStatusPanel() 
    {
        initComponents();
    }

    public void initFields(BoardStatus boardStatus)
    {
        txtSeqNr.setText(Integer.toString(boardStatus.seqNr));
        txtError.setText(Integer.toString(boardStatus.error));
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        lblSeqNr = new javax.swing.JLabel();
        lblError = new javax.swing.JLabel();
        txtSeqNr = new javax.swing.JTextField();
        txtError = new javax.swing.JTextField();

        setBorder(javax.swing.BorderFactory.createTitledBorder("MEP"));
        lblSeqNr.setText("Seq. Nr.");

        lblError.setText("Error");

        txtSeqNr.setEditable(false);

        txtError.setEditable(false);

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                .addContainerGap(68, Short.MAX_VALUE)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(lblError)
                    .add(lblSeqNr))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(txtSeqNr, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(txtError, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 64, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(txtSeqNr, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(lblSeqNr))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(lblError)
                    .add(txtError, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel lblError;
    private javax.swing.JLabel lblSeqNr;
    private javax.swing.JTextField txtError;
    private javax.swing.JTextField txtSeqNr;
    // End of variables declaration//GEN-END:variables
    
}
