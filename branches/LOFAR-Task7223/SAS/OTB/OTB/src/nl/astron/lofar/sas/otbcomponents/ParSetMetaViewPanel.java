/*
 * ParSetMetaViewPanel.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

import java.awt.Component;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.rmi.RemoteException;
import javax.swing.JFileChooser;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.table.DefaultTableModel;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBtree;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import org.apache.log4j.Logger;

/**
 * @created@
 * @author  coolen
 *
 * @version $Id$
 *
 */
public class ParSetMetaViewPanel extends javax.swing.JPanel implements IViewPanel{
    
    static Logger logger = Logger.getLogger(ParSetMetaViewPanel.class);    
    static String name = "ParSetMetaView";

   
    /** Creates new form BeanForm based upon aNode
     *
     * @params  aNode   node to obtain the info from
     *
     */
    public ParSetMetaViewPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode=aNode;
        initPanel();
    }
    
    /** Creates new form BeanForm */
    public ParSetMetaViewPanel() {
        initComponents();
    }
    
    @Override
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
        } else {
            logger.error("No Mainframe supplied");
        }
    }
    
    @Override
    public String getShortName() {
        return name;
    }
    
    @Override
    public void setContent(Object anObject) {
        itsNode = (jOTDBnode)anObject;
        initPanel();
        getParSetMeta();
    }

    @Override
    public boolean hasPopupMenu() {
        return true;
    }
    @Override
    public boolean isSingleton() {
        return false;
    }
    
    @Override
    public JPanel getInstance() {
        return new ParSetMetaViewPanel();
    }
    
    /** create popup menu for this panel
     *
     *  // build up the menu
     *  aPopupMenu= new JPopupMenu();
     *  aMenuItem=new JMenuItem("Choice 1");        
     *  aMenuItem.addActionListener(new java.awt.event.ActionListener() {
     *      public void actionPerformed(java.awt.event.ActionEvent evt) {
     *          popupMenuHandler(evt);
     *      }
     *  });
     *  aMenuItem.setActionCommand("Choice 1");
     *  aPopupMenu.add(aMenuItem);
     *  aPopupMenu.setOpaque(true);
     *
     *
     *  aPopupMenu.show(aComponent, x, y );        
     */
    @Override
    public void createPopupMenu(Component aComponent,int x, int y) {
        JPopupMenu aPopupMenu=null;
        JMenuItem  aMenuItem=null;
        
        aPopupMenu= new JPopupMenu();
        //  Fill in menu as in the example above
        aMenuItem=new JMenuItem("Create ParSet File");        
        aMenuItem.addActionListener(new java.awt.event.ActionListener() {
            @Override
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                popupMenuHandler(evt);
            }
        });
        aMenuItem.setActionCommand("Create ParSet File");
        aPopupMenu.add(aMenuItem);
            
        aMenuItem=new JMenuItem("Create ParSetMeta File");        
        aMenuItem.addActionListener(new java.awt.event.ActionListener() {
            @Override
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                popupMenuHandler(evt);
            }
        });
        aMenuItem.setActionCommand("Create ParSetMeta File");
        aPopupMenu.add(aMenuItem);
        aPopupMenu.setOpaque(true);
        aPopupMenu.show(aComponent, x, y ); 
    }

    /** handles the choice from the popupmenu 
     *
     * depending on the choices that are possible for this panel perform the action for it
     *
     *      if (evt.getActionCommand().equals("Choice 1")) {
     *          perform action
     *      }  
     */
    @Override
    public void popupMenuHandler(java.awt.event.ActionEvent evt) {
        if (!initialised) return;
        switch (evt.getActionCommand()) {
            case "Create ParSet File":
                logger.debug("Create ParSet File");
                saveParSet();
                break;
            case "Create ParSetMeta File":
                logger.debug("Create ParSetMeta File");
                saveParSetMeta();
                break;
        }
    }

    private void saveParSet() {
        int aTreeID=itsMainFrame.getSharedVars().getTreeID();
        if (fc == null) {
            fc = new JFileChooser();
        }
        // try to get a new filename to write the parsetfile to
        if (fc.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
            try {
                File aFile = fc.getSelectedFile();
                    
                // create filename that can be used at the remote site    
                String aRemoteFileName="/tmp/"+aTreeID+"-"+itsNode.name+"_"+itsMainFrame.getUserAccount().getUserName()+".ParSet";
                    
                // write the parset
                OtdbRmi.getRemoteMaintenance().exportResultTree(aTreeID,itsNode.nodeID(),aRemoteFileName); 
                    
                //obtain the remote file
                byte[] dldata = OtdbRmi.getRemoteFileTrans().downloadFile(aRemoteFileName);
                try (BufferedOutputStream output = new BufferedOutputStream(new FileOutputStream(aFile))) {
                    output.write(dldata,0,dldata.length);
                    output.flush();
                }
                logger.debug("File written to: " + aFile.getPath());
//                OtdbRmi.getRemoteFileTrans().deleteTempFile(aRemoteFileName);
            } catch (RemoteException ex) {
                String aS="ERROR: exportResultTree failed : " + ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            } catch (FileNotFoundException ex) {
                String aS="Error during saveParSet: "+ ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            } catch (IOException ex) {
                String aS="Error during saveParSet: "+ ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            }
        }
    }
    
    private void saveParSetMeta() {
        int aTreeID=itsMainFrame.getSharedVars().getTreeID();
        if (fc == null) {
            fc = new JFileChooser();
        }
        // try to get a new filename to write the parsetfile to
        if (fc.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
            try {
                File aFile = fc.getSelectedFile();
                    
                // create filename that can be used at the remote site    
                String aRemoteFileName="/tmp/"+aTreeID+"-"+itsNode.name+"_"+itsMainFrame.getUserAccount().getUserName()+".ParSetMeta";
                    
                // write the parset
                OtdbRmi.getRemoteMaintenance().exportResultTree(aTreeID,itsNode.nodeID(),aRemoteFileName); 
                    
                //obtain the remote file
                byte[] dldata = OtdbRmi.getRemoteFileTrans().downloadFile(aRemoteFileName);
                try (BufferedOutputStream output = new BufferedOutputStream(new FileOutputStream(aFile))) {
                    output.write(dldata,0,dldata.length);
                    output.flush();
                }
                logger.debug("File written to: " + aFile.getPath());
//                OtdbRmi.getRemoteFileTrans().deleteTempFile(aRemoteFileName);
            } catch (RemoteException ex) {
                String aS="ERROR: exportResultTree failed : " + ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            } catch (FileNotFoundException ex) {
                String aS="Error during saveParSetMeta: "+ ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            } catch (IOException ex) {
                String aS="Error during saveParSetMeta: "+ ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            }
        }
    }
    
    private void initPanel() {
        // check access
        UserAccount userAccount = itsMainFrame.getUserAccount();

        // for now:
        if(userAccount.isAdministrator()) {
            // enable/disable certain controls
        }
        if(userAccount.isAstronomer()) {
            // enable/disable certain controls
        }
        if(userAccount.isInstrumentScientist()) {
            // enable/disable certain controls
        }
                
         if (itsNode != null) {
            try {
                //figure out the caller
                jOTDBtree aTree = OtdbRmi.getRemoteOTDB().getTreeInfo(itsNode.treeID(),false);
                itsTreeType=OtdbRmi.getTreeType().get(aTree.type);
            } catch (RemoteException ex) {
                String aS="ParSetMetaViewPanel: Error getting treeInfo/treetype" + ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
                itsTreeType="";
            }
            
            // Fill the table
            
//            getParSetMeta();


        } else {
            logger.error("no node given");
        }
        initialised=true;
    }
    


    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    @Override
    public void enableButtons(boolean enabled) {
        this.SaveParsetMetaButton.setEnabled(enabled);
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    @Override
    public void setButtonsVisible(boolean visible) {
        this.SaveParsetMetaButton.setVisible(visible);
    }

    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    @Override
    public void setAllEnabled(boolean enabled) {
        enableButtons(enabled);
    }
    
    
    /** create the parset and get the created file from the server
     *
     */
    private void getParSetMeta() {
        int aTreeID=itsMainFrame.getSharedVars().getTreeID();
        try {
                    
            // create filename that can be used at the remote site    
            String aRemoteFileName="/tmp/"+aTreeID+"_"+itsNode.name+"_"+itsMainFrame.getUserAccount().getUserName()+".ParSetMeta";
                    
            // write the parset
            OtdbRmi.getRemoteMaintenance().exportResultTree(aTreeID,itsNode.nodeID(),aRemoteFileName); 
                    
            //obtain the remote file
            byte[] dldata = OtdbRmi.getRemoteFileTrans().downloadFile(aRemoteFileName);
             
            String aParSet=new String(dldata);
            
            // split inputfile in different lines on return.
            String[] lines = aParSet.split("\n");
            DefaultTableModel aModel=(DefaultTableModel)jTable1.getModel();
            
            for (int i=0; i< lines.length; i++) {
                String[] keyval = lines[i].split("=");
                String aS="";
                if (keyval.length>1) aS=keyval[1];
//                String aS=lines[i].replaceFirst(keyval[0]+"=", "");

                // no values available for PIC trees.
                if (itsTreeType.equals("hardware")) aS="";

                String [] str={keyval[0],aS};
                aModel.addRow(str);
            }
            jTable1.setModel(aModel);
            OtdbRmi.getRemoteFileTrans().deleteTempFile(aRemoteFileName);
            
        } catch (RemoteException ex) {
            String aS="exportTree failed : " + ex;
            logger.error(aS);
            LofarUtils.showErrorPanel(this,aS,new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
        }
    }
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        SaveParsetMetaButton = new javax.swing.JButton();
        jLabel1 = new javax.swing.JLabel();
        jScrollPane1 = new javax.swing.JScrollPane();
        jTable1 = new javax.swing.JTable();

        SaveParsetMetaButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_save.png"))); // NOI18N
        SaveParsetMetaButton.setText("Save ParsetMeta to File");
        SaveParsetMetaButton.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
        SaveParsetMetaButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SaveParsetMetaButtonActionPerformed(evt);
            }
        });

        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("ParSetMeta View Panel");

        jScrollPane1.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                jScrollPane1MouseClicked(evt);
            }
        });

        jTable1.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {

            },
            new String [] {
                "Parameter", "Value"
            }
        ) {
            Class[] types = new Class [] {
                java.lang.String.class, java.lang.String.class
            };
            boolean[] canEdit = new boolean [] {
                false, false
            };

            public Class getColumnClass(int columnIndex) {
                return types [columnIndex];
            }

            public boolean isCellEditable(int rowIndex, int columnIndex) {
                return canEdit [columnIndex];
            }
        });
        jTable1.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                jTable1MouseClicked(evt);
            }
        });
        jScrollPane1.setViewportView(jTable1);

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(SaveParsetMetaButton)
                        .addContainerGap(813, Short.MAX_VALUE))
                    .add(org.jdesktop.layout.GroupLayout.TRAILING, layout.createSequentialGroup()
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                            .add(org.jdesktop.layout.GroupLayout.LEADING, jLabel1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 869, Short.MAX_VALUE)
                            .add(jScrollPane1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 869, Short.MAX_VALUE))
                        .add(111, 111, 111))))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(jLabel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 20, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jScrollPane1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 405, Short.MAX_VALUE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(SaveParsetMetaButton)
                .addContainerGap())
        );
    }// </editor-fold>//GEN-END:initComponents

    private void SaveParsetMetaButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SaveParsetMetaButtonActionPerformed
        if (!initialised) return;
        if (evt.getActionCommand().equals("Save ParsetMeta to File")) {
            saveParSet();
        }
    }//GEN-LAST:event_SaveParsetMetaButtonActionPerformed

    private void jScrollPane1MouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_jScrollPane1MouseClicked

    }//GEN-LAST:event_jScrollPane1MouseClicked

    private void jTable1MouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_jTable1MouseClicked
        if (jTable1.getSelectedRow() == -1) return;
        String aS= (String)jTable1.getModel().getValueAt(jTable1.getSelectedRow(), 1);
        String newS = "";
        // add newlines per 80 chars to be able to have a smaller sized popupwindow
        for (int i=0; i< aS.length();i++) {
            newS=newS.concat(aS.substring(i, i+1));
            if ((i+1)%80==0) {
                newS=newS.concat("\n");
            }
        }

        //popup the result
        JOptionPane.showMessageDialog(this,newS,
                                    "Show full row value",
                                    JOptionPane.INFORMATION_MESSAGE);
    }//GEN-LAST:event_jTable1MouseClicked
    
    private jOTDBnode itsNode        = null;
    private MainFrame  itsMainFrame  = null;
    private String    itsTreeType    = "";
    private JFileChooser fc          = null;
    private boolean   initialised    = false;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton SaveParsetMetaButton;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JTable jTable1;
    // End of variables declaration//GEN-END:variables

    /**
     * Utility field used by event firing mechanism.
     */
    private javax.swing.event.EventListenerList myListenerList =  null;

    /**
     * Registers ActionListener to receive events.
     * @param listener The listener to register.
     */
    @Override
    public synchronized void addActionListener(java.awt.event.ActionListener listener) {

        if (myListenerList == null ) {
            myListenerList = new javax.swing.event.EventListenerList();
        }
        myListenerList.add (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    @Override
    public synchronized void removeActionListener(java.awt.event.ActionListener listener) {

        myListenerList.remove (java.awt.event.ActionListener.class, listener);
    }

    /**
     * Notifies all registered listeners about the event.
     * 
     * @param event The event to be fired
     */
    private void fireActionListenerActionPerformed(java.awt.event.ActionEvent event) {

        if (myListenerList == null) return;
        Object[] listeners = myListenerList.getListenerList ();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.ActionListener.class) {
                ((java.awt.event.ActionListener)listeners[i+1]).actionPerformed (event);
            }
        }
    }




    
}
