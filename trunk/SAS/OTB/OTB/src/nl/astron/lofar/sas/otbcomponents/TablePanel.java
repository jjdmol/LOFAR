/*
 * TablePanel.java
 *
 * Created on January 13, 2006, 4:47 PM
 */

package nl.astron.lofar.sas.otbcomponents;
import javax.swing.ListSelectionModel;
import javax.swing.table.AbstractTableModel;
import org.apache.log4j.Logger;

/**
 *
 * @author  blaakmeer/coolen
 */
public class TablePanel extends javax.swing.JPanel {
    
    static Logger logger = Logger.getLogger(TablePanel.class);
    static String name = "TablePanel";
    
    /** Creates new form BeanForm */
    public TablePanel() {
        initComponents();

    }
    
    /** Set the Model for this Table
     *
     * @param   tableModel    The Model to be used
     */
    public void setTableModel(AbstractTableModel tableModel) {
        jTable1.setModel(tableModel);
        jTable1.updateUI();
    }
    
    /** Return the model from this table
     *
     * @return The abstract TableModel
     */
    public AbstractTableModel getTableModel() {
        return (AbstractTableModel)jTable1.getModel();
    }
    
    /** get the selected Row
     *
     * @return the seledcted Row
     */
    public int getSelectedRow() {
        return jTable1.getSelectedRow();
    }
    
    /** set the selectionmode for this table
     *
     *For available selectionmodes see ListSelectionModel.
     *
     * @param selectionmode The chosen selectionmode 
     */
    public void setSelectionMode(int selectionMode) {
        jTable1.setSelectionMode(selectionMode);
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        jScrollPane1 = new javax.swing.JScrollPane();
        jTable1 = new javax.swing.JTable();

        setLayout(new java.awt.BorderLayout());

        jTable1.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {
                {null, null, null, null},
                {null, null, null, null},
                {null, null, null, null},
                {null, null, null, null}
            },
            new String [] {
                "Title 1", "Title 2", "Title 3", "Title 4"
            }
        ));
        jTable1.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                jTable1MouseClicked(evt);
            }
        });

        jScrollPane1.setViewportView(jTable1);

        add(jScrollPane1, java.awt.BorderLayout.CENTER);

    }// </editor-fold>//GEN-END:initComponents

    private void jTable1MouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_jTable1MouseClicked
        this.fireMouseListenerMouseClicked(evt);
    }//GEN-LAST:event_jTable1MouseClicked
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JTable jTable1;
    // End of variables declaration//GEN-END:variables

    /**
     * Utility field used by event firing mechanism.
     */
    private javax.swing.event.EventListenerList listenerList =  null;


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
