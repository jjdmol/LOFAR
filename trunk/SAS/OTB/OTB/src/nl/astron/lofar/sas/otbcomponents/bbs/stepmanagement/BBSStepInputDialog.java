/*
 * BBSStepInputDialog.java
 *
 * Created on July 31, 2006, 2:14 PM
 */

package nl.astron.lofar.sas.otbcomponents.bbs.stepmanagement;

import nl.astron.lofar.sas.otb.MainFrame;


/**
 * Dialog that contains a panel for BBS Step Explorer
 *
 * @created 31-07-2006, 13:37
 *
 * @author  pompert
 *
 * @version $Id$
 */
public class BBSStepInputDialog extends javax.swing.JDialog {
    
    /** Creates new form BBSStepInputDialog */
    public BBSStepInputDialog(MainFrame parent, boolean modal, BBSStep tobeDisplayedBBSStep) {
        super(parent, modal);
        initComponents();
        this.sePanel.setMainFrame(parent);
        this.sePanel.setContent(tobeDisplayedBBSStep);
        
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        sePanel = new nl.astron.lofar.sas.otbcomponents.bbs.BBSStepExplorerPanel();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .add(sePanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(sePanel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        pack();
    }// </editor-fold>//GEN-END:initComponents
    
    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new BBSStepInputDialog(null, true,null).setVisible(true);
            }
        });
    }
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.sas.otbcomponents.bbs.BBSStepExplorerPanel sePanel;
    // End of variables declaration//GEN-END:variables
    
}
