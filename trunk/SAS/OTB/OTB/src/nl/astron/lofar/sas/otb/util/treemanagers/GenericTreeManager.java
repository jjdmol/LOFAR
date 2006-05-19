/*
 * GenericTreeManager.java
 *
 * Created on January 26, 2006, 2:56 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package nl.astron.lofar.sas.otb.util.treemanagers;

import java.rmi.RemoteException;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.event.TreeModelEvent;
import javax.swing.event.TreeModelListener;
import nl.astron.lofar.sas.otb.SharedVars;
import nl.astron.lofar.sas.otb.jotdb2.jOTDBnode;
import nl.astron.lofar.sas.otb.util.UserAccount;
import nl.astron.lofar.sas.otb.util.jParmDBnode;
import nl.astron.lofar.sas.otb.util.treenodes.TreeNode;
import org.apache.log4j.Logger;

/**
 * Base GenericTreeManager Class
 *
 * @author pompert
 * @version $Id$
 */
public abstract class GenericTreeManager{
    
    protected static UserAccount anAccount;
    private javax.swing.event.EventListenerList listenerList =  null;
    
    /**
     * default constructor, protected by a singleton pattern
     */
    public GenericTreeManager(UserAccount anAccount) {
        this.anAccount = anAccount;
    }
    /**
     * Registers ActionListener to receive events.
     * @param listener The listener to register.
     */
    public synchronized void addTreeModelListener(TreeModelListener listener) {
        if (listenerList == null ) {
            listenerList = new javax.swing.event.EventListenerList();
        }
        listenerList.add(TreeModelListener.class, listener);
    }
    
    /**
     * Removes ActionListener from the list of listeners.
     * @param listener The listener to remove.
     */
    public synchronized void removeTreeModelListener(TreeModelListener listener) {
        listenerList.remove(TreeModelListener.class, listener);
    }
    
    /**
     * Notifies all registered listeners about the event.
     *
     * @param event The event to be fired
     */
    protected void fireTreeInsertionPerformed(TreeModelEvent event) {
        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList();
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
        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList();
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
        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList();
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
        if (listenerList == null) return;
        Object[] listeners = listenerList.getListenerList();
        for (int i = listeners.length - 2; i >= 0; i -= 2) {
            if (listeners[i]==TreeModelListener.class) {
                ((TreeModelListener)listeners[i+1]).treeStructureChanged(event);
            }
        }
    }
    
}
