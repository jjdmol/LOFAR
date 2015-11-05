/*
 * LoadFileDialog.java
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

import java.io.File;
import java.util.Iterator;
import java.util.TreeMap;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JFileChooser;
import nl.astron.lofar.sas.otb.util.OtdbRmi;

/**
 * @created 26-01-2006, 11:33
 *
 * @author  coolen
 *
 * @version $Id$
 *
 * @updated
 */
public class LoadFileDialog extends javax.swing.JDialog {
    
    /** Creates new form BeanForm
     * 
     * @param   parent      Frame where this dialog belongs to
     * @param   modal       Should the Dialog be modal or not
     * @param   aType       PIC-tree or VIC-component
     */
    public LoadFileDialog(java.awt.Frame parent, boolean modal, String aType) {
        super(parent, modal);
        initComponents();
        itsType=aType;
        setTopLabel();
        getRootPane().setDefaultButton(loadButton);
        
        ok=true;
        init();
    }
    
    public void setType(String aType) {
        itsType=aType;
        setTopLabel();
        getRootPane().setDefaultButton(loadButton);
        
        ok=true;
    }
    
    /* Sets the top label with the right information */
    private void setTopLabel() {
        this.topLabelInput.setText("Choose a file to create a new "+itsType);
    }

    private void init() {
        DefaultComboBoxModel aStateModel = new DefaultComboBoxModel();
        TreeMap aStateMap=OtdbRmi.getTreeState();
        Iterator stateIt = aStateMap.keySet().iterator();
        while (stateIt.hasNext()) {
            aStateModel.addElement((String)aStateMap.get(stateIt.next()));
        }
        statusInput.setModel(aStateModel);
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        topLabelInput = new javax.swing.JLabel();
        fileNameInput = new javax.swing.JTextField();
        browseButton = new javax.swing.JButton();
        statusLabel = new javax.swing.JLabel();
        statusInput = new javax.swing.JComboBox();
        jLabel3 = new javax.swing.JLabel();
        descriptionInput = new javax.swing.JTextArea();
        cancelButton = new javax.swing.JButton();
        loadButton = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("LOFAR Load File");
        setAlwaysOnTop(true);
        setModal(true);
        setName("loadFileDialog"); // NOI18N
        setResizable(false);
        getContentPane().setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        topLabelInput.setText("Choose a file");
        getContentPane().add(topLabelInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 10, 370, 20));

        fileNameInput.setToolTipText("Filename field");
        getContentPane().add(fileNameInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 40, 290, -1));

        browseButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_fileopen.gif"))); // NOI18N
        browseButton.setText("Browse");
        browseButton.setToolTipText("browse for a file");
        browseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                browseButtonActionPerformed(evt);
            }
        });
        getContentPane().add(browseButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(310, 40, 100, 20));

        statusLabel.setText("Status:");
        getContentPane().add(statusLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 80, -1, 20));

        statusInput.setToolTipText("State Selection");
        getContentPane().add(statusInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(80, 80, 130, -1));

        jLabel3.setText("Description :");
        getContentPane().add(jLabel3, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 120, 130, 20));

        descriptionInput.setRows(3);
        descriptionInput.setToolTipText("Set Description for this tree");
        getContentPane().add(descriptionInput, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 140, 400, 60));

        cancelButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_cancel.png"))); // NOI18N
        cancelButton.setText("Cancel");
        cancelButton.setToolTipText("Cancel filesearch");
        cancelButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        cancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                cancelButtonActionPerformed(evt);
            }
        });
        getContentPane().add(cancelButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 230, -1, -1));

        loadButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_apply.png"))); // NOI18N
        loadButton.setText("Load");
        loadButton.setToolTipText("Load this File");
        loadButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        loadButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                loadButtonActionPerformed(evt);
            }
        });
        getContentPane().add(loadButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 230, 100, -1));

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void browseButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_browseButtonActionPerformed
        if (fc == null) {
            fc = new JFileChooser();
        }
      int returnVal = fc.showOpenDialog(this);
      if (returnVal == JFileChooser.APPROVE_OPTION) {
          itsFile = fc.getSelectedFile();
          fileNameInput.setText(itsFile.getPath());
      }
    }//GEN-LAST:event_browseButtonActionPerformed

    private void loadButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_loadButtonActionPerformed
        ok = true;
        itsDescription=descriptionInput.getText();
        itsStatus=(String)statusInput.getSelectedItem();
        setVisible(false);
        dispose();
    }//GEN-LAST:event_loadButtonActionPerformed

    private void cancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_cancelButtonActionPerformed
        ok = false;
        setVisible(false);
        itsDescription="";
        itsStatus="";
        dispose();
    }//GEN-LAST:event_cancelButtonActionPerformed


    private File itsFile    = null;
    private String itsType  = "";
    private JFileChooser fc = null;
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton browseButton;
    private javax.swing.JButton cancelButton;
    private javax.swing.JTextArea descriptionInput;
    private javax.swing.JTextField fileNameInput;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JButton loadButton;
    private javax.swing.JComboBox statusInput;
    private javax.swing.JLabel statusLabel;
    private javax.swing.JLabel topLabelInput;
    // End of variables declaration//GEN-END:variables

    /** set Statuslabel/box invisible
     * 
     * @param visible  visible/invisible
     */
    public void setStatusVisible(boolean visible) {
        statusLabel.setVisible(visible);
        statusInput.setVisible(visible);
    }
    
    /**
     * Getter for property fileName.
     * @return Value of property fileName.
     */
    public File getFile()    {

        return this.itsFile;
    }

    /**
     * Holds value of property status.
     */
    private String itsStatus;

    /**
     * Getter for property itsStatus.
     * @return Value of property itsStatus.
     */
    public String getStatus()  {

        return this.itsStatus;
    }

    /**
     * Holds value of property description.
     */
    private String itsDescription;

    /**
     * Getter for property itsDescription.
     * @return Value of property itsDescription.
     */
    public String getDescription()  {

        return this.itsDescription;
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
