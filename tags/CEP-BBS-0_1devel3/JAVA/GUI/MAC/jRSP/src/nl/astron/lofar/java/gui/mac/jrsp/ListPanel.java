/*
 * ListPanel.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

package nl.astron.lofar.java.gui.mac.jrsp;

import javax.swing.JPanel;
import javax.swing.event.EventListenerList;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

/**
 * Panel containing a label and a list. Displays a array of Strings and has
 * methods to return the selected value and index.
 * This panel is used by the MainPanel to display the boards that are 
 * accessible.
 * SelectionListeners can be added to this class.
 *
 * @author  balken
 */
public class ListPanel extends JPanel implements ListSelectionListener
{
    /** List of listeners of this Panel */
    private EventListenerList listenerList;
    
    /** 
     * Creates new form ListPanel.
     */
    public ListPanel() 
    {
        listenerList = new EventListenerList();
        initComponents();
        lstList.addListSelectionListener(this);
    }
    
    /**
     * Sets the title to be displayed.
     * @param  title   Title to be displayed.
     */
    public void setTitle(String title)
    {
        lblTitle.setText(title);
    }
    
    /**
     * Deletes the current list and construct a new list based on the listItems.
     * @param   listItems   Array of Strings representing the new list.
     */
    public void newList(String[] listItems)
    {
       lstList.setListData(listItems);
    }
    
    /**
     * Called whenever the value of the selection changes.
     */
    public void valueChanged(ListSelectionEvent event)
    {
        fireValueChanged(event);
    }
    
    /**
     * @return  index   of selected item.
     */
    public int getSelectedIndex()
    {
        return lstList.getSelectedIndex();
    }
    
    /**
     * Sets the selected index.
     * @param   index   The index of the item that should be selected.
     */
    public void setSelectedIndex(int index)
    {
        lstList.setSelectedIndex(index);
    }
    
    /**
     * @return  value   of selected item.
     */
    public Object getSelectedValue()
    {
        return lstList.getSelectedValue();
    }
    
    /**
     * Registers ListSelectionListener to receive events.
     * @param   listener    The listener to register.
     */    
    public void addListSelectionListener(ListSelectionListener listener)
    {
        listenerList.add(ListSelectionListener.class, listener);
    }
    
    /**
     * Removes ListSelectionListener from the list of listeners.
     * @param   listener    The listener to remove.
     */
    public void removeListSelectionListener(ListSelectionListener listener)
    {
        listenerList.remove(ListSelectionListener.class, listener);
    }
   
    /**
     * Notifies all registered listeners about the event.
     * @param   event   The event to be fired
     */
    private void fireValueChanged(ListSelectionEvent event)
    {
        if(listenerList==null)
            return;
        
        Object[] listeners = listenerList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2) 
        {
            if (listeners[i] == ListSelectionListener.class) 
            {
                ((ListSelectionListener)listeners[i+1]).valueChanged(event);
            }
        }
    }    
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        lblTitle = new javax.swing.JLabel();
        jScrollPane = new javax.swing.JScrollPane();
        lstList = new javax.swing.JList();

        lblTitle.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        lblTitle.setText("No Title");

        lstList.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        jScrollPane.setViewportView(lstList);

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(org.jdesktop.layout.GroupLayout.TRAILING, lblTitle, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 400, Short.MAX_VALUE)
            .add(jScrollPane, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 400, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .add(lblTitle)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jScrollPane, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 473, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JScrollPane jScrollPane;
    private javax.swing.JLabel lblTitle;
    private javax.swing.JList lstList;
    // End of variables declaration//GEN-END:variables
    
}
