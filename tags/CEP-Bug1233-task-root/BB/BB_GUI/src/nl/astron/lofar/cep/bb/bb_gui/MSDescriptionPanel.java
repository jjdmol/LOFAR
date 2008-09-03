/*
 * MSDescriptionPanel.java
 *
 * Created on 3 januari 2006, 11:13
 */

package nl.astron.lofar.cep.bb.bb_gui;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;

/**
 *
 * @author  Coolen
 */
public class MSDescriptionPanel extends javax.swing.JDialog {
    private File itsFile;
    private BB_Gui itsGui;
    private boolean readOnly=false;
   
    /** Creates new form MSDescriptionPanel */
    public MSDescriptionPanel(File aFile,BB_Gui aGui,boolean ro) {
        initComponents();
        itsFile=aFile;
        itsGui=aGui;
        readOnly=ro;
        AcceptButton.setVisible(!readOnly);
        if (readOnly) {
            CancelButton.setText("Done");
            CancelButton.setToolTipText("Done Viewing this file");
        }
        DescriptionLabel.setText("Description for "+aFile.getName());
        readFile();
        DescriptionTextArea.setEditable(false);
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        jScrollPane1 = new javax.swing.JScrollPane();
        DescriptionTextArea = new javax.swing.JTextArea();
        DescriptionLabel = new javax.swing.JLabel();
        AcceptButton = new javax.swing.JButton();
        CancelButton = new javax.swing.JButton();

        getContentPane().setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jScrollPane1.setViewportView(DescriptionTextArea);

        getContentPane().add(jScrollPane1, new org.netbeans.lib.awtextra.AbsoluteConstraints(0, 30, 650, 420));

        DescriptionLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        DescriptionLabel.setText("label");
        getContentPane().add(DescriptionLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 10, 230, -1));

        AcceptButton.setText("Accept");
        AcceptButton.setToolTipText("Accept this Descriptionfile");
        AcceptButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                AcceptButtonActionPerformed(evt);
            }
        });

        getContentPane().add(AcceptButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 470, -1, -1));

        CancelButton.setText("Cancel");
        CancelButton.setToolTipText("Do not use this DescriptionFile");
        CancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                CancelButtonActionPerformed(evt);
            }
        });

        getContentPane().add(CancelButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 470, -1, -1));

    }
    // </editor-fold>//GEN-END:initComponents

    private void AcceptButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_AcceptButtonActionPerformed
        itsGui.readDescriptionFile(itsFile);
        this.dispose();
    }//GEN-LAST:event_AcceptButtonActionPerformed

    private void CancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_CancelButtonActionPerformed
        this.dispose();
    }//GEN-LAST:event_CancelButtonActionPerformed
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton AcceptButton;
    private javax.swing.JButton CancelButton;
    private javax.swing.JLabel DescriptionLabel;
    private javax.swing.JTextArea DescriptionTextArea;
    private javax.swing.JScrollPane jScrollPane1;
    // End of variables declaration//GEN-END:variables
    
    private void readFile() {
        try {
            Reader in = new FileReader(itsFile);
            StringBuffer sb = new StringBuffer();
            char [] b = new char[8192];
            int n;
        
            //Read a block. If it gets any chars, append them
            while ((n = in.read(b)) > 0) {
                sb.append(b, 0,  n);
            }
            DescriptionTextArea.setText(sb.toString());
        } catch (IOException e) {
            System.out.println("Error while reading file: "+e);
        }
    }

}
