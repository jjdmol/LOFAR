/*
 * StorageSelectionPanel.java
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

import java.rmi.RemoteException;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.DefaultListModel;
import javax.swing.border.TitledBorder;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBnode;
import nl.astron.lofar.sas.otb.jotdb3.jOTDBtree;
import nl.astron.lofar.sas.otb.util.OtdbRmi;
import org.apache.log4j.Logger;
/**
 * Panel to view (and edit) lists
 * The lists are being send and retained from the panel via a Vector like string:
 * [item1,item2,item3]
 *
 * @created 13-07-2006, 14:50
 *
 * @author  coolen
 *
 * @version $Id$
 */
public class StorageSelectionPanel extends javax.swing.JPanel {
    
    
    static Logger logger = Logger.getLogger(StorageSelectionPanel.class);
    static String name="StorageSelectionPanel";
    
    /** Creates new form BeanForm */
    public StorageSelectionPanel() {
        initComponents();
    }

    public StorageSelectionPanel(String aName) {
        initComponents();
        setTitle(aName);
    }
    

    public void setTitle(String aName) {
        itsName=aName;
        String aTitle = aName+" StorageNode Selection";
        ((TitledBorder)this.getBorder()).setTitle(aTitle);       
    }
    
    /** 
     * Get the List represented by this StorageSelectionPanel
     */
    public String getList() {
        return itsList;
    }
    
    private String itsName                            = "";
    private String itsList                            = "";
    private boolean isNew                             = false;
    private int itsTreeID                             = -1;
    private DefaultListModel itsUsedModel             = new DefaultListModel();
    private DefaultListModel itsAvailableModel        = new DefaultListModel();
    private jOTDBtree itsTree                         = null;
    private Vector<String> itsStorageNodeList             = new Vector<String>();
    private Vector<String> itsUsedStorageNodeList         = new Vector<String>();
    private Vector<String> itsAvailableStorageNodeList    = new Vector<String>();

    
    public void init() {
        AvailableStorageNodeList.setModel(itsAvailableModel);
        UsedStorageNodeList.setModel(itsUsedModel);
        try {
            Vector aTreeList = OtdbRmi.getRemoteOTDB().getTreeList(OtdbRmi.getRemoteTypes().getTreeType("hardware"),
                    OtdbRmi.getRemoteTypes().getClassif("operational"));
           for (int k = 0; k < aTreeList.size(); k++) {
                jOTDBtree tInfo = (jOTDBtree) aTreeList.elementAt(k);
                if (OtdbRmi.getTreeState().get(tInfo.state).equals("active")) {
                    itsTree = tInfo;
                    break;
                }
            }

            // if no active PIC tree we can't continue
            if (itsTree == null) {
                enableAllButtons(false);
                return;
            }

            // Now we have the operational PIC tree, we need to search for the available StorageNodes
            Vector storagenodes = OtdbRmi.getRemoteMaintenance().getItemList(itsTree.treeID(), "LOFAR_PermSW_Storage");
            Enumeration e = storagenodes.elements();
            while (e.hasMoreElements()) {
                
                jOTDBnode aRingNode = (jOTDBnode) e.nextElement();
                Vector childs = OtdbRmi.getRemoteMaintenance().getItemList(itsTree.treeID() ,aRingNode.nodeID(), 1);

                Enumeration ec = childs.elements();
                while (ec.hasMoreElements()) {
                   jOTDBnode aNode = (jOTDBnode) ec.nextElement();
                 
                   if (!aNode.leaf) {
                       // split the name
                       String aName=LofarUtils.keyName(aNode.name);
                       itsStorageNodeList.add(aName);
                   }
                }
            }
            if (itsStorageNodeList.isEmpty() ) {
                enableAllButtons(false);
            } else {
                enableAllButtons(true);
                validateModels();
            }
 
        } catch (RemoteException ex) {
            logger.debug("Error during init: "+ ex);
            return;
        }
    
    }
    
    /**
     * validate the used and available modesl against the real StorageNode list
     */
    private void validateModels() {
        for (int i=0; i< getUsedStorageNodeList().size();i++) {
            if (!itsStorageNodeList.contains(itsUsedStorageNodeList.get(i))) {
                itsUsedModel.removeElement(getUsedStorageNodeList().get(i));
            }
        }
        for (int i=0; i< itsStorageNodeList.size();i++) {
            if (!itsUsedModel.contains(itsStorageNodeList.get(i)) &&
                    !itsAvailableModel.contains(itsStorageNodeList.get(i))) {
                itsAvailableModel.addElement(itsStorageNodeList.get(i));
            }
        }

        itsUsedStorageNodeList.clear();
        Enumeration usE=itsUsedModel.elements();
        while (usE.hasMoreElements()) {
            itsUsedStorageNodeList.add(usE.nextElement().toString());
        }

        itsAvailableStorageNodeList.clear();
        Enumeration avE=itsAvailableModel.elements();
        while (avE.hasMoreElements()) {
            itsAvailableStorageNodeList.add(avE.nextElement().toString());
        }

        AvailableStorageNodeList.invalidate();
        AvailableStorageNodeList.repaint();
        UsedStorageNodeList.invalidate();
        UsedStorageNodeList.repaint();
    }
    
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        AddButton = new javax.swing.JButton();
        RemoveAllButton = new javax.swing.JButton();
        jScrollPane1 = new javax.swing.JScrollPane();
        AvailableStorageNodeList = new javax.swing.JList();
        jScrollPane2 = new javax.swing.JScrollPane();
        UsedStorageNodeList = new javax.swing.JList();
        AddAllButton = new javax.swing.JButton();
        RemoveButton = new javax.swing.JButton();

        setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "StorageNodeList", javax.swing.border.TitledBorder.CENTER, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11))); // NOI18N

        AddButton.setText("Add");
        AddButton.setToolTipText("Add item to list");
        AddButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                AddButtonActionPerformed(evt);
            }
        });

        RemoveAllButton.setText("Remove all");
        RemoveAllButton.setToolTipText("Remove all items from the list");
        RemoveAllButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                RemoveAllButtonActionPerformed(evt);
            }
        });

        AvailableStorageNodeList.setBorder(javax.swing.BorderFactory.createTitledBorder("Available"));
        AvailableStorageNodeList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "Item 1", "Item 2", "Item 3", "Item 4", "Item 5" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        jScrollPane1.setViewportView(AvailableStorageNodeList);

        UsedStorageNodeList.setBorder(javax.swing.BorderFactory.createTitledBorder("Used"));
        UsedStorageNodeList.setModel(new javax.swing.AbstractListModel() {
            String[] strings = { "Item 1", "Item 2", "Item 3", "Item 4", "Item 5" };
            public int getSize() { return strings.length; }
            public Object getElementAt(int i) { return strings[i]; }
        });
        jScrollPane2.setViewportView(UsedStorageNodeList);

        AddAllButton.setText("Add All");
        AddAllButton.setToolTipText("Add all stations to the list");
        AddAllButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                AddAllButtonActionPerformed(evt);
            }
        });

        RemoveButton.setText("Remove");
        RemoveButton.setToolTipText("Remove selected items from the list");
        RemoveButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                RemoveButtonActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(jScrollPane2, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 75, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jScrollPane1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 76, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                    .add(RemoveAllButton, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(AddAllButton, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 86, Short.MAX_VALUE)
                    .add(RemoveButton, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .add(AddButton, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING)
                    .add(layout.createSequentialGroup()
                        .add(AddButton)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .add(AddAllButton)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(RemoveButton)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(RemoveAllButton)
                        .add(32, 32, 32))
                    .add(org.jdesktop.layout.GroupLayout.LEADING, jScrollPane2, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 142, Short.MAX_VALUE)
                    .add(org.jdesktop.layout.GroupLayout.LEADING, jScrollPane1, 0, 0, Short.MAX_VALUE))
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents

    private void RemoveAllButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_RemoveAllButtonActionPerformed
       for (int i = 0; i < itsUsedModel.size();i++) {
            if (!itsAvailableModel.contains(itsUsedModel.elementAt(i))) {
                itsAvailableModel.addElement(itsUsedModel.get(i));
            }
            itsUsedModel.clear();
        }
        validateModels();
}//GEN-LAST:event_RemoveAllButtonActionPerformed

    private void AddButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_AddButtonActionPerformed
        int idx[] = AvailableStorageNodeList.getSelectedIndices();
        if (idx.length <= 0 ) {
            return;
        } else {
            for (int i = 0; i< idx.length; i++) {
                itsUsedModel.addElement(itsAvailableModel.get(idx[i]));
            }
            for (int i = idx.length-1; i >= 0; i--) {
                itsAvailableModel.remove(idx[i]);
            }
        }
        validateModels();
}//GEN-LAST:event_AddButtonActionPerformed

    private void AddAllButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_AddAllButtonActionPerformed
        for (int i = 0; i < itsAvailableModel.size();i++) {
            if (!itsUsedModel.contains(itsAvailableModel.elementAt(i))) {
                itsUsedModel.addElement(itsAvailableModel.get(i));
            }
        }
        itsAvailableModel.clear();
        validateModels();
}//GEN-LAST:event_AddAllButtonActionPerformed

    private void RemoveButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_RemoveButtonActionPerformed
        int idx[] = UsedStorageNodeList.getSelectedIndices();
        if (idx.length <= 0 ) {
            return;
        } else {
            for (int i = 0; i< idx.length; i++) {
                itsAvailableModel.addElement(itsUsedModel.get(idx[i]));
            }
            for (int i = idx.length-1; i >= 0; i--) {
                itsUsedModel.remove(idx[i]);
            }
        }
        validateModels();

}//GEN-LAST:event_RemoveButtonActionPerformed

    private void enableAllButtons(boolean b) {
        this.AddButton.setEnabled(b);
        this.AddAllButton.setEnabled(b);
        this.RemoveButton.setEnabled(b);
        this.RemoveAllButton.setEnabled(b);
    }
   
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton AddAllButton;
    private javax.swing.JButton AddButton;
    private javax.swing.JList AvailableStorageNodeList;
    private javax.swing.JButton RemoveAllButton;
    private javax.swing.JButton RemoveButton;
    private javax.swing.JList UsedStorageNodeList;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
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

    /**
     * @return the itsUsedStorageNodeList
     */
    public Vector<String> getUsedStorageNodeList() {
        return itsUsedStorageNodeList;
    }

    /**
     * @param itsUsedStorageNodeList the itsUsedStorageNodeList to set
     */
    public void setUsedStorageNodeList(Vector<String> itsUsedStorageNodeList) {
        this.itsUsedStorageNodeList = itsUsedStorageNodeList;
        this.itsUsedModel.clear();
        for (int i=0; i<itsUsedStorageNodeList.size();i++) {
            itsUsedModel.addElement(itsUsedStorageNodeList.get(i));
            if (itsAvailableModel.contains(itsUsedStorageNodeList.get(i))) {
                itsAvailableModel.removeElement(itsUsedStorageNodeList.get(i));
            }
        }
        this.validateModels();
    }


}
