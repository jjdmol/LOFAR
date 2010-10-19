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
import java.awt.event.ActionEvent;
import java.rmi.RemoteException;
import java.util.Iterator;
import java.util.TreeMap;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.jotdb2.jVICnodeDef;
import nl.astron.lofar.sas.otb.util.IViewPanel;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import nl.astron.lofar.sas.otb.util.UserAccount;
import org.apache.log4j.Logger;

/**
 * @created 15-03-2006, 15:47
 *
 * @author  coolen
 *
 * @version $Id$
 */
public class VICnodeDefViewPanel extends javax.swing.JPanel implements IViewPanel{
    
    static Logger logger = Logger.getLogger(VICnodeDefViewPanel.class); 
    static String name="VICNodeDef View";

   
    /** Creates new form BeanForm based upon aVICnodeDef
     *
     * @params  aNode   node to obtain the info from
     *
     */
    public VICnodeDefViewPanel(MainFrame aMainFrame,jVICnodeDef aNode) {
        initComponents();
        itsMainFrame = aMainFrame;
        itsNode = aNode;
        initComboLists();
        initPanel();
    }
    
    /** Creates new form BeanForm */
    public VICnodeDefViewPanel() {
        initComponents();
    }
    
    public void setMainFrame(MainFrame aMainFrame) {
        if (aMainFrame != null) {
            itsMainFrame=aMainFrame;
            initComboLists();
        } else {
            logger.debug("No Mainframe supplied");
        }
    }

    /** Returns the shortname of this class */
    public String getShortName() {
        return name;
    }
    
    /** Sets the content for this class
     *
     * @params anObject  The class that contains the content.
     */
    public void setContent(Object anObject) {
        if (anObject != null) {
            itsNode=(jVICnodeDef)anObject;
            initPanel();
        } else {
            logger.debug("No node supplied");
        }
    }
    
    public boolean hasPopupMenu() {
        return false;
    }
    public boolean isSingleton() {
        return false;
    }
    
    public JPanel getInstance() {
        return new VICnodeDefViewPanel();
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
        
        //  Fill in menu as in the example above        
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
    } 
    
    private void initComboLists() {
        DefaultComboBoxModel aClassifModel = new DefaultComboBoxModel();
        TreeMap aClassifMap = OtdbRmi.getClassif();
        Iterator classifIt = aClassifMap.keySet().iterator();
        while (classifIt.hasNext()) {
            aClassifModel.addElement((String)aClassifMap.get(classifIt.next()));
        }
        ClassificationText.setModel(aClassifModel);
    }
     
    private void initPanel() {
        // check access
        UserAccount userAccount = itsMainFrame.getUserAccount();
        
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
            setName(itsNode.name);
            setVersion(String.valueOf(itsNode.version));
            setClassif(String.valueOf(itsNode.classif));
            setConstraints(itsNode.constraints);
            setDescription(itsNode.description);
        } else {
            logger.debug("ERROR:  no node given");
        }
    }
    
    private String getNodeName() {
        return this.NameText.getText();
    }
    
    private void setNodeName(String aS) {
        this.NameText.setText(aS);
    }
    
    private void enableName(boolean enabled) {
        this.NameText.setEnabled(enabled);
    }
    
    private String getVersion() {
        return this.VersionText.getText();
    }
    
    private void setVersion(String aS) {
        this.VersionText.setText(aS);
    }
    
    private void enableVersion(boolean enabled) {
        this.VersionText.setEnabled(enabled);
    }
    

    private String getClassif() {
        return (String)this.ClassificationText.getSelectedItem();
    }
    
    private void setClassif(String aS) {
        try {
            this.ClassificationText.setSelectedItem(OtdbRmi.getRemoteTypes().getClassif(aS));
        } catch (RemoteException e) {
            logger.debug("Error: GetParamType failed " + e);
        }
    }
    
    private void enableClassif(boolean enabled) {
        this.ClassificationText.setEnabled(enabled);
    }
    
    private String getConstraints() {
        return this.ConstraintsText.getText();
    }
    
    private void setConstraints(String aS) {
        this.ConstraintsText.setText(aS);
    }

    private void enableConstraints(boolean enabled) {
        this.ConstraintsText.setEnabled(enabled);
    }    
    
    private String getDescription() {
        return this.DescriptionText.getText();
    }
    
    private void setDescription(String aS) {
        this.DescriptionText.setText(aS);
    }
    
    private void enableDescription(boolean enabled) {
        this.DescriptionText.setEnabled(enabled);
        this.DescriptionText.setEditable(enabled);
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
        enableName(enabled);
        enableVersion(enabled);
        enableClassif(enabled);
        enableConstraints(enabled);
        enableDescription(enabled);
        enableButtons(enabled);
    }
    
   
    private void saveInput() {
        // Just check all possible fields that CAN change. The enabled method will take care if they COULD be changed.
        // this way we keep this panel general for multiple use
        boolean hasChanged = false;
        if (itsNode != null) {
            try {


                if (!itsNode.constraints.equals(getConstraints())) { 
                    itsNode.constraints=getConstraints();
                    hasChanged=true;
                }

                if (!itsNode.description.equals(getDescription())) { 
                    itsNode.description=getDescription();
                    hasChanged=true;
                }

                if (hasChanged) {
                    if (!OtdbRmi.getRemoteMaintenance().saveComponentNode(itsNode)) {
                        logger.error("Saving node failed: "+ OtdbRmi.getRemoteMaintenance().errorMsg());
                    }
                    
                    ActionEvent evt = new ActionEvent(this,-1,"VICnodeChanged");
                    this.fireActionListenerActionPerformed(evt); 
                    
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
        jLabel1 = new javax.swing.JLabel();
        NameLabel = new javax.swing.JLabel();
        NameText = new javax.swing.JTextField();
        VersionLabel = new javax.swing.JLabel();
        VersionText = new javax.swing.JTextField();
        ClassificationLabel = new javax.swing.JLabel();
        ClassificationText = new javax.swing.JComboBox();
        ConstraintsLabel = new javax.swing.JLabel();
        ConstraintsText = new javax.swing.JTextField();
        DescriptionText = new javax.swing.JTextArea();
        NodeCancelButton = new javax.swing.JButton();
        NodeApplyButton = new javax.swing.JButton();

        jLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel1.setText("VICnodeDef View Panel");

        NameLabel.setText("Name :");

        NameText.setText("None");
        NameText.setToolTipText("Name for this Node");
        NameText.setMaximumSize(new java.awt.Dimension(440, 19));
        NameText.setMinimumSize(new java.awt.Dimension(440, 19));
        NameText.setPreferredSize(new java.awt.Dimension(440, 19));

        VersionLabel.setText("Version :");

        VersionText.setText("None");
        VersionText.setToolTipText("Version number for this VICnodeDef");
        VersionText.setMaximumSize(new java.awt.Dimension(200, 19));
        VersionText.setMinimumSize(new java.awt.Dimension(200, 19));
        VersionText.setPreferredSize(new java.awt.Dimension(200, 19));

        ClassificationLabel.setText("Classification :");

        ClassificationText.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));

        ConstraintsLabel.setText("Constraints :");

        ConstraintsText.setText("None");
        ConstraintsText.setToolTipText("Limits for this Node");
        ConstraintsText.setMaximumSize(new java.awt.Dimension(200, 19));
        ConstraintsText.setMinimumSize(new java.awt.Dimension(200, 19));
        ConstraintsText.setPreferredSize(new java.awt.Dimension(200, 19));

        DescriptionText.setRows(4);
        DescriptionText.setBorder(javax.swing.BorderFactory.createTitledBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 0, 0), 1, true), "Description"));

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
                            .add(DescriptionText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 540, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(layout.createSequentialGroup()
                                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                                    .add(NameLabel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 80, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(VersionLabel)
                                    .add(ClassificationLabel)
                                    .add(ConstraintsLabel))
                                .add(20, 20, 20)
                                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                                    .add(ConstraintsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 430, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(ClassificationText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 200, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(NameText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 430, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                                    .add(VersionText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                            .add(layout.createSequentialGroup()
                                .add(NodeCancelButton)
                                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                .add(NodeApplyButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 70, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))))
                .add(104, 104, 104))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(10, 10, 10)
                .add(jLabel1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 20, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(20, 20, 20)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(NameLabel)
                    .add(NameText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(VersionLabel)
                    .add(VersionText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(ClassificationLabel)
                    .add(ClassificationText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 20, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(ConstraintsLabel)
                    .add(ConstraintsText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(27, 27, 27)
                .add(DescriptionText, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 100, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .add(27, 27, 27)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(NodeCancelButton)
                    .add(NodeApplyButton))
                .add(37, 37, 37))
        );
    }// </editor-fold>//GEN-END:initComponents

    private void NodeApplyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NodeApplyButtonActionPerformed
        saveInput();
    }//GEN-LAST:event_NodeApplyButtonActionPerformed

    private void NodeCancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NodeCancelButtonActionPerformed
        initPanel();
    }//GEN-LAST:event_NodeCancelButtonActionPerformed
    
    private jVICnodeDef itsNode = null;
    private MainFrame   itsMainFrame;

    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel ClassificationLabel;
    private javax.swing.JComboBox ClassificationText;
    private javax.swing.JLabel ConstraintsLabel;
    private javax.swing.JTextField ConstraintsText;
    private javax.swing.JTextArea DescriptionText;
    private javax.swing.JLabel NameLabel;
    private javax.swing.JTextField NameText;
    private javax.swing.JButton NodeApplyButton;
    private javax.swing.JButton NodeCancelButton;
    private javax.swing.JLabel VersionLabel;
    private javax.swing.JTextField VersionText;
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
