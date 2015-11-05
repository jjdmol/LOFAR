/* TableDialog.java
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
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.rmi.RemoteException;
import javax.swing.JFileChooser;
import javax.swing.table.AbstractTableModel;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.tablemodels.MetadataTableModel;
import org.apache.log4j.Logger;


/**
 *
 * @created 14-07-2006,11:31
 *
 * @author  coolen
 *
 * @version $Id: TableDialog.java 16480 2010-10-05 13:42:32Z coolen $
 */
public class MetadataDialog extends javax.swing.JDialog {
    static Logger logger = Logger.getLogger(MetadataDialog.class);
    static String name = "TableDialog";
    
    /** Creates new form BeanForm
     * 
     * @param   parent      Frame where this dialog belongs
     * @param   modal       Should the Dialog be modal or not
     * @param   aModel      The model we work with
     * @param   aTitle      The title for this dialog
     */
    public MetadataDialog(java.awt.Frame parent, boolean modal, MetadataTableModel aModel, String aTitle, String aUser) {
        super(parent, modal);
        initComponents();
        itsModel = aModel;
        itsTitle = aTitle;
        itsUser = aUser;
        titleLabel.setText(aTitle);
        tablePanel1.setTableModel(aModel);
        tablePanel1.validate();
    }
    
    public AbstractTableModel getModel() {        
        return tablePanel1.getTableModel();
    }

    public void setModel(MetadataTableModel aModel) {
        tablePanel1.setTableModel(aModel);
        itsModel=aModel;
    }

    public void setTableCellAlignment(int alignment) {
        tablePanel1.setTableCellAlignment(alignment);
    }
    
    public boolean hasChanged() {
        return isChanged;
    }

    public void setWarning(String aS) {
        tablePanel1.setWarning(aS);
    }
    
    public void removeWarning() {
        tablePanel1.removeWarning();
    }
    
    public void showQuitButton(boolean aFlag) {
        this.quitButton.setVisible(aFlag);
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        titleLabel = new javax.swing.JLabel();
        quitButton = new javax.swing.JButton();
        tablePanel1 = new nl.astron.lofar.sas.otbcomponents.TablePanel();
        SaveButton = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("LOFAR View VIC Metadata");
        setAlwaysOnTop(true);
        setModal(true);
        setName("MetadataDialog"); // NOI18N
        setResizable(false);

        titleLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        titleLabel.setText("no Title");

        quitButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_apply.png"))); // NOI18N
        quitButton.setText("Quit");
        quitButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        quitButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                quitButtonActionPerformed(evt);
            }
        });

        SaveButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_save.png"))); // NOI18N
        SaveButton.setText("Save to File");
        SaveButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        SaveButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SaveButtonActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, titleLabel, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 1386, Short.MAX_VALUE)
                    .add(tablePanel1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 1386, Short.MAX_VALUE)
                    .add(layout.createSequentialGroup()
                        .add(SaveButton)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(quitButton)))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(titleLabel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 15, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(tablePanel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 458, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(18, 18, 18)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(SaveButton)
                    .add(quitButton))
                .add(22, 22, 22))
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void quitButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_quitButtonActionPerformed
        fireActionListenerActionPerformed(evt);
        setVisible(false);
        dispose();
    }//GEN-LAST:event_quitButtonActionPerformed

    private void SaveButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SaveButtonActionPerformed
        if (evt.getActionCommand().equals("Save to File")) {
            saveMetadata();
        }
    }//GEN-LAST:event_SaveButtonActionPerformed


    private void saveMetadata() {
        int aTreeID=itsModel.getTreeID();
        if (fc == null) {
            fc = new JFileChooser();
        }
        // try to get a new filename to write the parsetfile to
        if (fc.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
            try {
                File aFile = fc.getSelectedFile();
                    
                // create filename that can be used at the remote site    
                String aRemoteFileName="/tmp/"+aTreeID+"-"+itsUser+".Metadata";
                    
                // write the parset
                OtdbRmi.getRemoteMaintenance().exportMetadata(aTreeID,aRemoteFileName); 
                    
                //obtain the remote file
                byte[] dldata = OtdbRmi.getRemoteFileTrans().downloadFile(aRemoteFileName);
                try (BufferedOutputStream output = new BufferedOutputStream(new FileOutputStream(aFile))) {
                output.write(dldata,0,dldata.length);
                output.flush();
                }
                logger.debug("File written to: " + aFile.getPath());
//                OtdbRmi.getRemoteFileTrans().deleteTempFile(aRemoteFileName);
            } catch (RemoteException ex) {
                String aS="ERROR: exportMetadata failed : " + ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            } catch (FileNotFoundException ex) {
                String aS="Error during saveMetadata: "+ ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            } catch (IOException ex) {
                String aS="Error during saveMetadata: "+ ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            }
        }
        
    }


    private String itsUser = "";
    private MetadataTableModel itsModel = null;
    private String itsTitle = "";
    private boolean isChanged=false;
    private JFileChooser fc          = null;
            
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton SaveButton;
    private javax.swing.JButton quitButton;
    private nl.astron.lofar.sas.otbcomponents.TablePanel tablePanel1;
    private javax.swing.JLabel titleLabel;
    // End of variables declaration//GEN-END:variables

    /**
     * Utility field used by event firing mechanism.
     */
    private javax.swing.event.EventListenerList listenerList =  null;

    /**
     * Registers ActionListener to receive events.
     * @param listener The listener to register.
     */
    public synchronized void addActionListener(java.awt.event.ActionListener listener) {
        if (listenerList == null ) {
            listenerList = new javax.swing.event.EventListenerList();
        }
        listenerList.add (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeActionListener(java.awt.event.ActionListener listener) {
        listenerList.remove (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Notifies all registered listeners about the event.
     * 
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {
        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList ();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed (event);
            }
        }
    }

}
