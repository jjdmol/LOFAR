/*
 * NodeViewPanel.java
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

import java.awt.Component;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.rmi.RemoteException;
import javax.swing.JFileChooser;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBtree;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import org.apache.log4j.Logger;

/**
 * @created 26-01-2006, 15:47
 *
 * @author  coolen
 *
 * @version $Id$
 *
 */
public class NodeViewPanel extends javax.swing.JPanel implements IViewPanel{
    
    static Logger logger = Logger.getLogger(NodeViewPanel.class);    
    static String name = "Node";

   
    /** Creates new form BeanForm based upon aNode
     *
     * @params  aNode   node to obtain the info from
     *
     */
    public NodeViewPanel(MainFrame aMainFrame,jOTDBnode aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode=aNode;
        initPanel();
    }
    
    /** Creates new form BeanForm */
    public NodeViewPanel() {
        initComponents();
    }
    
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
          } else {
            logger.debug("No Mainframe supplied");
        }
    }
    
    public String getShortName() {
        return name;
    }
    
    public void setContent(Object anObject) {
        itsNode = (jOTDBnode)anObject;
        initPanel();
    }

    public boolean hasPopupMenu() {
        return true;
    }
    public boolean isSingleton() {
        return false;
    }
    
    public JPanel getInstance() {
        return new NodeViewPanel();
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
    public void createPopupMenu(Component aComponent,int x, int y) {
        JPopupMenu aPopupMenu=null;
        JMenuItem  aMenuItem=null;
        
        aPopupMenu= new JPopupMenu();
        // For VIC trees
        if (itsTreeType.equals("VHtree")) {
            //  Fill in menu as in the example above
            aMenuItem=new JMenuItem("Create ParSet File");        
            aMenuItem.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent evt) {
                    popupMenuHandler(evt);
                }
            });
            aMenuItem.setActionCommand("Create ParSet File");
            aPopupMenu.add(aMenuItem);
            
        // For template trees
        } else if (itsTreeType.equals("VItemplate")) {
                
        }
        
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
    public void popupMenuHandler(java.awt.event.ActionEvent evt) {
        if (evt.getActionCommand().equals("Create ParSet File")) {
            logger.debug("Create ParSet File");
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
                    OtdbRmi.getRemoteMaintenance().exportTree(aTreeID,itsNode.nodeID(),aRemoteFileName,2,false); 
                    
                    //obtain the remote file
                    byte[] dldata = OtdbRmi.getRemoteFileTrans().downloadFile(aRemoteFileName);

                    BufferedOutputStream output = new BufferedOutputStream(new FileOutputStream(aFile));
                    output.write(dldata,0,dldata.length);
                    output.flush();
                    output.close();
                    logger.debug("File written to: " + aFile.getPath());
                } catch (RemoteException ex) {
                    logger.debug("exportTree failed : " + ex);
                } catch (FileNotFoundException ex) {
                    logger.debug("Error during newPICTree creation: "+ ex);
                } catch (IOException ex) {
                    logger.debug("Error during newPICTree creation: "+ ex);
                }
            }
        }
    }
    
    private void initPanel() {
        // check access
        UserAccount userAccount = itsMainFrame.getUserAccount();

        // for now:
        enableInstances(true);
        enableLimits(true);
        enableDescription(false);
        enableNodeName(false);
        this.enableIndex(false);
        
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
                logger.debug("NodeViewPanel: Error getting treeInfo/treetype" + ex);
                itsTreeType="";
            }


            setNodeName(itsNode.name);
            setIndex(String.valueOf(itsNode.index));
            setInstances(String.valueOf(itsNode.instances));
            setLimits(String.valueOf(itsNode.limits));
            setDescription(itsNode.description);
        } else {
            logger.debug("ERROR:  no node given");
        }
    }
    
    /** Returns the Given Name for this Node */
    private String getNodeName() {
        return this.NodeNameText.getText();
    }
    
    private void setNodeName(String aS) {
        this.NodeNameText.setText(aS);
    }
    
    /** Returns the Given Index for this Node */
    private String getIndex() {
        return this.NodeIndexText.getText();
    }
    
    private void setIndex(String aS) {
        this.NodeIndexText.setText(aS);
    }
    
    /** Returns the Given Instances for this Node */
    private String getInstances() {
        return this.NodeInstancesText.getText();
    }
    
    private void setInstances(String aS) {
        this.NodeInstancesText.setText(aS);
    }

    /** Returns the Given Limits for this Node */
    private String getLimits() {
        return this.NodeLimitsText.getText();
    }
    
    private void setLimits(String aS) {
        this.NodeLimitsText.setText(aS);
    }

    /** Returns the Given Description for this Node */
    private String getDescription() {
        return this.NodeDescriptionText.getText();
    }
    
    private void setDescription(String aS) {
        this.NodeDescriptionText.setText(aS);
    }
    
    private void enableNodeName(boolean enabled) {
        this.NodeNameText.setEnabled(enabled);
    }

    private void enableIndex(boolean enabled) {
        this.NodeIndexText.setEnabled(enabled);
    }

    private void enableInstances(boolean enabled) {
        this.NodeInstancesText.setEnabled(enabled);
    }

    private void enableLimits(boolean enabled) {
        this.NodeLimitsText.setEnabled(enabled);
    }

    private void enableDescription(boolean enabled) {
        this.NodeDescriptionText.setEnabled(enabled);
        this.NodeDescriptionText.setEditable(enabled);
    }


    /** Enables/disables the buttons
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void enableButtons(boolean enabled) {
        this.NodeApplyButton.setEnabled(enabled);
        this.NodeCancelButton.setEnabled(enabled);
    }
    
    /** Sets the buttons visible/invisible
     *
     * @param   visible     true/false visible/invisible
     */
    public void setButtonsVisible(boolean visible) {
        this.NodeApplyButton.setVisible(visible);
        this.NodeCancelButton.setVisible(visible);
    }

    /** Enables/disables the complete form
     *
     * @param   enabled     true/false enabled/disabled
     */
    public void setAllEnabled(boolean enabled) {
        enableNodeName(enabled);
        enableIndex(enabled);
        enableInstances(enabled);
        enableLimits(enabled);
        enableDescription(enabled);
        enableButtons(enabled);
    }
    
    private void saveInput() {
        // Just check all possible fields that CAN change. The enabled method will take care if they COULD be changed.
        // this way we keep this panel general for multiple use
        boolean hasChanged = false;
        if (itsNode != null) {
            try {


                if (!String.valueOf(itsNode.instances).equals(getInstances())) { 
                    itsNode.instances=Integer.valueOf(getInstances()).shortValue();
                    hasChanged=true;
                }

                if (!itsNode.limits.equals(getLimits())) { 
                    itsNode.limits=getLimits();
                    hasChanged=true;
                }

                if (!itsNode.description.equals(getDescription())) { 
                    itsNode.description=getDescription();
                    hasChanged=true;
                }

                if (hasChanged) {
                    if (!OtdbRmi.getRemoteMaintenance().saveNode(itsNode)) {
                        logger.error("Saving node failed: "+ OtdbRmi.getRemoteMaintenance().errorMsg());
                    }
                } 
               
            } catch (RemoteException ex) {
                logger.debug("error in Remote connection");
            }
        } else {
            logger.debug("ERROR:  no Param given");
        }
    }
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        NodeNameLabel = new javax.swing.JLabel();
        NodeIndexLabel = new javax.swing.JLabel();
        NodeInstancesLabel = new javax.swing.JLabel();
        NodeLimitsLabel = new javax.swing.JLabel();
        NodeIndexText = new javax.swing.JTextField();
        NodeInstancesText = new javax.swing.JTextField();
        NodeLimitsText = new javax.swing.JTextField();
        NodeNameText = new javax.swing.JTextField();
        NodeCancelButton = new javax.swing.JButton();
        NodeApplyButton = new javax.swing.JButton();
        NodeDescriptionText = new javax.swing.JTextArea();
        jLabel1 = new javax.swing.JLabel();

        NodeNameLabel.setText("Name :");

        NodeIndexLabel.setText("Index :");

        NodeInstancesLabel.setText("Instances :");

        NodeLimitsLabel.setText("Constraints :");

        NodeIndexText.setText("None");
        NodeIndexText.setMaximumSize(new java.awt.Dimension(200, 19));
        NodeIndexText.setMinimumSize(new java.awt.Dimension(200, 19));
        NodeIndexText.setPreferredSize(new java.awt.Dimension(200, 19));

        NodeInstancesText.setText("-1");
        NodeInstancesText.setToolTipText("Number of Instances for this Node ");
        NodeInstancesText.setMaximumSize(new java.awt.Dimension(200, 19));
        NodeInstancesText.setMinimumSize(new java.awt.Dimension(200, 19));
        NodeInstancesText.setPreferredSize(new java.awt.Dimension(200, 19));

        NodeLimitsText.setText("None");
        NodeLimitsText.setToolTipText("Limits for this Node");
        NodeLimitsText.setMaximumSize(new java.awt.Dimension(200, 19));
        NodeLimitsText.setMinimumSize(new java.awt.Dimension(200, 19));
        NodeLimitsText.setPreferredSize(new java.awt.Dimension(200, 19));

        NodeNameText.setText("None");
        NodeNameText.setToolTipText("Name for this Node");
        NodeNameText.setMaximumSize(new java.awt.Dimension(440, 19));
        NodeNameText.setMinimumSize(new java.awt.Dimension(440, 19));
        NodeNameText.setPreferredSize(new java.awt.Dimension(440, 19));

        NodeCancelButton.setText("Cancel");
        NodeCancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                NodeCancelButtonActionPerformed(evt);
            }
        });

        NodeApplyButton.setText("Apply");
        NodeApplyButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                NodeApplyButtonActionPerformed(evt);
            }
        });

        NodeDescriptionText.setRows(4);
        NodeDescriptionText.setBorder(javax.swing.BorderFactory.createTitledBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true), "Description"));
        NodeDescriptionText.setEnabled(false);

        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("Node View Panel");

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(70, 70, 70)
                        .add(jLabel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 460, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                    .add(layout.createSequentialGroup()
                        .add(40, 40, 40)
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(layout.createSequentialGroup()
                                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                                    .add(NodeIndexLabel)
                                    .add(NodeNameLabel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 80, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(NodeInstancesLabel)
                                    .add(NodeLimitsLabel))
                                .add(20, 20, 20)
                                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                                    .add(NodeLimitsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(NodeInstancesText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(NodeIndexText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(NodeNameText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 430, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                            .add(NodeDescriptionText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 540, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(layout.createSequentialGroup()
                                .add(NodeCancelButton)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(NodeApplyButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 70, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))))
                .add(390, 390, 390))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(10, 10, 10)
                .add(jLabel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 20, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(20, 20, 20)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(NodeNameLabel)
                    .add(NodeNameText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(6, 6, 6)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(NodeIndexLabel)
                    .add(NodeIndexText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(NodeInstancesLabel)
                    .add(NodeInstancesText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(NodeLimitsLabel)
                    .add(NodeLimitsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(20, 20, 20)
                .add(NodeDescriptionText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 100, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(25, 25, 25)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(NodeCancelButton)
                    .add(NodeApplyButton))
                .addContainerGap())
        );
    }// </editor-fold>//GEN-END:initComponents

    private void NodeApplyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NodeApplyButtonActionPerformed
        saveInput();
    }//GEN-LAST:event_NodeApplyButtonActionPerformed

    private void NodeCancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NodeCancelButtonActionPerformed
        initPanel();
    }//GEN-LAST:event_NodeCancelButtonActionPerformed
    
    private jOTDBnode itsNode        = null;
    private MainFrame  itsMainFrame  = null;
    private String    itsTreeType    = "";
    private JFileChooser fc          = null;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton NodeApplyButton;
    private javax.swing.JButton NodeCancelButton;
    private javax.swing.JTextArea NodeDescriptionText;
    private javax.swing.JLabel NodeIndexLabel;
    private javax.swing.JTextField NodeIndexText;
    private javax.swing.JLabel NodeInstancesLabel;
    private javax.swing.JTextField NodeInstancesText;
    private javax.swing.JLabel NodeLimitsLabel;
    private javax.swing.JTextField NodeLimitsText;
    private javax.swing.JLabel NodeNameLabel;
    private javax.swing.JTextField NodeNameText;
    private javax.swing.JLabel jLabel1;
    // End of variables declaration//GEN-END:variables

    /**
     * Utility field used by event firing mechanism.
     */
    private javax.swing.event.EventListenerList myListenerList =  null;

    /**
     * Registers ActionListener to receive events.
     * @param listener The listener to register.
     */
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
