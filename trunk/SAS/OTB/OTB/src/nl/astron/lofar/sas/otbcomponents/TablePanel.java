/*
 * TablePanel.java
 *
 * Created on January 13, 2006, 4:47 PM
 */

package nl.astron.lofar.sas.otbcomponents;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableColumn;
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
    
    public void setAutoCreateRowSorter(boolean aFlag) {
        jTable1.setAutoCreateRowSorter(aFlag);
    }
    
    /** Return the model from this table
     *
     * @return The abstract TableModel
     */
    public AbstractTableModel getTableModel() {
        return (AbstractTableModel)jTable1.getModel();
    }
    
    public void setColumnSize(String colName, int size) {
        TableColumn column = jTable1.getColumn(colName);
        column.setPreferredWidth(size);
    }
    
    /** get the selected Row
     *
     * @return the seledcted Row
     */
    public int getSelectedRow() {
        if (jTable1.getAutoCreateRowSorter()) {
          if (jTable1.getSelectedRow() > -1) {
            return jTable1.convertRowIndexToModel(jTable1.getSelectedRow());
          } else {
            return -1;
          }
        } else {
          return jTable1.getSelectedRow();
        }
    }
    
    /** get the selected Rows
     *
     * @return the selected Rows
     */
    public int[] getSelectedRows() {
        
      if (jTable1.getAutoCreateRowSorter()) {
        int i[] = jTable1.getSelectedRows();
        for (int j=0; j<i.length;j++) {
            if (i[j] > -1) {
              i[j] = jTable1.convertRowIndexToModel(i[j]);
            } else {
                i[j]=-1;
            }
        }
        return i;
      } else {
        return jTable1.getSelectedRows();
      }
    }

    /** get the nr of selected Rows
     *
     * @return the selected Rowcount
     */
    public int getSelectedRowCount() {
        return jTable1.getSelectedRowCount();
    }

    /** set the selected Row
     *
     * @param aRow the selected Row
     */
    public void setSelectedID(int anID) {
        int IDcol=-1;
        int IDrow=-1;
        for (int j=0 ; j< jTable1.getModel().getColumnCount(); j++) {
            if (jTable1.getColumnName(j).equals("ID")) {
                IDcol=j;
                break;
            }
        }
        if (IDcol <= -1) {
            return;
        }
        
        for (int i =0; i <jTable1.getModel().getRowCount(); i++ ) {
            if(jTable1.getValueAt(i,IDcol).equals(anID)) {
                IDrow=i;
                break;
            }
        }
        
        if (IDrow <= -1) {
            return;
        }
        jTable1.getSelectionModel().setSelectionInterval(IDrow,IDrow);
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
    
     public void setWarning(String aWarning) {
        warningText.setText(aWarning);
        warningText.setEnabled(false);
        warningText.setVisible(true);
    } 

    
    
    public void removeWarning() {
        warningText.setText("");
        warningText.setEnabled(true);
        warningText.setVisible(false);
    }
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        warningText = new javax.swing.JLabel();
        jScrollPane1 = new javax.swing.JScrollPane();
        jTable1 = new javax.swing.JTable();

        setLayout(new java.awt.BorderLayout());

        warningText.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        warningText.setText(" ");
        warningText.setVisible(false);
        add(warningText, java.awt.BorderLayout.NORTH);

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
        jTable1.addMouseMotionListener(new java.awt.event.MouseMotionAdapter() {
            public void mouseDragged(java.awt.event.MouseEvent evt) {
                jTable1MouseDragged(evt);
            }
        });

        jScrollPane1.setViewportView(jTable1);

        add(jScrollPane1, java.awt.BorderLayout.CENTER);

    }// </editor-fold>//GEN-END:initComponents

    private void jTable1MouseDragged(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_jTable1MouseDragged
        fireMouseListenerMouseClicked(evt);
    }//GEN-LAST:event_jTable1MouseDragged

    private void jTable1MouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_jTable1MouseClicked
        fireMouseListenerMouseClicked(evt);
    }//GEN-LAST:event_jTable1MouseClicked
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JTable jTable1;
    private javax.swing.JLabel warningText;
    // End of variables declaration//GEN-END:variables

    /**
     * Utility field used by event firing mechanism.
     */
    private javax.swing.event.EventListenerList myListenerList =  null;


    /**
     * Registers MouseListener to receive events.
     * @param listener The listener to register.
     */
    @Override
    public synchronized void addMouseListener(java.awt.event.MouseListener listener) {
        if (myListenerList == null ) {
            myListenerList = new javax.swing.event.EventListenerList();
        }
        myListenerList.add (java.awt.event.MouseListener.class, listener);
    }

    /**
     * Removes MouseListener from the list of listeners.
     * @param listener The listener to remove.
     */
    @Override
    public synchronized void removeMouseListener(java.awt.event.MouseListener listener) {
        myListenerList.remove (java.awt.event.MouseListener.class, listener);
    }

    /**
     * Notifies all registered listeners about the event.
     * 
     * @param event The event to be fired
     */
    private void fireMouseListenerMouseClicked(java.awt.event.MouseEvent event) {
        if (myListenerList == null) return;
        Object[] listeners = myListenerList.getListenerList ();
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
