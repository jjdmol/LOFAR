/*
 * TreePanel.java
 *
 * Created on January 26, 2006, 1:58 PM
 */

package nl.astron.lofar.sas.otbcomponents;

import javax.swing.JTree;
import nl.astron.lofar.sas.otb.util.treenodes.TreeNode;
import org.apache.log4j.Logger;

/**
 * Panel containing a JScrollPane and a JTree. The tree accepts nodes that
 * are derived from the DefaultMutableTreeNode class, for example OTDBtreeNode
 * 
 * The panel exports the TreeSelection event
 * Note: When I tried to add the TreeSelection event as a multicast event source
 * (through the Bean Patterns submenu) I noticed that the TreeSelection event is
 * not part of the list of events supplied by NetBeans. Therefore I first added a 
 * TreeModel event source and then adapted the generated methods to handle the 
 * TreeSelection event.
 *
 * @author  blaakmeer
 */
public class TreePanel extends javax.swing.JPanel {
    
    private String itsTitle;
    
    static Logger logger = Logger.getLogger(TreePanel.class);
    static String name="TreePanel";
    
    /** Creates new form BeanForm */
    public TreePanel() {
        initComponents();
    }
    
    /**
     * set the title for this treewindow
     * @params  aTitle  The title for this tree
     */ 
    public void setTitle(String aTitle) {
        treeTitleLabel.setText(aTitle);
        itsTitle=aTitle;
    }
    /** 
     *  Destroys the current tree and creates a new tree with the given root
     *  All event handlers have to be added to the new tree 
     *  @param root  the new root of the tree
     */
    public void newRootNode(TreeNode root) {
        jTree1 = null;
        
        jTree1 = new JTree(root);
        
        // code copied from initComponents
        jTree1.setShowsRootHandles(true);
        jTree1.addTreeSelectionListener(new javax.swing.event.TreeSelectionListener() {
            public void valueChanged(javax.swing.event.TreeSelectionEvent evt) {
                jTree1ValueChanged(evt);
            }
        });
        jTree1.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                jTree1MouseClicked(evt);
            }
            public void mousePressed(java.awt.event.MouseEvent evt) {
                jTree1MousePressed(evt);
            }
            public void mouseReleased(java.awt.event.MouseEvent evt) {
                jTree1MouseReleased(evt);
            }
        });
        jScrollPane1.setViewportView(jTree1);
        jTree1.setSelectionRow(0);
    }
    
    public int[] getSelectedRows() {
        return jTree1.getSelectionRows();
    }
    
    private void setSelection(java.awt.event.MouseEvent evt) {
        jTree1.setSelectionPath(jTree1.getPathForLocation(evt.getX(),evt.getY()));
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        treeTitleLabel = new javax.swing.JLabel();
        jScrollPane1 = new javax.swing.JScrollPane();
        jTree1 = new javax.swing.JTree();

        setLayout(new java.awt.BorderLayout());

        treeTitleLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        treeTitleLabel.setText("No Title");
        add(treeTitleLabel, java.awt.BorderLayout.NORTH);

        jTree1.setShowsRootHandles(true);
        jTree1.addTreeSelectionListener(new javax.swing.event.TreeSelectionListener() {
            public void valueChanged(javax.swing.event.TreeSelectionEvent evt) {
                jTree1ValueChanged(evt);
            }
        });
        jTree1.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                jTree1MouseClicked(evt);
            }
            public void mousePressed(java.awt.event.MouseEvent evt) {
                jTree1MousePressed(evt);
            }
            public void mouseReleased(java.awt.event.MouseEvent evt) {
                jTree1MouseReleased(evt);
            }
        });

        jScrollPane1.setViewportView(jTree1);

        add(jScrollPane1, java.awt.BorderLayout.CENTER);

    }// </editor-fold>//GEN-END:initComponents

    private void jTree1MouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_jTree1MouseClicked
        setSelection(evt);
        fireMouseListenerMouseClicked(evt);
    }//GEN-LAST:event_jTree1MouseClicked

    private void jTree1ValueChanged(javax.swing.event.TreeSelectionEvent evt) {//GEN-FIRST:event_jTree1ValueChanged
        fireTreeSelectionListenerValueChanged(evt);
    }//GEN-LAST:event_jTree1ValueChanged

    private void jTree1MouseReleased(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_jTree1MouseReleased
        setSelection(evt);
        fireMouseListenerMouseReleased(evt);
    }//GEN-LAST:event_jTree1MouseReleased

    private void jTree1MousePressed(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_jTree1MousePressed
        setSelection(evt);
        fireMouseListenerMousePressed(evt);
    }//GEN-LAST:event_jTree1MousePressed

    /**
     * Exports the TreeSelection.valueChanged event from tree to the outside world of this panel.
     * @param evt The TreeSelection event
     */    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JTree jTree1;
    private javax.swing.JLabel treeTitleLabel;
    // End of variables declaration//GEN-END:variables

    /**
     * Utility field used by event firing mechanism.
     */
    private javax.swing.event.EventListenerList listenerList =  null;

    /**
     * Registers TreeSelectionListener to receive events.
     * @param listener The listener to register.
     */
    public synchronized void addTreeSelectionListener(javax.swing.event.TreeSelectionListener listener) {

        if (listenerList == null ) {
            listenerList = new javax.swing.event.EventListenerList();
        }
        listenerList.add (javax.swing.event.TreeSelectionListener.class, listener);
    }

    /**
     * Removes TreeSelectionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeTreeSelectionListener(javax.swing.event.TreeSelectionListener listener) {

        listenerList.remove (javax.swing.event.TreeSelectionListener.class, listener);
    }

    /**
     * Notifies all registered listeners about the event.
     * 
     * @param event The event to be fired
     */
    private void fireTreeSelectionListenerValueChanged(javax.swing.event.TreeSelectionEvent event) {

        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList ();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==javax.swing.event.TreeSelectionListener.class) {
                ((javax.swing.event.TreeSelectionListener)listeners[i+1]).valueChanged (event);
            }
        }
    }

    /**
     * Registers MouseListener to receive events.
     * @param listener The listener to register.
     */
    public synchronized void addMouseListener(java.awt.event.MouseListener listener) {
        if (listenerList == null ) {
            listenerList = new javax.swing.event.EventListenerList();
        }
        listenerList.add (java.awt.event.MouseListener.class, listener);
    }

    /**
     * Removes MouseListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeMouseListener(java.awt.event.MouseListener listener) {
        listenerList.remove (java.awt.event.MouseListener.class, listener);
    }

    /**
     * Notifies all registered listeners about the event.
     * 
     * @param event The event to be fired
     */
    private void fireMouseListenerMouseClicked(java.awt.event.MouseEvent event) {
        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList ();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.MouseListener.class) {
                ((java.awt.event.MouseListener)listeners[i+1]).mouseClicked (event);
            }
        }
    }

    /**
     * Notifies all registered listeners about the event.
     * 
     * @param event The event to be fired
     */
    private void fireMouseListenerMousePressed(java.awt.event.MouseEvent event) {
        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList ();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.MouseListener.class) {
                ((java.awt.event.MouseListener)listeners[i+1]).mousePressed (event);
            }
        }
    }

    /**
     * Notifies all registered listeners about the event.
     * 
     * @param event The event to be fired
     */
    private void fireMouseListenerMouseReleased(java.awt.event.MouseEvent event) {
        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList ();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.MouseListener.class) {
                ((java.awt.event.MouseListener)listeners[i+1]).mouseReleased (event);
            }
        }
    }

    /**
     * Notifies all registered listeners about the event.
     * 
     * @param event The event to be fired
     */
    private void fireMouseListenerMouseEntered(java.awt.event.MouseEvent event) {
        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList ();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.MouseListener.class) {
                ((java.awt.event.MouseListener)listeners[i+1]).mouseEntered (event);
            }
        }
    }

    /**
     * Notifies all registered listeners about the event.
     * 
     * @param event The event to be fired
     */
    private void fireMouseListenerMouseExited(java.awt.event.MouseEvent event) {
        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList ();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==java.awt.event.MouseListener.class) {
                ((java.awt.event.MouseListener)listeners[i+1]).mouseExited (event);
            }
        }
    }
}
