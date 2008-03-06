/*
 * GenericTreeManager.java
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

package nl.astron.lofar.sas.otb.util.treemanagers;

import javax.swing.event.TreeModelEvent;
import javax.swing.event.TreeModelListener;
import nl.astron.lofar.sas.otb.util.UserAccount;

/**
 * Base GenericTreeManager Class
 *
 * @created 18-05-2006, 13:34
 *
 * @author pompert
 *
 * @version $Id$
 *
 * @updated
 */
public abstract class GenericTreeManager{
    
    protected static UserAccount anAccount;
    private javax.swing.event.EventListenerList myListenerList =  null;
    
    /**
     * default constructor, protected by a singleton pattern
     */
    public GenericTreeManager(UserAccount anAccount) {
        GenericTreeManager.anAccount = anAccount;
    }
    /**
     * Registers ActionListener to receive events.
     * @param listener The listener to register.
     */
    public synchronized void addTreeModelListener(TreeModelListener listener) {
        if (myListenerList == null ) {
            myListenerList = new javax.swing.event.EventListenerList();
        }
        myListenerList.add(TreeModelListener.class, listener);
    }
    
    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeTreeModelListener(TreeModelListener listener) {
        myListenerList.remove(TreeModelListener.class, listener);
    }
    
    /**
     * Notifies all registered listeners about the event.
     *
     * @param event The event to be fired
     */
    protected void fireTreeInsertionPerformed(TreeModelEvent event) {
        if (myListenerList == null) return;
        Object[] listeners = myListenerList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==TreeModelListener.class) {
                ((TreeModelListener)listeners[i+1]).treeNodesInserted(event);
            }
        }
    }
    /**
     * Notifies all registered listeners about the event.
     *
     * @param event The event to be fired
     */
    protected void fireTreeRemovalPerformed(TreeModelEvent event) {
        if (myListenerList == null) return;
        Object[] listeners = myListenerList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==TreeModelListener.class) {
                ((TreeModelListener)listeners[i+1]).treeNodesRemoved(event);
            }
        }
    }
    /**
     * Notifies all registered listeners about the event.
     *
     * @param event The event to be fired
     */
    protected void fireTreeChangePerformed(TreeModelEvent event) {
        if (myListenerList == null) return;
        Object[] listeners = myListenerList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==TreeModelListener.class) {
                ((TreeModelListener)listeners[i+1]).treeNodesChanged(event);
            }
        }
    }
    /**
     * Notifies all registered listeners about the event.
     *
     * @param event The event to be fired
     */
    protected void fireTreeStructureChanged(TreeModelEvent event) {
        if (myListenerList == null) return;
        Object[] listeners = myListenerList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==TreeModelListener.class) {
                ((TreeModelListener)listeners[i+1]).treeStructureChanged(event);
            }
        }
    }
    
}
