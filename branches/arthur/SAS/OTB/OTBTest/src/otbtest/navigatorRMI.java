/*
 * navigatorRMI.java
 *
 * Created on 7 juni 2005, 15:22
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package otbtest;

import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.ArrayList;

/**
 *
 * @author Arthur Coolen
 */
public interface navigatorRMI extends Remote {
    ArrayList<String []> getList() throws RemoteException;
}
